/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuMD.cpp: MD emulation code.                                           *
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

#include "EmuMD.hpp"

// VDP.
#include "Vdp/Vdp.hpp"

// CPU emulators.
#include "cpu/M68K.cpp"
#include "cpu/Z80.hpp"

// Byteswapping macros and functions.
#include "Util/byteswap.h"

// I/O devices.
#include "IO/IoBase.hpp"

// Sound Manager.
#include "sound/SoundMgr.hpp"

// LibGens OSD handler.
#include "lg_osd.h"

// C includes.
#include <stdio.h>
#include <math.h>

namespace LibGens
{

/**
 * EmuMD(): Initialize a Mega Drive context.
 * @param rom MD ROM.
 * @param region System region.
 */
EmuMD::EmuMD(Rom *rom, SysVersion::RegionCode_t region )
	: EmuContext(rom, region)
{
	// Load the ROM image.
	m_rom = rom;	// NOTE: This is already done in EmuContext::EmuContext()...
	if (!m_rom)
	{
		// NULL specified.
		// TODO: Set an error code.
		return;
	}
	
	// Check the ROM size.
	if ((rom->romSize() == 0) || (rom->romSize() > (int)sizeof(M68K_Mem::Rom_Data)))
	{
		// ROM is either empty or too big.
		// TODO: Set an error code.
		m_rom = NULL;
		return;
	}
	
	// Load the ROM into memory.
	M68K_Mem::Rom_Size = rom->romSize();
	size_t siz_loaded = rom->loadRom(&M68K_Mem::Rom_Data.u8[0], M68K_Mem::Rom_Size);
	if (siz_loaded != M68K_Mem::Rom_Size)
	{
		// Error loading the ROM.
		// TODO: Set an error code.
		m_rom = NULL;
		return;
	}
	
	// Byteswap the ROM data.
	be16_to_cpu_array(&M68K_Mem::Rom_Data.u8[0], M68K_Mem::Rom_Size);
	
	// Autofix the ROM checksum, if enabled.
	if (AutoFixChecksum())
		fixChecksum();
	
	// Initialize the VDP.
	Vdp::Reset();
	
	// Initialize the M68K.
	M68K::InitSys(M68K::SYSID_MD);
	
	// Reinitialize the Z80.
	// Z80's initial state is RESET.
	M68K_Mem::Z80_State = (Z80_STATE_ENABLED | Z80_STATE_RESET);	// TODO: "Sound, Z80" setting.
	Z80::ReInit();
	
	// Reset the controller ports.
	m_port1->reset();
	m_port2->reset();
	m_portE->reset();
	
	// Set the system version settings.
	M68K_Mem::ms_SysVersion.setDisk(false);		// No MCD connected.
	setRegion_int(region, false);			// Initialize region code.
	
	// Finished initializing.
	return;
}


EmuMD::~EmuMD()
{
	// TODO
}


/**
 * softReset(): Perform a soft reset.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::softReset(void)
{
	// ROM checksum:
	// - If autofix is enabled, fix the checksum.
	// - If autofix is disabled, restore the checksum.
	if (AutoFixChecksum())
		fixChecksum();
	else
		restoreChecksum();
	
	// Reset the M68K, Z80, and YM2612.
	M68K::Reset();
	Z80::Reset();
	SoundMgr::ms_Ym2612.reset();
	
	// Z80 state should be reset to the default value.
	// Z80's initial state is RESET.
	M68K_Mem::Z80_State = (Z80_STATE_ENABLED | Z80_STATE_RESET);	// TODO: "Sound, Z80" setting.
	
	// TODO: Genesis Plus randomizes the restart line.
	// See genesis.c:176.
	return 0;
}


/**
 * hardReset(): Perform a hard reset.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::hardReset(void)
{
	// Reset the controllers.
	m_port1->reset();
	m_port2->reset();
	m_portE->reset();
	
	// ROM checksum:
	// - If autofix is enabled, fix the checksum.
	// - If autofix is disabled, restore the checksum.
	if (AutoFixChecksum())
		fixChecksum();
	else
		restoreChecksum();
	
	// Hard-Reset the M68K, Z80, VDP, PSG, and YM2612.
	// This includes clearing RAM.
	M68K::InitSys(M68K::SYSID_MD);
	Z80::ReInit();
	Vdp::Reset();
	SoundMgr::ms_Psg.reset();
	SoundMgr::ms_Ym2612.reset();
	
	// Make sure the VDP's video mode bit is set properly.
	// TODO: Make a VdpIo inline function for this?
	if (M68K_Mem::ms_SysVersion.isPal())
		Vdp::SetRegion(true);	// PAL: Set the PAL bit.
	else
		Vdp::SetRegion(false);	// NTSC: Clear the PAL bit.
	
	// Reset successful.
	return 0;
}


/**
 * setRegion(): Set the region code.
 * @param region Region code.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::setRegion(SysVersion::RegionCode_t region)
	{ return setRegion_int(region, true); }

/**
 * setRegion_int(): Set the region code. (INTERNAL VERSION)
 * @param region Region code.
 * @param preserveState If true, preserve the audio IC state.
 * @return 0 on success; non-zero on error.
 */
int EmuMD::setRegion_int(SysVersion::RegionCode_t region, bool preserveState)
{
	SysVersion newRegion(region);
	if (preserveState && (M68K_Mem::ms_SysVersion.isPal() == newRegion.isPal()))
	{
		// preserveState was specified, and the current NTSC/PAL setting
		// matches the new NTSC/PAL setting. Don't reset anything.
		M68K_Mem::ms_SysVersion.setRegion(region);
		return 0;
	}
	
	// Set the region.
	M68K_Mem::ms_SysVersion.setRegion(region);
	
	// TODO: Vdp::VDP_Lines.Display.Total isn't being set properly...
	Vdp::VDP_Lines.Display.Total = (M68K_Mem::ms_SysVersion.isPal() ? 312 : 262);
	Vdp::Set_Visible_Lines();
	
	// Initialize CPL.
	if (M68K_Mem::ms_SysVersion.isPal())
	{
		M68K_Mem::CPL_M68K = (int)floor((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
		M68K_Mem::CPL_Z80 = (int)floor((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
		Vdp::SetRegion(true);	// PAL: Set the PAL bit.
	}
	else
	{
		M68K_Mem::CPL_M68K = (int)floor((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		M68K_Mem::CPL_Z80 = (int)floor((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		Vdp::SetRegion(false);	// NTSC: Clear the PAL bit.
	}
	
	// Initialize audio.
	// NOTE: Only set the region. Sound rate is set by the UI.
	SoundMgr::SetRegion(M68K_Mem::ms_SysVersion.isPal(), preserveState);
	
	// Region set successfully.
	return 0;
}


/**
 * T_execLine(): Run a scanline.
 * @param LineType Line type.
 * @param VDP If true, VDP is updated.
 */
template<EmuMD::LineType_t LineType, bool VDP>
FORCE_INLINE void EmuMD::T_execLine(void)
{
	int writePos = SoundMgr::GetWritePos(Vdp::VDP_Lines.Display.Current);
	int32_t *bufL = &SoundMgr::ms_SegBufL[writePos];
	int32_t *bufR = &SoundMgr::ms_SegBufR[writePos];
	
	// Update the sound chips.
	int writeLen = SoundMgr::GetWriteLen(Vdp::VDP_Lines.Display.Current);
	SoundMgr::ms_Ym2612.updateDacAndTimers(bufL, bufR, writeLen);
	SoundMgr::ms_Ym2612.addWriteLen(writeLen);
	SoundMgr::ms_Psg.addWriteLen(writeLen);
	
	// Notify controllers that a new scanline is being drawn.
	m_port1->doScanline();
	m_port2->doScanline();
	m_portE->doScanline();
	
	// Increment the cycles counter.
	// These values are the "last cycle to execute".
	// e.g. if Cycles_M68K is 5000, then we'll execute instructions
	// until the 68000's "odometer" reaches 5000.
	M68K_Mem::Cycles_M68K += M68K_Mem::CPL_M68K;
	M68K_Mem::Cycles_Z80 += M68K_Mem::CPL_Z80;
	
	if (Vdp::DMAT_Length)
		M68K::AddCycles(Vdp::Update_DMA());
	
	switch (LineType)
	{
		case LINETYPE_ACTIVEDISPLAY:
			// In visible area.
			Vdp::SetHBlank(true);	// HBlank = 1
			M68K::Exec(M68K_Mem::Cycles_M68K - 404);
			Vdp::SetHBlank(false);	// HBlank = 0
			
			if (--Vdp::HInt_Counter < 0)
			{
				Vdp::VDP_Int |= 0x4;
				Vdp::Update_IRQ_Line();
				Vdp::HInt_Counter = Vdp::VDP_Reg.m5.H_Int;
			}
			
			break;
		
		case LINETYPE_VBLANKLINE:
		{
			// VBlank line!
			if (--Vdp::HInt_Counter < 0)
			{
				Vdp::VDP_Int |= 0x4;
				Vdp::Update_IRQ_Line();
			}
			
#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_PRECHECK();
#endif
			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
			Vdp::SetHBlank(true);
			Vdp::SetVBlank(true);
			
			// If we're using NTSC V30 and this is an "even" frame,
			// don't set the VBlank flag.
			if (Vdp::VDP_Lines.NTSC_V30.VBlank_Div != 0)
				Vdp::SetVBlank(false);
			
			M68K::Exec(M68K_Mem::Cycles_M68K - 360);
			Z80::Exec(168);
#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_POSTCHECK();
#endif
			
			Vdp::SetHBlank(false);	// HBlank = 0
			if (Vdp::VDP_Lines.NTSC_V30.VBlank_Div == 0)
			{
				Vdp::SetVIntHappened(true);	// V Int happened
				
				Vdp::VDP_Int |= 0x8;
				Vdp::Update_IRQ_Line();
				
				// Z80 interrupt.
				// TODO: Does this trigger on all VBlanks,
				// or only if VINTs are enabled in the VDP?
				Z80::Interrupt(0xFF);
			}
			
			break;
		}
		
		case LINETYPE_BORDER:
		default:
			break;
	}
	
	if (VDP)
	{
		// VDP needs to be updated.
		Vdp::Render_Line();
	}
	
	M68K::Exec(M68K_Mem::Cycles_M68K);
	Z80::Exec(0);
}


/**
 * T_execFrame(): Run a frame.
 * @param VDP If true, VDP is updated.
 */
template<bool VDP>
FORCE_INLINE void EmuMD::T_execFrame(void)
{
	// Initialize VDP_Lines.Display.
	Vdp::Set_Visible_Lines();
	
	// Check if VBlank is allowed.
	Vdp::Check_NTSC_V30_VBlank();
	
	// Update I/O devices.
	// TODO: Determine the best place for the I/O devices to be updated:
	// - Beginning of frame.
	// - Before VBlank.
	// - End of frame.
	DevManager::Update();	// Update the Device Manager first.
	m_port1->update();
	m_port2->update();
	m_portE->update();
	
	// Reset the sound chip buffer pointers and write length.
	SoundMgr::ResetPtrsAndLens();
	
	// Clear all of the cycle counters.
	M68K_Mem::Cycles_M68K = 0;
	M68K_Mem::Cycles_Z80 = 0;
	M68K_Mem::Last_BUS_REQ_Cnt = -1000;
	M68K::TripOdometer();
	Z80::ClearOdometer();
	
	// If the full palette is dirty, force a CRam update.
	if (Vdp::m_palette.isDirty())
		Vdp::MarkCRamDirty();
	
	// TODO: MDP. (LibGens)
#if 0
	// Raise the MDP_EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP_EVENT_PRE_FRAME, NULL);
#endif
	
	// Set the VRam flag to force a VRam update.
	Vdp::MarkVRamDirty();
	
	// Interlaced frame status.
	// Both Interlaced Modes 1 and 2 set this bit on odd frames.
	// This bit is cleared on even frames and if not running in interlaced mode.
	if (Vdp::VDP_Reg.m5.Set4 & 0x06)
		Vdp::ToggleOddLine();
	else
		Vdp::ClearOddLine();
	
	/** Main execution loops. **/
	
	/** Loop 0: Top border. **/
	/** NOTE: Vdp::VDP_Lines.Visible.Current may initially be 0! (NTSC V30) **/
	Vdp::VDP_Lines.Display.Current = 0;
	while (Vdp::VDP_Lines.Visible.Current < 0)
	{
		T_execLine<LINETYPE_BORDER, VDP>();
		
		// Next line.
		Vdp::VDP_Lines.Display.Current++;
		Vdp::VDP_Lines.Visible.Current++;
	}
	
	/** Visible line 0. **/
	Vdp::HInt_Counter = Vdp::VDP_Reg.m5.H_Int;	// Initialize HInt_Counter.
	Vdp::SetVBlank(false);				// Clear VBlank status.
	
	/** Loop 1: Active display. **/
	do
	{
		T_execLine<LINETYPE_ACTIVEDISPLAY, VDP>();
		
		// Next line.
		Vdp::VDP_Lines.Display.Current++;
		Vdp::VDP_Lines.Visible.Current++;
	} while (Vdp::VDP_Lines.Visible.Current < Vdp::VDP_Lines.Visible.Total);
	
	/** Loop 2: VBlank line. **/
	T_execLine<LINETYPE_VBLANKLINE, VDP>();
	Vdp::VDP_Lines.Display.Current++;
	Vdp::VDP_Lines.Visible.Current++;
	
	/** Loop 3: Bottom border. **/
	do
	{
		T_execLine<LINETYPE_BORDER, VDP>();
		
		// Next line.
		Vdp::VDP_Lines.Display.Current++;
		Vdp::VDP_Lines.Visible.Current++;
	} while (Vdp::VDP_Lines.Display.Current < Vdp::VDP_Lines.Display.Total);
	
	// Update the PSG and YM2612 output.
	SoundMgr::SpecialUpdate();
	
#if 0
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		wav_dump_update();
	if (GYM_Dumping)
		gym_dump_update(0, 0, 0);
#endif
	
	// TODO: MDP. (LibGens)
#if 0
	// Raise the MDP_EVENT_POST_FRAME event.
	mdp_event_post_frame_t post_frame;
	post_frame.width = vdp_getHPix();
	post_frame.height = VDP_Lines.Visible.Total;
	post_frame.pitch = 336;
	post_frame.bpp = bppMD;
	
	int screen_offset = (TAB336[VDP_Lines.Visible.Border_Size] + 8);
	if (post_frame.width < 320)
		screen_offset += ((320 - post_frame.width) / 2);
	
	if (bppMD == 32)
		post_frame.md_screen = &MD_Screen.u32[screen_offset];
	else
		post_frame.md_screen = &MD_Screen.u16[screen_offset];
	
	EventMgr::RaiseEvent(MDP_EVENT_POST_FRAME, &post_frame);
#endif
}

void EmuMD::execFrame(void)
{
	T_execFrame<true>();
}

void EmuMD::execFrameFast(void)
{
	T_execFrame<false>();
}

}
