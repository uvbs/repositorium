////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyrighted 2000 - 2012 by Artpol Software - Tadeusz Dracz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// For the licensing details refer to the License.txt file.
//
// Web Site: http://www.artpol-software.com
////////////////////////////////////////////////////////////////////////////////
// Modified by Elijah Zarezky aka SchweinDeBurg (elijah.zarezky@gmail.com):
// - reformatted using Artistic Style 2.02 with the following options:
//      --indent=tab=3
//      --indent=force-tab=3
//      --indent-cases
//      --min-conditional-indent=0
//      --max-instatement-indent=2
//      --style=allman
//      --add-brackets
//      --pad-oper
//      --unpad-paren
//      --pad-header
//      --align-pointer=type
//      --lineend=windows
//      --suffix=none
// - implemented support for the Windows Mobile/CE tragets
// - added possibility to seamless usage in the ATL-based projects
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Private/ZipCompressor.h"
#include "Private/BytesWriter.h"
#include "DeflateCompressor.h"

using namespace ZipArchiveLib;

CZipCompressor* CZipCompressor::CreateCompressor(WORD uMethod, CZipStorage* pStorage)
{
	if (uMethod == methodStore || uMethod == methodDeflate)
	{
		return new CDeflateCompressor(pStorage);
	}

	return NULL;
}

void CZipCompressor::UpdateFileCrc(const void* pBuffer, DWORD uSize)
{
	m_pFile->m_uCrc32 = z_crc32(m_pFile->m_uCrc32, (z_Bytef*)pBuffer, uSize);
}

void CZipCompressor::UpdateCrc(const void* pBuffer, DWORD uSize)
{
	m_uCrc32 = z_crc32(m_uCrc32, (z_Bytef*)pBuffer, uSize);
}

void CZipCompressor::UpdateOptions(const COptionsMap& optionsMap)
{
	const COptions* pOptions = GetOptions();
	if (pOptions == NULL)
	{
		return;
	}
	const COptions* pNewOptions = optionsMap.Get(pOptions->GetType());
	if (pNewOptions != NULL)
	{
		UpdateOptions(pNewOptions);
	}
}

void CZipCompressor::InitBuffer()
{
	// This should be greater that 64k for deflate when creating offsets pairs is enabled
	// otherwise deflate will not be able to write one block in one go and will never report
	// a flushed block for low-compressable data
	const COptions* pOptions = GetOptions();
	DWORD bufferSize = 0;
	if (pOptions != NULL)
	{
		bufferSize = pOptions->m_iBufferSize;
	}
	if (bufferSize == 0)
	{
		bufferSize = COptions::cDefaultBufferSize;
	}
	m_pBuffer.Allocate(bufferSize);
}


void CZipCompressor::COptionsMap::Set(const CZipCompressor::COptions* pOptions)
{
	if (pOptions == NULL)
	{
		return;
	}
	int iType = pOptions->GetType();
	Remove(iType);
	SetAt(iType, pOptions->Clone());
}

CZipCompressor::COptions* CZipCompressor::COptionsMap::Get(int iType) const
{
	COptions* pTemp = NULL;
	if (Lookup(iType, pTemp))
	{
		return pTemp;
	}
	else
	{
		return NULL;
	}
}

void CZipCompressor::COptionsMap::Remove(int iType)
{
	COptions* pTemp = Get(iType);
	if (pTemp != NULL)
	{
		delete pTemp;
		RemoveKey(iType);
	}
}

CZipCompressor::COptionsMap::~COptionsMap()
{
	COptionsMap::iterator iter = GetStartPosition();
	while (IteratorValid(iter))
	{
		COptions* pOptions = NULL;
		int iDummyType;
		GetNextAssoc(iter, iDummyType, pOptions);
		delete pOptions;
	}
	RemoveAll();
}

