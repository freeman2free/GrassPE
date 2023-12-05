// ResourceTlb.cpp: 实现文件
//

#include "pch.h"
#include "GrassPE.h"
#include "ResourceTlb.h"
#include "afxdialogex.h"
#include "GrassPEDlg.h"


 // 系统预定义的资源类型
const char* g_szPredResTypes[0x10] =
{
  "光标Cursor",
  "位图Bitmap",
  "图标Icon",
  "菜单Menu",
  "对话框Dialog",
  "字符串String",
  "字体目录FontDirectory",
  "字体Font",
  "加速键Accelerators",
  "未格式化资源Unformatted",
  "消息表MessageTable",
  "光标组GroupCursor",
  "NULL",
  "图标组GroupIcon",
  "NULL",
  "版本信息VersionInformation"
};

// CResourceTlb 对话框

IMPLEMENT_DYNAMIC(CResourceTlb, CDialogEx)

CResourceTlb::CResourceTlb(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_RESOURCE, pParent)
	, m_csNameNum(_T(""))
	, m_csIDNum(_T(""))
	, m_csNameNumSelect(_T(""))
	, m_csResRva(_T(""))
	, m_csResOffset(_T(""))
	, m_csResSize(_T(""))
{

}

CResourceTlb::~CResourceTlb()
{
  
}

void CResourceTlb::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_RES, m_treeRes);
	DDX_Text(pDX, IDC_EDIT_NUMNAME, m_csNameNum);
	DDX_Text(pDX, IDC_EDIT_NUMID, m_csIDNum);
	DDX_Text(pDX, IDC_EDIT_NUMNAMESELECT, m_csNameNumSelect);
	DDX_Text(pDX, IDC_EDIT_NUMIDSELECT, m_csIDNumSelect);
	DDX_Text(pDX, IDC_EDIT_RESRVA, m_csResRva);
	DDX_Text(pDX, IDC_EDIT_RESOFFSET, m_csResOffset);
	DDX_Text(pDX, IDC_EDIT_RESSIZE, m_csResSize);
}


BEGIN_MESSAGE_MAP(CResourceTlb, CDialogEx)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_RES, &CResourceTlb::OnTvnSelchangedTreeRes)
END_MESSAGE_MAP()


// CResourceTlb 消息处理程序


BOOL CResourceTlb::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
  m_bX64 = CGrassPEDlg::Is64PE(g_szPEBuff);
  if (m_bX64)
  {
    InitResourceTlbInfo64();
  }
  else
  {
    InitResourceTlbInfo();
  }
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
// 将资源表的信息进行加载(32)
void CResourceTlb::InitResourceTlbInfo()
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS32 pNTHeader = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的资源表的va和Size
  PIMAGE_DATA_DIRECTORY pResourceDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_RESOURCE; // 资源表的基址
  // 找到资源目录结构(根目录)
  PIMAGE_RESOURCE_DIRECTORY pRootResTable = (PIMAGE_RESOURCE_DIRECTORY)(RVAToFOA32(pResourceDir->VirtualAddress, g_szPEBuff) + (DWORD)g_szPEBuff);
  // 资源项目总数
  DWORD dwNumOfRes = pRootResTable->NumberOfNamedEntries + pRootResTable->NumberOfIdEntries;
  // 找到第一个资源项目
  PIMAGE_RESOURCE_DIRECTORY_ENTRY pFirstResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pRootResTable + 16);
  // 根目录相关信息
  UpdateData(TRUE);
  // 名称条目
  m_csNameNum.Format(L"%04X", pRootResTable->NumberOfNamedEntries);
  // ID条目
  m_csIDNum.Format(L"%04X", pRootResTable->NumberOfIdEntries);
  HTREEITEM hTreeItem;
  // 开发者自订的资源类型
  wchar_t* wszNameResDir = NULL;
  // 两种资源类型(系统预设的/自定义的)
  wchar_t wszResType1[256] = { 0 };
  wchar_t wszResType2[256] = { 0 };
  UpdateData(FALSE);
  
  for (DWORD i = 0; i < dwNumOfRes; i++)
  {
    // 第一层目录
    // 判断是否是名称条目
    if (pFirstResEntry->NameIsString == 1)
    {
      // 名称条目,表示该资源是开发者自己设置的资源,输出该字符串即可
      PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pFirstResEntry->NameOffset + (DWORD)pRootResTable);
      wszNameResDir = new wchar_t[pResName->Length + 1];
      memset(wszNameResDir, 0, sizeof(wchar_t) * (pResName->Length + 1));
      wcsncpy_s(wszNameResDir, pResName->Length + 1, pResName->NameString, pResName->Length);
      hTreeItem = m_treeRes.InsertItem(wszNameResDir, 0, 3, TVI_ROOT, TVI_LAST);
      delete[] wszNameResDir;
    }
    else // ID条目 第一层目录低32位字段作为资源名称使用
    {
      if (pFirstResEntry->Id > 0x11)
      {
        // 大于0x11的ID直接输出ID号即可
        wsprintf(wszResType1, L"%d", pFirstResEntry->Id);
        hTreeItem = m_treeRes.InsertItem(wszResType1, 0, 3, TVI_ROOT, TVI_LAST);
      }
      else
      {
        // 小于0x11的ID说明是系统预设的资源,输出相应的名称
        wsprintf(wszResType2, L"%S", g_szPredResTypes[pFirstResEntry->Id-1]);
        hTreeItem = m_treeRes.InsertItem(wszResType2, 0, 3, TVI_ROOT, TVI_LAST);
      }
    }

    // 如果有下一级目录
    if (pFirstResEntry->DataIsDirectory == 1)
    {
      // 跳转到下一级目录的位置,获取相关数据
      PIMAGE_RESOURCE_DIRECTORY pSecondResTable = (PIMAGE_RESOURCE_DIRECTORY)(pFirstResEntry->OffsetToDirectory + (DWORD)pRootResTable);
      DWORD dwSecondNumOfRes = (pSecondResTable->NumberOfNamedEntries + pSecondResTable->NumberOfIdEntries);
      PIMAGE_RESOURCE_DIRECTORY_ENTRY pSecondResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pSecondResTable + 16);

      for (DWORD j = 0; j < dwSecondNumOfRes; j++)
      {
        if (pSecondResEntry->NameIsString == 1) // 名称条目
        {
          PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pSecondResEntry->NameOffset + (DWORD)pRootResTable);
          wchar_t* wszResName = new wchar_t[pResName->Length + 1];
          memset(wszResName, 0, sizeof(wchar_t) * (pResName->Length + 1));
          wcsncpy_s(wszResName, pResName->Length + 1, pResName->NameString, pResName->Length);
          m_treeRes.InsertItem(wszResName, i, 2, hTreeItem, TVI_LAST);
          delete[] wszResName;
        }
        else // ID条目 由于是第二层目录,那么此时低32位字段就成为资源名称了所以直接输出ID即可
        {
          CString csId;
          csId.Format(L"%d", pSecondResEntry->Id);
          m_treeRes.InsertItem(csId, i, 1, hTreeItem, TVI_LAST);
        }
        // 判断是否还有下一级资源
        if (pSecondResEntry->DataIsDirectory == 1)
        {
          PIMAGE_RESOURCE_DIRECTORY pThirdResTable = (PIMAGE_RESOURCE_DIRECTORY)(pSecondResEntry->OffsetToDirectory + (DWORD)pRootResTable);
          PIMAGE_RESOURCE_DIRECTORY_ENTRY pThirdResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pThirdResTable + 16);

          if (pThirdResEntry->DataIsDirectory != 1)
          {
            PIMAGE_RESOURCE_DATA_ENTRY pResData = (PIMAGE_RESOURCE_DATA_ENTRY)(pThirdResEntry->OffsetToData + (DWORD)pRootResTable);
          }
        }
        pSecondResEntry++;
      }
    }
    pFirstResEntry++;
  }
}
// 将资源表的信息进行加载(64)
void CResourceTlb::InitResourceTlbInfo64()
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS64 pNTHeader = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER64 pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的资源表的va和Size
  PIMAGE_DATA_DIRECTORY pResourceDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_RESOURCE; // 资源表的基址
  // 找到资源目录结构(根目录)
  PIMAGE_RESOURCE_DIRECTORY pRootResTable = (PIMAGE_RESOURCE_DIRECTORY)(RVAToFOA32(pResourceDir->VirtualAddress, g_szPEBuff) + (DWORD)g_szPEBuff);
  // 资源项目总数
  DWORD dwNumOfRes = pRootResTable->NumberOfNamedEntries + pRootResTable->NumberOfIdEntries;
  // 找到第一个资源项目
  PIMAGE_RESOURCE_DIRECTORY_ENTRY pFirstResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pRootResTable + 16);
  // 根目录相关信息
  UpdateData(TRUE);
  // 名称条目
  m_csNameNum.Format(L"%04X", pRootResTable->NumberOfNamedEntries);
  // ID条目
  m_csIDNum.Format(L"%04X", pRootResTable->NumberOfIdEntries);
  HTREEITEM hTreeItem;
  // 开发者自订的资源类型
  wchar_t* wszNameResDir = NULL;
  // 两种资源类型(系统预设的/自定义的)
  wchar_t wszResType1[256] = { 0 };
  wchar_t wszResType2[256] = { 0 };
  UpdateData(FALSE);

  for (DWORD i = 0; i < dwNumOfRes; i++)
  {
    // 第一层目录
    // 判断是否是名称条目
    if (pFirstResEntry->NameIsString == 1)
    {
      // 名称条目,表示该资源是开发者自己设置的资源,输出该字符串即可
      PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pFirstResEntry->NameOffset + (DWORD)pRootResTable);
      wszNameResDir = new wchar_t[pResName->Length + 1];
      memset(wszNameResDir, 0, sizeof(wchar_t) * (pResName->Length + 1));
      wcsncpy_s(wszNameResDir, pResName->Length + 1, pResName->NameString, pResName->Length);
      hTreeItem = m_treeRes.InsertItem(wszNameResDir, 0, 3, TVI_ROOT, TVI_LAST);
      delete[] wszNameResDir;
    }
    else // ID条目 第一层目录低32位字段作为资源名称使用
    {
      if (pFirstResEntry->Id > 0x11)
      {
        // 大于0x11的ID直接输出ID号即可
        wsprintf(wszResType1, L"%d", pFirstResEntry->Id);
        hTreeItem = m_treeRes.InsertItem(wszResType1, 0, 3, TVI_ROOT, TVI_LAST);
      }
      else
      {
        // 小于0x11的ID说明是系统预设的资源,输出相应的名称
        wsprintf(wszResType2, L"%S", g_szPredResTypes[pFirstResEntry->Id - 1]);
        hTreeItem = m_treeRes.InsertItem(wszResType2, 0, 3, TVI_ROOT, TVI_LAST);
      }
    }

    // 如果有下一级目录
    if (pFirstResEntry->DataIsDirectory == 1)
    {
      // 跳转到下一级目录的位置,获取相关数据
      PIMAGE_RESOURCE_DIRECTORY pSecondResTable = (PIMAGE_RESOURCE_DIRECTORY)(pFirstResEntry->OffsetToDirectory + (DWORD)pRootResTable);
      DWORD dwSecondNumOfRes = (pSecondResTable->NumberOfNamedEntries + pSecondResTable->NumberOfIdEntries);
      PIMAGE_RESOURCE_DIRECTORY_ENTRY pSecondResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pSecondResTable + 16);

      for (DWORD j = 0; j < dwSecondNumOfRes; j++)
      {
        if (pSecondResEntry->NameIsString == 1) // 名称条目
        {
          PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pSecondResEntry->NameOffset + (DWORD)pRootResTable);
          wchar_t* wszResName = new wchar_t[pResName->Length + 1];
          memset(wszResName, 0, sizeof(wchar_t) * (pResName->Length + 1));
          wcsncpy_s(wszResName, pResName->Length + 1, pResName->NameString, pResName->Length);
          m_treeRes.InsertItem(wszResName, i, 2, hTreeItem, TVI_LAST);
          delete[] wszResName;
        }
        else // ID条目 由于是第二层目录,那么此时低32位字段就成为资源名称了所以直接输出ID即可
        {
          CString csId;
          csId.Format(L"%d", pSecondResEntry->Id);
          m_treeRes.InsertItem(csId, i, 1, hTreeItem, TVI_LAST);
        }
        // 判断是否还有下一级资源
        if (pSecondResEntry->DataIsDirectory == 1)
        {
          PIMAGE_RESOURCE_DIRECTORY pThirdResTable = (PIMAGE_RESOURCE_DIRECTORY)(pSecondResEntry->OffsetToDirectory + (DWORD)pRootResTable);
          PIMAGE_RESOURCE_DIRECTORY_ENTRY pThirdResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pThirdResTable + 16);

          if (pThirdResEntry->DataIsDirectory != 1)
          {
            PIMAGE_RESOURCE_DATA_ENTRY pResData = (PIMAGE_RESOURCE_DATA_ENTRY)(pThirdResEntry->OffsetToData + (DWORD)pRootResTable);
          }
        }
        pSecondResEntry++;
      }
    }
    pFirstResEntry++;
  }
}

// 将选中的资源表的信息进行加载(32)
void CResourceTlb::InitSelectResourceTlbInfo(CString csNode)
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS32 pNTHeader = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的资源表的va和Size
  PIMAGE_DATA_DIRECTORY pResourceDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_RESOURCE; // 资源表的基址
  // 找到资源目录结构(根目录)
  PIMAGE_RESOURCE_DIRECTORY pRootResTable = (PIMAGE_RESOURCE_DIRECTORY)(RVAToFOA32(pResourceDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 资源项目总数
  DWORD dwNumOfRes = pRootResTable->NumberOfNamedEntries + pRootResTable->NumberOfIdEntries;
  // 找到第一个资源项目
  PIMAGE_RESOURCE_DIRECTORY_ENTRY pFirstResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pRootResTable + 16);
  // 根目录相关信息
  UpdateData(TRUE);
  // 开发者自订的资源类型
  wchar_t* wszNameResDir = NULL;
  // 两种资源类型(系统预设的/自定义的)
  wchar_t wszResType1[256] = { 0 }; // 记录ID
  wchar_t wszResType2[256] = { 0 }; // 记录系统预设的ID
  UpdateData(FALSE);

  for (DWORD i = 0; i < dwNumOfRes; i++)
  {
    // 第一层目录
    // 判断是否是名称条目
    if (pFirstResEntry->NameIsString == 1)
    {
      // 名称条目,表示该资源是开发者自己设置的资源,输出该字符串即可
      PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pFirstResEntry->NameOffset + (DWORD)pRootResTable);
      wszNameResDir = new wchar_t[pResName->Length + 1];
      memset(wszNameResDir, 0, sizeof(wchar_t) * (pResName->Length + 1));
      wcsncpy_s(wszNameResDir, pResName->Length + 1, pResName->NameString, pResName->Length);
      //delete[] wszNameResDir;
    }
    else // ID条目 第一层目录低32位字段作为资源名称使用
    {
      if (pFirstResEntry->Id > 0x11)
      {
        // 大于0x11的ID直接输出ID号即可
        wsprintf(wszResType1, L"%d", pFirstResEntry->Id);
      }
      else
      {
        // 小于0x11的ID说明是系统预设的资源,输出相应的名称
        wsprintf(wszResType2, L"%S", g_szPredResTypes[pFirstResEntry->Id - 1]);
      }
    }

    // 如果有下一级目录
    if (pFirstResEntry->DataIsDirectory == 1)
    {
      // 跳转到下一级目录的位置,获取相关数据
      PIMAGE_RESOURCE_DIRECTORY pSecondResTable = (PIMAGE_RESOURCE_DIRECTORY)(pFirstResEntry->OffsetToDirectory + (DWORD)pRootResTable);
      DWORD dwSecondNumOfRes = (pSecondResTable->NumberOfNamedEntries + pSecondResTable->NumberOfIdEntries);
      PIMAGE_RESOURCE_DIRECTORY_ENTRY pSecondResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pSecondResTable + 16);

      // 需要判断一下是否是所选中的资源目录,再决定是否更新
      BOOL bRes = (wszResType2 == csNode ||
        wszResType1 == csNode ||
        (wszNameResDir != NULL && wszNameResDir == csNode));

      if (bRes)
      {
        // 选中的目录
        UpdateData(TRUE);
        // 名称条目数量
        m_csNameNumSelect.Format(L"%04X", pSecondResTable->NumberOfNamedEntries);
        // ID条目数量
        m_csIDNumSelect.Format(L"%04X", pSecondResTable->NumberOfIdEntries);
        UpdateData(FALSE);
        break;
      }
    }
    pFirstResEntry++;
  }
  if (wszNameResDir != NULL)
  {
    delete[] wszNameResDir;
  }
}
// 将选中的资源表的信息进行加载(64)
void CResourceTlb::InitSelectResourceTlbInfo64(CString csNode)
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS64 pNTHeader = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER64 pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的资源表的va和Size
  PIMAGE_DATA_DIRECTORY pResourceDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_RESOURCE; // 资源表的基址
  // 找到资源目录结构(根目录)
  PIMAGE_RESOURCE_DIRECTORY pRootResTable = (PIMAGE_RESOURCE_DIRECTORY)(RVAToFOA32(pResourceDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 资源项目总数
  DWORD dwNumOfRes = pRootResTable->NumberOfNamedEntries + pRootResTable->NumberOfIdEntries;
  // 找到第一个资源项目
  PIMAGE_RESOURCE_DIRECTORY_ENTRY pFirstResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pRootResTable + 16);
  // 根目录相关信息
  UpdateData(TRUE);
  // 开发者自订的资源类型
  wchar_t* wszNameResDir = NULL;
  // 两种资源类型(系统预设的/自定义的)
  wchar_t wszResType1[256] = { 0 }; // 记录ID
  wchar_t wszResType2[256] = { 0 }; // 记录系统预设的ID
  UpdateData(FALSE);

  for (DWORD i = 0; i < dwNumOfRes; i++)
  {
    // 第一层目录
    // 判断是否是名称条目
    if (pFirstResEntry->NameIsString == 1)
    {
      // 名称条目,表示该资源是开发者自己设置的资源,输出该字符串即可
      PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pFirstResEntry->NameOffset + (DWORD)pRootResTable);
      wszNameResDir = new wchar_t[pResName->Length + 1];
      memset(wszNameResDir, 0, sizeof(wchar_t) * (pResName->Length + 1));
      wcsncpy_s(wszNameResDir, pResName->Length + 1, pResName->NameString, pResName->Length);
      //delete[] wszNameResDir;
    }
    else // ID条目 第一层目录低32位字段作为资源名称使用
    {
      if (pFirstResEntry->Id > 0x11)
      {
        // 大于0x11的ID直接输出ID号即可
        wsprintf(wszResType1, L"%d", pFirstResEntry->Id);
      }
      else
      {
        // 小于0x11的ID说明是系统预设的资源,输出相应的名称
        wsprintf(wszResType2, L"%S", g_szPredResTypes[pFirstResEntry->Id - 1]);
      }
    }

    // 如果有下一级目录
    if (pFirstResEntry->DataIsDirectory == 1)
    {
      // 跳转到下一级目录的位置,获取相关数据
      PIMAGE_RESOURCE_DIRECTORY pSecondResTable = (PIMAGE_RESOURCE_DIRECTORY)(pFirstResEntry->OffsetToDirectory + (DWORD)pRootResTable);
      DWORD dwSecondNumOfRes = (pSecondResTable->NumberOfNamedEntries + pSecondResTable->NumberOfIdEntries);
      PIMAGE_RESOURCE_DIRECTORY_ENTRY pSecondResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pSecondResTable + 16);

      // 需要判断一下是否是所选中的资源目录,再决定是否更新
      BOOL bRes = (wszResType2 == csNode ||
        wszResType1 == csNode ||
        (wszNameResDir != NULL && wszNameResDir == csNode));

      if (bRes)
      {
        // 选中的目录
        UpdateData(TRUE);
        // 名称条目数量
        m_csNameNumSelect.Format(L"%04X", pSecondResTable->NumberOfNamedEntries);
        // ID条目数量
        m_csIDNumSelect.Format(L"%04X", pSecondResTable->NumberOfIdEntries);
        UpdateData(FALSE);
        break;
      }
    }
    pFirstResEntry++;
  }
  if (wszNameResDir != NULL)
  {
    delete[] wszNameResDir;
  }
}

// 将选中的资源表的信息进行加载(32)
void CResourceTlb::InitResourceDataInfo(CString csNode, CString csParentNode)
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS32 pNTHeader = (PIMAGE_NT_HEADERS32)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的资源表的va和Size
  PIMAGE_DATA_DIRECTORY pResourceDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_RESOURCE; // 资源表的基址
  // 找到资源目录结构(根目录)
  PIMAGE_RESOURCE_DIRECTORY pRootResTable = (PIMAGE_RESOURCE_DIRECTORY)(RVAToFOA32(pResourceDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 资源项目总数
  DWORD dwNumOfRes = pRootResTable->NumberOfNamedEntries + pRootResTable->NumberOfIdEntries;
  // 找到第一个资源项目
  PIMAGE_RESOURCE_DIRECTORY_ENTRY pFirstResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pRootResTable + 16);
  // 根目录相关信息
  UpdateData(TRUE);
  // 开发者自订的资源类型
  wchar_t* wszNameResDir = NULL;
  // 两种资源类型(系统预设的/自定义的)
  wchar_t wszResType1[256] = { 0 }; // 记录ID(有可能是)
  wchar_t wszResType2[256] = { 0 }; // 记录系统预设的ID
  UpdateData(FALSE);

  for (DWORD i = 0; i < dwNumOfRes; i++)
  {
    // 第一层目录
    // 判断是否是名称条目
    if (pFirstResEntry->NameIsString == 1)
    {
      // 名称条目,表示该资源是开发者自己设置的资源,输出该字符串即可
      PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pFirstResEntry->NameOffset + (DWORD)pRootResTable);
      wszNameResDir = new wchar_t[pResName->Length + 1];
      memset(wszNameResDir, 0, sizeof(wchar_t) * (pResName->Length + 1));
      wcsncpy_s(wszNameResDir, pResName->Length + 1, pResName->NameString, pResName->Length);
      //delete[] wszNameResDir;
    }
    else // ID条目 第一层目录低32位字段作为资源名称使用
    {
      if (pFirstResEntry->Id > 0x11)
      {
        // 大于0x11的ID直接输出ID号即可
        wsprintf(wszResType1, L"%d", pFirstResEntry->Id);
      }
      else
      {
        // 小于0x11的ID说明是系统预设的资源,输出相应的名称
        wsprintf(wszResType2, L"%S", g_szPredResTypes[pFirstResEntry->Id - 1]);
      }
    }

    // 如果有下一级目录
    if (pFirstResEntry->DataIsDirectory == 1)
    {
      // 跳转到下一级目录的位置,获取相关数据
      PIMAGE_RESOURCE_DIRECTORY pSecondResTable = (PIMAGE_RESOURCE_DIRECTORY)(pFirstResEntry->OffsetToDirectory + (DWORD)pRootResTable);
      DWORD dwSecondNumOfRes = (pSecondResTable->NumberOfNamedEntries + pSecondResTable->NumberOfIdEntries);
      PIMAGE_RESOURCE_DIRECTORY_ENTRY pSecondResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pSecondResTable + 16);

      // 需要进行一些判断:如果
      BOOL bRes = (
          (wszResType1 == csNode) || (wszResType1 == csParentNode) || (wszResType2 == csNode) || (wszResType2 == csParentNode) || ((wszNameResDir != NULL) && (wszNameResDir == csNode || wszNameResDir == csParentNode))
        );

      // 以下情况才开始遍历
      if (bRes)
      {
        // 更新一下该具体资源的资源目录相关信息
        // 选中的目录
        UpdateData(TRUE);
        // 名称条目数量
        m_csNameNumSelect.Format(L"%04X", pSecondResTable->NumberOfNamedEntries);
        // ID条目数量
        m_csIDNumSelect.Format(L"%04X", pSecondResTable->NumberOfIdEntries);
        UpdateData(FALSE);

        for (DWORD j = 0; j < dwSecondNumOfRes; j++)
        {
          if (pSecondResEntry->NameIsString == 1) // 名称条目
          {
            PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pSecondResEntry->NameOffset + (DWORD)pRootResTable);
            wszNameResDir = new wchar_t[pResName->Length + 1];
            memset(wszNameResDir, 0, sizeof(wchar_t) * (pResName->Length + 1));
            wcsncpy_s(wszNameResDir, pResName->Length + 1, pResName->NameString, pResName->Length);
            //delete[] wszNameResDir;
          }
          else // ID条目 由于是第二层目录,那么此时低32位字段就成为资源名称了所以直接输出ID即可
          {
            wsprintf(wszResType1, L"%d", pSecondResEntry->Id);
          }

          if (wszResType1 == csNode || (wszNameResDir != NULL && wszNameResDir == csNode))
          {
            // 判断是否还有下一级资源
            if (pSecondResEntry->DataIsDirectory == 1)
            {
              PIMAGE_RESOURCE_DIRECTORY pThirdResTable = (PIMAGE_RESOURCE_DIRECTORY)(pSecondResEntry->OffsetToDirectory + (DWORD)pRootResTable);
              PIMAGE_RESOURCE_DIRECTORY_ENTRY pThirdResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pThirdResTable + 16);

              if (pThirdResEntry->DataIsDirectory != 1)
              {
                PIMAGE_RESOURCE_DATA_ENTRY pResData = (PIMAGE_RESOURCE_DATA_ENTRY)(pThirdResEntry->OffsetToData + (DWORD)pRootResTable);
                UpdateData(TRUE);
                // 资源数据的RVA
                m_csResRva.Format(L"%08X", pResData->OffsetToData);
                // 资源数据的文件偏移
                m_csResOffset.Format(L"%08X", RVAToFOA32(pResData->OffsetToData, g_szPEBuff));
                // 资源数据的大小
                m_csResSize.Format(L"%08X", pResData->Size);
                UpdateData(FALSE);
              }
            }
            goto END_LOOP;
          }
          pSecondResEntry++;
        }
      }
    }
    pFirstResEntry++;
  }
END_LOOP:
  if (wszNameResDir != NULL)
  {
    delete[] wszNameResDir;
  }
  return;
}
// 将选中的资源表的信息进行加载(64)
void CResourceTlb::InitResourceDataInfo64(CString csNode, CString csParentNode)
{
  // DOS头
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_szPEBuff;
  // NT头
  PIMAGE_NT_HEADERS64 pNTHeader = (PIMAGE_NT_HEADERS64)(g_szPEBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 扩展头
  PIMAGE_OPTIONAL_HEADER64 pOptHeader = &(pNTHeader->OptionalHeader);
  // 定位数据目录表中的资源表的va和Size
  PIMAGE_DATA_DIRECTORY pResourceDir = pOptHeader->DataDirectory + IMAGE_DIRECTORY_ENTRY_RESOURCE; // 资源表的基址
  // 找到资源目录结构(根目录)
  PIMAGE_RESOURCE_DIRECTORY pRootResTable = (PIMAGE_RESOURCE_DIRECTORY)(RVAToFOA32(pResourceDir->VirtualAddress, g_szPEBuff) + g_szPEBuff);
  // 资源项目总数
  DWORD dwNumOfRes = pRootResTable->NumberOfNamedEntries + pRootResTable->NumberOfIdEntries;
  // 找到第一个资源项目
  PIMAGE_RESOURCE_DIRECTORY_ENTRY pFirstResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pRootResTable + 16);
  // 根目录相关信息
  UpdateData(TRUE);
  // 开发者自订的资源类型
  wchar_t* wszNameResDir = NULL;
  // 两种资源类型(系统预设的/自定义的)
  wchar_t wszResType1[256] = { 0 }; // 记录ID
  wchar_t wszResType2[256] = { 0 }; // 记录系统预设的ID
  UpdateData(FALSE);

  for (DWORD i = 0; i < dwNumOfRes; i++)
  {
    // 第一层目录
    // 判断是否是名称条目
    if (pFirstResEntry->NameIsString == 1)
    {
      // 名称条目,表示该资源是开发者自己设置的资源,输出该字符串即可
      PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pFirstResEntry->NameOffset + (DWORD)pRootResTable);
      wszNameResDir = new wchar_t[pResName->Length + 1];
      memset(wszNameResDir, 0, sizeof(wchar_t) * (pResName->Length + 1));
      wcsncpy_s(wszNameResDir, pResName->Length + 1, pResName->NameString, pResName->Length);
      //delete[] wszNameResDir;
    }
    else // ID条目 第一层目录低32位字段作为资源名称使用
    {
      if (pFirstResEntry->Id > 0x11)
      {
        // 大于0x11的ID直接输出ID号即可
        wsprintf(wszResType1, L"%d", pFirstResEntry->Id);
      }
      else
      {
        // 小于0x11的ID说明是系统预设的资源,输出相应的名称
        wsprintf(wszResType2, L"%S", g_szPredResTypes[pFirstResEntry->Id - 1]);
      }
    }

    // 如果有下一级目录
    if (pFirstResEntry->DataIsDirectory == 1)
    {
      PIMAGE_RESOURCE_DIRECTORY pSecondResTable = (PIMAGE_RESOURCE_DIRECTORY)(pFirstResEntry->OffsetToDirectory + (DWORD)pRootResTable);
      DWORD dwSecondNumOfRes = (pSecondResTable->NumberOfNamedEntries + pSecondResTable->NumberOfIdEntries);
      PIMAGE_RESOURCE_DIRECTORY_ENTRY pSecondResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pSecondResTable + 16);

      // 需要进行一些判断
      BOOL bRes = (
        (wszResType1 == csNode) || (wszResType1 == csParentNode) || (wszResType2 == csNode) || (wszResType2 == csParentNode) || ((wszNameResDir != NULL) && (wszNameResDir == csNode || wszNameResDir == csParentNode))
        );

      // 以下情况才开始遍历
      if (bRes)
      {
        // 更新一下该具体资源的资源目录相关信息
        // 选中的目录
        UpdateData(TRUE);
        // 名称条目数量
        m_csNameNumSelect.Format(L"%04X", pSecondResTable->NumberOfNamedEntries);
        // ID条目数量
        m_csIDNumSelect.Format(L"%04X", pSecondResTable->NumberOfIdEntries);
        UpdateData(FALSE);

        for (DWORD j = 0; j < dwSecondNumOfRes; j++)
        {
          if (pSecondResEntry->NameIsString == 1) // 名称条目
          {
            PIMAGE_RESOURCE_DIR_STRING_U pResName = (PIMAGE_RESOURCE_DIR_STRING_U)(pSecondResEntry->NameOffset + (DWORD)pRootResTable);
            wszNameResDir = new wchar_t[pResName->Length + 1];
            memset(wszNameResDir, 0, sizeof(wchar_t) * (pResName->Length + 1));
            wcsncpy_s(wszNameResDir, pResName->Length + 1, pResName->NameString, pResName->Length);
            //delete[] wszNameResDir;
          }
          else // ID条目 由于是第二层目录,那么此时低32位字段就成为资源名称了所以直接输出ID即可
          {
            wsprintf(wszResType1, L"%d", pSecondResEntry->Id);
          }

          if (wszResType1 == csNode || (wszNameResDir != NULL && wszNameResDir == csNode))
          {
            // 判断是否还有下一级资源
            if (pSecondResEntry->DataIsDirectory == 1)
            {
              PIMAGE_RESOURCE_DIRECTORY pThirdResTable = (PIMAGE_RESOURCE_DIRECTORY)(pSecondResEntry->OffsetToDirectory + (DWORD)pRootResTable);
              PIMAGE_RESOURCE_DIRECTORY_ENTRY pThirdResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pThirdResTable + 16);

              if (pThirdResEntry->DataIsDirectory != 1)
              {
                PIMAGE_RESOURCE_DATA_ENTRY pResData = (PIMAGE_RESOURCE_DATA_ENTRY)(pThirdResEntry->OffsetToData + (DWORD)pRootResTable);
                UpdateData(TRUE);
                // 资源数据的RVA
                m_csResRva.Format(L"%08X", pResData->OffsetToData);
                // 资源数据的文件偏移
                m_csResOffset.Format(L"%08X", RVAToFOA32(pResData->OffsetToData, g_szPEBuff));
                // 资源数据的大小
                m_csResSize.Format(L"%08X", pResData->Size);
                UpdateData(FALSE);
              }
            }
            goto END_LOOP;
          }
          pSecondResEntry++;
        }
      }
    }
    pFirstResEntry++;
  }
END_LOOP:
  if (wszNameResDir != NULL)
  {
    delete[] wszNameResDir;
  }
  return;
}

// 当点选树控件(树控件出现变动时)时进行显示
void CResourceTlb::OnTvnSelchangedTreeRes(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	// 获取选中结点上的文本
	CString csCurrentNode = m_treeRes.GetItemText(pNMTreeView->itemNew.hItem);
	// 获取选中结点的父结点
	HTREEITEM hCurrentNode = m_treeRes.GetParentItem(pNMTreeView->itemNew.hItem);
	// 获取选中父结点上的文本
	CString csParentNode = m_treeRes.GetItemText(hCurrentNode);

  if (csParentNode.IsEmpty() && !(csCurrentNode.IsEmpty()))
  {
    // 当点选资源目录时,具体资源的数据就不需要显示了
    UpdateData(TRUE);
    m_csResOffset = L"";
    m_csResRva = L"";
    m_csResSize = L"";
    UpdateData(FALSE);
    // 将选取到的结点进行加载(选中的资源目录,根据是32/64位来判断)
    if (m_bX64)
    {
      InitSelectResourceTlbInfo64(csCurrentNode);
    }
    else
    {
      InitSelectResourceTlbInfo(csCurrentNode);
    }
  }
  else if(!(csParentNode.IsEmpty()) && !(csCurrentNode.IsEmpty()))
  {

    // 将选取到的结点进行加载(具体的资源数据,根据是32/64位来判断)
    if (m_bX64)
    {
      InitResourceDataInfo64(csCurrentNode, csParentNode);
    }
    else
    {
      InitResourceDataInfo(csCurrentNode, csParentNode);
    }
  }


	*pResult = 0;
}

DWORD CResourceTlb::RVAToFOA32(DWORD dwRva, char* szBuff)
{
  DWORD dwFOA = 0;
  // 获取DosHeader
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)szBuff;
  // 获取NTHeader
  PIMAGE_NT_HEADERS32 pNTHeader = (PIMAGE_NT_HEADERS32)(szBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 得到区段表
  PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeader);
  // 判断RVA属于哪个区段
  // 首先判断是否落在头部区段
  if (dwRva < pSectionHeader[0].VirtualAddress)
  {
    // 如果小于第1个区段的虚拟地址直接返回(是否落在头部)
    // 因为头在文件中和内存中是一样展开的
    return dwRva;
  }
  // 如果dwRva正好是某区段的起始RVA
  for (int i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    if (dwRva == pSectionHeader[i].VirtualAddress)
    {
      DWORD k = (pSectionHeader[i].VirtualAddress - pSectionHeader[i].PointerToRawData);
      dwFOA = dwRva - k;
      return dwFOA;
    }
  }

  // 遍历所有区段.同一区段中,内存中的偏移和文件中的偏移都是相等的
  for (int i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    // 判断是否在这个区段中
    // 区段起始地址(在内存中的偏移RVA) = virtualAddress
    // 区段end地址 = virtuallAddress + size
    // RVA >= 区段.VirtualAddress && RVA < 区段.VirtualAddress + 区段.VirtualSize
    if (dwRva >= pSectionHeader[i].VirtualAddress && dwRva <= pSectionHeader[i].VirtualAddress + pSectionHeader[i].Misc.VirtualSize)
    {
      // 如果属于这个区段,那么计算出该位置在文件中的偏移(FOA)
      /* 第一种方法
      * dwRva - VirtualAddress = 当前地址到所在区段首部的偏移(差值)
      * PointerToRawData 当前区段距离文件头的长度
      * PointerToRawData + (dwRva - VirtualAddress) = 指定内存地址在文件中的真实地址(也就是FOA)
      * dwFOA = (dwRva - pSectionHeader[i].VirtualAddress) + pSectionHeader[i].PointerToRawData;
      */

      /*第二种方法
      * 差值 = VirtualAddress(区段在内存中的偏移起始处RVA) - PointerToRawData(区段在文件中的偏移起始处FOA)
      * FOA = RVA - 差值
      */
      DWORD k = (pSectionHeader[i].VirtualAddress - pSectionHeader[i].PointerToRawData);
      dwFOA = dwRva - k;
      break;
    }
  }
  return dwFOA; // 返回该FOA偏移
}

ULONGLONG CResourceTlb::RVAToFOA64(ULONGLONG ullRva, char* szBuff)
{
  ULONGLONG ullFOA = 0;
  // 获取DosHeader
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)szBuff;
  // 获取NTHeader
  PIMAGE_NT_HEADERS64 pNTHeader = (PIMAGE_NT_HEADERS64)(szBuff + pDosHeader->e_lfanew); // 根据Dos头中的偏移信息跳转到NT头开始处
  // 得到区段表
  PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeader);
  // 判断RVA属于哪个区段
  // 首先判断是否落在头部区段
  if (ullRva < pSectionHeader[0].VirtualAddress)
  {
    // 如果小于第1个区段的虚拟地址直接返回(位于PE文件头部)
    // 因为头在文件中和内存中是一样展开的
    return ullRva;
  }
  // 如果dwRva正好是某区段的起始RVA
  for (size_t i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    if (ullRva == pSectionHeader[i].VirtualAddress)
    {
      DWORD k = (pSectionHeader[i].VirtualAddress - pSectionHeader[i].PointerToRawData);
      ullFOA = ullRva - k;
      return ullFOA;
    }
  }

  for (size_t i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++)
  {
    // 判断是否在这个区段中
    // 区段start地址 = virtualAddress
    // 区段end地址   = virtuallAddress+size
    // RVA >= 区段.VirtualAddress && RVA <= 区段.VirtualAddress + 区段.VirtualSize
    if (ullRva >= pSectionHeader[i].VirtualAddress && ullRva < ((ULONGLONG)pSectionHeader[i].VirtualAddress + (ULONGLONG)pSectionHeader[i].Misc.VirtualSize))
    {
      // 如果属于这个区段,那么计算出该位置在文件中的偏移(FOA)
      // dwRva - VirtualAddress = 当前地址到所在区段首部的偏移(差值)
      // PointerToRawData 当前区段距离文件头的长度
      // PointerToRawData + (dwRva - VirtualAddress) = 指定内存地址在文件中的真实地址(也就是FOA)
      ullFOA = (ullRva - pSectionHeader[i].VirtualAddress) + pSectionHeader[i].PointerToRawData;
      break;
    }
  }
  return ullFOA; // 返回该FOA偏移
}