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
//*****************************************************************************

// AutoFlag.h: interface for the CAutoFlag class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOFLAG_H__EEBD725A_2C54_4554_96D8_E4CCDC940258__INCLUDED_)
#define AFX_AUTOFLAG_H__EEBD725A_2C54_4554_96D8_E4CCDC940258__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// macro to help prevent re-entrancy in functions
#define AF_NOREENTRANT  \
	static BOOL bAFInHere = FALSE; \
	if (bAFInHere) \
	return; \
CAutoFlag af(bAFInHere, TRUE);

#define AF_NOREENTRANT_RET(ret)  \
	static BOOL bAFInHere = FALSE; \
	if (bAFInHere) \
	return ret; \
CAutoFlag af(bAFInHere, TRUE);

class CAutoFlag
{
public:
	CAutoFlag(BOOL& bFlag, BOOL bState);
	virtual ~CAutoFlag();

protected:
	BOOL& m_bFlag;
	BOOL m_bInitialState;

};

#endif // !defined(AFX_AUTOFLAG_H__EEBD725A_2C54_4554_96D8_E4CCDC940258__INCLUDED_)
