#ifndef _osdretro_h_
#define _osdretro_h_

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"
#include "modules/font/font_module.h"
#include <chrono>

#include "libretro-internal/libretro_shared.h"

#define SOUND_DISABLE_THREADING

//============================================================
//  Defines
//============================================================

#define RETROOPTION_INIPATH               "inipath"

#define RETROOPTION_CENTERH               "centerh"
#define RETROOPTION_CENTERV               "centerv"

#define RETROOPTION_SCALEMODE             "scalemode"

#define RETROOPTION_SIXAXIS               "sixaxis"
#define RETROOPTION_JOYINDEX              "joy_idx"
#define RETROOPTION_KEYBINDEX             "keyb_idx"
#define RETROOPTION_MOUSEINDEX            "mouse_index"
#if (USE_XINPUT)
#define RETROOPTION_LIGHTGUNINDEX         "lightgun_index"
#endif

#define RETROOPTION_AUDIODRIVER           "audiodriver"
#define RETROOPTION_VIDEODRIVER           "videodriver"
#define RETROOPTION_RENDERDRIVER          "renderdriver"

#define RETROOPTVAL_SOFT                  "soft"

#define RETROMAME_LED(x)                  "led" #x


//============================================================
//  TYPE DEFINITIONS
//============================================================

class retro_options : public osd_options
{
public:
	// construction/destruction
	retro_options();

	// video options
	bool centerh() const { return bool_value(RETROOPTION_CENTERH); }
	bool centerv() const { return bool_value(RETROOPTION_CENTERV); }
	const char *scale_mode() const { return value(RETROOPTION_SCALEMODE); }

	// joystick mapping
	const char *joy_index(int index) const { return value(util::string_format("%s%d", RETROOPTION_JOYINDEX, index).c_str()); }
	bool sixaxis() const { return bool_value(RETROOPTION_SIXAXIS); }

	const char *mouse_index(int index) const { return value(util::string_format("%s%d", RETROOPTION_MOUSEINDEX, index).c_str()); }
	const char *keyboard_index(int index) const { return value(util::string_format("%s%d", RETROOPTION_KEYBINDEX, index).c_str()); }

	const char *video_driver() const { return value(RETROOPTION_VIDEODRIVER); }
	const char *render_driver() const { return value(RETROOPTION_RENDERDRIVER); }
	const char *audio_driver() const { return value(RETROOPTION_AUDIODRIVER); }

private:
	static const options_entry s_option_entries[];
};


class retro_osd_interface : public osd_common_t
{
public:
	// construction/destruction
	retro_osd_interface(retro_options &options);
	virtual ~retro_osd_interface();

	// general overridables
	virtual void init(running_machine &machine) override;
	virtual void update(bool skip_redraw) override;
	virtual void input_update(bool relative_reset) override;
	virtual void check_osd_inputs() override;

	// input overridables
	virtual void customize_input_type_list(std::vector<input_type_entry> &typelist) override;

	virtual bool video_init() override;
	virtual bool window_init() override;

	virtual void video_exit() override;
	virtual void window_exit() override;

	// retro specific
	void poll_inputs(running_machine &machine);
 	void process_mouse_state(running_machine &machine);
	void process_keyboard_state(running_machine &machine);
	void process_joystick_state(running_machine &machine);
	void process_lightgun_state(running_machine &machine);
	void retro_push_char(running_machine &machine, int retro_key_name);

	virtual bool has_focus() const override { return true; }
	void release_keys();
	bool should_hide_mouse();
	void process_events_buf();

	virtual retro_options &options() override { return m_options; }

	virtual void process_events() override {}

private:
	virtual void osd_exit() override;

	void extract_video_config();
	void output_oslog(const char *buffer);

	retro_options &m_options;

	std::chrono::steady_clock::time_point m_last_click_time;
	int m_last_click_x;
	int m_last_click_y;
};

//============================================================
//  work.c
//============================================================

extern int osd_num_processors;

#endif
