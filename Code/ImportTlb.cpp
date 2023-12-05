// ImportTlb.cpp: 实现文件
//

#include "pch.h"
#include "GrassPE.h"
#include "ImportTlb.h"
#include "afxdialogex.h"
#include "GrassPEDlg.h"

// CImportTlb 对话框

IMPLEMENT_DYNAMIC(CImportTlb, CDialogEx)

CImportTlb::CImportTlb(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_IMPORT, pParent)
{

}

CImportTlb::~CImportTlb()
{
}

void CImportTlb::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_IMPORTINFO, m_lstImpInfo);
	DDX_Control(pDX, IDC_LIST_THUNKINFO, m_lstThunkInfo);
	DDX_Text(pDX, IDC_EDIT_ALLTHUNK, m_csThunkNum);
}


BEGIN_MESSAGE_MAP(CImportTlb, CDialogEx)
  ON_NOTIFY(NM_CLICK, IDC_LIST_IMPORTINFO, &CImportTlb::OnClickListImportinfo)
END_MESSAGE_MAP()


// CImportTlb 消息处理程序


BOOL CImportTlb::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 将两个列表控件初始化
	m_lstImpInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lstImpInfo.InsertColumn(0, L"DLL名称", LVCFMT_CENTER, 100);
	m_lstImpInfo.InsertColumn(1, L"OrignalFirstThunk", LVCFMT_CENTER, 100);
	m_lstImpInfo.InsertColumn(2, L"日期时间标志", LVCFMT_CENTER, 100);
	m_lstImpInfo.InsertColumn(3, L"ForwarderChain", LVCFMT_CENTER, 100);
	m_lstImpInfo.InsertColumn(4, L"名称", LVCFMT_CENTER, 100);
	m_lstImpInfo.InsertColumn(5, L"FirstThunk", LVCFMT_CENTER, 100);
	m_lstImpInfo.DeleteAllItems();
  InitImportTableInfo();
	m_lstThunkInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lstThunkInfo.InsertColumn(0, L"ThunkRVA", LVCFMT_CENTER, 100);
	m_lstThunkInfo.InsertColumn(1, L"Thunk 偏移", LVCFMT_CENTER, 100);
	m_lstThunkInfo.InsertColumn(2, L"Thunk 值", LVCFMT_CENTER, 100);
	m_lstThunkInfo.InsertColumn(3, L"Hint", LVCFMT_CENTER, 100);
	m_lstThunkInfo.InsertColumn(4, L"API名称", LVCFMT_CENTER, 100);
	m_lstThunkInfo.DeleteAllItems();
	return TRUE;  // return TRUE unless you set the focus to a control
								// 异常: OCX 属性页应返回 FALSE
}

void CImportTlb::InitImportTableInfo()
{
  // 找到第一张导入表
  PIMAGE_IMPORT_DESCRIPTOR pImportTable = GetFirstImportDesc();

  // 如果能找到导入表就进行遍历
  DWORD dwIndex = 0;
  while (pImportTable->Name != NULL)
  {
    // 所有导入表的相关信息
    // DLL 名称
    char* szDllName = (char*)(RVAToFOA32(pImportTable->Name, g_szPEBuff) + g_szPEBuff);
    WCHAR wszDllName[256] = { 0 };
    wsprintf(wszDllName, L"%S", szDllName);
    m_lstImpInfo.InsertItem(dwIndex, wszDllName);
    // OriginalFirstThunk
    CString csOriginalThunk;
    csOriginalThunk.Format(L"%08X", pImportTable->OriginalFirstThunk);
    m_lstImpInfo.SetItemText(dwIndex, 1, csOriginalThunk);
    // 日期时间标志
    CString csTimeStamp;
    csTimeStamp.Format(L"%08X", pImportTable->TimeDateStamp);
    m_lstImpInfo.SetItemText(dwIndex, 2, csTimeStamp);
    // ForwarderChain
    CString csForwarderChain;
    csForwarderChain.Format(L"%08X", pImportTable->ForwarderChain);
    m_lstImpInfo.SetItemText(dwIndex, 3, csForwarderChain);
    // 名称RVA
    CString csNameRva;
    csNameRva.Format(L"%08X", pImportTable->Name);
    m_lstImpInfo.SetItemText(dwIndex, 4, csNameRva);
    // FirstThunk
    CString csFirstThunk;
    csFirstThunk.Format(L"%08X", pImportTable->FirstThunk);
    m_lstImpInfo.SetItemText(dwIndex, 5, csFirstThunk);

    dwIndex++; // 行+1
    pImportTable++; // 指向下一张导入表
  }
}

void CImportTlb::InsertThunkDataInfo(LPCWSTR wszDllName)
{
  // 找到第一张导入表
  PIMAGE_IMPORT_DESCRIPTOR pImportTable = GetFirstImportDesc();
  PIMAGE_THUNK_DATA pIAT = NULL;
  PIMAGE_THUNK_DATA pINT = NULL;
  // 用于遍历导入名称或者导入地址表的下标(每次自增一个IMAGE_THUNK_DATA,相当于指向下一个导入函数)
  DWORD dwIndex = 0;
  // 用于记录列表控件的行数
  DWORD dwIndexLine = 0;
  // 导入函数在文件中的偏移
  DWORD dwOffset = 0;
  // 首先找到选中的Dll文件的名称,并循环到该Dll(导入表)处退出循环
  while (pImportTable->Name != NULL)
  {
    char* szCurrentName = (char*)(RVAToFOA32(pImportTable->Name, g_szPEBuff) + g_szPEBuff);
    WCHAR wszCurrentName[256] = { 0 };
    wsprintf(wszCurrentName, L"%S", szCurrentName);
    if (*wszCurrentName == *wszDllName)
    {
      break;
    }
    pImportTable++; // 指向下一张导入表
  }

  // 开始遍历该张导入表的具体信息
  // 这里有两种情况:首先找导入名称表,如果没有导入名称表就找导入地址表
  if (pImportTable->OriginalFirstThunk == NULL)
  {
    // 找到第一张导入地址表(导入地址表总会找到函数地址,而导入函数不一定每一个都会有名字)
    pIAT = (PIMAGE_THUNK_DATA)(RVAToFOA32(pImportTable->FirstThunk, g_szPEBuff) + g_szPEBuff);
    // 如果IMAGE_THUNK_DATA数组(此处指的是导入地址表)不为空,表示这张导入表里还有导入函数
    while (pIAT->u1.Ordinal != NULL)
    {
      // Thunk RVA 导入函数地址表IAT在内存中的偏移
      CString csThunkRva;
      csThunkRva.Format(L"%08X", pImportTable->FirstThunk + dwIndex);
      m_lstThunkInfo.InsertItem(dwIndexLine, csThunkRva);
      // Thunk 偏移 导入函数地址表IAT在文件中的偏移
      dwOffset = RVAToFOA32(pImportTable->FirstThunk, g_szPEBuff);
      CString csThunkOffset;
      csThunkOffset.Format(L"%08X", dwOffset + dwIndex);
      m_lstThunkInfo.SetItemText(dwIndexLine, 1, csThunkOffset);
      // Thunk 值 导入函数的内存地址(RVA)
      CString csFuncRva;
      csFuncRva.Format(L"%08X", pIAT->u1.Function);
      m_lstThunkInfo.SetItemText(dwIndexLine, 2, csFuncRva);

      CString csHint;
      CString csAPIName;
      // 判断最高位,如果最高位为1表示导入函数以序号方式导入 输出导入函数的序号
      if ((pIAT->u1.Ordinal & 0x80000000) != 0)
      {
        // 如果没有导入函数的名字就输出他的序号(输出两遍一遍16进制一遍10进制)
        int  nOut = pIAT->u1.Function - 0x80000000;
        // Hint 导入函数序号此处输出-
        m_lstThunkInfo.SetItemText(dwIndexLine, 3, L"-");
        // API名称 此处输出导入函数的序号
        csAPIName.Format(L"序号: %Xh %dd", nOut, nOut);
        m_lstThunkInfo.SetItemText(dwIndexLine, 4, csAPIName);
      }
      // 否则就会有函数名,输出导入函数的名称
      else
      {
        PIMAGE_IMPORT_BY_NAME pName = (PIMAGE_IMPORT_BY_NAME)(RVAToFOA32(pIAT->u1.AddressOfData, g_szPEBuff) + g_szPEBuff);
        // Hint 导入函数序号
        csHint.Format(L"%04X", pName->Hint);
        m_lstThunkInfo.SetItemText(dwIndexLine, 3, csHint);
        // API名称 此处输出导入函数的名称
        WCHAR wszFuncName[256] = { 0 };
        wsprintf(wszFuncName, L"%S", pName->Name);
        m_lstThunkInfo.SetItemText(dwIndexLine, 4, wszFuncName);
      }
      dwIndex += sizeof(PIMAGE_THUNK_DATA); // 下标自增一,得到下一个导入函数的下标
      dwIndexLine++; // 行数+1
      pIAT++; // 指向下一个导入函数
    }
    // 该张导入表中导入函数的总数
    UpdateData(TRUE);
    m_csThunkNum.Format(L"Thunk Num: %Xh / %dd", dwIndexLine, dwIndexLine);
    UpdateData(FALSE);
  }
  else
  {
    // 导入名称表也是一个IMAGE_THUNK_DATA数组
    pINT = (PIMAGE_THUNK_DATA)(RVAToFOA32(pImportTable->OriginalFirstThunk, g_szPEBuff) + g_szPEBuff);
    // 如果IMAGE_THUNK_DATA数组(此处指的是导入名称表)不为空,表示这张导入表里还有导入函数
    while (pINT->u1.Ordinal != NULL)
    {
      // Thunk RVA 导入函数地址表IAT在内存中的偏移
      CString csThunkRva;
      csThunkRva.Format(L"%08X", pImportTable->OriginalFirstThunk + dwIndex);
      m_lstThunkInfo.InsertItem(dwIndexLine, csThunkRva);
      // Thunk 偏移 导入函数地址表IAT在文件中的偏移
      dwOffset = RVAToFOA32(pImportTable->OriginalFirstThunk, g_szPEBuff);
      CString csThunkOffset;
      csThunkOffset.Format(L"%08X", dwOffset + dwIndex);
      m_lstThunkInfo.SetItemText(dwIndexLine, 1, csThunkOffset);
      // Thunk 值 导入函数的内存地址(RVA)
      CString csFuncRva;
      csFuncRva.Format(L"%08X", pINT->u1.Function);
      m_lstThunkInfo.SetItemText(dwIndexLine, 2, csFuncRva);

      CString csHint;
      CString csAPIName;
      if ((pINT->u1.Ordinal & 0x80000000) != 0) // 如果最高位为1表示导入函数以序号方式导入 输出导入函数的序号
      {
        // 如果没有导入函数的名字就输出他的序号(输出两遍一遍16进制一遍10进制)
        int  nOut = pINT->u1.Function - 0x80000000;
        // Hint 导入函数序号此处输出-
        m_lstThunkInfo.SetItemText(dwIndexLine, 3, L"-");
        // API名称 此处输出导入函数的序号
        csAPIName.Format(L"序号: %Xh %dd", nOut, nOut);
        m_lstThunkInfo.SetItemText(dwIndexLine, 4, csAPIName);
      }
      else
      {
        PIMAGE_IMPORT_BY_NAME pName = (PIMAGE_IMPORT_BY_NAME)(RVAToFOA32(pINT->u1.AddressOfData, g_szPEBuff) + g_szPEBuff);
        // Hint 导入函数序号
        csHint.Format(L"%04X", pName->Hint);
        m_lstThunkInfo.SetItemText(dwIndexLine, 3, csHint);
        // API名称 此处输出导入函数的名称
        WCHAR wszFuncName[256] = { 0 };
        wsprintf(wszFuncName, L"%S", pName->Name);
        m_lstThunkInfo.SetItemText(dwIndexLine, 4, wszFuncName);
      }
      dwIndex += sizeof(PIMAGE_THUNK_DATA);
      dwIndexLine++; // 行数+1
      pINT++; // 指向下一个导入函数
    }
    // 该张导入表中导入函数的总数
    UpdateData(TRUE);
    m_csThunkNum.Format(L"Thunk Num: %Xh / %dd", dwIndexLine, dwIndexLine);
    UpdateData(FALSE);
  }
}

PIMAGE_IMPORT_DESCRIPTOR CImportTlb::GetFirstImportDesc()
{
  PIMAGE_IMPORT_DESCRIPTOR pRetImpDesc = NULL;
  PIMAGE_DOS_HEADER pDosHead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  if (CGrassPEDlg::Is64PE(g_szPEBuff))
  {
    PIMAGE_NT_HEADERS64 pNtHead = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDosHead->e_lfanew);
    PIMAGE_OPTIONAL_HEADER64 pOptHead = &(pNtHead->OptionalHeader);
    // 找到导入表的VA和Size
    PIMAGE_DATA_DIRECTORY pImpDir = pOptHead->DataDirectory + IMAGE_DIRECTORY_ENTRY_IMPORT;
    // 此时是文件加载的PE,因此需要先转换为文件中的偏移再加基地址找到导入表
    pRetImpDesc = (PIMAGE_IMPORT_DESCRIPTOR)(RVAToFOA64(pImpDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
    return pRetImpDesc;
  }
  PIMAGE_NT_HEADERS32 pNtHead = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDosHead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER pOptHead = &(pNtHead->OptionalHeader);
  PIMAGE_DATA_DIRECTORY pImpDir = pOptHead->DataDirectory + IMAGE_DIRECTORY_ENTRY_IMPORT;
  pRetImpDesc = (PIMAGE_IMPORT_DESCRIPTOR)(RVAToFOA32(pImpDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);

  return pRetImpDesc;
}

DWORD CImportTlb::RVAToFOA32(DWORD dwRva, char* szBuff)
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
  // 如果区段对齐值与内存页对齐值相同那么就不需要转换直接返回即可
  if (pNTHeader->OptionalHeader.FileAlignment == pNTHeader->OptionalHeader.SectionAlignment)
  {
    return dwRva;
  }
  /*for (int i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    if (dwRva == pSectionHeader[i].VirtualAddress)
    {
      DWORD k = (pSectionHeader[i].VirtualAddress - pSectionHeader[i].PointerToRawData);
      dwFOA = dwRva - k;
      return dwFOA;
    }
  }*/

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

ULONGLONG CImportTlb::RVAToFOA64(ULONGLONG ullRva, char* szBuff)
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

  // 如果区段对齐值与内存页对齐值相同那么就不需要转换直接返回即可
  if (pNTHeader->OptionalHeader.FileAlignment == pNTHeader->OptionalHeader.SectionAlignment)
  {
    return ullRva;
  }
  //for (size_t i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  //{
  //  if (ullRva == pSectionHeader[i].VirtualAddress)
  //  {
  //    DWORD k = (pSectionHeader[i].VirtualAddress - pSectionHeader[i].PointerToRawData);
  //    ullFOA = ullRva - k;
  //    return ullFOA;
  //  }
  //}


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

void CImportTlb::OnClickListImportinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
  LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
  // TODO: 在此添加控件通知处理程序代码
  // 获取点选位置
  POSITION ps;
  int iIndex;
  ps = m_lstImpInfo.GetFirstSelectedItemPosition();
  iIndex = m_lstImpInfo.GetNextSelectedItem(ps);
  if (-1 == iIndex)
  {
    return;
  }
  // 获取到点选位置行的文本(此处要获取的是第0列的文本)
  m_lstImpInfo.SetItemState(iIndex, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
  CString csDllName;
  csDllName = csDllName + m_lstImpInfo.GetItemText(iIndex, 0);
  m_lstThunkInfo.DeleteAllItems();
  // 加载信息
  InsertThunkDataInfo(csDllName);
  *pResult = 0;
}
