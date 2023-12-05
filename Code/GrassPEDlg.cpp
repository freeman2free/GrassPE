
// GrassPEDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "GrassPE.h"
#include "GrassPEDlg.h"
#include "afxdialogex.h"
#include "SectionTable.h"
#include "DataDirectory.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGrassPEDlg 对话框


char* g_szPEBuff;
CGrassPEDlg::CGrassPEDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GRASSPE_DIALOG, pParent)
	, m_csPath(_T(""))
	, m_csImageBase(_T(""))
	, m_csMachine(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGrassPEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILEPATH, m_csPath);
	DDX_Text(pDX, IDC_EDIT_ENTRY, m_csEntry);
	DDX_Text(pDX, IDC_EDIT_IMAGEBASE, m_csImageBase);

	DDX_Text(pDX, IDC_EDIT_CODEBASE, m_csCodeBase);
	DDX_Text(pDX, IDC_EDITDATABASE, m_csDataBase);
	DDX_Text(pDX, IDC_EDIT_IMAGESIZE, m_csImageSize);
	DDX_Text(pDX, IDC_EDIT_HEADERSIZE, m_csHeaderSize);
	DDX_Text(pDX, IDC_EDITSECTIONALIG, m_csSectionAlig);
	DDX_Text(pDX, IDC_EDITFILEALIG, m_csFileAlig);
	DDX_Text(pDX, IDC_EDIT_SUBSYSTEM, m_csSubsystem);
	DDX_Text(pDX, IDC_EDIT_CHECKSUM, m_csCheckSum);
	DDX_Text(pDX, IDC_EDIT_FLAGS, m_csFlags);
	DDX_Text(pDX, IDC_EDIT_MACHINE, m_csMachine);
	DDX_Text(pDX, IDC_EDIT_NUMSECTION, m_csNumSection);
	DDX_Text(pDX, IDC_EDIT_TIMESTAMP, m_csTimeStamp);
	DDX_Text(pDX, IDC_EDIT_SYMBOLPOINTER, m_csPSymbol);
	DDX_Text(pDX, IDC_EDIT_NUMSYMBOL, m_csNumSymbol);
	DDX_Text(pDX, IDC_EDIT_SIZEOPTHEADER, m_csSizeOptHead);
	DDX_Text(pDX, IDC_EDIT_CHARACTER, m_csCharact);
	DDX_Text(pDX, IDC_EDIT_MACHINE, m_csMachine);
	DDX_Control(pDX, IDC_BUTTON_SECTIONTABLE, m_btnSectionTlb);
	DDX_Control(pDX, IDC_BUTTON_DATADIRECT, m_btnDataDir);
}

BEGIN_MESSAGE_MAP(CGrassPEDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPENFILE, &CGrassPEDlg::OnBnClickedButtonOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_SECTIONTABLE, &CGrassPEDlg::OnBnClickedButtonSectiontable)
	ON_BN_CLICKED(IDC_BUTTON_DATADIRECT, &CGrassPEDlg::OnBnClickedButtonDatadirect)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CGrassPEDlg::OnBnClickedButtonExit)
END_MESSAGE_MAP()


// CGrassPEDlg 消息处理程序

BOOL CGrassPEDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 将两个按钮先禁用
	m_btnSectionTlb.EnableWindow(FALSE);
	m_btnDataDir.EnableWindow(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGrassPEDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGrassPEDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CGrassPEDlg::OnBnClickedButtonOpenfile()
{
	// TODO: 在此添加控件通知处理程序代码
	// 快速选择文件获取其完整路径(根据指定的正则)
	CFileDialog objFileDlg(TRUE, NULL, NULL, NULL, L"PE文件|*.exe;*.dll;*.dmp||", this);
	objFileDlg.DoModal(); // 此处会弹出一个通用对话框(类似于文件搜索的对话框)
	// 将文件的完整路径显示到编辑框中(根据控件ID进行设置)
	m_csPath = objFileDlg.GetPathName();
	SetDlgItemText(IDC_EDIT_FILEPATH, m_csPath);

	// 
	HANDLE hPEFile = CreateFile(m_csPath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hPEFile || NULL == hPEFile)
	{
		AfxMessageBox(L"文件打开失败(File open failure)", MB_OK);
		return;
	}
	DWORD dwFileSize = GetFileSize(hPEFile, NULL);
	DWORD dwFileReadSize = 0;
	g_szPEBuff = new char[dwFileSize + 1]{ 0 };
	BOOL bRet = ReadFile(hPEFile, g_szPEBuff, dwFileSize, &dwFileReadSize, NULL);
	if (!bRet)
	{
		AfxMessageBox(L"文件打开失败(File open failure)", MB_OK);
		return;
	}
	CloseHandle(hPEFile);
	// 判断一下是否是真的PE文件
	if (!IsPEFile(g_szPEBuff))
	{
		AfxMessageBox(L"不是标准PE文件(It's not normal PE File)", MB_OK);
		return;
	}
	// 显示PE文件 头部的相关信息
	DWORD dwRet = 0;
	dwRet = PEHeaderInfo(g_szPEBuff);
	// 根据返回值来进行处理
	if (IS_OK == dwRet)
	{
		ActiveButton();
	}
}

BOOL CGrassPEDlg::IsPEFile(char* szFileBuff)
{
	PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)szFileBuff;
	if (IMAGE_DOS_SIGNATURE != pDoshead->e_magic)
	{
		return FALSE;
	}
	PIMAGE_NT_HEADERS pNThead = (PIMAGE_NT_HEADERS)(szFileBuff + pDoshead->e_lfanew);
	if (IMAGE_NT_SIGNATURE != pNThead->Signature)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CGrassPEDlg::Is64PE(char* PEFile)
{
	PIMAGE_DOS_HEADER pDoshead = (PIMAGE_DOS_HEADER)PEFile;
	PIMAGE_NT_HEADERS pNThead = (PIMAGE_NT_HEADERS)(PEFile + pDoshead->e_lfanew);
	if (pNThead->FileHeader.Machine == 0x8664)
	{
		return TRUE;
	}
	return FALSE;
}


DWORD CGrassPEDlg::PEHeaderInfo(char* PEFile)
{
	PIMAGE_DOS_HEADER pDosHead = (PIMAGE_DOS_HEADER)PEFile;
	PIMAGE_NT_HEADERS pNtHead = (PIMAGE_NT_HEADERS)(PEFile + pDosHead->e_lfanew);
	// 如果是64位PE文件
	if (Is64PE(PEFile))
	{
		SetPEHeadInfo(pNtHead, 1);
	}
	else
	{
		SetPEHeadInfo(pNtHead, 0);
	}
	return IS_OK;
}

void CGrassPEDlg::SetPEHeadInfo(PIMAGE_NT_HEADERS pNtHead, BYTE bFlag /*=0*/)
{
	if (NULL == pNtHead)
	{
		return;
	}
	PIMAGE_FILE_HEADER pFileHead = &(pNtHead->FileHeader);
	PIMAGE_OPTIONAL_HEADER32 pOptHead = NULL;
	PIMAGE_OPTIONAL_HEADER64 pOptHead64 = NULL;
	// 64位PE文件的信息显示
	UpdateData(TRUE);
	if (bFlag == 1)
	{
		PIMAGE_NT_HEADERS64 pNtHead64 = (PIMAGE_NT_HEADERS64)pNtHead;
		pOptHead64 = &(pNtHead64->OptionalHeader);
		// 显示选项头信息
		m_csEntry.Format(L"%08X", pOptHead64->AddressOfEntryPoint);

		m_csImageBase.Format(L"%016llX", pOptHead64->ImageBase);

		m_csCodeBase.Format(L"%08X", pOptHead64->BaseOfCode);

		m_csImageSize.Format(L"%08X", pOptHead64->SizeOfImage);

		m_csHeaderSize.Format(L"%08X", pOptHead64->SizeOfHeaders);

		m_csSectionAlig.Format(L"%08X", pOptHead64->SectionAlignment);

		m_csFileAlig.Format(L"%08X", pOptHead64->FileAlignment);

		m_csSubsystem.Format(L"%08X", pOptHead64->Subsystem);

		m_csCheckSum.Format(L"%04X", pOptHead64->CheckSum);

		m_csFlags.Format(L"%04X", pOptHead64->DllCharacteristics);
	}
	else
	{
		pOptHead = &(pNtHead->OptionalHeader);
		// 显示选项头信息
		m_csEntry.Format(L"%08X", pOptHead->AddressOfEntryPoint);

		m_csImageBase.Format(L"%08X", pOptHead->ImageBase);

		m_csCodeBase.Format(L"%08X", pOptHead->BaseOfCode);

		m_csDataBase.Format(L"%08X", pOptHead->BaseOfData);

		m_csImageSize.Format(L"%08X", pOptHead->SizeOfImage);

		m_csHeaderSize.Format(L"%08X", pOptHead->SizeOfHeaders);
		
		m_csSectionAlig.Format(L"%08X", pOptHead->SectionAlignment);
		
		m_csFileAlig.Format(L"%08X", pOptHead->FileAlignment);
		
		m_csSubsystem.Format(L"%08X", pOptHead->Subsystem);
		
		m_csCheckSum.Format(L"%04X", pOptHead->CheckSum);
		
		m_csFlags.Format(L"%04X", pOptHead->DllCharacteristics);
	}

	// 显示文件头信息
	m_csMachine.Format(L"%04X", pFileHead->Machine);

	m_csNumSection.Format(L"%d", pFileHead->NumberOfSections);

	m_csTimeStamp.Format(L"%08X", pFileHead->TimeDateStamp);
	
	m_csPSymbol.Format(L"%08X", pFileHead->PointerToSymbolTable);
	
	m_csNumSymbol.Format(L"%08X", pFileHead->NumberOfSymbols);
	
	m_csSizeOptHead.Format(L"%04X", pFileHead->SizeOfOptionalHeader);
	
	m_csCharact.Format(L"%04X", pFileHead->Characteristics);
	UpdateData(FALSE);
}

void CGrassPEDlg::ActiveButton()
{
	m_btnSectionTlb.EnableWindow(TRUE);
	m_btnDataDir.EnableWindow(TRUE);
}

void CGrassPEDlg::OnBnClickedButtonSectiontable()
{
	// TODO: 在此添加控件通知处理程序代码
	CSectionTable SectionTlbDlg;
	SectionTlbDlg.DoModal();
}


void CGrassPEDlg::OnBnClickedButtonDatadirect()
{
	// TODO: 在此添加控件通知处理程序代码
	CDataDirectory DataDirectoryDlg;
	DataDirectoryDlg.DoModal();
}


void CGrassPEDlg::OnBnClickedButtonExit()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}
