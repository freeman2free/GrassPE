#pragma once


// CImportTlb 对话框

class CImportTlb : public CDialogEx
{
	DECLARE_DYNAMIC(CImportTlb)

public:
	CImportTlb(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CImportTlb();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_IMPORT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 所有导入表信息的列表控件
	CListCtrl m_lstImpInfo;
	// 每张导入表具体信息的列表控件
	CListCtrl m_lstThunkInfo;
	//
	CString m_csThunkNum;
public:
	virtual BOOL OnInitDialog();

	// 找到导入表的入口位置
	PIMAGE_IMPORT_DESCRIPTOR GetFirstImportDesc();

	// 加载所有导入表的信息
	void InitImportTableInfo();

	// 加载某张导入表的具体信息
	void InsertThunkDataInfo(LPCWSTR wszDllName);

	// RVA转换FOA函数
	DWORD RVAToFOA32(DWORD dwRva, char* szBuff);
	ULONGLONG RVAToFOA64(ULONGLONG ullRva, char* szBuff);

	// 点选导入表时,显示对应的导入表的详细信息
	afx_msg void OnClickListImportinfo(NMHDR* pNMHDR, LRESULT* pResult);
};
