#pragma once


// CTlsTlb 对话框

class CTlsTlb : public CDialogEx
{
	DECLARE_DYNAMIC(CTlsTlb)

public:
	CTlsTlb(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CTlsTlb();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_TLS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	DWORD RVAToFOA32(DWORD dwRva, char* szBuff);
	ULONGLONG RVAToFOA64(ULONGLONG ullRva, char* szBuff);
public:
	// 内存起始地址(VA)
	CString m_csStartVa;
	// 内存结束地址(VA)
	CString m_csEndVa;
	// 索引VA
	CString m_csAddIndexVa;
	// 回调函数指针数组的地址(VA)
	CString m_csCallBackVa;
	// 后面补零的个数
	CString m_csZeroFill;
	// 特征值
	CString m_csCharact;
public:
	virtual BOOL OnInitDialog();
	// 加载Tls表数据(32)
	void InitTLSTableInfo();
	// 加载Tls表数据(64)
	void InitTLSTableInfo64();
};
