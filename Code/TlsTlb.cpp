// TlsTlb.cpp: 实现文件
//

#include "pch.h"
#include "GrassPE.h"
#include "TlsTlb.h"
#include "afxdialogex.h"
#include "GrassPEDlg.h"

// CTlsTlb 对话框

IMPLEMENT_DYNAMIC(CTlsTlb, CDialogEx)

CTlsTlb::CTlsTlb(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_TLS, pParent)
	, m_csStartVa(_T(""))
	, m_csEndVa(_T(""))
	, m_csAddIndexVa(_T(""))
	, m_csCallBackVa(_T(""))
	, m_csZeroFill(_T(""))
	, m_csCharact(_T(""))
{

}

CTlsTlb::~CTlsTlb()
{
}

void CTlsTlb::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_STARTVA, m_csStartVa);
	DDX_Text(pDX, IDC_EDIT_ENDVA, m_csEndVa);
	DDX_Text(pDX, IDC_EDIT_ADDINDEXVA, m_csAddIndexVa);
	DDX_Text(pDX, IDC_EDIT_CALLBACKVA, m_csCallBackVa);
	DDX_Text(pDX, IDC_EDIT_ZEROFILL, m_csZeroFill);
	DDX_Text(pDX, IDC_EDIT_CHARACT, m_csCharact);
}

DWORD CTlsTlb::RVAToFOA32(DWORD dwRva, char* szBuff)
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

ULONGLONG CTlsTlb::RVAToFOA64(ULONGLONG ullRva, char* szBuff)
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


BEGIN_MESSAGE_MAP(CTlsTlb, CDialogEx)
END_MESSAGE_MAP()


// CTlsTlb 消息处理程序


BOOL CTlsTlb::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
  if (CGrassPEDlg::Is64PE(g_szPEBuff))
  {
    InitTLSTableInfo64();
  }
  else
  {
    InitTLSTableInfo();
  }
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CTlsTlb::InitTLSTableInfo()
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS32 pNTHeader = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER32 pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的TLS表的va和Size
  PIMAGE_DATA_DIRECTORY pTLSDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_TLS; // TLS表的基址

  PIMAGE_TLS_DIRECTORY32 pTLSTable = (PIMAGE_TLS_DIRECTORY32)(RVAToFOA32(pTLSDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
	UpdateData(TRUE);
	// 内存起始地址(VA),用于初始化一个新线程的TLS
	m_csStartVa.Format(L"%08X", pTLSTable->StartAddressOfRawData);
	// 内存结束地址(VA),用于初始化一个新线程的TLS
	m_csEndVa.Format(L"%08X", pTLSTable->EndAddressOfRawData);
	// 索引VA,运行库使用这个索引来定位线程局部数据
	m_csAddIndexVa.Format(L"%08X", pTLSTable->AddressOfIndex);
	// PIMAGE_TLS_CALLBACK函数(回调)指针数组的地址(线程一般在建立和退出都有回调函数)
	m_csCallBackVa.Format(L"%08X", pTLSTable->AddressOfCallBacks);
	// 后面补零的个数
	m_csZeroFill.Format(L"%08X", pTLSTable->SizeOfZeroFill);
	// 特征值
	m_csCharact.Format(L"%08X", pTLSTable->Characteristics);
	UpdateData(FALSE);
}

void CTlsTlb::InitTLSTableInfo64()
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS64 pNTHeader = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER64 pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的TLS表的va和Size
  PIMAGE_DATA_DIRECTORY pTLSDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_TLS; // TLS表的基址

  PIMAGE_TLS_DIRECTORY64 pTLSTable = (PIMAGE_TLS_DIRECTORY64)(RVAToFOA64(pTLSDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  UpdateData(TRUE);
  // 内存起始地址(VA),用于初始化一个新线程的TLS
  m_csStartVa.Format(L"%016lX", pTLSTable->StartAddressOfRawData);
  // 内存结束地址(VA),用于初始化一个新线程的TLS
  m_csEndVa.Format(L"%016lX", pTLSTable->EndAddressOfRawData);
  // 索引VA,运行库使用这个索引来定位线程局部数据
  m_csAddIndexVa.Format(L"%016lX", pTLSTable->AddressOfIndex);
  // PIMAGE_TLS_CALLBACK函数(回调)指针数组的地址(线程一般在建立和退出都有回调函数)
  m_csCallBackVa.Format(L"%016lX", pTLSTable->AddressOfCallBacks);
  // 后面补零的个数
  m_csZeroFill.Format(L"%08X", pTLSTable->SizeOfZeroFill);
  // 特征值
  m_csCharact.Format(L"%08X", pTLSTable->Characteristics);
  UpdateData(FALSE);
}
