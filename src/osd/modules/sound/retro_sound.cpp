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
	sound_retro() :
		osd_module(OSD_SOUND_PROVIDER, "retro"), sound_module(),
		attenuation(0)
	{
	}
	virtual ~sound_retro() { }

	virtual int init(const osd_options &options)
	{
		set_mastervolume(attenuation);
		return 0;
	}

	virtual void exit() { }

	// sound_module

	virtual void update_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame)
	{
		if (attenuation)
			attenuate((int16_t *)buffer, samples_this_frame * sizeof(*buffer));

		retro_audio_queue(buffer, samples_this_frame * sizeof(*buffer));
	}

	virtual void set_mastervolume(int attenuation) override;

private:
	void attenuate(int16_t *data, int bytes);
	int attenuation;
};


//============================================================
//  Apply attenuation
//============================================================

void sound_retro::attenuate(int16_t *data, int bytes_to_copy)
{
	int level = (int) (pow(10.0, (double) attenuation / 20.0) * 128.0);
	int count = bytes_to_copy * sizeof(*data);
	while (count > 0)
	{
		*data = (*data * level) >> 7; /* / 128 */
		data++;
		count--;
	}
}

//============================================================
//  set_mastervolume
//============================================================

void sound_retro::set_mastervolume(int _attenuation)
{
	// clamp the attenuation to 0-32 range
	attenuation = std::clamp(_attenuation, -32, 0);
}

MODULE_DEFINITION(SOUND_RETRO, sound_retro)
