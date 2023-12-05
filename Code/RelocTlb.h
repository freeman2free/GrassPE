#pragma once


// CRelocTlb 对话框

class CRelocTlb : public CDialogEx
{
	DECLARE_DYNAMIC(CRelocTlb)

public:
	CRelocTlb(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CRelocTlb();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_RELOC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	DWORD RVAToFOA32(DWORD dwRva, char* szBuff);
	ULONGLONG RVAToFOA64(ULONGLONG ullRva, char* szBuff);
public:
	// 需要重定位的区段的信息(列表控件)
	CListCtrl m_lstSectionInfo;
	// 需要重定位区段的具体信息(列表控件)
	CListCtrl m_lstBlockInfo;
public:
	virtual BOOL OnInitDialog();
	// 点选指定重定位表后显示相应的信息
	afx_msg void OnNMClickListSectioninfo(NMHDR* pNMHDR, LRESULT* pResult);
	// 加载所有需要重定位的区段信息(32)
	void InitRelocTableInfo();
	// 加载所有需要重定位的区段信息(64)
	void InitRelocTableInfo64();
	// 加载选中的区段的重定位信息(32)
	void InitSelectRelocInfo(CString csRVA);
	// 加载选中的区段的重定位信息(64)
	void InitSelectRelocInfo64(CString csRVA);
	// 获取区段名与序号
	char* GetSectionName(DWORD dwRva, DWORD* pIndex);
};
