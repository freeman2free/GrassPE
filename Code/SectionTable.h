#pragma once


// CSectionTable 对话框

class CSectionTable : public CDialogEx
{
	DECLARE_DYNAMIC(CSectionTable)

public:
	CSectionTable(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSectionTable();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SECTIONTLB };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	// 区段表信息的列表控件
	CListCtrl m_lstSectionTlb;
public:
	virtual BOOL OnInitDialog();
	// 将区段表信息设置到列表控件上(32)
	void InitSectionTableInfo();
	// 将区段表信息设置到列表控件上(64)
	void InitSectionTableInfo64();
};
