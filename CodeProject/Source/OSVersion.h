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
//*****************************************************************************

// OSVersion.h: interface for the COSVersion class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OSVERSION_H__2D4F4362_09C2_48B2_91EB_C9671C4E08C4__INCLUDED_)
#define AFX_OSVERSION_H__2D4F4362_09C2_48B2_91EB_C9671C4E08C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum OSVERSION
{
	OSV_UNKNOWN = -1,
	OSV_95,
	OSV_98,
	OSV_ME,
	OSV_NT4,
	OSV_2K,
	OSV_XP,
	OSV_XPP, // after xp
};


class COSVersion
{
public:
	COSVersion();
	virtual ~COSVersion();

	operator OSVERSION()
	{
		return GetOSVersion();
	}

protected:
	OSVERSION GetOSVersion();
};

#endif // !defined(AFX_OSVERSION_H__2D4F4362_09C2_48B2_91EB_C9671C4E08C4__INCLUDED_)
