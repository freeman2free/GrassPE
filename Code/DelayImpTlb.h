#pragma once


// CDelayImpTlb 对话框

class CDelayImpTlb : public CDialogEx
{
	DECLARE_DYNAMIC(CDelayImpTlb)

public:
	CDelayImpTlb(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDelayImpTlb();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DELAYIMP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	DWORD RVAToFOA32(DWORD dwRva, char* szBuff);
	ULONGLONG RVAToFOA64(ULONGLONG ullRva, char* szBuff);
public:
	virtual BOOL OnInitDialog();

	// 加载延迟导入表(32)
	void InitDelayImportTableInfo();
	// 加载延迟导入表(64)
	void InitDelayImportTableInfo64();

public:
	// 延迟导入表的列表控件
	CListCtrl m_lstDelayImp;
};
