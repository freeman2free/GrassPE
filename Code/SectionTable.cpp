// SectionTable.cpp: 实现文件
//

#include "pch.h"
#include "GrassPE.h"
#include "SectionTable.h"
#include "afxdialogex.h"
#include "GrassPEDlg.h"

// CSectionTable 对话框

IMPLEMENT_DYNAMIC(CSectionTable, CDialogEx)

CSectionTable::CSectionTable(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SECTIONTLB, pParent)
{

}

CSectionTable::~CSectionTable()
{
}

void CSectionTable::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SECTIONTLB, m_lstSectionTlb);
}


BEGIN_MESSAGE_MAP(CSectionTable, CDialogEx)
END_MESSAGE_MAP()


// CSectionTable 消息处理程序


BOOL CSectionTable::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_lstSectionTlb.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	// 设置6列
	m_lstSectionTlb.InsertColumn(0, L"Name", LVCFMT_CENTER, 80);
	m_lstSectionTlb.InsertColumn(1, L"VAddress", LVCFMT_CENTER, 80);
	m_lstSectionTlb.InsertColumn(2, L"VSize", LVCFMT_CENTER, 80);
	m_lstSectionTlb.InsertColumn(3, L"ROffset", LVCFMT_CENTER, 80);
	m_lstSectionTlb.InsertColumn(4, L"RSize", LVCFMT_CENTER, 80);
	m_lstSectionTlb.InsertColumn(5, L"Flags", LVCFMT_CENTER, 80);
  if (CGrassPEDlg::Is64PE(g_szPEBuff))
  {
    InitSectionTableInfo64();
  }
  else
  {
    InitSectionTableInfo();
  }
  
	return TRUE;  // return TRUE unless you set the focus to a control
								// 异常: OCX 属性页应返回 FALSE
}

void CSectionTable::InitSectionTableInfo()
{
  // 清空列表区准备显示区段信息
  m_lstSectionTlb.DeleteAllItems();
  if (NULL == g_szPEBuff)
  {
    return;
  }
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS pNThead = (PIMAGE_NT_HEADERS)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_FILE_HEADER pFilehead = &pNThead->FileHeader;
  PIMAGE_OPTIONAL_HEADER pOpthead = &pNThead->OptionalHeader;
  // 区段数
  WORD wSectionCnt = pFilehead->NumberOfSections;
  // 得到区段表
  PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNThead);

  DWORD dwIndex = 0; // 行计数
  for (size_t i = 0; i < wSectionCnt; i++)
  {
    CString csNameBuf;
    csNameBuf.Format(L"%S", pSectionHeader[i].Name);
    // 区段名显示在第0列上
    m_lstSectionTlb.InsertItem(dwIndex, csNameBuf);
    // 内存中区段的RVA
    CString csVAddress;
    csVAddress.Format(L"%08X", pSectionHeader[i].VirtualAddress);
    m_lstSectionTlb.SetItemText(dwIndex, 1, csVAddress);
    // 内存中区段大小
    CString csVSize;
    csVSize.Format(L"%08X", pSectionHeader[i].Misc.VirtualSize);
    m_lstSectionTlb.SetItemText(dwIndex, 2, csVSize);

    // 文件偏移
    CString csROffset;
    csROffset.Format(L"%08X", pSectionHeader[i].PointerToRawData);
    m_lstSectionTlb.SetItemText(dwIndex, 3, csROffset);

    // 文件内区段大小
    CString csRSize;
    csRSize.Format(L"%08X", pSectionHeader[i].SizeOfRawData);
    m_lstSectionTlb.SetItemText(dwIndex, 4, csRSize);

    // 区段属性标志
    CString csCharact;
    csCharact.Format(L"%08X", pSectionHeader[i].Characteristics);
    m_lstSectionTlb.SetItemText(dwIndex, 5, csCharact);

    dwIndex++; // 行+1
  }
}

void CSectionTable::InitSectionTableInfo64()
{
  // 清空列表区准备显示区段信息
  m_lstSectionTlb.DeleteAllItems();
  if (NULL == g_szPEBuff)
  {
    return;
  }
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS64 pNThead = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_FILE_HEADER pFilehead = &pNThead->FileHeader;
  PIMAGE_OPTIONAL_HEADER64 pOpthead = &pNThead->OptionalHeader;
  // 区段数
  WORD wSectionCnt = pFilehead->NumberOfSections;
  // 得到区段表
  PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNThead);

  DWORD dwIndex = 0; // 行计数
  for (size_t i = 0; i < wSectionCnt; i++)
  {
    CString csNameBuf;
    csNameBuf.Format(L"%S", pSectionHeader[i].Name);
    // 区段名显示在第0列上
    m_lstSectionTlb.InsertItem(dwIndex, csNameBuf);
    // 内存中区段的RVA
    CString csVAddress;
    csVAddress.Format(L"%08X", pSectionHeader[i].VirtualAddress);
    m_lstSectionTlb.SetItemText(dwIndex, 1, csVAddress);
    // 内存中区段大小
    CString csVSize;
    csVSize.Format(L"%08X", pSectionHeader[i].Misc.VirtualSize);
    m_lstSectionTlb.SetItemText(dwIndex, 2, csVSize);

    // 文件偏移
    CString csROffset;
    csROffset.Format(L"%08X", pSectionHeader[i].PointerToRawData);
    m_lstSectionTlb.SetItemText(dwIndex, 3, csROffset);

    // 文件内区段大小
    CString csRSize;
    csRSize.Format(L"%08X", pSectionHeader[i].SizeOfRawData);
    m_lstSectionTlb.SetItemText(dwIndex, 4, csRSize);

    // 区段属性标志
    CString csCharact;
    csCharact.Format(L"%08X", pSectionHeader[i].Characteristics);
    m_lstSectionTlb.SetItemText(dwIndex, 5, csCharact);

    dwIndex++; // 行+1
  }
}