//============================================================
//
//  retro_sound.c - libretro implementation of MAME sound routines
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "sound_module.h"
#include "modules/osdmodule.h"

// MAME headers
#include "emu.h"
#include "emuopts.h"

#include "libretro-internal/libretro.h"

extern void retro_audio_queue(const int16_t *data, int32_t samples);

//============================================================
//  DEBUGGING
//============================================================

#define LOG_SOUND       0

//  CLASS
//============================================================

class sound_retro : public osd_module, public sound_module
{
public:
	sound_retro()
	: osd_module(OSD_SOUND_PROVIDER, "retro"), sound_module()
	{
	}
	virtual ~sound_retro() { }

	virtual int init(const osd_options &options) { return 0; }
	virtual void exit() { }

	// sound_module

	virtual void update_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame)
	{
		retro_audio_queue(buffer, samples_this_frame * 2);
	}

	virtual void set_mastervolume(int attenuation) {}
};

MODULE_DEFINITION(SOUND_RETRO, sound_retro)
