// ExportTlb.cpp: 实现文件
//

#include "pch.h"
#include "GrassPE.h"
#include "ExportTlb.h"
#include "afxdialogex.h"
#include "GrassPEDlg.h"

// CExportTlb 对话框

IMPLEMENT_DYNAMIC(CExportTlb, CDialogEx)

CExportTlb::CExportTlb(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_EXPORT, pParent)
	, m_csExportOffset(_T(""))
	, m_csBasic(_T(""))
	, m_csNameRva(_T(""))
	, m_csName(_T(""))
	, m_csCntFunc(_T(""))
	, m_csCntFuncName(_T(""))
	, m_csEATRva(_T(""))
	, m_csENTRva(_T(""))
	, m_csEOTRva(_T(""))
{

}

CExportTlb::~CExportTlb()
{
}

void CExportTlb::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_EXPORTINFO, m_lstExportInfo);
	DDX_Text(pDX, IDC_EDIT_EXPOFFSET, m_csExportOffset);
	DDX_Text(pDX, IDC_EDIT_EXPCHARACT, m_csCharact);
	DDX_Text(pDX, IDC_EDIT_BASIC, m_csBasic);
	DDX_Text(pDX, IDC_EDIT_NAMERVA, m_csNameRva);
	DDX_Text(pDX, IDC_EDIT_NAMESTR, m_csName);
	DDX_Text(pDX, IDC_EDIT_NUMFUNC, m_csCntFunc);
	DDX_Text(pDX, IDC_EDIT_NUMFUNCNAME, m_csCntFuncName);
	DDX_Text(pDX, IDC_EDIT_ADDFUNC, m_csEATRva);
	DDX_Text(pDX, IDC_EDIT_ADDFUNCNAME, m_csENTRva);
	DDX_Text(pDX, IDC_EDIT_ADDFUNCORDINAL, m_csEOTRva);
}


BEGIN_MESSAGE_MAP(CExportTlb, CDialogEx)
END_MESSAGE_MAP()


// CExportTlb 消息处理程序


BOOL CExportTlb::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_lstExportInfo.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lstExportInfo.InsertColumn(0, L"序号", LVCFMT_CENTER, 50);
	m_lstExportInfo.InsertColumn(1, L"RVA", LVCFMT_CENTER, 80);
	m_lstExportInfo.InsertColumn(2, L"偏移", LVCFMT_CENTER, 80);
	m_lstExportInfo.InsertColumn(3, L"函数名", LVCFMT_CENTER, 200);
	// 先清空一下列表控件
	m_lstExportInfo.DeleteAllItems();

	if (CGrassPEDlg::Is64PE(g_szPEBuff))
	{
		InitExportTableInfo64();
	}
	else
	{
		InitExportTableInfo();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
								// 异常: OCX 属性页应返回 FALSE
}

void CExportTlb::InitExportTableInfo()

{
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS32 pNThead = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER32 pOpthead = &(pNThead->OptionalHeader);

  // 找到导出表的VA和Size
  PIMAGE_DATA_DIRECTORY pExportDir = (pOpthead->DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT);
  // 此时是文件加载的PE,因此需要先转换为文件中的偏移再加基地址找到导出表
  PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)(RVAToFOA32(pExportDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 导出函数总数
  DWORD dwExportFunctionsCnt = pExportTable->NumberOfFunctions;
  // 指向导入地址表,该表中存储所有导入函数的地址
  PDWORD pEAT = (PDWORD)(RVAToFOA32(pExportTable->AddressOfFunctions, g_szPEBuff) + g_szPEBuff);
  // 指向导入名称表,该表中存储所有有名字的导入函数的名称字符串
  PDWORD pENT = (PDWORD)(RVAToFOA32(pExportTable->AddressOfNames, g_szPEBuff) + g_szPEBuff);
  // 指向导入序号表,该表中存储所有有名字的导入函数在EAT中的索引下标
  PWORD pEOT = (PWORD)(RVAToFOA32(pExportTable->AddressOfNameOrdinals, g_szPEBuff) + g_szPEBuff);

  // 显示导出表总的一些信息(IMAGE_EXPORT_DIRECTORY结构体相关信息)
  UpdateData(TRUE);
  // 导出表偏移
  m_csExportOffset.Format(L"%08X", RVAToFOA32(pExportDir->VirtualAddress, g_szPEBuff));
  // 导出函数数量
  m_csCntFunc.Format(L"%08X", dwExportFunctionsCnt);
  // 特征值
  m_csCharact.Format(L"%08X", pExportTable->Characteristics);
  // 函数名数量(带名字的导出函数数量)
  m_csCntFuncName.Format(L"%08X", pExportTable->NumberOfNames);
  // 基数
  m_csBasic.Format(L"%08X", pExportTable->Base);
  // 函数地址(EAT表起始RVA)
  m_csEATRva.Format(L"%08X", pExportTable->AddressOfFunctions);
  // 名称(Dll文件的名称字符串RVA)
  m_csNameRva.Format(L"%08X", pExportTable->Name);
  // 函数名称地址(ENT表起始RVA)
  m_csENTRva.Format(L"%08X", pExportTable->AddressOfNames);
  // 名称字串
  char* szDllName = (char*)(RVAToFOA32(pExportTable->Name, g_szPEBuff) + (DWORD)g_szPEBuff);
  WCHAR wszDllName[256] = { 0 };
  wsprintf(wszDllName, L"%S", szDllName);
  m_csName.Format(L"%s", wszDllName);
  // 函数名序号地址(EOT表起始RVA)
  m_csEOTRva.Format(L"%08X", pExportTable->AddressOfNameOrdinals);
  UpdateData(FALSE);

  // 循环遍历导出表,显示导出函数具体的信息
  DWORD dwIndex = 0;
  for (size_t i = 0; i < dwExportFunctionsCnt; i++)
  {
    // 导入地址表当前项为空继续遍历下一个
    if (pEAT[i] == NULL)
    {
      continue;
    }
    // 根据具有函数名的导出函数的总数进行循环,用序号作为下标
    // 导出函数序号表的某项上的值(序号)对应的是导出函数名称表的索引
    // 遍历导出序号表,如果导出序号表当前项上的值与i相同就退出循环,表示EAT与EOT表关联了起来
    DWORD dwOrdinal = 0;
    for (dwOrdinal; dwOrdinal < pExportTable->NumberOfNames; dwOrdinal++)
    {
      // 循环到当前序号跳出
      if (pEOT[dwOrdinal] == i)
      {
        break;
      }
    }
    // 该导出函数的序号
    CString csBaseOrdinal;
    csBaseOrdinal.Format(L"%04X", pExportTable->Base + i);
    m_lstExportInfo.InsertItem(dwIndex, csBaseOrdinal);
    // 该导出函数的地址在内存中的RVA
    CString csFuncRva;
    csFuncRva.Format(L"%08X", pEAT[i]);
    m_lstExportInfo.SetItemText(dwIndex, 1, csFuncRva);
    // 该导出函数的地址在文件中的偏移
    CString csFuncOffset;
    csFuncOffset.Format(L"%08X", RVAToFOA32(pEAT[i], g_szPEBuff));
    m_lstExportInfo.SetItemText(dwIndex, 2, csFuncOffset);
    // 该导出函数的函数名
    // 如果是没有名称的导出函数(相当于遍历到ENT表的最后一个索引+1的位置,表示ENT中没有这个函数名)
    if (dwOrdinal == pExportTable->NumberOfNames)
    {
      m_lstExportInfo.SetItemText(dwIndex, 3, L"-"); // 显示"-"
    }
    else // 有名称的导出函数
    {
      char* szFuncName = (char*)(RVAToFOA32(pENT[dwOrdinal], g_szPEBuff) + (DWORD)g_szPEBuff);
      WCHAR wszFuncName[256] = { 0 };
      wsprintf(wszFuncName, L"%S", szFuncName);
      m_lstExportInfo.SetItemText(dwIndex, 3, wszFuncName);  // 显示名称
    }
    dwIndex++; // 行+1
  }
}

void CExportTlb::InitExportTableInfo64()
{
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS64 pNThead = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER64 pOpthead = &(pNThead->OptionalHeader);

  // 找到导出表的VA和Size
  PIMAGE_DATA_DIRECTORY pExportDir = (pOpthead->DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT);
  // 此时是文件加载的PE,因此需要先转换为文件中的偏移再加基地址找到导出表
  PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)(RVAToFOA64(pExportDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 导出函数总数
  DWORD dwExportFunctionsCnt = pExportTable->NumberOfFunctions;
  // 指向导入地址表,该表中存储所有导入函数的地址
  PDWORD pEAT = (PDWORD)(RVAToFOA64(pExportTable->AddressOfFunctions, g_szPEBuff) + g_szPEBuff);
  // 指向导入名称表,该表中存储所有有名字的导入函数的名称字符串
  PDWORD pENT = (PDWORD)(RVAToFOA64(pExportTable->AddressOfNames, g_szPEBuff) + g_szPEBuff);
  // 指向导入序号表,该表中存储所有有名字的导入函数在EAT中的索引下标
  PWORD pEOT = (PWORD)(RVAToFOA64(pExportTable->AddressOfNameOrdinals, g_szPEBuff) + g_szPEBuff);

  // 显示导出表总的一些信息(IMAGE_EXPORT_DIRECTORY结构体相关信息)
  UpdateData(TRUE);
  // 导出表偏移
  m_csExportOffset.Format(L"%08X", RVAToFOA32(pExportDir->VirtualAddress, g_szPEBuff));
  // 导出函数数量
  m_csCntFunc.Format(L"%08X", dwExportFunctionsCnt);
  // 特征值
  m_csCharact.Format(L"%08X", pExportTable->Characteristics);
  // 函数名数量(带名字的导出函数数量)
  m_csCntFuncName.Format(L"%08X", pExportTable->NumberOfNames);
  // 基数
  m_csBasic.Format(L"%08X", pExportTable->Base);
  // 函数地址(EAT表起始RVA)
  m_csEATRva.Format(L"%08X", pExportTable->AddressOfFunctions);
  // 名称(Dll文件的名称字符串RVA)
  m_csNameRva.Format(L"%08X", pExportTable->Name);
  // 函数名称地址(ENT表起始RVA)
  m_csENTRva.Format(L"%08X", pExportTable->AddressOfNames);
  // 名称字串
  char* szDllName = (char*)(RVAToFOA32(pExportTable->Name, g_szPEBuff) + (DWORD)g_szPEBuff);
  WCHAR wszDllName[256] = { 0 };
  wsprintf(wszDllName, L"%S", szDllName);
  m_csName.Format(L"%s", wszDllName);
  // 函数名序号地址(EOT表起始RVA)
  m_csEOTRva.Format(L"%08X", pExportTable->AddressOfNameOrdinals);
  UpdateData(FALSE);

  // 循环遍历导出表,显示导出函数具体的信息
  DWORD dwIndex = 0;
  for (size_t i = 0; i < dwExportFunctionsCnt; i++)
  {
    // 导入地址表当前项为空继续遍历下一个
    if (pEAT[i] == NULL)
    {
      continue;
    }
    // 根据具有函数名的导出函数的总数进行循环,用序号作为下标
    // 导出函数序号表的某项上的值(序号)对应的是导出函数名称表的索引
    // 遍历导出序号表,如果导出序号表当前项上的值与i相同就退出循环,表示EAT与EOT表关联了起来
    DWORD dwOrdinal = 0;
    for (dwOrdinal; dwOrdinal < pExportTable->NumberOfNames; dwOrdinal++)
    {
      // 循环到当前序号跳出
      if (pEOT[dwOrdinal] == i)
      {
        break;
      }
    }
    // 该导出函数的序号
    CString csBaseOrdinal;
    csBaseOrdinal.Format(L"%04X", pExportTable->Base + i);
    m_lstExportInfo.InsertItem(dwIndex, csBaseOrdinal);
    // 该导出函数的地址在内存中的RVA
    CString csFuncRva;
    csFuncRva.Format(L"%08X", pEAT[i]);
    m_lstExportInfo.SetItemText(dwIndex, 1, csFuncRva);
    // 该导出函数的地址在文件中的偏移
    CString csFuncOffset;
    csFuncOffset.Format(L"%08X", RVAToFOA32(pEAT[i], g_szPEBuff));
    m_lstExportInfo.SetItemText(dwIndex, 2, csFuncOffset);
    // 该导出函数的函数名
    // 如果是没有名称的导出函数(相当于遍历到ENT表的最后一个索引+1的位置,表示ENT中没有这个函数名)
    if (dwOrdinal == pExportTable->NumberOfNames)
    {
      m_lstExportInfo.SetItemText(dwIndex, 3, L"-");
    }
    else // 有名称的导出函数
    {
      char* szFuncName = (char*)(RVAToFOA32(pENT[dwOrdinal], g_szPEBuff) + (DWORD)g_szPEBuff);
      WCHAR wszFuncName[256] = { 0 };
      wsprintf(wszFuncName, L"%S", szFuncName);
      m_lstExportInfo.SetItemText(dwIndex, 3, wszFuncName);
    }
    dwIndex++; // 行+1
  }
}


DWORD CExportTlb::RVAToFOA32(DWORD dwRva, char* szBuff)
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

ULONGLONG CExportTlb::RVAToFOA64(ULONGLONG ullRva, char* szBuff)
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