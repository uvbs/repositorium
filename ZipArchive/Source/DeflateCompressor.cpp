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
#include "DeflateCompressor.h"
#include "../../zlib/Source/deflate.h"

namespace ZipArchiveLib
{

#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif


CDeflateCompressor::CDeflateCompressor(CZipStorage* pStorage)
	: CBaseLibCompressor(pStorage)
{
	m_stream.zalloc = (z_alloc_func)_zipalloc;
	m_stream.zfree = (z_free_func)_zipfree;
}


void CDeflateCompressor::InitCompression(int iLevel, CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
{
	CZipCompressor::InitCompression(iLevel, pFile, pCryptograph);

	m_stream.avail_in = (z_uInt)0;
	m_stream.avail_out = (z_uInt)m_pBuffer.GetSize();
	m_stream.next_out = (z_Bytef*)(char*)m_pBuffer;
	m_stream.total_in = 0;
	m_stream.total_out = 0;

	if (pFile->m_uMethod == methodDeflate)
	{
		SetOpaque(&m_stream.opaque, &m_options);

		int err = z_deflateInit2_(&m_stream, iLevel,
			Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(z_stream));

		CheckForError(err);
	}
}

void CDeflateCompressor::Compress(const void* pBuffer, DWORD uSize)
{
	UpdateFileCrc(pBuffer, uSize);

	if (m_pFile->m_uMethod == methodDeflate)
	{
		m_stream.next_in = (z_Bytef*)pBuffer;
		m_stream.avail_in = uSize;

		while (m_stream.avail_in > 0)
		{
			if (m_stream.avail_out == 0)
			{
				FlushWriteBuffer();
				m_stream.avail_out = m_pBuffer.GetSize();
				m_stream.next_out = (z_Bytef*)(char*)m_pBuffer;
			}

			ZIP_ZLIB_TYPE uTotal = m_stream.total_out;
			CheckForError(z_deflate(&m_stream,  Z_NO_FLUSH));
			m_uComprLeft += m_stream.total_out - uTotal;
		}
	}
	else if (uSize > 0)
	{
		if (m_pCryptograph)
		{
			if (m_pBuffer.GetSize() < uSize)
			{
				m_pBuffer.Allocate(uSize);
			}
			memcpy(m_pBuffer, pBuffer, uSize);
			WriteBuffer(m_pBuffer, uSize);
		}
		else
		{
			m_pStorage->Write(pBuffer, uSize, false);
		}
		m_stream.total_in += uSize;
		m_stream.total_out += uSize;
	}
}

void CDeflateCompressor::FinishCompression(bool bAfterException)
{
	m_stream.avail_in = 0;
	if (!bAfterException)
	{
		if (m_pFile->m_uMethod == methodDeflate)
		{
			int err;
			do
			{
				if (m_stream.avail_out == 0)
				{
					FlushWriteBuffer();
					m_stream.avail_out = m_pBuffer.GetSize();
					m_stream.next_out = (z_Bytef*)(char*)m_pBuffer;
				}
				ZIP_SIZE_TYPE uTotal = m_stream.total_out;
				err = z_deflate(&m_stream,  Z_FINISH);
				m_uComprLeft += m_stream.total_out - uTotal;
			}
			while (err == Z_OK);

			if (err == Z_STREAM_END)
			{
				err = Z_OK;
			}
			CheckForError(err);

			if (m_uComprLeft > 0)
			{
				FlushWriteBuffer();
			}

			CheckForError(z_deflateEnd(&m_stream));
		}

		// it may be increased by the encrypted header size in CZipFileHeader::PrepareData
		m_pFile->m_uComprSize += m_stream.total_out;
		m_pFile->m_uUncomprSize = m_stream.total_in;
	}
	EmptyPtrList();
	ReleaseBuffer();
}

void CDeflateCompressor::InitDecompression(CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
{
	CBaseLibCompressor::InitDecompression(pFile, pCryptograph);
	if (m_pFile->m_uMethod == methodDeflate)
	{
		SetOpaque(&m_stream.opaque, &m_options);
		CheckForError(z_inflateInit2_(&m_stream, -MAX_WBITS, ZLIB_VERSION, sizeof(z_stream)));
	}
	m_stream.total_out = 0;
	m_stream.avail_in = 0;
}

DWORD CDeflateCompressor::Decompress(void* pBuffer, DWORD uSize)
{
	if (m_bDecompressionDone)
	{
		return 0;
	}

	DWORD uRead = 0;
	if (m_pFile->m_uMethod == methodDeflate)
	{
		m_stream.next_out = (z_Bytef*)pBuffer;
		m_stream.avail_out = uSize > m_uUncomprLeft ? (DWORD)m_uUncomprLeft : uSize;

		// may happen when the file is 0 sized
		bool bForce = m_stream.avail_out == 0 && m_uComprLeft > 0;
		while (m_stream.avail_out > 0 || (bForce && m_uComprLeft > 0))
		{
			if ((m_stream.avail_in == 0) &&
				(m_uComprLeft >= 0)) // Also when there are zero bytes left
			{
				DWORD uToRead = FillBuffer();

				m_stream.next_in = (z_Bytef*)(char*)m_pBuffer;
				m_stream.avail_in = uToRead;
			}


			ZIP_SIZE_TYPE uTotal = m_stream.total_out;
			z_Bytef* pOldBuf =  m_stream.next_out;
			int ret = z_inflate(&m_stream, Z_SYNC_FLUSH);
			// will not exceed DWORD
			DWORD uToCopy = (DWORD)(m_stream.total_out - uTotal);

			UpdateCrc(pOldBuf, uToCopy);

			m_uUncomprLeft -= uToCopy;
			uRead += uToCopy;

			if (ret == Z_STREAM_END)
			{
				m_bDecompressionDone = true;
				return uRead;
			}
			else
			{
				CheckForError(ret);
			}
		}

		if (!uRead && m_options.m_bCheckLastBlock && uSize != 0)
		{
			if (z_inflate(&m_stream, Z_SYNC_FLUSH) != Z_STREAM_END)
				// there were no more bytes to read and there was no ending block,
				// otherwise the method would return earlier
			{
				ThrowError(CZipException::badZipFile);
			}
		}
	}
	else
	{
		if (m_uComprLeft < uSize)
		{
			uRead = (DWORD)m_uComprLeft;
		}
		else
		{
			uRead = uSize;
		}

		if (uRead > 0)
		{
			m_pStorage->Read(pBuffer, uRead, false);
			if (m_pCryptograph)
			{
				m_pCryptograph->Decode((char*)pBuffer, uRead);
			}
			UpdateCrc(pBuffer, uRead);
			m_uComprLeft -= uRead;
			m_uUncomprLeft -= uRead;
			m_stream.total_out += uRead;
		}
	}
	return uRead;
}

void CDeflateCompressor::FinishDecompression(bool bAfterException)
{
	if (!bAfterException && m_pFile->m_uMethod == methodDeflate)
	{
		z_inflateEnd(&m_stream);
	}
	EmptyPtrList();
	ReleaseBuffer();
}


} // namespace
