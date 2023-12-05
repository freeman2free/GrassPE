// DataDirectory.cpp: 实现文件
//

#include "pch.h"
#include "GrassPE.h"
#include "DataDirectory.h"
#include "afxdialogex.h"
#include "GrassPEDlg.h"
#include "ExportTlb.h"
#include "ImportTlb.h"
#include "ResourceTlb.h"
#include "RelocTlb.h"
#include "TlsTlb.h"
#include "DelayImpTlb.h"
// CDataDirectory 对话框

IMPLEMENT_DYNAMIC(CDataDirectory, CDialogEx)

CDataDirectory::CDataDirectory(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DATADIR, pParent)
{

}

CDataDirectory::~CDataDirectory()
{
}

void CDataDirectory::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDataDirectory, CDialogEx)
  ON_BN_CLICKED(IDC_BUTTON_EXPORT, &CDataDirectory::OnBnClickedButtonExport)
  ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CDataDirectory::OnBnClickedButtonImport)
  ON_BN_CLICKED(IDC_BUTTON_RES, &CDataDirectory::OnBnClickedButtonRes)
  ON_BN_CLICKED(IDC_BUTTON_RELOC, &CDataDirectory::OnBnClickedButtonReloc)
  ON_BN_CLICKED(IDC_BUTTON_TLS, &CDataDirectory::OnBnClickedButtonTls)
  ON_BN_CLICKED(IDC_BUTTON_DELAYIMP, &CDataDirectory::OnBnClickedButtonDelayimp)
END_MESSAGE_MAP()


// CDataDirectory 消息处理程序


BOOL CDataDirectory::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
  if (CGrassPEDlg::Is64PE(g_szPEBuff))
  {
    InitDataDirectorInfo64();
  }
  else
  {
    InitDataDirectorInfo();
  }
	return TRUE;  // return TRUE unless you set the focus to a control
								// 异常: OCX 属性页应返回 FALSE
}

void CDataDirectory::InitDataDirectorInfo()
{
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS32 pNThead = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER pOpthead = &pNThead->OptionalHeader;
  // 将每个表的信息填入(每种表的RVA和Size)
  CString csRva;
  CString csSize;
  for (size_t i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++)
  {
    csRva.Format(L"%08X", pOpthead->DataDirectory[i].VirtualAddress);
    csSize.Format(L"%08X", pOpthead->DataDirectory[i].Size);
    SetDlgItemText(m_EditIDArr[i].ID_RVA, csRva);
    SetDlgItemText(m_EditIDArr[i].ID_SIZE, csSize);
  }
}

void CDataDirectory::InitDataDirectorInfo64()
{
  PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)g_szPEBuff;
  PIMAGE_NT_HEADERS64 pNThead = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDoshead->e_lfanew);
  PIMAGE_OPTIONAL_HEADER64 pOpthead = &(pNThead->OptionalHeader);
  // 将每个表的信息填入
  CString csRva;
  CString csSize;
  for (size_t i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++)
  {
    csRva.Format(L"%08X", pOpthead->DataDirectory[i].VirtualAddress);
    csSize.Format(L"%08X", pOpthead->DataDirectory[i].Size);
    SetDlgItemText(m_EditIDArr[i].ID_RVA, csRva);
    SetDlgItemText(m_EditIDArr[i].ID_SIZE, csSize);
  }
}


void CDataDirectory::OnBnClickedButtonExport()
{
  // 先判断是否有导出表再确定是否弹出导出表对话框
  CString csExpRva;
  CString csExpSize;
  GetDlgItem(m_EditIDArr[0].ID_RVA)->GetWindowText(csExpRva);
  GetDlgItem(m_EditIDArr[0].ID_SIZE)->GetWindowText(csExpSize);
  if (csExpRva == "00000000" && csExpSize == "00000000")
  {
    ::MessageBox(NULL, L"当前无导出表!", L"ERROR", MB_OK | MB_ICONSTOP);
    return;
  }
  CExportTlb ExportTlbDlg;
  ExportTlbDlg.DoModal();
}


void CDataDirectory::OnBnClickedButtonImport()
{
  // 先判断是否有导入表再确定是否弹出导出表对话框
  CString csImpRva;
  CString csImpSize;
  GetDlgItem(m_EditIDArr[1].ID_RVA)->GetWindowText(csImpRva);
  GetDlgItem(m_EditIDArr[1].ID_SIZE)->GetWindowText(csImpSize);
  if (csImpRva == "00000000" && csImpSize == "00000000")
  {
    ::MessageBox(NULL, L"当前无导入表!", L"ERROR", MB_OK | MB_ICONSTOP);
    return;
  }
  CImportTlb ImportTlbDlg;
  ImportTlbDlg.DoModal();
}


void CDataDirectory::OnBnClickedButtonRes()
{
  // TODO: 在此添加控件通知处理程序代码
  CString csResRva;
  CString csResSize;
  GetDlgItem(m_EditIDArr[2].ID_RVA)->GetWindowText(csResRva);
  GetDlgItem(m_EditIDArr[2].ID_SIZE)->GetWindowText(csResSize);
  if (csResRva == "00000000" && csResSize == "00000000")
  {
    ::MessageBox(NULL, L"当前无资源表!", L"ERROR", MB_OK | MB_ICONSTOP);
    return;
  }
  CResourceTlb ResourceTlbDlg;
  ResourceTlbDlg.DoModal();
}


void CDataDirectory::OnBnClickedButtonReloc()
{
  // TODO: 在此添加控件通知处理程序代码
  CString csRelocRva;
  CString csRelocSize;
  GetDlgItem(m_EditIDArr[5].ID_RVA)->GetWindowText(csRelocRva);
  GetDlgItem(m_EditIDArr[5].ID_SIZE)->GetWindowText(csRelocSize);
  if (csRelocRva == "00000000" && csRelocSize == "00000000")
  {
    ::MessageBox(NULL, L"当前无重定位表!", L"ERROR", MB_OK | MB_ICONSTOP);
    return;
  }
  CRelocTlb RelocTlbDlg;
  RelocTlbDlg.DoModal();
}


void CDataDirectory::OnBnClickedButtonTls()
{
  // TODO: 在此添加控件通知处理程序代码
  CString csTLSRva;
  CString csTLSSize;
  GetDlgItem(m_EditIDArr[9].ID_RVA)->GetWindowText(csTLSRva);
  GetDlgItem(m_EditIDArr[9].ID_SIZE)->GetWindowText(csTLSSize);
  if (csTLSRva == "00000000" && csTLSSize == "00000000")
  {
    ::MessageBox(NULL, L"当前无TLS表!", L"ERROR", MB_OK | MB_ICONSTOP);
    return;
  }
  CTlsTlb TlsTlbDlg;
  TlsTlbDlg.DoModal();
}


void CDataDirectory::OnBnClickedButtonDelayimp()
{
  // TODO: 在此添加控件通知处理程序代码
  CString csDelayRva;
  CString csDelaySize;
  GetDlgItem(m_EditIDArr[13].ID_RVA)->GetWindowText(csDelayRva);
  GetDlgItem(m_EditIDArr[13].ID_SIZE)->GetWindowText(csDelaySize);
  if (csDelayRva == "00000000" && csDelaySize == "00000000")
  {
    ::MessageBox(NULL, L"当前无延迟导入表!", L"ERROR", MB_OK | MB_ICONSTOP);
    return;
  }
  CDelayImpTlb DelayImpTlbDlg;
  DelayImpTlbDlg.DoModal();
}
