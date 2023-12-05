#pragma once


// CDataDirectory 对话框

class CDataDirectory : public CDialogEx
{
	DECLARE_DYNAMIC(CDataDirectory)

public:
	CDataDirectory(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDataDirectory();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DATADIR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	// 设计一个结构体该结构体的两个成员对应的是两种控件
	typedef struct
	{
		UINT ID_RVA;
		UINT ID_SIZE;
	}DataDirToEditID;

	// 将每一个Edit控件的ID一一对应上放入一个数组中,数组类型为一个结构体
	DataDirToEditID m_EditIDArr[IMAGE_NUMBEROF_DIRECTORY_ENTRIES] =
	{
		{IDC_EDIT_RVAEXPORT, IDC_EDIT_SIZEEXPORT}, // 导出表
		{IDC_EDIT_RVAIMPORT, IDC_EDIT_SIZEIMPORT}, // 导入表
		{IDC_EDIT_RVARES, IDC_EDIT_SIZERES},       // 资源表
		{IDC_EDIT_RVAEXCEP, IDC_EDIT_SIZEEXCEP},
		{IDC_EDIT_RVASECUR, IDC_EDIT_SIZESECUR},
		{IDC_EDIT_RVARELOC, IDC_EDIT_SIZERELOC},   // 重定位表
		{IDC_EDIT_RVADBG, IDC_EDIT_SIZEDBG},
		{IDC_EDIT_RVACPR, IDC_EDIT_SIZECPR},
		{IDC_EDIT_RVAGLOBALPTR, IDC_EDIT_SIZEGLOBALPTR},
		{IDC_EDIT_RVATLS, IDC_EDIT_SIZETLS},
		{IDC_EDIT_RVACONFIG, IDC_EDIT_SIZECONFIG},
		{IDC_EDIT_RVABOUNDIMP, IDC_EDIT_SIZEBOUNDIMP},
		{IDC_EDIT_RVAIAT, IDC_EDIT_SIZEIAT},
		{IDC_EDIT_RVADELAYIMP, IDC_EDIT_SIZEDELAYIMP},  // 延迟导入表
		{IDC_EDIT_RVACOM, IDC_EDIT_SIZECOM},
		{IDC_EDIT_RVANOUSE, IDC_EDIT_SIZENOUSE}
	};
public:
	virtual BOOL OnInitDialog();
	// 加载数据目录表信息(32)
	void InitDataDirectorInfo();
	// 加载数据目录表信息(64)
	void InitDataDirectorInfo64();
	// 弹出导出表信息的对话框
	afx_msg void OnBnClickedButtonExport();
	// 弹出导入表信息的对话框
	afx_msg void OnBnClickedButtonImport();
	// 弹出资源表信息的对话框
	afx_msg void OnBnClickedButtonRes();
	// 弹出重定位表信息的对话框
	afx_msg void OnBnClickedButtonReloc();
	// 弹出TLS表信息的对话框
	afx_msg void OnBnClickedButtonTls();
	// 弹出延迟导入表信息的对话框
	afx_msg void OnBnClickedButtonDelayimp();
};
