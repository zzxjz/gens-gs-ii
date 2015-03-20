/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EEPRom.hpp: Serial EEPROM handler. (I2C)                                *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __LIBGENS_SAVE_EEPROM_HPP__
#define __LIBGENS_SAVE_EEPROM_HPP__

// C includes.
#include <stdint.h>
#include <string.h>
#include <limits.h>

// C++ includes.
#include <string>

namespace LibGens {

class EEPRomPrivate;
class EEPRom
{
	public:
		EEPRom();
		~EEPRom();

	protected:
		friend class EEPRomPrivate;
		EEPRomPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		EEPRom(const EEPRom &);
		EEPRom &operator=(const EEPRom &);

	public:
		/**
		 * Clear the EEPRom and initialize settings.
		 * This function does NOT reset the EEPRom type!
		 */
		void reset(void);

		/**
		 * Detect the EEPRom type used by the specified ROM.
		 * @param serial Serial number. (NOTE: This does NOT include the "GM " prefix!)
		 * @param serial_len Length of the serial number string.
		 * @param checksum Checksum.
		 * @return EEPRom type, or -1 if this ROM isn't known.
		 */
		static int DetectEEPRomType(const char *serial, size_t serial_len, uint16_t checksum);

		/**
		 * Set the EEPRom type.
		 * @param type EEPRom type. (Specify a negative number to clear)
		 * @return 0 on success; non-zero on error.
		 */
		int setEEPRomType(int type);

		/**
		 * Determine if the EEPRom type is set.
		 * @return True if the EEPRom type is set; false if not.
		 */
		bool isEEPRomTypeSet(void) const;

		/**
		 * Address verification functions.
		 *
		 * Notes:
		 *
		 * - Address 0 doesn't need to be checked, since the M68K memory handler
		 *   never checks EEPROM in the first bank (0x000000 - 0x07FFFF).
		 *
		 * - Word-wide addresses are checked by OR'ing both the specified address
		 *   and the preset address with 1.
		 *
		 * @param address Address.
		 * @return True if the address is usable for the specified purpose.
		 */
		bool isReadBytePort(uint32_t address) const;
		bool isReadWordPort(uint32_t address) const;
		bool isWriteBytePort(uint32_t address) const;
		bool isWriteWordPort(uint32_t address) const;

		/**
		 * Check if the EEPRom is dirty.
		 * @return True if EEPRom has been modified since the last save; false otherwise.
		 */
		bool isDirty(void) const;

		/** EEPRom access functions. **/

		uint8_t readByte(uint32_t address);
		uint16_t readWord(uint32_t address);

		void writeByte(uint32_t address, uint8_t data);
		void writeWord(uint32_t address, uint16_t data);

		// EEPRom filename and pathname.
		void setFilename(const std::string& filename);
		void setPathname(const std::string& pathname);

		/**
		 * Load the EEPRom file.
		 * @return Positive value indicating EEPRom size on success; negative on error.
		 */
		int load(void);

		/**
		 * Save the EEPRom file.
		 * @return Positive value indicating EEPRom size on success; 0 if no save is needed; negative on error.
		 */
		int save(void);

		/**
		 * Autosave the EEPRom file.
		 * This saves the EEPRom file if its last modification time is past a certain threshold.
		 * @param framesElapsed Number of frames elapsed, or -1 for paused. (force autosave)
		 * @return Positive value indicating SRam size on success; 0 if no save is needed; negative on error.
		 */
		int autoSave(int framesElapsed);

	protected:
		/** EEPRom file handling functions. **/
		int getUsedSize(void);
};

}

#endif /* __LIBGENS_SAVE_EEPROM_HPP__ */
