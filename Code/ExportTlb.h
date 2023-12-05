#pragma once


// CExportTlb 对话框

class CExportTlb : public CDialogEx
{
	DECLARE_DYNAMIC(CExportTlb)

public:
	CExportTlb(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CExportTlb();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_EXPORT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	DWORD RVAToFOA32(DWORD dwRva, char* szBuff);
	ULONGLONG RVAToFOA64(ULONGLONG ullRva, char* szBuff);
public:
	// 导出表具体信息的列表控件
	CListCtrl m_lstExportInfo;
	// 导出表偏移
	CString m_csExportOffset;
	// 特征值
	CString m_csCharact;
	// 基数
	CString m_csBasic;
	// 名称(Dll文件的名称字符串RVA)
	CString m_csNameRva;
	// 导入表的名称字符串
	CString m_csName;
	// 导出函数数量
	CString m_csCntFunc;
	// 带名字的导出函数数量
	CString m_csCntFuncName;
	// 函数地址(EAT表起始RVA)
	CString m_csEATRva;
	// 函数名称表地址(ENT表起始RVA)
	CString m_csENTRva;
	// 函数名序号表地址(EOT表起始RVA)
	CString m_csEOTRva;
public:
	virtual BOOL OnInitDialog();
	// 加载导出表信息(32)
	void InitExportTableInfo();
	// 加载导出表信息(64)
	void InitExportTableInfo64();
};
