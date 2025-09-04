
// only for oslog callback
#include <functional>

// standard includes
#if !defined(RETROMAME_WIN32)
#include <unistd.h>
#endif

// only for strconv.h
#if defined(RETROMAME_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

// MAME headers
#include "osdepend.h"
#include "emu.h"
#include "emuopts.h"
#include "main.h"
#include "strconv.h"
#include "corestr.h"

// OSD headers
#include "video.h"
#include "osdretro.h"
#include "modules/lib/osdlib.h"
#include "modules/diagnostics/diagnostics_module.h"

bool fexists(std::string path)
{
    return osd_stat(path) ? true : false;
}

extern bool get_MgamePath(void);

//============================================================
// retro_output
//============================================================

class retro_output : public osd_output
{
public:
	virtual void output_callback(osd_output_channel channel, const util::format_argument_pack<char> &args) override
	{
		std::ostringstream buffer;
		retro_log_level lvl;

		util::stream_format(buffer, args);
		
		switch (channel)
		{
			case OSD_OUTPUT_CHANNEL_ERROR:
				lvl = RETRO_LOG_ERROR;
				break;
			default:
			case OSD_OUTPUT_CHANNEL_INFO:
			case OSD_OUTPUT_CHANNEL_LOG:
			case OSD_OUTPUT_CHANNEL_COUNT:
				lvl = RETRO_LOG_INFO;
				break;
			case OSD_OUTPUT_CHANNEL_WARNING:
				lvl = RETRO_LOG_WARN;
				break;
			case OSD_OUTPUT_CHANNEL_DEBUG:
			case OSD_OUTPUT_CHANNEL_VERBOSE:
				lvl = RETRO_LOG_DEBUG;
				break;	
		}

		log_cb(lvl, buffer.str().c_str());
		
		/* Log errors also to console */
		if (lvl == RETRO_LOG_ERROR)
			printf("%s", buffer.str().c_str());
	}
};

//============================================================
//  OPTIONS
//============================================================

#ifndef INI_PATH
#if defined(RETROMAME_WIN32)
	#define INI_PATH ""
#elif defined(RETROMAME_MACOSX)
	#define INI_PATH "$HOME/Library/Application Support/APP_NAME;$HOME/.APP_NAME"
#else
	#define INI_PATH "$HOME/.APP_NAME"
#endif // MACOSX
#endif // INI_PATH


//============================================================
//  Global variables
//============================================================

#if defined(RETROMAME_UNIX) || defined(RETROMAME_WIN32)
int retro_entered_debugger;
#endif

//============================================================
//  Local variables
//============================================================

const options_entry retro_options::s_option_entries[] =
{
	{ RETROOPTION_INIPATH,                  INI_PATH,       core_options::option_type::STRING,  "path to ini files" },

	// video options
	{ nullptr,                              nullptr,        core_options::option_type::HEADER,  "SDL VIDEO OPTIONS" },
	{ RETROOPTION_CENTERH,                  "1",            core_options::option_type::BOOLEAN, "center horizontally within the view area" },
	{ RETROOPTION_CENTERV,                  "1",            core_options::option_type::BOOLEAN, "center vertically within the view area" },
	{ RETROOPTION_SCALEMODE ";sm",          OSDOPTVAL_NONE, core_options::option_type::STRING,  "scale mode: none, hwblit, hwbest, yv12, yuy2, yv12x2, yuy2x2 (-video soft only)" },

	// joystick mapping
	{ nullptr,                              nullptr,        core_options::option_type::HEADER,  "SDL JOYSTICK MAPPING" },
	{ RETROOPTION_JOYINDEX "1",             OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of joystick mapped to joystick #1" },
	{ RETROOPTION_JOYINDEX "2",             OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of joystick mapped to joystick #2" },
	{ RETROOPTION_JOYINDEX "3",             OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of joystick mapped to joystick #3" },
	{ RETROOPTION_JOYINDEX "4",             OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of joystick mapped to joystick #4" },
	{ RETROOPTION_JOYINDEX "5",             OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of joystick mapped to joystick #5" },
	{ RETROOPTION_JOYINDEX "6",             OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of joystick mapped to joystick #6" },
	{ RETROOPTION_JOYINDEX "7",             OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of joystick mapped to joystick #7" },
	{ RETROOPTION_JOYINDEX "8",             OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of joystick mapped to joystick #8" },
	{ RETROOPTION_SIXAXIS,                  "0",            core_options::option_type::BOOLEAN, "Use special handling for PS3 Sixaxis controllers" },

#if (USE_XINPUT)
	// lightgun mapping
	{ nullptr,                              nullptr,        core_options::option_type::HEADER,  "SDL LIGHTGUN MAPPING" },
	{ RETROOPTION_LIGHTGUNINDEX "1",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of lightgun mapped to lightgun #1" },
	{ RETROOPTION_LIGHTGUNINDEX "2",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of lightgun mapped to lightgun #2" },
	{ RETROOPTION_LIGHTGUNINDEX "3",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of lightgun mapped to lightgun #3" },
	{ RETROOPTION_LIGHTGUNINDEX "4",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of lightgun mapped to lightgun #4" },
	{ RETROOPTION_LIGHTGUNINDEX "5",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of lightgun mapped to lightgun #5" },
	{ RETROOPTION_LIGHTGUNINDEX "6",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of lightgun mapped to lightgun #6" },
	{ RETROOPTION_LIGHTGUNINDEX "7",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of lightgun mapped to lightgun #7" },
	{ RETROOPTION_LIGHTGUNINDEX "8",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of lightgun mapped to lightgun #8" },
#endif

	{ nullptr,                              nullptr,        core_options::option_type::HEADER,  "SDL MOUSE MAPPING" },
	{ RETROOPTION_MOUSEINDEX "1",           OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of mouse mapped to mouse #1" },
	{ RETROOPTION_MOUSEINDEX "2",           OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of mouse mapped to mouse #2" },
	{ RETROOPTION_MOUSEINDEX "3",           OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of mouse mapped to mouse #3" },
	{ RETROOPTION_MOUSEINDEX "4",           OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of mouse mapped to mouse #4" },
	{ RETROOPTION_MOUSEINDEX "5",           OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of mouse mapped to mouse #5" },
	{ RETROOPTION_MOUSEINDEX "6",           OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of mouse mapped to mouse #6" },
	{ RETROOPTION_MOUSEINDEX "7",           OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of mouse mapped to mouse #7" },
	{ RETROOPTION_MOUSEINDEX "8",           OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of mouse mapped to mouse #8" },

	{ nullptr,                              nullptr,        core_options::option_type::HEADER,  "SDL KEYBOARD MAPPING" },
	{ RETROOPTION_KEYBINDEX "1",            OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of keyboard mapped to keyboard #1" },
	{ RETROOPTION_KEYBINDEX "2",            OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of keyboard mapped to keyboard #2" },
	{ RETROOPTION_KEYBINDEX "3",            OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of keyboard mapped to keyboard #3" },
	{ RETROOPTION_KEYBINDEX "4",            OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of keyboard mapped to keyboard #4" },
	{ RETROOPTION_KEYBINDEX "5",            OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of keyboard mapped to keyboard #5" },
	{ RETROOPTION_KEYBINDEX "6",            OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of keyboard mapped to keyboard #6" },
	{ RETROOPTION_KEYBINDEX "7",            OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of keyboard mapped to keyboard #7" },
	{ RETROOPTION_KEYBINDEX "8",            OSDOPTVAL_AUTO, core_options::option_type::STRING,  "name of keyboard mapped to keyboard #8" },

#if 0
	// SDL low level driver options
	{ nullptr,                              nullptr,        core_options::option_type::HEADER,  "SDL LOWLEVEL DRIVER OPTIONS" },
	{ RETROOPTION_VIDEODRIVER ";vd",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "sdl video driver to use ('x11', 'directfb', ... or 'auto' for SDL default" },
	{ RETROOPTION_RENDERDRIVER ";rd",       OSDOPTVAL_AUTO, core_options::option_type::STRING,  "sdl render driver to use ('software', 'opengl', 'directfb' ... or 'auto' for SDL default" },
	{ RETROOPTION_AUDIODRIVER ";ad",        OSDOPTVAL_AUTO, core_options::option_type::STRING,  "sdl audio driver to use ('alsa', 'arts', ... or 'auto' for SDL default" },
#endif

	// End of list
	{ nullptr }
};

//============================================================
//  retro_options
//============================================================

retro_options::retro_options() : osd_options()
{
	std::string ini_path(INI_PATH);
	add_entries(retro_options::s_option_entries);

#if defined(RETROMAME_WIN32)
	/* Pretend a home path for Windows */
	const char *userprofile = getenv("USERPROFILE");
	if (userprofile)
	{
		ini_path.append(userprofile);
		ini_path.append(slash_str);
		ini_path.append(".APP_NAME");
	}
#endif

	strreplace(ini_path, "APP_NAME", emulator_info::get_appname_lower());

	/* Add libretro system path as failsafe */
	ini_path.append(";");
	ini_path.append(retro_system_directory);
	ini_path.append(slash_str);
	ini_path.append(emulator_info::get_appname_lower());
	ini_path.append(slash_str);
	ini_path.append("ini");

	set_default_value(RETROOPTION_INIPATH, ini_path.c_str());
}

//============================================================
//  main
//============================================================

retro_osd_interface *retro_global_osd;

// translated to utf8_main
int mmain(int argc, char *argv[])
{
	std::vector<std::string> args(argv, argv+argc);
	int res = 0;

	// disable I/O buffering
	setvbuf(stdout, (char *) nullptr, _IONBF, 0);
	setvbuf(stderr, (char *) nullptr, _IONBF, 0);

#if 0
	// Initialize crash diagnostics
	diagnostics_module::get_instance()->init_crash_diagnostics();
#endif

#if defined(SDLMAME_ANDROID) && !defined(__LIBRETRO__)
	/* Enable standard application logging */
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
#endif

	// FIXME: this should be done differently

#ifdef RETROMAME_UNIX
	retro_entered_debugger = 0;
#if (!defined(SDLMAME_MACOSX)) && (!defined(SDLMAME_HAIKU)) && (!defined(SDLMAME_EMSCRIPTEN)) && (!defined(SDLMAME_ANDROID)) && (!defined(RETROMAME))
	FcInit();
#endif
#endif

	{
		static retro_options retro_global_options;
		retro_global_osd = new retro_osd_interface(retro_global_options);

		retro_output retrooutput;
		osd_output::push(&retrooutput);

		retro_global_osd->register_options();

		res = emulator_info::start_frontend(retro_global_options, *retro_global_osd, args);

		osd_output::pop(&retrooutput);

		return res;
	}

#ifdef RETROMAME_UNIX
#if (!defined(SDLMAME_MACOSX)) && (!defined(SDLMAME_HAIKU)) && (!defined(SDLMAME_EMSCRIPTEN)) && (!defined(SDLMAME_ANDROID)) && (!defined(RETROMAME))
	if (!retro_entered_debugger)
	{
		FcFini();
	}
#endif
#endif

	exit(res);
}

//============================================================
//  constructor
//============================================================

retro_osd_interface::retro_osd_interface(retro_options &options) :
	osd_common_t(options),
	m_options(options),
	m_last_click_time(std::chrono::steady_clock::time_point::min()),
	m_last_click_x(0),
	m_last_click_y(0)
{
}


//============================================================
//  destructor
//============================================================

retro_osd_interface::~retro_osd_interface()
{
}


//============================================================
//  osd_exit
//============================================================

void retro_osd_interface::osd_exit()
{
	osd_common_t::osd_exit();
}


//============================================================
//  output_oslog
//============================================================

void retro_osd_interface::output_oslog(const char *buffer)
{
	fputs(buffer, stderr);
}

//============================================================
//  osd_setup_osd_specific_emu_options
//============================================================

void osd_setup_osd_specific_emu_options(emu_options &opts)
{
	opts.add_entries(osd_options::s_option_entries);
}

//============================================================
//  init
//============================================================

void retro_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);

#if 0
	const char *stemp;

	/* get number of processors */
	stemp = options().numprocessors();
#endif

	osd_num_processors = 0;

	osd_common_t::init_subsystems();

	if (options().oslog())
	{
		using namespace std::placeholders;
		machine.add_logerror_callback(std::bind(&retro_osd_interface::output_oslog, this, _1));
	}
}


//============================================================
//  customize_input_type_list
//============================================================
void retro_osd_interface::customize_input_type_list(std::vector<input_type_entry> &typelist)
{
	// This function is called on startup, before reading the
	// configuration from disk. Scan the list, and change the
	// default control mappings you want. It is quite possible
	// you won't need to change a thing.

	// loop over the defaults
	for (input_type_entry &entry : typelist)
		switch (entry.type())
		{
			// Replace default mouse button order from "1 3 2" to "1 2 3"
			case IPT_BUTTON2:
				switch (entry.player())
				{
					case 0:
						entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_LALT, input_seq::or_code, MOUSECODE_BUTTON2_INDEXED(0), input_seq::or_code, GUNCODE_BUTTON2_INDEXED(0));
						break;
					case 1:
						entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_S, input_seq::or_code, MOUSECODE_BUTTON2_INDEXED(1), input_seq::or_code, GUNCODE_BUTTON2_INDEXED(1));
						break;
				}
				break;
			case IPT_BUTTON3:
				switch (entry.player())
				{
					case 0:
						entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_SPACE, input_seq::or_code, MOUSECODE_BUTTON3_INDEXED(0));
						break;
					case 1:
						entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_Q, input_seq::or_code, MOUSECODE_BUTTON3_INDEXED(1));
						break;
				}
				break;

			// Add default lightgun port 1+2 Start+Select to global "other" controls
			case IPT_START1:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_1, input_seq::or_code, JOYCODE_START_INDEXED(0), input_seq::or_code, GUNCODE_BUTTON5_INDEXED(0));
				break;
			case IPT_START2:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_2, input_seq::or_code, JOYCODE_START_INDEXED(1), input_seq::or_code, GUNCODE_BUTTON5_INDEXED(1));
				break;
			case IPT_COIN1:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_5, input_seq::or_code, JOYCODE_SELECT_INDEXED(0), input_seq::or_code, GUNCODE_BUTTON6_INDEXED(0));
				break;
			case IPT_COIN2:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_6, input_seq::or_code, JOYCODE_SELECT_INDEXED(1), input_seq::or_code, GUNCODE_BUTTON6_INDEXED(1));
				break;

			// Select + X
			case IPT_UI_MENU:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_TAB, input_seq::or_code, JOYCODE_SELECT, JOYCODE_BUTTON3);
				break;

			// Select + Start
			case IPT_UI_CANCEL:
				entry.defseq(SEQ_TYPE_STANDARD).set(KEYCODE_ESC, input_seq::or_code, JOYCODE_SELECT, JOYCODE_START);
				break;

			// leave everything else alone
			default:
				break;
		}

}
