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
// - merged with ToDoList version 6.2.2 sources
//*****************************************************************************

// TimeComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "TimeComboBox.h"
#include "TimeHelper.h"
#include "Misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimeComboBox

CTimeComboBox::CTimeComboBox(DWORD dwStyle):
m_dwStyle(dwStyle)
{
}

CTimeComboBox::~CTimeComboBox()
{
}


BEGIN_MESSAGE_MAP(CTimeComboBox, CComboBox)
	//{{AFX_MSG_MAP(CTimeComboBox)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeComboBox message handlers

void CTimeComboBox::PreSubclassWindow()
{
	BuildCombo();

	// hook the edit ctrl so we can convert '.' and ',' to ':'
	CWnd* pEdit = GetDlgItem(1001);

	if (pEdit)
	{
		ScHookWindow(pEdit->GetSafeHwnd());
	}

	CComboBox::PreSubclassWindow();
}

int CTimeComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	BuildCombo();

	// hook the edit ctrl so we can convert '.' and ',' to ':'
	CWnd* pEdit = GetDlgItem(1001);

	if (pEdit)
	{
		ScHookWindow(pEdit->GetSafeHwnd());
	}

	return 0;
}

void CTimeComboBox::BuildCombo(BOOL bReset)
{
	ASSERT(bReset || GetCount() == 0);

	if (!bReset && GetCount())
	{
		return;
	}

	ResetContent();

	for (int nHour = 0; nHour < 24; nHour++)
	{
		CString sTime;

		if ((m_dwStyle & TCB_NOTIME) && nHour == 0)
		{
			// add empty string representing 'no time'
		}
		else if (m_dwStyle & TCB_ISO)
		{
			sTime = CTimeHelper::FormatISOTime(nHour, 0);
		}
		else // regional settings
		{
			sTime = CTimeHelper::Format24HourTime(nHour, 0);
		}

		AddString(sTime);

		if (m_dwStyle & TCB_HALFHOURS)
		{
			if (m_dwStyle & TCB_ISO)
			{
				sTime = CTimeHelper::FormatISOTime(nHour, 30);
			}
			else // regional settings
			{
				sTime = CTimeHelper::Format24HourTime(nHour, 30);
			}

			AddString(sTime);
		}
	}
}

double CTimeComboBox::GetOleTime() const
{
	return (Get24HourTime() / 24.0);
}

BOOL CTimeComboBox::SetOleTime(double dTime)
{
	// truncate to extract the time only component if it has one
	dTime -= (int)dTime;

	if (dTime <= 0)
	{
		SetCurSel(0);
		return TRUE;
	}

	// else
	return Set24HourTime(dTime * 24.0);
}

void CTimeComboBox::SetStyle(DWORD dwStyle)
{
	BOOL bWasISO = (m_dwStyle & TCB_ISO);
	BOOL bIsISO = (dwStyle & TCB_ISO);

	m_dwStyle = dwStyle;

	if (bWasISO != bIsISO)
	{
		double date = GetOleTime();
		BuildCombo(TRUE);
		SetOleTime(date);
	}
}

double CTimeComboBox::Get24HourTime() const
{
	// is this a hack? I'm not sure
	// but we look at the current Windows message being handled and if it's a
	// combo sel change then we use the current selection else we use the
	// window text (it may be an edit change notification)
	const MSG* pMsg = CComboBox::GetCurrentMessage();

	if (pMsg->message == WM_COMMAND && pMsg->lParam == (LPARAM)GetSafeHwnd() &&
		HIWORD(pMsg->wParam) == CBN_SELCHANGE)
	{
		// since the items in the combo are ordered from 1am to 11pm
		// we can use the selection index as a direct link to the hour
		int nSel = GetCurSel();

		if (nSel <= 0) // 'no time'
		{
			return 0;
		}

		// else
		if (m_dwStyle & TCB_HALFHOURS)
		{
			return (nSel * 0.5);
		}
		else
		{
			return nSel;
		}
	}

	// else use window text
	double dTime = 0;
	CString sTime;
	GetWindowText(sTime);

	if (sTime.IsEmpty())
	{
		if ((m_dwStyle & TCB_NOTIME) != 0)
		{
			return 0;   // midnight
		}
		else
		{
			return -1;   // error
		}
	}

	// look for a separator
	CString sSep = Misc::GetTimeSeparator();
	int nColon = sTime.Find(sSep);

	if (nColon != -1) // extract hours and minutes
	{
		dTime = _tstof(sTime.Left(nColon)) + _tstof(sTime.Mid(nColon + sSep.GetLength())) / 60;
	}
	else
	{
		dTime = _tstof(sTime);
	}

	// look for PM signifier
	if (!Misc::GetPM().IsEmpty() && dTime < 12)
	{
		sTime.MakeUpper();

		if (sTime.Find(Misc::GetPM()) != -1)
		{
			dTime += 12;
		}
	}

	return (dTime <= 0) ? 0 : dTime;
}

BOOL CTimeComboBox::Set24HourTime(double dTime)
{
	if (dTime < 0)
	{
		SetCurSel(0);
		return TRUE;
	}

	// else
	if (dTime >= 24.0)
	{
		return FALSE;
	}

	// add on half a minute to handle floating point inaccuracies
	dTime += (0.5 / (24 * 60));

	int nHour = (int)dTime;
	int nMin = (int)((dTime - nHour) * 60); // nearest 1 minute

	CString sTime;

	if (m_dwStyle & TCB_ISO)
	{
		sTime = CTimeHelper::FormatISOTime(nHour, nMin);
	}
	else
	{
		sTime = CTimeHelper::Format24HourTime(nHour, nMin);
	}

	if (SelectString(-1, sTime) == CB_ERR)
	{
		SetWindowText(sTime);
	}

	return TRUE;
}

LRESULT CTimeComboBox::ScWindowProc(HWND hRealWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CHAR:
		if (wParam == _T(',') || wParam == _T('.') && !Misc::GetTimeSeparator().IsEmpty())
		{
			CString sSep = Misc::GetTimeSeparator();
			return CSubclasser::ScWindowProc(hRealWnd, message, sSep[0], lParam);
		}
		break;
	}

	// else
	return ScDefault(hRealWnd);
}
