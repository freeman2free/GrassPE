// DelayImpTlb.cpp: 实现文件
//

#include "pch.h"
#include "GrassPE.h"
#include "DelayImpTlb.h"
#include "afxdialogex.h"
#include "GrassPEDlg.h"

// CDelayImpTlb 对话框

IMPLEMENT_DYNAMIC(CDelayImpTlb, CDialogEx)

CDelayImpTlb::CDelayImpTlb(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DELAYIMP, pParent)
{

}

CDelayImpTlb::~CDelayImpTlb()
{
}

void CDelayImpTlb::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DELAYIMP, m_lstDelayImp);
}


BEGIN_MESSAGE_MAP(CDelayImpTlb, CDialogEx)
END_MESSAGE_MAP()


// CDelayImpTlb 消息处理程序


BOOL CDelayImpTlb::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_lstDelayImp.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lstDelayImp.InsertColumn(0, L"DLL名称", LVCFMT_CENTER, 100);
	m_lstDelayImp.InsertColumn(1, L"ModuleHandle RVA", LVCFMT_CENTER, 100);
	m_lstDelayImp.InsertColumn(2, L"IAT RVA", LVCFMT_CENTER, 100);
	m_lstDelayImp.InsertColumn(3, L"INT RVA", LVCFMT_CENTER, 100);
	m_lstDelayImp.InsertColumn(4, L"Bound IAT RVA", LVCFMT_CENTER, 100);
	m_lstDelayImp.InsertColumn(5, L"Unload InformationTable RVA", LVCFMT_CENTER, 100);
	m_lstDelayImp.InsertColumn(6, L"TimeDateStamp", LVCFMT_CENTER, 100);
	m_lstDelayImp.InsertColumn(7, L"Attributes", LVCFMT_CENTER, 100);
	m_lstDelayImp.DeleteAllItems();

  if (CGrassPEDlg::Is64PE(g_szPEBuff))
  {

  }
  else
  {
    InitDelayImportTableInfo();
  }
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CDelayImpTlb::InitDelayImportTableInfo()
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS32 pNTHeader = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER32 pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的TLS表的va和Size
  PIMAGE_DATA_DIRECTORY pDelayDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT; // 延迟导入表的基址

  PIMAGE_DELAYLOAD_DESCRIPTOR pDelayTable = (PIMAGE_DELAYLOAD_DESCRIPTOR)(RVAToFOA32(pDelayDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  DWORD dwIndex = 0;
  while (pDelayTable->DllNameRVA != NULL)
  {

    printf("Dll Name RVA: %08X\r\n", pDelayTable->DllNameRVA);
    // DLL 名称
    char* szDllName = (char*)(RVAToFOA32(pDelayTable->DllNameRVA, g_szPEBuff) + (DWORD)g_szPEBuff);
    WCHAR wszDllName[256] = { 0 };
    wsprintf(wszDllName, L"%S", szDllName);
    m_lstDelayImp.InsertItem(dwIndex, wszDllName);
    // ModuleHandle RVA
    CString csModuleHandle;
    csModuleHandle.Format(L"%08X", pDelayTable->ModuleHandleRVA);
    m_lstDelayImp.SetItemText(dwIndex, 1, csModuleHandle);
    // IAT RVA 
    CString csIAT;
    csIAT.Format(L"%08X", pDelayTable->ImportAddressTableRVA);
    m_lstDelayImp.SetItemText(dwIndex, 2, csIAT);
    // INT RVA
    CString csINT;
    csINT.Format(L"%08X", pDelayTable->ImportNameTableRVA);
    m_lstDelayImp.SetItemText(dwIndex, 3, csINT);
    // Bound IAT RVA
    CString csBoundIAT;
    csBoundIAT.Format(L"%08X", pDelayTable->BoundImportAddressTableRVA);
    m_lstDelayImp.SetItemText(dwIndex, 4, csBoundIAT);
    // Unload InformationTable RVA
    CString csUnloadInfoTlb;
    csUnloadInfoTlb.Format(L"%08X", pDelayTable->UnloadInformationTableRVA);
    m_lstDelayImp.SetItemText(dwIndex, 5, csUnloadInfoTlb);
    // TimeDateStamp
    CString csTimeDateStamp;
    csTimeDateStamp.Format(L"%08X", pDelayTable->TimeDateStamp);
    m_lstDelayImp.SetItemText(dwIndex, 6, csTimeDateStamp);
    // Attributes
    CString csAttributes;
    csAttributes.Format(L"%08X", pDelayTable->Attributes.AllAttributes);
    m_lstDelayImp.SetItemText(dwIndex, 7, csAttributes);

    dwIndex++;
    pDelayTable++; // 下一张延迟导入表
  }
}

void CDelayImpTlb::InitDelayImportTableInfo64()
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS64 pNTHeader = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER64 pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的TLS表的va和Size
  PIMAGE_DATA_DIRECTORY pDelayDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT; // 延迟导入表的基址

  PIMAGE_DELAYLOAD_DESCRIPTOR pDelayTable = (PIMAGE_DELAYLOAD_DESCRIPTOR)(RVAToFOA64(pDelayDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  DWORD dwIndex = 0;
  while (pDelayTable->DllNameRVA != NULL)
  {
    printf("Dll Name RVA: %08X\r\n", pDelayTable->DllNameRVA);
    // DLL 名称
    char* szDllName = (char*)(RVAToFOA32(pDelayTable->DllNameRVA, g_szPEBuff) + (DWORD)g_szPEBuff);
    WCHAR wszDllName[256] = { 0 };
    wsprintf(wszDllName, L"%S", szDllName);
    m_lstDelayImp.InsertItem(dwIndex, wszDllName);
    // ModuleHandle RVA
    CString csModuleHandle;
    csModuleHandle.Format(L"%08X", pDelayTable->ModuleHandleRVA);
    m_lstDelayImp.SetItemText(dwIndex, 1, csModuleHandle);
    // IAT RVA 
    CString csIAT;
    csIAT.Format(L"%08X", pDelayTable->ImportAddressTableRVA);
    m_lstDelayImp.SetItemText(dwIndex, 2, csIAT);
    // INT RVA
    CString csINT;
    csINT.Format(L"%08X", pDelayTable->ImportNameTableRVA);
    m_lstDelayImp.SetItemText(dwIndex, 3, csINT);
    // Bound IAT RVA
    CString csBoundIAT;
    csBoundIAT.Format(L"%08X", pDelayTable->BoundImportAddressTableRVA);
    m_lstDelayImp.SetItemText(dwIndex, 4, csBoundIAT);
    // Unload InformationTable RVA
    CString csUnloadInfoTlb;
    csUnloadInfoTlb.Format(L"%08X", pDelayTable->UnloadInformationTableRVA);
    m_lstDelayImp.SetItemText(dwIndex, 5, csUnloadInfoTlb);
    // TimeDateStamp
    CString csTimeDateStamp;
    csTimeDateStamp.Format(L"%08X", pDelayTable->TimeDateStamp);
    m_lstDelayImp.SetItemText(dwIndex, 6, csTimeDateStamp);
    // Attributes
    CString csAttributes;
    csAttributes.Format(L"%08X", pDelayTable->Attributes);
    m_lstDelayImp.SetItemText(dwIndex, 7, csAttributes);

    dwIndex++;
    pDelayTable++; // 下一张延迟导入表
  }
}

DWORD CDelayImpTlb::RVAToFOA32(DWORD dwRva, char* szBuff)
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

ULONGLONG CDelayImpTlb::RVAToFOA64(ULONGLONG ullRva, char* szBuff)
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


