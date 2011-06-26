// Copyright (C) 2003-2011 AbstractSpoon Software.
//
// This license applies to everything in the ToDoList package, except where
// otherwise noted.
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software in a
// product, an acknowledgment in the product documentation would be appreciated
// but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.

//*****************************************************************************
// Modified by Elijah Zarezky aka SchweinDeBurg (elijah.zarezky@gmail.com):
// - improved compatibility with the Unicode-based builds
// - added AbstractSpoon Software copyright notice and license information
// - taken out from the original ToDoList package for better sharing
// - reformatted using Artistic Style 2.02 with the following options:
//      --indent=tab=3
//      --indent=force-tab=3
//      --indent-cases
//      --max-instatement-indent=2
//      --style=allman
//      --add-brackets
//      --pad-oper
//      --unpad-paren
//      --pad-header
//      --align-pointer=type
//      --lineend=windows
//      --suffix=none
// - restyled using ProFactor StyleManager v1.17:
//      * removed unnecessary spaces and empty lines
//      * wrapped extremely long lines
//      * reformatted all the ctors to be more readable
//      * eliminated dead commented code
//*****************************************************************************

// DeferWndMove.cpp: implementation of the CDeferWndMove class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeferWndMove.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDeferWndMove::CDeferWndMove(int nNumWindows):
m_hdwp(NULL)
{
	if (nNumWindows)
	{
		BeginMove(nNumWindows);
	}
}

CDeferWndMove::~CDeferWndMove()
{
	if (m_hdwp)
	{
		EndMove();
	}
}

BOOL CDeferWndMove::BeginMove(int nNumWindows)
{
	ASSERT(nNumWindows > 0);

	if (m_hdwp)
	{
		EndMove();
	}

	if (nNumWindows > 0)
	{
		m_hdwp = ::BeginDeferWindowPos(nNumWindows);
	}

	return (m_hdwp != NULL);
}

BOOL CDeferWndMove::EndMove()
{
	ASSERT(m_hdwp);

	BOOL bRes = ::EndDeferWindowPos(m_hdwp);
	m_hdwp = NULL;

	return bRes;
}

BOOL CDeferWndMove::MoveWindow(CWnd* pWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint)
{
	ASSERT(m_hdwp);

	if (!m_hdwp)
	{
		return FALSE;
	}

	ASSERT(pWnd->GetParent());

	// figure out what flags we need
	int nFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	CRect rWnd;

	pWnd->GetWindowRect(rWnd);
	pWnd->GetParent()->ScreenToClient(rWnd);

	if (x == rWnd.left && y == rWnd.top)
	{
		nFlags |= SWP_NOMOVE;
	}

	if (nWidth == rWnd.Width() && nHeight == rWnd.Height())
	{
		nFlags |= SWP_NOSIZE;
	}

	BOOL bNeedMove = !((nFlags & SWP_NOMOVE) && (nFlags & SWP_NOSIZE));

	if (!bNeedMove)
	{
		return TRUE;
	}

	if (!bRepaint)
	{
		nFlags |= SWP_NOREDRAW;
	}

	HWND hWnd = pWnd->GetSafeHwnd();

	m_hdwp = ::DeferWindowPos(m_hdwp, hWnd, HWND_BOTTOM, x, y, nWidth, nHeight, nFlags);

	// fallback
	if (!m_hdwp)
	{
		ASSERT(0);  // shouldn't fail but it might
		::SetWindowPos(hWnd, HWND_BOTTOM, x, y, nWidth, nHeight, nFlags);
	}

	return TRUE;
}

BOOL CDeferWndMove::MoveWindow(CWnd* pWnd, LPCRECT lpRect, BOOL bRepaint)
{
	return MoveWindow(pWnd, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
		bRepaint);
}

CRect CDeferWndMove::OffsetCtrl(CWnd* pParent, UINT nCtrlID, int x, int y)
{
	CWnd* pCtrl = pParent->GetDlgItem(nCtrlID);

	if (pCtrl)
	{
		CRect rChild;
		pCtrl->GetWindowRect(rChild);
		pParent->ScreenToClient(rChild);

		if (x || y)
		{
			rChild.OffsetRect(x, y);
			MoveWindow(pCtrl, rChild); // our own version
		}

		return rChild;
	}

	return CRect(0, 0, 0, 0);
}

CRect CDeferWndMove::MoveCtrl(CWnd* pParent, UINT nCtrlID, int x, int y)
{
	CWnd* pCtrl = pParent->GetDlgItem(nCtrlID);

	if (pCtrl)
	{
		CRect rChild;
		pCtrl->GetWindowRect(rChild);
		pParent->ScreenToClient(rChild);

		if (x || y)
		{
			rChild.OffsetRect(x - rChild.left, y - rChild.top);
			MoveWindow(pCtrl, rChild); // our own version
		}

		return rChild;
	}

	return CRect(0, 0, 0, 0);
}

CRect CDeferWndMove::ResizeCtrl(CWnd* pParent, UINT nCtrlID, int cx, int cy)
{
	CWnd* pCtrl = pParent->GetDlgItem(nCtrlID);

	if (pCtrl)
	{
		CRect rChild, rParent;
		pCtrl->GetWindowRect(rChild);
		pParent->ScreenToClient(rChild);
		pParent->GetClientRect(rParent);

		if (cx || cy)
		{
			rChild.right += cx;
			rChild.bottom += cy;

			// make sure it also intersects with parent
			if (rChild.IntersectRect(rChild, rParent))
			{
				MoveWindow(pCtrl, rChild);   // our own version
			}
		}

		return rChild;
	}

	return CRect(0, 0, 0, 0);
}
