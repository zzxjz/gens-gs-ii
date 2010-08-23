/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * W32U_mini.hpp: Win32 Unicode Translation Layer. (Mini Version)          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "W32U_mini.hpp"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// C includes.
#include <stdlib.h>

namespace W32U
{

/**
 * IsUnicode: Indicates if the system is Unicode.
 * NOTE: Do NOT edit this variable outside of W32U!
 */
bool IsUnicode = false;


/**
 * Init(): Initialize the Win32 Unicode Translation Layer.
 * @return 0 on success; non-zero on error.
 */
int Init(void)
{
	IsUnicode = (GetModuleHandleW(NULL) != NULL);
	return 0;
}


/**
 * End(): Shut down the Win32 Unicode Translation Layer.
 * @return 0 on success; non-zero on error.
 */
int End(void)
{
	IsUnicode = false;
	return 0;
}


/**
 * mbs_to_UTF16(): Convert a multibyte string to UTF-16.
 * TODO: Move to another file.
 * @param mbs UTF-8 string.
 * @param codepage mbs codepage.
 * @return UTF-16 string, or NULL on error.
 */
wchar_t *mbs_to_UTF16(const utf8_str *mbs, unsigned int codepage)
{
	int cchWcs = MultiByteToWideChar(codepage, 0, mbs, -1, NULL, 0);
	if (cchWcs <= 0)
		return NULL;
	
	wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, mbs, -1, wcs, cchWcs);
	return wcs;
}


/**
 * UTF16_to_mbs(): Convert a UTF-16 string to multibyte.
 * @param wcs UTF-16 string.
 * @param codepage mbs codepage.
 * @return Multibyte string, or NULL on error.
 */
char *UTF16_to_mbs(const wchar_t *wcs, unsigned int codepage)
{
	int cbMbs = WideCharToMultiByte(codepage, 0, wcs, -1, NULL, 0, NULL, NULL);
	if (cbMbs <= 0)
		return NULL;
	
	char *mbs = (char*)malloc(cbMbs);
	WideCharToMultiByte(codepage, 0, wcs, -1, mbs, cbMbs, NULL, NULL);
	return mbs;
}

}
