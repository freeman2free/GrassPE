
// GrassPEDlg.h: 头文件
//

#pragma once

#define IS_OK     0x1111
// 用于保存PE文件的内容
extern char* g_szPEBuff;
// CGrassPEDlg 对话框
class CGrassPEDlg : public CDialogEx
{
// 构造
public:
	CGrassPEDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GRASSPE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	// 保存文件完整路径
	CString m_csPath;
	// 显示入口点
	CString m_csEntry;
	// 显示ImageBase
	CString m_csImageBase;
	// 显示Code Base
	CString m_csCodeBase;
	// 显示Data Base
	CString m_csDataBase;
	// 显示Image Size
	CString m_csImageSize;
	// 显示Header Size
	CString m_csHeaderSize;
	// 显示Section Alignment
	CString m_csSectionAlig;
	// 显示File Alignment
	CString m_csFileAlig;
	// 显示Sub system
	CString m_csSubsystem;
	// 显示Check Sum
	CString m_csCheckSum;
	// 显示Dll Flags
	CString m_csFlags;
	// 显示Machine
	CString m_csMachine;
	// 显示Number of sections
	CString m_csNumSection;
	// 显示时间戳
	CString m_csTimeStamp;
	// 显示符号表的指针
	CString m_csPSymbol;
	// 显示符号数量
	CString m_csNumSymbol;
	// 显示选项头大小
	CString m_csSizeOptHead;
	// 显示特性
	CString m_csCharact;

	// 该按钮用于弹出区段表信息的对话框
	CButton m_btnSectionTlb;
	// 该按钮用于弹出数据目录表信息的对话框
	CButton m_btnDataDir;
public:
	// 打开文件
	afx_msg void OnBnClickedButtonOpenfile();
	// 判断是否是PE文件
	static BOOL IsPEFile(char* szFileBuff);
	// 判断是64/32位PEwenj
	static BOOL Is64PE(char* PEFile);
	// 解析PE文件头信息
	DWORD PEHeaderInfo(char* PEFile);
	// 将PE文件头的各种信息设置到界面中
	void SetPEHeadInfo(PIMAGE_NT_HEADERS pNtHead, BYTE bFlag = 0);
	// 激活按钮
	void ActiveButton();

	// 弹出区段表的对话框,显示区段信息
	afx_msg void OnBnClickedButtonSectiontable();
	// 弹出数据目录表的对话框,显示数据目录信息
	afx_msg void OnBnClickedButtonDatadirect();
	// 退出按钮
	afx_msg void OnBnClickedButtonExit();
};
