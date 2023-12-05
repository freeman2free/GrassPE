// RelocTlb.cpp: 实现文件
//

#include "pch.h"
#include "GrassPE.h"
#include "RelocTlb.h"
#include "afxdialogex.h"
#include "GrassPEDlg.h"

// CRelocTlb 对话框

IMPLEMENT_DYNAMIC(CRelocTlb, CDialogEx)

CRelocTlb::CRelocTlb(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_RELOC, pParent)
{

}

CRelocTlb::~CRelocTlb()
{
}

void CRelocTlb::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SECTIONINFO, m_lstSectionInfo);
	DDX_Control(pDX, IDC_LIST_BLOCKINFO, m_lstBlockInfo);
}

DWORD CRelocTlb::RVAToFOA32(DWORD dwRva, char* szBuff)

{
  DWORD dwFOA = 0;
  // 获取DosHeader
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)szBuff;
  // 获取NTHeader
  PIMAGE_NT_HEADERS32 pNTHeader = (PIMAGE_NT_HEADERS32)(szBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 得到区段表
  PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeader);
  // 判断RVA属于哪个区段
  // 首先判断是否落在头部区段
  if (dwRva < pSectionHeader[0].VirtualAddress)
  {
    // 如果小于第1个区段的虚拟地址直接返回(是否落在头部)
    // 因为头在文件中和内存中是一样展开的
    return dwRva;
  }
  // 如果dwRva正好是某区段的起始RVA
  for (int i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    if (dwRva == pSectionHeader[i].VirtualAddress)
    {
      DWORD k = (pSectionHeader[i].VirtualAddress - pSectionHeader[i].PointerToRawData);
      dwFOA = dwRva - k;
      return dwFOA;
    }
  }

  // 遍历所有区段.同一区段中,内存中的偏移和文件中的偏移都是相等的
  for (int i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    // 判断是否在这个区段中
    // 区段起始地址(在内存中的偏移RVA) = virtualAddress
    // 区段end地址 = virtuallAddress + size
    // RVA >= 区段.VirtualAddress && RVA < 区段.VirtualAddress + 区段.VirtualSize
    if (dwRva >= pSectionHeader[i].VirtualAddress && dwRva <= pSectionHeader[i].VirtualAddress + pSectionHeader[i].Misc.VirtualSize)
    {
      // 如果属于这个区段,那么计算出该位置在文件中的偏移(FOA)
      /* 第一种方法
      * dwRva - VirtualAddress = 当前地址到所在区段首部的偏移(差值)
      * PointerToRawData 当前区段距离文件头的长度
      * PointerToRawData + (dwRva - VirtualAddress) = 指定内存地址在文件中的真实地址(也就是FOA)
      * dwFOA = (dwRva - pSectionHeader[i].VirtualAddress) + pSectionHeader[i].PointerToRawData;
      */

      /*第二种方法
      * 差值 = VirtualAddress(区段在内存中的偏移起始处RVA) - PointerToRawData(区段在文件中的偏移起始处FOA)
      * FOA = RVA - 差值
      */
      DWORD k = (pSectionHeader[i].VirtualAddress - pSectionHeader[i].PointerToRawData);
      dwFOA = dwRva - k;
      break;
    }
  }
  return dwFOA; // 返回该FOA偏移
}

ULONGLONG CRelocTlb::RVAToFOA64(ULONGLONG ullRva, char* szBuff)
{
  ULONGLONG ullFOA = 0;
  // 获取DosHeader
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)szBuff;
  // 获取NTHeader
  PIMAGE_NT_HEADERS64 pNTHeader = (PIMAGE_NT_HEADERS64)(szBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 得到区段表
  PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeader);
  // 判断RVA属于哪个区段
  // 首先判断是否落在头部区段
  if (ullRva < pSectionHeader[0].VirtualAddress)
  {
    // 如果小于第1个区段的虚拟地址直接返回(位于PE文件头部)
    // 因为头在文件中和内存中是一样展开的
    return ullRva;
  }
  // 如果dwRva正好是某区段的起始RVA
  for (size_t i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    if (ullRva == pSectionHeader[i].VirtualAddress)
    {
      DWORD k = (pSectionHeader[i].VirtualAddress - pSectionHeader[i].PointerToRawData);
      ullFOA = ullRva - k;
      return ullFOA;
    }
  }

  for (size_t i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    // 判断是否在这个区段中
    // 区段start地址 = virtualAddress
    // 区段end地址   = virtuallAddress+size
    // RVA >= 区段.VirtualAddress && RVA <= 区段.VirtualAddress + 区段.VirtualSize
    if (ullRva >= pSectionHeader[i].VirtualAddress && ullRva < ((ULONGLONG)pSectionHeader[i].VirtualAddress + (ULONGLONG)pSectionHeader[i].Misc.VirtualSize))
    {
      // 如果属于这个区段,那么计算出该位置在文件中的偏移(FOA)
      // dwRva - VirtualAddress = 当前地址到所在区段首部的偏移(差值)
      // PointerToRawData 当前区段距离文件头的长度
      // PointerToRawData + (dwRva - VirtualAddress) = 指定内存地址在文件中的真实地址(也就是FOA)
      ullFOA = (ullRva - pSectionHeader[i].VirtualAddress) + pSectionHeader[i].PointerToRawData;
      break;
    }
  }
  return ullFOA; // 返回该FOA偏移
}


BEGIN_MESSAGE_MAP(CRelocTlb, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_LIST_SECTIONINFO, &CRelocTlb::OnNMClickListSectioninfo)
END_MESSAGE_MAP()


// CRelocTlb 消息处理程序


BOOL CRelocTlb::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_lstSectionInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lstSectionInfo.InsertColumn(0, L"索引", LVCFMT_CENTER, 100);
	m_lstSectionInfo.InsertColumn(1, L"区段", LVCFMT_CENTER, 100);
	m_lstSectionInfo.InsertColumn(2, L"RVA", LVCFMT_CENTER, 100);
	m_lstSectionInfo.InsertColumn(3, L"项目", LVCFMT_CENTER, 100);
	m_lstSectionInfo.DeleteAllItems();

	m_lstBlockInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lstBlockInfo.InsertColumn(0, L"索引", LVCFMT_CENTER, 100);
	m_lstBlockInfo.InsertColumn(1, L"RVA", LVCFMT_CENTER, 100);
	m_lstBlockInfo.InsertColumn(2, L"偏移", LVCFMT_CENTER, 100);
	m_lstBlockInfo.InsertColumn(3, L"类型", LVCFMT_CENTER, 100);
	m_lstBlockInfo.InsertColumn(4, L"FAR地址", LVCFMT_CENTER, 100);
	m_lstBlockInfo.DeleteAllItems();

  if (CGrassPEDlg::Is64PE(g_szPEBuff))
  {
    InitRelocTableInfo64();
  }
  else
  {
    InitRelocTableInfo();
  }
  
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CRelocTlb::OnNMClickListSectioninfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	// 获取点选位置
	POSITION ps;
	int iIndex;
	ps = m_lstSectionInfo.GetFirstSelectedItemPosition();
	iIndex = m_lstSectionInfo.GetNextSelectedItem(ps);
	if (-1 == iIndex)
	{
		return;
	}
	// 获取到点选位置行的文本(此处要获取的是第3列的文本:RVA)
	m_lstSectionInfo.SetItemState(iIndex, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	CString csRVA;
  csRVA = csRVA + m_lstSectionInfo.GetItemText(iIndex, 2);
  m_lstBlockInfo.DeleteAllItems();
	// 加载已选中需要重定位区段的具体信息
  if (CGrassPEDlg::Is64PE(g_szPEBuff))
  {
    InitSelectRelocInfo64(csRVA);
  }
  else
  {
    InitSelectRelocInfo(csRVA);
  }
	*pResult = 0;
}

void CRelocTlb::InitRelocTableInfo()
{
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS32 pNThead = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER pOpthead = &(pNThead->OptionalHeader);

  // 找到导出表的VA和Size
  PIMAGE_DATA_DIRECTORY pRelocDir = (pOpthead->DataDirectory + IMAGE_DIRECTORY_ENTRY_BASERELOC);
  // 找到重定位表
  PIMAGE_BASE_RELOCATION pRelocTable = (PIMAGE_BASE_RELOCATION)(RVAToFOA32(pRelocDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 遍历重定位表(整个重定位区段以一个VirtualAddress为空的IMAGE_BASE_RELOCATION结构体结束)
  DWORD dwIndex = 0; // 索引值
  while (pRelocTable->VirtualAddress != NULL && pRelocTable->SizeOfBlock != NULL)
  {
    DWORD dwIndexOfSection = 0; // 用于接收区段的序号
    // 获取到所在区段的区段名
    char* szNameBuf = GetSectionName(pRelocTable->VirtualAddress, &dwIndexOfSection);
    // 输出重定位表的一些信息
    // 索引
    CString csIndex;
    csIndex.Format(L"%d", dwIndex + 1);
    m_lstSectionInfo.InsertItem(dwIndex, csIndex);
    // 区段
    wchar_t wszNameBuf[256] = { 0 };
    wsprintf(wszNameBuf, L"%d (\"%S\")", dwIndexOfSection, szNameBuf);
    m_lstSectionInfo.SetItemText(dwIndex, 1, wszNameBuf);
    // 输出当前重定位数据块(重定位表)在区段中的起始RVA
    CString csSectRVA;
    csSectRVA.Format(L"%08X", pRelocTable->VirtualAddress);
    m_lstSectionInfo.SetItemText(dwIndex, 2, csSectRVA);
    // 得到重定位数据的个数(TypeOffset数组的项数)
    DWORD dwNumOfBlock = (pRelocTable->SizeOfBlock - (sizeof(IMAGE_BASE_RELOCATION))) / sizeof(WORD);
    printf("项目:  %Xh / %dd\r\n", dwNumOfBlock, dwNumOfBlock);
    CString csNumOfBlock;
    csNumOfBlock.Format(L"%Xh / %dd", dwNumOfBlock, dwNumOfBlock);
    m_lstSectionInfo.SetItemText(dwIndex, 3, csNumOfBlock);

    dwIndex++; // 行+1
    //指向下一张重定位表
    pRelocTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocTable + pRelocTable->SizeOfBlock);
  }
}

void CRelocTlb::InitRelocTableInfo64()
{
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS64 pNThead = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER64 pOpthead = &(pNThead->OptionalHeader);

  // 找到导出表的VA和Size
  PIMAGE_DATA_DIRECTORY pRelocDir = (pOpthead->DataDirectory + IMAGE_DIRECTORY_ENTRY_BASERELOC);
  // 找到重定位表
  PIMAGE_BASE_RELOCATION pRelocTable = (PIMAGE_BASE_RELOCATION)(RVAToFOA64(pRelocDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 遍历重定位表(整个重定位区段以一个VirtualAddress为空的IMAGE_BASE_RELOCATION结构体结束)
  DWORD dwIndex = 0; // 索引值
  while (pRelocTable->VirtualAddress != NULL && pRelocTable->SizeOfBlock != NULL)
  {
    DWORD dwIndexOfSection = 0; // 用于接收区段的序号
    // 获取到所在区段的区段名
    char* szNameBuf = GetSectionName(pRelocTable->VirtualAddress, &dwIndexOfSection);
    // 输出重定位表的一些信息
    // 索引
    CString csIndex;
    csIndex.Format(L"%d", dwIndex + 1);
    m_lstSectionInfo.InsertItem(dwIndex, csIndex);
    // 区段
    wchar_t wszNameBuf[256] = { 0 };
    wsprintf(wszNameBuf, L"%d (\"%S\")", dwIndexOfSection, szNameBuf);
    m_lstSectionInfo.SetItemText(dwIndex, 1, wszNameBuf);
    // 输出当前重定位数据块(重定位表)在区段中的起始RVA
    CString csSectRVA;
    csSectRVA.Format(L"%08X", pRelocTable->VirtualAddress);
    m_lstSectionInfo.SetItemText(dwIndex, 2, csSectRVA);
    // 得到重定位数据的个数(TypeOffset数组的项数)
    DWORD dwNumOfBlock = (pRelocTable->SizeOfBlock - (sizeof(IMAGE_BASE_RELOCATION))) / sizeof(WORD);
    printf("项目:  %Xh / %dd\r\n", dwNumOfBlock, dwNumOfBlock);
    CString csNumOfBlock;
    csNumOfBlock.Format(L"%Xh / %dd", dwNumOfBlock, dwNumOfBlock);
    m_lstSectionInfo.SetItemText(dwIndex, 3, csNumOfBlock);

    dwIndex++; // 行+1
    //指向下一张重定位表
    pRelocTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocTable + pRelocTable->SizeOfBlock);
  }
}

void CRelocTlb::InitSelectRelocInfo(CString csRVA)
{
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS32 pNThead = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER pOpthead = &(pNThead->OptionalHeader);

  // 找到导出表的VA和Size
  PIMAGE_DATA_DIRECTORY pRelocDir = (pOpthead->DataDirectory + IMAGE_DIRECTORY_ENTRY_BASERELOC);
  // 找到重定位表
  PIMAGE_BASE_RELOCATION pRelocTable = (PIMAGE_BASE_RELOCATION)(RVAToFOA32(pRelocDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 遍历重定位表(整个重定位区段以一个VirtualAddress为空的IMAGE_BASE_RELOCATION结构体结束)
  DWORD dwIndex = 0; // 索引值
  while (pRelocTable->VirtualAddress != NULL && pRelocTable->SizeOfBlock != NULL)
  {
    DWORD dwIndexOfSection = 0; // 用于接收区段的序号
    // 输出当前重定位数据块(重定位表)在区段中的起始RVA
    CString csSectRVA;
    csSectRVA.Format(L"%08X", pRelocTable->VirtualAddress);
    m_lstSectionInfo.SetItemText(dwIndex, 2, csSectRVA);
    // 遍历到当前这张重定位表(用他们的Rva判断)
    if (csRVA == csSectRVA)
    {
      // 得到重定位数据的个数(TypeOffset数组的项数)
      DWORD dwNumOfBlock = (pRelocTable->SizeOfBlock - (sizeof(IMAGE_BASE_RELOCATION))) / sizeof(WORD);
      // 找到该重定位表的TypeOffset数组(+8字节跳转到该数组的起始位置)
      PWORD pBlock = (PWORD)((DWORD)pRelocTable + 0x8);
      // 遍历这个数组,找到需要重定位数据的信息
      for (size_t j = 0; j < dwNumOfBlock; j++)
      {
        // 需要重定位的数据项的索引
        CString csBlockIndex;
        csBlockIndex.Format(L"%d", j + 1);
        m_lstBlockInfo.InsertItem(j, csBlockIndex);
        // 得到高4位(重定位类型)
        DWORD dwTypeOfBlock = *pBlock >> 12;
        // 得到低12位(重定位的偏移地址)
        DWORD dwAddOfBlock = *pBlock & 0xFFF;
        // x86下判断重定位类型是否为IMAGE_REL_BASED_HIGHLOW
        if (dwTypeOfBlock == IMAGE_REL_BASED_HIGHLOW)
        {
          // 需要重定位的项的RVA
          CString csBlockRva;
          csBlockRva.Format(L"%08X", dwAddOfBlock + pRelocTable->VirtualAddress);
          m_lstBlockInfo.SetItemText(j, 1, csBlockRva);
          // 需要重定位的项的文件偏移
          CString csBlockOffset;
          csBlockOffset.Format(L"%08X", RVAToFOA32((dwAddOfBlock + pRelocTable->VirtualAddress), g_szPEBuff));
          m_lstBlockInfo.SetItemText(j, 2, csBlockOffset);
          // 需要重定位的项的类型(X86下一般是HIGHLOW (3))
          m_lstBlockInfo.SetItemText(j, 3, L"HIGHLOW (3)");
          // FAR地址
          PDWORD pFar = (PDWORD)(RVAToFOA32((dwAddOfBlock + pRelocTable->VirtualAddress), g_szPEBuff) + g_szPEBuff);
          CString csFarAdd;
          csFarAdd.Format(L"%08X", *pFar);
          m_lstBlockInfo.SetItemText(j, 4, csFarAdd);
        }
        else if (dwTypeOfBlock == IMAGE_REL_BASED_ABSOLUTE) // 如果为零
        {
          // 需要重定位的项的RVA
          m_lstBlockInfo.SetItemText(j, 1, L"-");
          // 需要重定位的项的文件偏移
          m_lstBlockInfo.SetItemText(j, 2, L"-");
          // 需要重定位的项的类型(X86下一般是HIGHLOW (3))
          m_lstBlockInfo.SetItemText(j, 3, L"ABSOLUTE (0)");
          // FAR地址
          m_lstBlockInfo.SetItemText(j, 4, L"-");
        }
        pBlock++; // 指向下一个重定位数据项(TypeOffset数组的下一项)
      }
      break;
    }
    dwIndex++;
    //指向下一张重定位表
    pRelocTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocTable + pRelocTable->SizeOfBlock);
  }
}

void CRelocTlb::InitSelectRelocInfo64(CString csRVA)
{
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS64 pNThead = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER64 pOpthead = &(pNThead->OptionalHeader);

  // 找到导出表的VA和Size
  PIMAGE_DATA_DIRECTORY pRelocDir = (pOpthead->DataDirectory + IMAGE_DIRECTORY_ENTRY_BASERELOC);
  // 找到重定位表
  PIMAGE_BASE_RELOCATION pRelocTable = (PIMAGE_BASE_RELOCATION)(RVAToFOA32(pRelocDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 遍历重定位表(整个重定位区段以一个VirtualAddress为空的IMAGE_BASE_RELOCATION结构体结束)
  DWORD dwIndex = 0; // 索引值
  while (pRelocTable->VirtualAddress != NULL && pRelocTable->SizeOfBlock != NULL)
  {
    DWORD dwIndexOfSection = 0; // 用于接收区段的序号
    // 当前重定位数据块(重定位表)在区段中的起始RVA
    CString csSectRVA;
    csSectRVA.Format(L"%08X", pRelocTable->VirtualAddress);
    // 遍历到当前这张重定位表(用他们的Rva判断是否是同一张重定位表)
    if (csRVA == csSectRVA)
    {
      // 得到重定位数据的个数(TypeOffset数组的项数)
      DWORD dwNumOfBlock = (pRelocTable->SizeOfBlock - (sizeof(IMAGE_BASE_RELOCATION))) / sizeof(WORD);
      // 找到该重定位表的TypeOffset数组(+8字节跳转到该数组的起始位置)
      PWORD pBlock = (PWORD)((DWORD)pRelocTable + 0x8);
      // 遍历这个数组,找到需要重定位数据的信息
      for (size_t j = 0; j < dwNumOfBlock; j++)
      {
        // 需要重定位的数据项的索引
        CString csBlockIndex;
        csBlockIndex.Format(L"%d", j + 1);
        m_lstBlockInfo.InsertItem(j, csBlockIndex);
        // 得到高4位(重定位类型)
        DWORD dwTypeOfBlock = *pBlock >> 12;
        // 得到低12位(重定位的偏移地址)
        DWORD dwAddOfBlock = *pBlock & 0xFFF;
        // x64下判断重定位类型是否为IMAGE_REL_BASED_DIR64
        if (dwTypeOfBlock == IMAGE_REL_BASED_DIR64)
        {
          // 需要重定位的项的RVA
          CString csBlockRva;
          csBlockRva.Format(L"%08X", dwAddOfBlock + pRelocTable->VirtualAddress);
          m_lstBlockInfo.SetItemText(j, 1, csBlockRva);
          // 需要重定位的项的文件偏移
          CString csBlockOffset;
          csBlockOffset.Format(L"%08X", RVAToFOA32((dwAddOfBlock + pRelocTable->VirtualAddress), g_szPEBuff));
          m_lstBlockInfo.SetItemText(j, 2, csBlockOffset);
          // 需要重定位的项的类型(X86下一般是HIGHLOW (3))
          m_lstBlockInfo.SetItemText(j, 3, L"DIR64 (10)");
          // FAR地址
          PULONGLONG pFar = (PULONGLONG)(RVAToFOA64((dwAddOfBlock + pRelocTable->VirtualAddress), g_szPEBuff) + g_szPEBuff);
          CString csFarAdd;
          csFarAdd.Format(L"%016lX", *pFar);
          m_lstBlockInfo.SetItemText(j, 4, csFarAdd);
        }
        else if (dwTypeOfBlock == IMAGE_REL_BASED_ABSOLUTE) // 如果为零
        {
          // 需要重定位的项的RVA
          m_lstBlockInfo.SetItemText(j, 1, L"-");
          // 需要重定位的项的文件偏移
          m_lstBlockInfo.SetItemText(j, 2, L"-");
          // 需要重定位的项的类型(X86下一般是HIGHLOW (3))
          m_lstBlockInfo.SetItemText(j, 3, L"ABSOLUTE (0)");
          // FAR地址
          m_lstBlockInfo.SetItemText(j, 4, L"-");
        }
        pBlock++; // 指向下一个重定位数据项(TypeOffset数组的下一项)
      }
      break;
    }
    dwIndex++;
    //指向下一张重定位表
    pRelocTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocTable + pRelocTable->SizeOfBlock);
  }
}

char* CRelocTlb::GetSectionName(DWORD dwRva, DWORD* pIndex)
{
  char* szNameBuf = new char[9]{ 0 };
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS32 pNThead = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_FILE_HEADER pFilehead = &pNThead->FileHeader;
  // 区段数
  WORD wSectionCnt = pFilehead->NumberOfSections;
  // 得到区段表
  PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNThead);
  // 首先判断是否落在头部区段
  if (dwRva < pSectionHeader[0].VirtualAddress)
  {
    // 如果小于第1个区段的虚拟地址直接返回(是否落在头部)
    return NULL;
  }

  for (size_t i = 0; i < wSectionCnt; i++)
  {
    // 找到该区段
    if (dwRva >= pSectionHeader[i].VirtualAddress && dwRva <= pSectionHeader[i].VirtualAddress + pSectionHeader[i].Misc.VirtualSize)
    {
      // 将区段的名字记录下
      memcpy(szNameBuf, pSectionHeader[i].Name, 8);
      // 同时返回这是第几个区段
      *pIndex = i + 1;
      break;
    }
  }
  return szNameBuf;
}
