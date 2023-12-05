#pragma once


// CResourceTlb 对话框

class CResourceTlb : public CDialogEx
{
	DECLARE_DYNAMIC(CResourceTlb)

public:
	CResourceTlb(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CResourceTlb();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_RESOURCE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	DWORD RVAToFOA32(DWORD dwRva, char* szBuff);
	ULONGLONG RVAToFOA64(ULONGLONG ullRva, char* szBuff);
public:
	// 资源目录的树控件
	CTreeCtrl m_treeRes;
	// 总的带有名称的目录个数
	CString m_csNameNum;
	// 总的ID目录的个数
	CString m_csIDNum;
	// 选中的带有名称的资源目录的个数
	CString m_csNameNumSelect;
	// 选中的带有ID的资源目录的个数
	CString m_csIDNumSelect;
	// 资源的RVA
	CString m_csResRva;
	// 资源在文件中的偏移(FOA)
	CString m_csResOffset;
	// 资源的大小
	CString m_csResSize;
	// 用于判断PE是否是64位
	BOOL m_bX64;
public:
	virtual BOOL OnInitDialog();
	// 将资源表的信息进行加载(32)
	void InitResourceTlbInfo();
	// 将资源表的信息进行加载(64)
	void InitResourceTlbInfo64();

	// 将选中的资源表的信息进行加载(32)
	void InitSelectResourceTlbInfo(CString csNode);
	// 将选中的资源表的信息进行加载(64)
	void InitSelectResourceTlbInfo64(CString csNode);

	// 加载具体的资源数据相关信息(32)
	void InitResourceDataInfo(CString csNode, CString csParentNode);
	// 加载具体的资源数据相关信息(64)
	void InitResourceDataInfo64(CString csNode, CString csParentNode);
	// 当点选树控件时进行显示
	afx_msg void OnTvnSelchangedTreeRes(NMHDR* pNMHDR, LRESULT* pResult);
};
