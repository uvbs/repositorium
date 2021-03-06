// CJListView.cpp : implementation file
// Copyright � 1998-1999 CodeJock.com, All Rights Reserved.
// See ReadMe.txt for TERMS OF USE.
//
// Based on the CListView articles from http://www.codeguru.com/listview
//
/////////////////////////////////////////////////////////////////////////////
/****************************************************************************
 *
 * $Date: 15/12/02 18:02 $
 * $Revision: 5 $
 * $Archive: /Projects/Libraries/CJLibrary/CJLibrary/CJListView.cpp $
 *
 * $History: CJListView.cpp $
 * 
 * *****************  Version 5  *****************
 * User: Anna         Date: 15/12/02   Time: 18:02
 * Updated in $/Projects/Libraries/CJLibrary/CJLibrary
 * Added support for native (ComCtl v6) sort images in header controls
 * 
 * 
 * *****************  Version 4  *****************
 * User: Andy         Date: 13/06/02   Time: 17:01
 * Updated in $/Projects/AddIns/ResOrg/CJLibrary/CJLibrary
 * Modified CCJListCtrl/CCJListViewl::AutoSizeColumn() to better handle
 * columns with sort images - still not perfect, but definately an
 * improvement
 * 
 * *****************  Version 3  *****************
 * User: Andy         Date: 2/26/02    Time: 3:13p
 * Updated in $/Projects/Libraries/CJLibrary/CJLibrary
 * 1.  CCJListView::m_clrText and m_clrTextBk are now initialised to
 * CLR_DEFAULT instead of COLOR_WINDOWTEXT and COLOR_WINDOW respectively.
 * This allows the control to paint correctly if the system colour scheme
 * changes and the text/background colours haven't been explicitly set
 * 2.  CCJListView:SetRowColors() now takes default parameters of value
 * CLR_DEFAULT to allow the text/background colours to be reset to system
 * defaults
 * 3.  Corrected CCJListView::OnCustomDraw() to fix the "setting the
 * colour of every other row" bug fixed in CCJListCtrl some time ago
 * 
 * *****************  Version 2  *****************
 * User: Andy         Date: 5/03/01    Time: 15:50
 * Updated in $/Projects/AddIns/ResOrg/CJLibrary/CJLibrary
 * Bug fixes
 * 
 * *****************  Version 9  *****************
 * User: Kirk Stowell Date: 10/30/99   Time: 2:04p
 * Updated in $/CodeJock/CJLibrary
 * Overrode OnEraseBkgnd(...) for flicker free drawing.
 * 
 * *****************  Version 8  *****************
 * User: Kirk Stowell Date: 10/25/99   Time: 10:52p
 * Updated in $/CodeJock/CJLibrary
 * Modified resource include for static builds.
 * 
 * *****************  Version 7  *****************
 * User: Kirk Stowell Date: 8/29/99    Time: 9:14p
 * Updated in $/CodeJockey/CJLibrary
 * Added Unicode compliance, thanks to Barry Burton for his help with
 * this.
 * 
 * 
 * *****************  Version 6  *****************
 * User: Kirk Stowell Date: 7/25/99    Time: 12:30a
 * Updated in $/CodeJockey/CJLibrary
 * 
 * *****************  Version 5  *****************
 * User: Kirk Stowell Date: 6/23/99    Time: 12:33a
 * Updated in $/CodeJockey/CJLibrary
 * 
 * *****************  Version 4  *****************
 * User: Kirk Stowell Date: 6/12/99    Time: 2:21a
 * Updated in $/CodeJockey/CJ60Lib
 * Cleaned up initialization, sorting and column info saving. Removed sort
 * functions, which were replaced with the virtual function SortList(...).
 * Override this function in your derived class to perform custom sorting.
 * See CCJShellList for an implementation of this virtual function.
 * 
 * *****************  Version 3  *****************
 * User: Kirk Stowell Date: 4/03/99    Time: 12:06a
 * Updated in $/CodeJockey/CJ60Lib
 * Added comments and cleaned up code.
 * 
 * *****************  Version 2  *****************
 * User: Kirk Stowell Date: 3/03/99    Time: 12:06a
 * Updated in $/CodeJockey/CJ60Lib
 * Added InitializeFlatHeader() in order to initialize the list control
 * manually.
 * 
 * *****************  Version 1  *****************
 * User: Kirk Stowell Date: 1/16/99    Time: 12:06a
 * Created in $/CodeJockey/CJ60Lib
 * Initial release.
 *
 ***************************************************************************/
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CJListView.h"
#include "CJFlatHeaderCtrl.h"
#include "CJResource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#ifndef HDF_SORTUP
	#define HDF_SORTUP              0x0400
	#define HDF_SORTDOWN            0x0200
#endif


/////////////////////////////////////////////////////////////////////////////
// CCJListView

IMPLEMENT_DYNCREATE(CCJListView, CListView)

CCJListView::CCJListView()
{
	m_nMinSize		= 0;
	m_nMinColWidth	= 50;
	m_nMaxColWidth	= 500;
	m_nSortedCol	= -1;
	m_bAscending	= true;
	m_bAutoSave		= false;	
	m_bNoColSizing	= false;
	m_pHeaderCtrl	= NULL;
	m_pListCtrl		= NULL;
	m_clrText		= CLR_DEFAULT;
	m_clrTextBk 	= CLR_DEFAULT;
}
	

CCJListView::~CCJListView()
{
	_delete( m_pHeaderCtrl );
}


BEGIN_MESSAGE_MAP(CCJListView, CListView)
	//{{AFX_MSG_MAP(CCJListView)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_DESTROY()
//	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCJListView drawing

void CCJListView::OnDraw( CDC* pDC )
{
	UNUSED_ALWAYS( pDC );
}

/////////////////////////////////////////////////////////////////////////////
// CCJListView diagnostics

#ifdef _DEBUG
void CCJListView::AssertValid() const
{
	CListView::AssertValid();
}

void CCJListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCJListView message handlers

void CCJListView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	// Set a pointer to the list control.
	m_pListCtrl = &GetListCtrl();
}

// Taken from: Copying/Moving Rows in CListCtrl by James Spibey [spib@bigfoot.com]. 
// http://www.codeguru.com/listview/moving_rows.html
bool CCJListView::MoveRow(int nFrom, int nTo)
{
	//Can't move to the same place, or from or to a negative index
	if(nFrom == nTo || nFrom < 0 || nTo < 0)
		return false;
	
	//First Copy the row to the new location
	if(CopyRow(nFrom, nTo))
	{
		//If we have just inserted a row before
		//this one in the list, we need to increment
		//our index.
		if(nFrom > nTo)
			m_pListCtrl->DeleteItem(nFrom + 1);
		else
			m_pListCtrl->DeleteItem(nFrom);
		
		return true;
	}
	else
		return false;
}

// Taken from: Copying/Moving Rows in CListCtrl by James Spibey [spib@bigfoot.com]. 
// http://www.codeguru.com/listview/moving_rows.html
bool CCJListView::CopyRow(int nFrom, int nTo)
{
	//Can't move to the same place, or from or to a negative index
	if(nFrom == nTo || nFrom < 0 || nTo < 0)
		return false;

	//Copy the row to the new index
	m_pListCtrl->InsertItem(nTo, m_pListCtrl->GetItemText(nFrom, 0));

	//If row has been inserted before original
	//increment the original
	if(nFrom > nTo)
		nFrom++;

	//Loop through subitems
	for(int i = 1; i < GetColumnCount(); i++)
	{
		m_pListCtrl->SetItemText(nTo, i, m_pListCtrl->GetItemText(nFrom, i));
	}

	return true;
}

// Taken from: Getting the number of columns in report view by Zafir Anjum.
// http://www.codeguru.com/listview/num_cols.shtml
int CCJListView::GetColumnCount() const
{
	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_pListCtrl->GetDlgItem(0);
	return pHeader->GetItemCount();
}

// Taken from: Simpler way to do this (using new version of Comctl32.dll) by Vidas [vibal@lrtc.lt].
// http://www.codeguru.com/mfc/comments/6432.shtml
void CCJListView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
	
    switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		// Request prepaint notifications for each item.
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
		
    case CDDS_ITEMPREPAINT: // Requested notification
		if(lplvcd->nmcd.dwItemSpec >= 0)		// Was dwItemSpec % 2 before!
		{										// AJM 11.9.2000 [andy.metcalfe@lineone.net]
			lplvcd->clrText   = m_clrText;

			// Only change the background colour if the window is enabled
			// Otherwise, use the system default
			// AJM 11.9.2000 [andy.metcalfe@lineone.net]
			if (IsWindowEnabled() )
			{
				lplvcd->clrTextBk = m_clrTextBk;
			}
		}
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

BOOL CCJListView::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	HD_NOTIFY *pHDNotify = (HD_NOTIFY*)lParam;

	switch( pHDNotify->hdr.code )
	{
	// Sorting the list when user clicks on column header by Zafir Anjum.
	// http://www.codeguru.com/listview/sort_on_col_click.shtml
	case HDN_ITEMCLICKA:
	case HDN_ITEMCLICKW:
		{
			if( pHDNotify->iButton == 0 )
			{
				bool bAscending = m_bAscending;

				// set the sort order.
				if( pHDNotify->iItem == m_nSortedCol )
					bAscending = !bAscending;
				else
					bAscending = true;

				// virtual call to sort list.
				SortList(pHDNotify->iItem, bAscending );

				// set the sort image in the header control.
				// Moved to SortList() AJM 3.3.2001
				//if( m_pHeaderCtrl && m_pHeaderCtrl->GetSafeHwnd( ))
				//	m_pHeaderCtrl->SetSortImage( m_nSortedCol, m_bAscending );
			}
		}
		break;

	// How to force a minimum column width Cees Mechielsen.
	// http://www.codeguru.com/mfc/comments/866.shtml
	case HDN_ITEMCHANGINGA:
	case HDN_ITEMCHANGINGW:
	case HDN_ENDTRACK:
		{
			if( pHDNotify->pitem->mask & HDI_WIDTH &&
				pHDNotify->pitem->cxy < m_nMinSize &&
				pHDNotify->pitem->cxy >= 0 )
			{
				pHDNotify->pitem->cxy = m_nMinSize;	// Set the column width
			}
		}
		break;

	// Prevent CListCtrl column resizing by Zafir Anjum.
	// http://www.codeguru.com/listview/no_col_resize.shtml
	case HDN_DIVIDERDBLCLICKA:
	case HDN_DIVIDERDBLCLICKW:
	case HDN_BEGINTRACKW:
	case HDN_BEGINTRACKA:
		{
			if(m_bNoColSizing)
			{
				*pResult = TRUE;                // disable tracking
				return TRUE;                    // Processed message
			}
			break;
		}
	}

	return CListView::OnNotify(wParam, lParam, pResult);
}

bool CCJListView::SortList(int nCol, bool bAscending)
{
#ifdef _DEBUG
	//TRACE2("Column sort on column index %d, ascending=%d\n", nCol, bAscending);
#endif
	// Set the sort image in the header control.
	// AJM 3.3.2001
	SetSortImage( nCol, bAscending );

	return true;
}


// Taken from: Adding a column by Zafir Anjum.
// http://www.codeguru.com/listview/add_column.shtml

// AddColumn      - adds column after the right most column
// Returns        - the index of the new column if successful or -1 otherwise.
// lpszColHeading - the column's heading
// nWidth         - Width of the column, in pixels. If this parameter is -1, 
//                  the column width is the same as previous column
// nFormat        - the alignment of the column. Can be LVCFMT_LEFT,
//                  LVCFMT_RIGHT, or LVCFMT_CENTER.
int CCJListView::AddColumn(LPCTSTR lpszColHeading, int nWidth/*=-1*/, int nFormat/*=LVCFMT_LEFT*/)
{
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	
	if( nWidth == -1 )
	{
		// The line below return zero for ICON views
		//nWidth = GetColumnWidth( nColumnCount - 1 );
		
		// Get the column width of the previous column from header control
		HD_ITEM hd_item;
		hd_item.mask = HDI_WIDTH;               //indicate that we want the width
		pHeader->GetItem( nColumnCount - 1, &hd_item );
		nWidth = hd_item.cxy;
	}
	return m_pListCtrl->InsertColumn( nColumnCount, lpszColHeading, nFormat, nWidth, nColumnCount );
}

bool CCJListView::BuildColumns(int nCols, int* nWidth, int* nColString)
{
	for( int i = 0; i < nCols; ++i ) {
		CString	strTemp; strTemp.LoadString( nColString[i] );
		if(AddColumn(strTemp, nWidth[i])==-1)
			return false;
	}

	return true;
}

bool CCJListView::BuildColumns(int nCols, int* nWidth, CString* strColString)
{
	for( int i = 0; i < nCols; ++i ) {
		if(AddColumn(strColString[i], nWidth[i])==-1)
			return false;
	}

	return true;
}

// Taken from: Detecting column index of the item clicked by Zafir Anjum.
// http://www.codeguru.com/listview/col_index.shtml

// HitTestEx	- Determine the row index and column index for a point
// Returns		- the row index or -1 if point is not over a row
// point		- point to be tested.
// nCol			- to hold the column index
int CCJListView::HitTestEx(CPoint& point, int* nCol) const
{
	int colnum = 0;
	int row = m_pListCtrl->HitTest( point, NULL );
	
	if( nCol ) *nCol = 0;

	// Make sure that the ListView is in LVS_REPORT
	if( (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT )
		return row;

	// Get the top and bottom row visible
	row = m_pListCtrl->GetTopIndex();
	int bottom = row + m_pListCtrl->GetCountPerPage();
	if( bottom > m_pListCtrl->GetItemCount() )
		bottom = m_pListCtrl->GetItemCount();
	
	// Get the number of columns
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();

	// Loop through the visible rows
	for( ;row <= bottom;row++)
	{
		// Get bounding rect of item and check whether point falls in it.
		CRect rect;
		m_pListCtrl->GetItemRect( row, &rect, LVIR_BOUNDS );
		if( rect.PtInRect(point) )
		{
			// Now find the column
			for( colnum = 0; colnum < nColumnCount; colnum++ )
			{
				int colwidth = m_pListCtrl->GetColumnWidth(colnum);
				if( point.x >= rect.left 
					&& point.x <= (rect.left + colwidth ) )
				{
					if( nCol ) *nCol = colnum;
					return row;
				}
				rect.left += colwidth;
			}
		}
	}
	return -1;
}



void CCJListView::SetThemeHelper(CCJThemeHelper* pTheme)
{
	m_pTheme = pTheme;
}


int CCJListView::SetSortImage(int nCol, bool bAsc)
{
	int nPrevCol = m_nSortedCol;

	m_nSortedCol	= nCol;
	m_bAscending	= bAsc;

	if (NULL != m_pHeaderCtrl)
	{
		return m_pHeaderCtrl->SetSortImage( m_nSortedCol, m_bAscending );
	}

	// Set the sort image
	HWND hWnd = GetDlgItem(0)->GetSafeHwnd();
	ASSERT(NULL != hWnd);

	HDITEM hditem;

	// Remove the sort image from the previous column (if any)
	if (nPrevCol >= 0)
	{
		hditem.mask = HDI_FORMAT;
		VERIFY( Header_GetItem(hWnd, nPrevCol, &hditem) );

		hditem.fmt = hditem.fmt & ~HDF_SORTDOWN & ~HDF_SORTUP;

		Header_SetItem(hWnd, nPrevCol, &hditem);
	}

	// Now set the sort image on the new column
	hditem.mask = HDI_FORMAT;
	VERIFY( Header_GetItem(hWnd, m_nSortedCol, &hditem) );

	hditem.fmt = hditem.fmt & ~HDF_SORTDOWN & ~HDF_SORTUP;

	hditem.fmt |= m_bAscending ? HDF_SORTUP : HDF_SORTDOWN;

	Header_SetItem(hWnd, m_nSortedCol, &hditem);

	return nPrevCol;
}


bool CCJListView::SubclassHeader(bool bBoldFont/*=false*/)
{
	// allocate memory for the flat header.
	m_pHeaderCtrl = new CCJFlatHeaderCtrl;
	ASSERT( m_pHeaderCtrl );

	// get the window handle to the existing header
	// control.
	HWND hWnd = GetDlgItem(0)->GetSafeHwnd();
	ASSERT( hWnd );

	// subclass the flat header control.
	if( !m_pHeaderCtrl->SubclassWindow( hWnd ))
		return false;

	// finish header initialization.
	m_pHeaderCtrl->InitializeHeader( bBoldFont );
	
	return true;
}

void CCJListView::AutoSaveColumns(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	m_bAutoSave = true;

	// initialize registry strings.
	if( lpszSection == NULL ) m_strSection = _T("Settings");
	else m_strSection = lpszSection;

	if( lpszEntry == NULL ) m_strEntry = _T("Column Info");
	else m_strEntry = lpszEntry;

	if( lpszDefault == NULL ) m_strDefault = _T("");
	else m_strDefault = lpszDefault;

	LoadColumnWidths();
}

void CCJListView::SaveColumnWidths()
{
	// get a pointer to the header control.
	CHeaderCtrl* pHeader = DYNAMIC_DOWNCAST(CHeaderCtrl, GetDlgItem(0));
	ASSERT_KINDOF( CHeaderCtrl, pHeader );

	CString strValue;
	for( int i = 0; i < pHeader->GetItemCount(); ++i )
	{
		CString strTemp;
		strTemp.Format(_T("%d,"), m_pListCtrl->GetColumnWidth( i ));
		strValue += strTemp;
	}

	AfxGetApp()->WriteProfileString( m_strSection, m_strEntry, strValue );
}

void CCJListView::LoadColumnWidths()
{
	// get a pointer to the header control.
	CHeaderCtrl* pHeader = DYNAMIC_DOWNCAST(CHeaderCtrl, GetDlgItem(0));
	ASSERT_KINDOF( CHeaderCtrl, pHeader );

	for( int i = 0; i < pHeader->GetItemCount(); ++i )
		SetColumnWidth(i);
}

void CCJListView::SetColumnWidth(int nCol)
{
	int nWidth = GetStoredWidth( nCol );
	if( nWidth > m_nMinColWidth )
		m_pListCtrl->SetColumnWidth( nCol, nWidth );
	else
		AutoSizeColumn( nCol );
}

int CCJListView::GetStoredWidth(int nCol)
{
	// get the value from the registry.
	CString strValue = AfxGetApp()->GetProfileString(
		m_strSection, m_strEntry, m_strDefault );

	// extract the sub string to get the column width.
	CString strSubString;
	AfxExtractSubString( strSubString, strValue, nCol, _T(','));

	// return the width from the registry.
	return _ttoi( strSubString );
}

void CCJListView::OnDestroy() 
{
	if( m_bAutoSave ) SaveColumnWidths();
	CListView::OnDestroy();
}

// Taken from: Autosize a column to fit its content by Roger Onslow.
// http://www.codeguru.com/listview/autosize_col.shtml
// If you don't supply a column number, it will resize all columns.
void CCJListView::AutoSizeColumn(int nCol/*=-1*/)
{
	// Call this after your list control is filled
	SetRedraw( false );
	
	int nMinCol = nCol < 0 ? 0 : nCol;
	int nMaxCol = nCol < 0 ? GetColumnCount()-1 : nCol;
	
	for (nCol = nMaxCol; nCol >= nMinCol; nCol--)
	{
		m_pListCtrl->SetColumnWidth( nCol, LVSCW_AUTOSIZE );
		int wc1 = m_pListCtrl->GetColumnWidth( nCol );
		
		m_pListCtrl->SetColumnWidth( nCol, LVSCW_AUTOSIZE_USEHEADER );
		int wc2 = m_pListCtrl->GetColumnWidth( nCol );

		if ( (NULL != m_pHeaderCtrl) && (m_nSortedCol == nCol) )
		{
			wc2 += m_pHeaderCtrl->GetSortImageWidth();
		}

		int wc = max( m_nMinColWidth, max( wc1, wc2 ));
	
		if( wc > m_nMaxColWidth )
			wc = m_nMaxColWidth;
		
		// set the column width.
		m_pListCtrl->SetColumnWidth( nCol,wc );
	}
	
	SetRedraw();
}

BOOL CCJListView::OnEraseBkgnd(CDC* pDC) 
{
	CRect rectClient;
	GetClientRect( &rectClient );
	pDC->ExcludeClipRect( &rectClient );
	return CListView::OnEraseBkgnd(pDC);
}
