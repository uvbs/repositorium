// Written for the ToDoList (http://www.codeproject.com/KB/applications/todolist2.aspx)
// Design and latest version - http://www.codeproject.com/KB/applications/TDL_Calendar.aspx
// by demon.code.monkey@googlemail.com

//*****************************************************************************
// Modified by Elijah Zarezky aka SchweinDeBurg (elijah.zarezky@gmail.com):
// - improved compatibility with the Unicode-based builds
// - taken out from the original TDL_Calendar package for better sharing
// - adjusted #include's paths
// - slightly reformatted source code
// - restyled using ProFactor StyleManager v1.17:
//      * removed unnecessary spaces and empty lines
//      * wrapped extremely long lines
//      * reformatted all the ctors to be more readable
//      * eliminated dead commented code
//*****************************************************************************

#ifndef _CALENDARUTILS_H_
#define _CALENDARUTILS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//CCalendarUtils: a collection of static utils used by the Calendar DLL
class CCalendarUtils
{
//construction
public:
	CCalendarUtils()
	{
		ASSERT(FALSE);   //don't instantiate me
	}
	~CCalendarUtils()
	{
		ASSERT(FALSE);   //don't instantiate me
	}

//static functions
public:
	static CString LongToString(long lValue);
	static time_t DateToTime(const COleDateTime& _dt);
	static time_t TimeToTime(const time_t& _t);
#ifdef _DEBUG
	static CString  DateToString(const COleDateTime& _dt);
#endif

	static void GetDateOnly(COleDateTime& _dt);
	static void GetToday(COleDateTime& _dt);
	static CString GetWeekday(int _iDayOfWeek, BOOL _bShort);
	static void GetWeekdays(CStringList& _listDays, BOOL _bShort, BOOL _bWeekendsHidden);

	static int CompareDates(const COleDateTime& _dt1, const COleDateTime& _dt2);

	static BOOL IsDateValid(const COleDateTime& _dt);
	static int GetDaysInMonth(int _iMonth, int _iYear);

	static void AddDay(COleDateTime& _dt, int _nNumDays = 1);
	static void SubtractDay(COleDateTime& _dt, int _nNumDays = 1);
	static void AddMonth(COleDateTime& _dt, int _nNumMonths = 1);
	static void SubtractMonth(COleDateTime& _dt, int _nNumMonths = 1);
};

#endif//_CALENDARUTILS_H_
