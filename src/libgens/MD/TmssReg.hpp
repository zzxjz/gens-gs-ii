/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * TmssReg.hpp: MD TMSS registers.                                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2013 by David Korth.                                 *
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

#ifndef __LIBGENS_MD_TMSSREG_HPP__
#define __LIBGENS_MD_TMSSREG_HPP__

// C includes.
#include <stdint.h>

namespace LibGens
{

class TmssReg
{
	public:
		TmssReg()
			: a14000({0})
			, cart_ce(0)
			, tmss_rom_valid(false)
			, tmss_en(false)
			{ }

		/**
		 * $A14000: TMSS control register.
		 * This must have 'SEGA' in order to use the VDP.
		 * Otherwise, the VDP will lock on when accessed.
		 * NOTE: This is not emulated at the moment.
		 */
		union {
			uint32_t d;
			uint16_t w[2];
			uint8_t  b[4];
		} a14000;

		/**
		 * $A14101: TMSS ROM mapping register.
		 * Bit 0 indicates if the cartridge or TMSS ROM is mapped.
		 * - 0: TMSS ROM is mapped.
		 * - 1: Cartridge is mapped.
		 */
		uint8_t cart_ce;

		/**
		 * Is the TMSS ROM valid?
		 * If it isn't, TMSS will be disabled.
		 */
		bool tmss_rom_valid;

		/**
		 * Is TMSS enabled?
		 * This is set on emulation startup and hard reset.
		 */
		bool tmss_en;

		/**
		 * Check if TMSS is mapped.
		 * @return True if mapped; false if not.
		 */
		bool isTmssMapped(void) const;

		/**
		 * Reset the TMSS registers.
		 */
		void reset(void);
};

/**
 * Check if TMSS is mapped.
 * @return True if mapped; false if not.
 */
inline bool TmssReg::isTmssMapped(void) const
	{ return (tmss_en && !(cart_ce & 1)); }

/**
 * Reset the TMSS registers.
 */
inline void TmssReg::reset(void)
{
	a14000.d = 0;
	cart_ce = 0;
}

}

#endif /* __LIBGENS_MD_TMSSREG_HPP__ */
