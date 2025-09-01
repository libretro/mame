//============================================================
//
//  input_retro.cpp - retro input implementation
//
//============================================================

#include "input_module.h"
#include "modules/osdmodule.h"
#include "assignmenthelper.h"

// emu
#include "inpttype.h"

// MAME headers
#include "emu.h"
#include "uiinput.h"
#include "corestr.h"

#include "input_common.h"
#include "input_retro.h"

#include "libretro/osdretro.h"
#include "libretro/window.h"
#include "libretro/libretro-internal/libretro.h"
#include "libretro/libretro-internal/libretro_shared.h"

extern bool libretro_supports_bitmasks;

static unsigned short retro_key_state[RETROK_LAST] = {0};
static unsigned short retro_key_event_state[RETROK_LAST] = {0};
static bool retro_key_capslock = false;

joystickstate_t joystickstate[RETRO_MAX_PLAYERS];
mousestate_t mousestate[RETRO_MAX_PLAYERS];
lightgunstate_t lightgunstate[RETRO_MAX_PLAYERS];

uint8_t mouse_count = 0;
uint8_t joystick_count = 0;
uint8_t lightgun_count = 0;

keyboard_table_t const keyboard_table[] =
{
   {"A",         RETROK_a,            ITEM_ID_A},
   {"B",         RETROK_b,            ITEM_ID_B},
   {"C",         RETROK_c,            ITEM_ID_C},
   {"D",         RETROK_d,            ITEM_ID_D},
   {"E",         RETROK_e,            ITEM_ID_E},
   {"F",         RETROK_f,            ITEM_ID_F},
   {"G",         RETROK_g,            ITEM_ID_G},
   {"H",         RETROK_h,            ITEM_ID_H},
   {"I",         RETROK_i,            ITEM_ID_I},
   {"J",         RETROK_j,            ITEM_ID_J},
   {"K",         RETROK_k,            ITEM_ID_K},
   {"L",         RETROK_l,            ITEM_ID_L},
   {"M",         RETROK_m,            ITEM_ID_M},
   {"N",         RETROK_n,            ITEM_ID_N},
   {"O",         RETROK_o,            ITEM_ID_O},
   {"P",         RETROK_p,            ITEM_ID_P},
   {"Q",         RETROK_q,            ITEM_ID_Q},
   {"R",         RETROK_r,            ITEM_ID_R},
   {"S",         RETROK_s,            ITEM_ID_S},
   {"T",         RETROK_t,            ITEM_ID_T},
   {"U",         RETROK_u,            ITEM_ID_U},
   {"V",         RETROK_v,            ITEM_ID_V},
   {"W",         RETROK_w,            ITEM_ID_W},
   {"X",         RETROK_x,            ITEM_ID_X},
   {"Y",         RETROK_y,            ITEM_ID_Y},
   {"Z",         RETROK_z,            ITEM_ID_Z},
   {"0",         RETROK_0,            ITEM_ID_0},
   {"1",         RETROK_1,            ITEM_ID_1},
   {"2",         RETROK_2,            ITEM_ID_2},
   {"3",         RETROK_3,            ITEM_ID_3},
   {"4",         RETROK_4,            ITEM_ID_4},
   {"5",         RETROK_5,            ITEM_ID_5},
   {"6",         RETROK_6,            ITEM_ID_6},
   {"7",         RETROK_7,            ITEM_ID_7},
   {"8",         RETROK_8,            ITEM_ID_8},
   {"9",         RETROK_9,            ITEM_ID_9},
   {"F1",        RETROK_F1,           ITEM_ID_F1},
   {"F2",        RETROK_F2,           ITEM_ID_F2},
   {"F3",        RETROK_F3,           ITEM_ID_F3},
   {"F4",        RETROK_F4,           ITEM_ID_F4},
   {"F5",        RETROK_F5,           ITEM_ID_F5},
   {"F6",        RETROK_F6,           ITEM_ID_F6},
   {"F7",        RETROK_F7,           ITEM_ID_F7},
   {"F8",        RETROK_F8,           ITEM_ID_F8},
   {"F9",        RETROK_F9,           ITEM_ID_F9},
   {"F10",       RETROK_F10,          ITEM_ID_F10},
   {"F11",       RETROK_F11,          ITEM_ID_F11},
   {"F12",       RETROK_F12,          ITEM_ID_F12},
   {"F13",       RETROK_F13,          ITEM_ID_F13},
   {"F14",       RETROK_F14,          ITEM_ID_F14},
   {"F15",       RETROK_F15,          ITEM_ID_F15},
   {"ESC",       RETROK_ESCAPE,       ITEM_ID_ESC},
   {"TILDE",     RETROK_BACKQUOTE,    ITEM_ID_TILDE},
   {"MINUS",     RETROK_MINUS,        ITEM_ID_MINUS},
   {"EQUALS",    RETROK_EQUALS,       ITEM_ID_EQUALS},
   {"BACKSPACE", RETROK_BACKSPACE,    ITEM_ID_BACKSPACE},
   {"TAB",       RETROK_TAB,          ITEM_ID_TAB},
   {"[",         RETROK_LEFTBRACKET,  ITEM_ID_OPENBRACE},
   {"]",         RETROK_RIGHTBRACKET, ITEM_ID_CLOSEBRACE},
   {"ENTER",     RETROK_RETURN,       ITEM_ID_ENTER},
   {";",         RETROK_SEMICOLON,    ITEM_ID_COLON},
   {"\'",        RETROK_QUOTE,        ITEM_ID_QUOTE},
   {"\\",        RETROK_BACKSLASH,    ITEM_ID_BACKSLASH},
   {"|",         RETROK_OEM_102,      ITEM_ID_BACKSLASH2},
   {",",         RETROK_COMMA,        ITEM_ID_COMMA},
   {".",         RETROK_PERIOD,       ITEM_ID_STOP},
   {"/",         RETROK_SLASH,        ITEM_ID_SLASH},
   {"SPACE",     RETROK_SPACE,        ITEM_ID_SPACE},
   {"INS",       RETROK_INSERT,       ITEM_ID_INSERT},
   {"DEL",       RETROK_DELETE,       ITEM_ID_DEL},
   {"HOME",      RETROK_HOME,         ITEM_ID_HOME},
   {"END",       RETROK_END,          ITEM_ID_END},
   {"PGUP",      RETROK_PAGEUP,       ITEM_ID_PGUP},
   {"PGDW",      RETROK_PAGEDOWN,     ITEM_ID_PGDN},
   {"LEFT",      RETROK_LEFT,         ITEM_ID_LEFT},
   {"RIGHT",     RETROK_RIGHT,        ITEM_ID_RIGHT},
   {"UP",        RETROK_UP,           ITEM_ID_UP},
   {"DOWN",      RETROK_DOWN,         ITEM_ID_DOWN},
   {"KPO",       RETROK_KP0,          ITEM_ID_0_PAD},
   {"KP1",       RETROK_KP1,          ITEM_ID_1_PAD},
   {"KP2",       RETROK_KP2,          ITEM_ID_2_PAD},
   {"KP3",       RETROK_KP3,          ITEM_ID_3_PAD},
   {"KP4",       RETROK_KP4,          ITEM_ID_4_PAD},
   {"KP5",       RETROK_KP5,          ITEM_ID_5_PAD},
   {"KP6",       RETROK_KP6,          ITEM_ID_6_PAD},
   {"KP7",       RETROK_KP7,          ITEM_ID_7_PAD},
   {"KP8",       RETROK_KP8,          ITEM_ID_8_PAD},
   {"KP9",       RETROK_KP9,          ITEM_ID_9_PAD},
   {"KP/",       RETROK_KP_DIVIDE,    ITEM_ID_SLASH_PAD},
   {"KP*",       RETROK_KP_MULTIPLY,  ITEM_ID_ASTERISK},
   {"KP-",       RETROK_KP_MINUS,     ITEM_ID_MINUS_PAD},
   {"KP+",       RETROK_KP_PLUS,      ITEM_ID_PLUS_PAD},
   {"KP.",       RETROK_KP_PERIOD,    ITEM_ID_DEL_PAD},
   {"KPENTER",   RETROK_KP_ENTER,     ITEM_ID_ENTER_PAD},
   {"PRTSCR",    RETROK_PRINT,        ITEM_ID_PRTSCR},
   {"PAUSE",     RETROK_PAUSE,        ITEM_ID_PAUSE},
   {"LSHIFT",    RETROK_LSHIFT,       ITEM_ID_LSHIFT},
   {"RSHIFT",    RETROK_RSHIFT,       ITEM_ID_RSHIFT},
   {"LCTRL",     RETROK_LCTRL,        ITEM_ID_LCONTROL},
   {"RCTRL",     RETROK_RCTRL,        ITEM_ID_RCONTROL},
   {"LALT",      RETROK_LALT,         ITEM_ID_LALT},
   {"RALT",      RETROK_RALT,         ITEM_ID_RALT},
   {"SCRLOCK",   RETROK_SCROLLOCK,    ITEM_ID_SCRLOCK},
   {"NUMLOCK",   RETROK_NUMLOCK,      ITEM_ID_NUMLOCK},
   {"CPSLOCK",   RETROK_CAPSLOCK,     ITEM_ID_CAPSLOCK},
   {"LSUPER",    RETROK_LSUPER,       ITEM_ID_LWIN},
   {"RSUPER",    RETROK_RSUPER,       ITEM_ID_RWIN},
   {"MENU",      RETROK_MENU,         ITEM_ID_MENU},
   {"BREAK",     RETROK_BREAK,        ITEM_ID_CANCEL},
   {"-1",        -1,                  ITEM_ID_INVALID},
};

const char *mouse_button_name[RETRO_MAX_MOUSE_BUTTONS] =
{
	"L",
	"R",
	"M",
	"4",
	"5",
	"Wheel Up",
	"Wheel Down",
	"Wheel Left",
	"Wheel Right"
};

const char *lightgun_button_name[RETRO_MAX_LIGHTGUN_BUTTONS] =
{
	"Trigger",
	"Aux A",
	"Aux B",
	"Aux C",
	"Start",
	"Select",
	"D-Pad Up",
	"D-Pad Down",
	"D-Pad Left",
	"D-Pad Right"
};

const char *joystick_button_name[RETRO_MAX_JOYSTICK_BUTTONS] =
{
	"B",
	"Y",
	"Select",
	"Start",
	"D-Pad Up",
	"D-Pad Down",
	"D-Pad Left",
	"D-Pad Right",
	"A",
	"X",
	"L1",
	"R1",
	"L2",
	"R2",
	"L3",
	"R3",
};

int button_mapping[] =
{
	RETROPAD_B,
	RETROPAD_A,
	RETROPAD_Y,
	RETROPAD_X,
	RETROPAD_L,
	RETROPAD_R
};

void Input_Binding(running_machine &machine)
{
   log_cb(RETRO_LOG_INFO, "SOURCE FILE: %s\n", machine.system().type.source());
   log_cb(RETRO_LOG_INFO, "PARENT: %s\n", machine.system().parent);
   log_cb(RETRO_LOG_INFO, "NAME: %s\n", machine.system().name);
   log_cb(RETRO_LOG_INFO, "DESCRIPTION: %s\n", machine.system().type.fullname());
   log_cb(RETRO_LOG_INFO, "YEAR: %s\n", machine.system().year);
   log_cb(RETRO_LOG_INFO, "MANUFACTURER: %s\n", machine.system().manufacturer);

   button_mapping[0] = RETROPAD_B;
   button_mapping[1] = RETROPAD_A;
   button_mapping[2] = RETROPAD_Y;
   button_mapping[3] = RETROPAD_X;
   button_mapping[4] = RETROPAD_L;
   button_mapping[5] = RETROPAD_R;

   if (
         !core_stricmp(machine.system().name, "avengrgs")    ||
         !core_stricmp(machine.system().parent, "avengrgs")  ||
         !core_stricmp(machine.system().name, "bloodwar")    ||
         !core_stricmp(machine.system().parent, "bloodwar")  ||
         !core_stricmp(machine.system().name, "daraku")    ||
         !core_stricmp(machine.system().parent, "daraku")  ||
         !core_stricmp(machine.system().name, "drgnmst")   ||
         !core_stricmp(machine.system().parent, "drgnmst")   ||
         !core_stricmp(machine.system().name, "primrage")   ||
         !core_stricmp(machine.system().parent, "primrage")   ||
         !core_stricmp(machine.system().name, "rabbit")    ||
         !core_stricmp(machine.system().parent, "rabbit")  ||
         !core_stricmp(machine.system().name, "shogwarr")   ||
         !core_stricmp(machine.system().parent, "shogwarr")   ||
         !core_stricmp(machine.system().name, "tekken")    ||
         !core_stricmp(machine.system().parent, "tekken")  ||
         !core_stricmp(machine.system().name, "tekken2")   ||
         !core_stricmp(machine.system().parent, "tekken2")   ||
         !core_stricmp(machine.system().name, "tkdensho")   ||
         !core_stricmp(machine.system().parent, "tkdensho")   ||
         !core_stricmp(machine.system().name, "vf")   ||
         !core_stricmp(machine.system().parent, "vf")
      )
   {
      /* Tekken 1/2/Virtua Fighter/Etc.*/
      button_mapping[0] = RETROPAD_Y;
      button_mapping[1] = RETROPAD_X;
      button_mapping[2] = RETROPAD_B;
      button_mapping[3] = RETROPAD_A;
      button_mapping[4] = RETROPAD_L;
      button_mapping[5] = RETROPAD_R;
   }
   else if (
              !core_stricmp(machine.system().name, "jojo")    ||
              !core_stricmp(machine.system().parent, "jojo")  ||
              !core_stricmp(machine.system().name, "jojoba")    ||
              !core_stricmp(machine.system().parent, "jojoba")  ||
              !core_stricmp(machine.system().name, "souledge")    ||
              !core_stricmp(machine.system().parent, "souledge")  ||
              !core_stricmp(machine.system().name, "soulclbr")    ||
              !core_stricmp(machine.system().parent, "soulclbr")    ||
              !core_stricmp(machine.system().name, "svg")    ||
              !core_stricmp(machine.system().parent, "svg")
           )
   {
      /* Soul Edge/Soul Calibur/JoJo/SVG */
      button_mapping[0] = RETROPAD_Y;
      button_mapping[1] = RETROPAD_X;
      button_mapping[2] = RETROPAD_A;
      button_mapping[3] = RETROPAD_B;
      button_mapping[4] = RETROPAD_L;
      button_mapping[5] = RETROPAD_R;
   }
   else if (
              !core_stricmp(machine.system().name, "doapp")
           )
   {
      /* Dead or Alive++ */
      button_mapping[0] = RETROPAD_B;
      button_mapping[1] = RETROPAD_Y;
      button_mapping[2] = RETROPAD_X;
      button_mapping[3] = RETROPAD_A;
      button_mapping[4] = RETROPAD_L;
      button_mapping[5] = RETROPAD_R;
   }
   else if (
              !core_stricmp(machine.system().name, "ehrgeiz") ||
              !core_stricmp(machine.system().parent, "ehrgeiz")
           )
   {
      /* Ehrgeiz */
      button_mapping[0] = RETROPAD_Y;
      button_mapping[1] = RETROPAD_B;
      button_mapping[2] = RETROPAD_A;
      button_mapping[3] = RETROPAD_X;
      button_mapping[4] = RETROPAD_L;
      button_mapping[5] = RETROPAD_R;
   }
   else if (
              !core_stricmp(machine.system().name, "ts2") ||
              !core_stricmp(machine.system().parent, "ts2")
           )
   {
      /* Toshinden 2 */
      button_mapping[0] = RETROPAD_L;
      button_mapping[1] = RETROPAD_Y;
      button_mapping[2] = RETROPAD_X;
      button_mapping[3] = RETROPAD_R;
      button_mapping[4] = RETROPAD_B;
      button_mapping[5] = RETROPAD_A;
   }
   else if (
              !core_stricmp(machine.system().name, "dstlk") ||
              !core_stricmp(machine.system().parent, "dstlk") ||
              !core_stricmp(machine.system().name, "hsf2") ||
              !core_stricmp(machine.system().parent, "hsf2") ||
              !core_stricmp(machine.system().name, "msh") ||
              !core_stricmp(machine.system().parent, "msh") ||
              !core_stricmp(machine.system().name, "mshvsf") ||
              !core_stricmp(machine.system().parent, "mshvsf") ||
              !core_stricmp(machine.system().name, "mvsc") ||
              !core_stricmp(machine.system().parent, "mvsc") ||
              !core_stricmp(machine.system().name, "nwarr") ||
              !core_stricmp(machine.system().parent, "nwarr") ||
              !core_stricmp(machine.system().name, "redearth") ||
              !core_stricmp(machine.system().parent, "redearth") ||
              !core_stricmp(machine.system().name, "rvschool") ||
              !core_stricmp(machine.system().parent, "rvschool") ||
              !core_stricmp(machine.system().name, "sf2") ||
              !core_stricmp(machine.system().parent, "sf2") ||
              !core_stricmp(machine.system().name, "sf2ce") ||
              !core_stricmp(machine.system().parent, "sf2ce") ||
              !core_stricmp(machine.system().name, "sf2hf") ||
              !core_stricmp(machine.system().parent, "sf2hf") ||
              !core_stricmp(machine.system().name, "sfa") ||
              !core_stricmp(machine.system().parent, "sfa") ||
              !core_stricmp(machine.system().name, "sfa2") ||
              !core_stricmp(machine.system().parent, "sfa2") ||
              !core_stricmp(machine.system().name, "sfa3") ||
              !core_stricmp(machine.system().parent, "sfa3") ||
              !core_stricmp(machine.system().name, "sfex") ||
              !core_stricmp(machine.system().parent, "sfex") ||
              !core_stricmp(machine.system().name, "sfex2") ||
              !core_stricmp(machine.system().parent, "sfex2") ||
              !core_stricmp(machine.system().name, "sfex2p") ||
              !core_stricmp(machine.system().parent, "sfex2p") ||
              !core_stricmp(machine.system().name, "sfexp") ||
              !core_stricmp(machine.system().parent, "sfexp") ||
              !core_stricmp(machine.system().name, "sfiii") ||
              !core_stricmp(machine.system().parent, "sfiii") ||
              !core_stricmp(machine.system().name, "sfiii2") ||
              !core_stricmp(machine.system().parent, "sfiii2") ||
              !core_stricmp(machine.system().name, "sfiii3") ||
              !core_stricmp(machine.system().parent, "sfiii3") ||
              !core_stricmp(machine.system().name, "sftm") ||
              !core_stricmp(machine.system().parent, "sftm") ||
              !core_stricmp(machine.system().name, "sfz2al") ||
              !core_stricmp(machine.system().parent, "sfz2al") ||
              !core_stricmp(machine.system().name, "sfzch") ||
              !core_stricmp(machine.system().parent, "sfzch") ||
              !core_stricmp(machine.system().name, "ssf2") ||
              !core_stricmp(machine.system().parent, "ssf2") ||
              !core_stricmp(machine.system().name, "ssf2t") ||
              !core_stricmp(machine.system().parent, "ssf2t") ||
              !core_stricmp(machine.system().name, "vhunt2") ||
              !core_stricmp(machine.system().parent, "vhunt2") ||
              !core_stricmp(machine.system().name, "vsav") ||
              !core_stricmp(machine.system().parent, "vsav") ||
              !core_stricmp(machine.system().name, "vsav2") ||
              !core_stricmp(machine.system().parent, "vsav2") ||
              !core_stricmp(machine.system().name, "xmcota") ||
              !core_stricmp(machine.system().parent, "xmcota") ||
              !core_stricmp(machine.system().name, "xmvsf") ||
              !core_stricmp(machine.system().parent, "xmvsf") ||
              !core_stricmp(machine.system().name, "astrass") ||
              !core_stricmp(machine.system().parent, "astrass") ||
              !core_stricmp(machine.system().name, "bestbest") ||
              !core_stricmp(machine.system().parent, "bestbest") ||
              !core_stricmp(machine.system().name, "brival") ||
              !core_stricmp(machine.system().parent, "brival") ||
              !core_stricmp(machine.system().name, "btlkroad") ||
              !core_stricmp(machine.system().parent, "btlkroad") ||
              !core_stricmp(machine.system().name, "dankuga") ||
              !core_stricmp(machine.system().parent, "dankuga") ||
              !core_stricmp(machine.system().name, "dragoona") ||
              !core_stricmp(machine.system().parent, "dragoona") ||
              !core_stricmp(machine.system().name, "ffreveng") ||
              !core_stricmp(machine.system().parent, "ffreveng") ||
              !core_stricmp(machine.system().name, "fghthist") ||
              !core_stricmp(machine.system().parent, "fghthist") ||
              !core_stricmp(machine.system().name, "fgtlayer") ||
              !core_stricmp(machine.system().parent, "fgtlayer") ||
              !core_stricmp(machine.system().name, "gaxeduel") ||
              !core_stricmp(machine.system().parent, "gaxeduel") ||
              !core_stricmp(machine.system().name, "groovef") ||
              !core_stricmp(machine.system().parent, "groovef") ||
              !core_stricmp(machine.system().name, "kaiserkn") ||
              !core_stricmp(machine.system().parent, "kaiserkn") ||
              !core_stricmp(machine.system().name, "kinst") ||
              !core_stricmp(machine.system().parent, "kinst") ||
              !core_stricmp(machine.system().name, "kinst2") ||
              !core_stricmp(machine.system().parent, "kinst2") ||
              !core_stricmp(machine.system().name, "suikoenb") ||
              !core_stricmp(machine.system().parent, "suikoenb") ||
              !core_stricmp(machine.system().name, "ssoldier") ||
              !core_stricmp(machine.system().parent, "ssoldier")
           )
   {
      /* 6-button fighting games (Mainly Capcom (CPS-1, CPS-2, CPS-3, ZN-1, ZN-2) + Others)*/
      button_mapping[0] = RETROPAD_Y;
      button_mapping[1] = RETROPAD_X;
      button_mapping[2] = RETROPAD_L;
      button_mapping[3] = RETROPAD_B;
      button_mapping[4] = RETROPAD_A;
      button_mapping[5] = RETROPAD_R;
   }
   else if (
              !core_stricmp(machine.system().parent, "aof") ||
              !core_stricmp(machine.system().parent, "aof2") ||
              !core_stricmp(machine.system().parent, "aof3") ||
              !core_stricmp(machine.system().parent, "breakers") ||
              !core_stricmp(machine.system().parent, "breakrev") ||
              !core_stricmp(machine.system().parent, "doubledr") ||
              !core_stricmp(machine.system().parent, "fatfury1") ||
              !core_stricmp(machine.system().parent, "fatfury2") ||
              !core_stricmp(machine.system().parent, "fatfury3") ||
              !core_stricmp(machine.system().parent, "fatfursp") ||
              !core_stricmp(machine.system().parent, "fightfev") ||
              !core_stricmp(machine.system().parent, "galaxyfg") ||
              !core_stricmp(machine.system().parent, "garou") ||
              !core_stricmp(machine.system().parent, "gowcaizr") ||
              !core_stricmp(machine.system().parent, "neogeo") ||
              !core_stricmp(machine.system().parent, "karnovr") ||
              !core_stricmp(machine.system().parent, "kizuna") ||
              !core_stricmp(machine.system().parent, "kabukikl") ||
              !core_stricmp(machine.system().parent, "matrim") ||
              !core_stricmp(machine.system().parent, "mslug") ||
              !core_stricmp(machine.system().parent, "mslug2") ||
              !core_stricmp(machine.system().parent, "mslugx") ||
              !core_stricmp(machine.system().parent, "mslug3") ||
              !core_stricmp(machine.system().parent, "mslug4") ||
              !core_stricmp(machine.system().parent, "mslug5") ||
              !core_stricmp(machine.system().parent, "kof94") ||
              !core_stricmp(machine.system().parent, "kof95") ||
              !core_stricmp(machine.system().parent, "kof96") ||
              !core_stricmp(machine.system().parent, "kof97") ||
              !core_stricmp(machine.system().parent, "kof98") ||
              !core_stricmp(machine.system().parent, "kof99") ||
              !core_stricmp(machine.system().parent, "kof2000") ||
              !core_stricmp(machine.system().parent, "kof2001") ||
              !core_stricmp(machine.system().parent, "kof2002") ||
              !core_stricmp(machine.system().parent, "kof2003") ||
              !core_stricmp(machine.system().parent, "lresort") ||
              !core_stricmp(machine.system().parent, "lastblad") ||
              !core_stricmp(machine.system().parent, "lastbld2") ||
              !core_stricmp(machine.system().parent, "ninjamas") ||
              !core_stricmp(machine.system().parent, "rotd") ||
              !core_stricmp(machine.system().parent, "rbff1") ||
              !core_stricmp(machine.system().parent, "rbff2") ||
              !core_stricmp(machine.system().parent, "rbffspec") ||
              !core_stricmp(machine.system().parent, "savagere") ||
              !core_stricmp(machine.system().parent, "sengoku3") ||
              !core_stricmp(machine.system().parent, "samsho") ||
              !core_stricmp(machine.system().parent, "samsho2") ||
              !core_stricmp(machine.system().parent, "samsho3") ||
              !core_stricmp(machine.system().parent, "samsho4") ||
              !core_stricmp(machine.system().parent, "samsho5") ||
              !core_stricmp(machine.system().parent, "samsh5sp") ||
              !core_stricmp(machine.system().parent, "svc") ||
              !core_stricmp(machine.system().parent, "viewpoin") ||
              !core_stricmp(machine.system().parent, "wakuwak7") ||
              !core_stricmp(machine.system().parent, "wh1") ||
              !core_stricmp(machine.system().parent, "wh2") ||
              !core_stricmp(machine.system().parent, "wh2j") ||
              !core_stricmp(machine.system().parent, "whp")
           )
   {
      /* Neo Geo */
      button_mapping[0] = RETROPAD_B;
      button_mapping[1] = RETROPAD_A;
      button_mapping[2] = RETROPAD_Y;
      button_mapping[3] = RETROPAD_X;
      button_mapping[4] = RETROPAD_L;
      button_mapping[5] = RETROPAD_R;
   }
   else if (
              !core_stricmp(machine.system().name, "tektagt")   ||
              !core_stricmp(machine.system().parent, "tektagt") ||
              !core_stricmp(machine.system().name, "tekken3")   ||
              !core_stricmp(machine.system().parent, "tekken3")
           )
   {
      /* Tekken 3/Tekken Tag Tournament */
      button_mapping[0] = RETROPAD_Y;
      button_mapping[1] = RETROPAD_X;
      button_mapping[2] = RETROPAD_R;
      button_mapping[3] = RETROPAD_B;
      button_mapping[4] = RETROPAD_A;
      button_mapping[5] = RETROPAD_L;
   }
   else if (
              !core_stricmp(machine.system().name, "mk")       ||
              !core_stricmp(machine.system().parent, "mk")     ||
              !core_stricmp(machine.system().name, "mk2")      ||
              !core_stricmp(machine.system().parent, "mk2")    ||
              !core_stricmp(machine.system().name, "mk3")      ||
              !core_stricmp(machine.system().parent, "mk3")    ||
              !core_stricmp(machine.system().name, "umk3")     ||
              !core_stricmp(machine.system().parent, "umk3")   ||
              !core_stricmp(machine.system().name, "wwfmania") ||
              !core_stricmp(machine.system().parent, "wwfmania")
           )
   {
      /* Mortal Kombat 1/2/3/Ultimate/WWF: Wrestlemania */
      button_mapping[0] = RETROPAD_Y;
      button_mapping[1] = RETROPAD_L;
      button_mapping[2] = RETROPAD_X;
      button_mapping[3] = RETROPAD_B;
      button_mapping[4] = RETROPAD_A;
      button_mapping[5] = RETROPAD_R;
   }
}

bool retro_osd_interface::should_hide_mouse()
{
	// if we are paused, no
	if (machine().paused())
		return false;

	// if neither mice nor lightguns are enabled in the core, then no
	if (!options().mouse() && !options().lightgun())
		return false;

	// otherwise, yes
	return true;
}

void retro_osd_interface::release_keys()
{
	auto const keybd = dynamic_cast<input_module_base*>(m_keyboard_input);
	if (keybd)
		keybd->reset_devices();
}

void retro_keyboard_event(bool down, unsigned code,
      uint32_t character, uint16_t mod)
{
   switch (code)
   {
      case RETROK_UNKNOWN:
      case RETROK_PAUSE:
         return;
   }

   retro_key_capslock = (down && code == RETROK_CAPSLOCK) || (mod & RETROKMOD_CAPSLOCK);
   retro_key_event_state[code] = down;
}

void retro_osd_interface::retro_push_char(running_machine &machine, int retro_key_name)
{
   char32_t push_char = retro_key_name;
   bool shifted = retro_key_capslock || retro_key_state[RETROK_LSHIFT] || retro_key_state[RETROK_RSHIFT];

   /* Case shift for letters */
   if (shifted)
   {
      if (push_char > 0x60 && push_char < 0x7B)
         push_char -= 0x20;
      else if (push_char > 0x30 && push_char < 0x3A)
         push_char -= 0x10;
   }

   /* Allow only practical chars and backspace */
   if ((push_char > 0x1F && push_char < 0x80) || push_char == 0x8)
      machine.ui_input().push_char_event(osd_common_t::s_window_list.front()->target(), push_char);
}

#define PUSH_CHAR_REPEAT_TRIGGER 10

void retro_osd_interface::process_keyboard_state(running_machine &machine)
{
   unsigned short i = 0;
   static unsigned char repeat = 0;
   static unsigned char repeat_trigger = PUSH_CHAR_REPEAT_TRIGGER;

   do
   {
      if (retro_key_event_state[keyboard_table[i].retro_key_name] && !retro_key_state[keyboard_table[i].retro_key_name])
      {
         retro_key_state[keyboard_table[i].retro_key_name] = 0x80;
         retro_push_char(machine, keyboard_table[i].retro_key_name);
         repeat_trigger = PUSH_CHAR_REPEAT_TRIGGER;
         repeat = 0;
      }
      else if (!retro_key_event_state[keyboard_table[i].retro_key_name] && retro_key_state[keyboard_table[i].retro_key_name])
      {
         retro_key_state[keyboard_table[i].retro_key_name] = 0;
         repeat_trigger = PUSH_CHAR_REPEAT_TRIGGER;
         repeat = 0;
      }
      else if (retro_key_event_state[keyboard_table[i].retro_key_name] && retro_key_state[keyboard_table[i].retro_key_name])
      {
         repeat++;
         if (repeat > repeat_trigger)
         {
            retro_push_char(machine, keyboard_table[i].retro_key_name);
            repeat = 0;
            if (repeat_trigger)
               repeat_trigger--;
         }
      }

      i++;
   } while (keyboard_table[i].retro_key_name != -1);
}

void retro_osd_interface::process_joystick_state(running_machine &machine)
{
   unsigned i, j;
   int analog_l2, analog_r2;
   int16_t ret[RETRO_MAX_PLAYERS];

   if (libretro_supports_bitmasks)
   {
      for (j = 0; j < RETRO_MAX_PLAYERS; j++)
         ret[j] = input_state_cb(j, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
   }
   else
   {
      for (j = 0; j < RETRO_MAX_PLAYERS; j++)
      {
         ret[j] = 0;
         for (i = 0; i < RETRO_MAX_JOYSTICK_BUTTONS; i++)
            if (input_state_cb(j, RETRO_DEVICE_JOYPAD, 0, i))
               ret[j] |= (1 << i);
      }
   }

   for (j = 0; j < RETRO_MAX_PLAYERS; j++)
   {
      for (i = 0; i < RETRO_MAX_JOYSTICK_BUTTONS; i++)
      {
         if (ret[j] & (1 << i))
            joystickstate[j].button[i] = 0x80;
         else
            joystickstate[j].button[i] = 0;
      }

      joystickstate[j].a1[0] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X)), -32767, 32767);
      joystickstate[j].a1[1] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y)), -32767, 32767);
      joystickstate[j].a2[0] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X)), -32767, 32767);
      joystickstate[j].a2[1] = normalize_absolute_axis((input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y)), -32767, 32767);

      analog_l2 = input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_BUTTON, RETRO_DEVICE_ID_JOYPAD_L2);
      analog_r2 = input_state_cb(j, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_BUTTON, RETRO_DEVICE_ID_JOYPAD_R2);

      /* Fallback, if no analog trigger support, use digital */
      if (analog_l2 == 0)
      {
         if (ret[j] & (1 << RETRO_DEVICE_ID_JOYPAD_L2))
            analog_l2 = 32767;
      }

      if (analog_r2 == 0)
      {
         if (ret[j] & (1 << RETRO_DEVICE_ID_JOYPAD_R2))
            analog_r2 = 32767;
      }

      joystickstate[j].a3[0] = -normalize_absolute_axis(analog_l2, -32767, 32767);
      joystickstate[j].a3[1] = -normalize_absolute_axis(analog_r2, -32767, 32767);
   }
}

void retro_osd_interface::process_mouse_state(running_machine &machine)
{
	unsigned i;
	auto &window    = osd_common_t::window_list().front();
	static int vmx  = fb_width/2, vmy  = fb_height/2;
	static int ovmx = fb_width/2, ovmy = fb_height/2;

	if (!mouse_enable)
		return;

	for (i = 0; i < RETRO_MAX_PLAYERS; i++)
	{
		int16_t mouse_x;
		int16_t mouse_y;
		bool mouse_l;
		bool mouse_r;
		bool mouse_m;
		bool mouse_4;
		bool mouse_5;
		bool mouse_wu;
		bool mouse_wd;
		bool mouse_wl;
		bool mouse_wr;

		mouse_x  = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
		mouse_y  = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
		mouse_l  = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
		mouse_r  = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
		mouse_m  = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_MIDDLE);
		mouse_4  = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_BUTTON_4);
		mouse_5  = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_BUTTON_5);
		mouse_wu = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_WHEELUP);
		mouse_wd = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_WHEELDOWN);
		mouse_wl = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP);
		mouse_wr = input_state_cb(i, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN);

		mousestate[i].x = mouse_x * osd::input_device::RELATIVE_PER_PIXEL;
		mousestate[i].y = mouse_y * osd::input_device::RELATIVE_PER_PIXEL;

		// internal UI mouse
		if (i == 0)
		{
			vmx += mouse_x / 4;
			vmy += mouse_y / 4;

			if (vmx > fb_width)
				vmx = fb_width - 1;
			if (vmy > fb_height)
				vmy = fb_height - 1;

			if (vmx < 0)
				vmx = 0;
			if (vmy < 0)
				vmy = 0;

			if (vmx != ovmx || vmy != ovmy)
			{
				int cx = -1, cy = -1;
				if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
					machine.ui_input().push_pointer_update(
							window->target(),
							osd::ui_event_handler::pointer::MOUSE,
							i,
							0,
							cx, cy,
							0, 0, 0, 0);
			}

			ovmx = vmx;
			ovmy = vmy;
		}

		// mouse buttons
		if (!mousestate[i].button[MOUSE_LEFT] && mouse_l)
		{
			mousestate[i].button[MOUSE_LEFT] = 0x80;

			// internal UI
			if (i == 0)
			{
				int cx = -1, cy = -1;
				if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
				{
					auto const double_click_speed = std::chrono::milliseconds(250);
					auto const click = std::chrono::steady_clock::now();

					machine.ui_input().push_pointer_update(
							window->target(),
							osd::ui_event_handler::pointer::MOUSE,
							i,
							0,
							cx, cy,
							1, 1, 0, 1);

					if (click < (m_last_click_time + double_click_speed)
						&& (cx >= (m_last_click_x - 4) && cx <= (m_last_click_x + 4))
						&& (cy >= (m_last_click_y - 4) && cy <= (m_last_click_y + 4)))
					{
						m_last_click_time = std::chrono::time_point<std::chrono::steady_clock>::min();
						machine.ui_input().push_pointer_update(
								window->target(),
								osd::ui_event_handler::pointer::MOUSE,
								i,
								0,
								cx, cy,
								1, 1, 1, 2);
					}
					else
					{
						m_last_click_time = click;
						m_last_click_x = cx;
						m_last_click_y = cy;
					}
				}
			}
		}
		else if (mousestate[i].button[MOUSE_LEFT] && !mouse_l)
		{
			mousestate[i].button[MOUSE_LEFT] = 0;

			if (i == 0)
			{
				int cx = -1, cy = -1;
				if (window != nullptr && window->renderer().xy_to_render_target(vmx, vmy, &cx, &cy))
					machine.ui_input().push_pointer_update(
						window->target(),
						osd::ui_event_handler::pointer::MOUSE,
						i,
						0,
						cx, cy,
						0, 0, 1, 0);
			}
		}

		if (!mousestate[i].button[MOUSE_RIGHT] && mouse_r)
			mousestate[i].button[MOUSE_RIGHT] = 0x80;
		else if (mousestate[i].button[MOUSE_RIGHT] && !mouse_r)
			mousestate[i].button[MOUSE_RIGHT] = 0;

		if (!mousestate[i].button[MOUSE_MIDDLE] && mouse_m)
			mousestate[i].button[MOUSE_MIDDLE] = 0x80;
		else if (mousestate[i].button[MOUSE_MIDDLE] && !mouse_m)
			mousestate[i].button[MOUSE_MIDDLE] = 0;

		if (!mousestate[i].button[MOUSE_4] && mouse_4)
			mousestate[i].button[MOUSE_4] = 0x80;
		else if (mousestate[i].button[MOUSE_4] && !mouse_4)
			mousestate[i].button[MOUSE_4] = 0;

		if (!mousestate[i].button[MOUSE_5] && mouse_5)
			mousestate[i].button[MOUSE_5] = 0x80;
		else if (mousestate[i].button[MOUSE_5] && !mouse_5)
			mousestate[i].button[MOUSE_5] = 0;

		if (!mousestate[i].button[MOUSE_WHEEL_UP] && mouse_wu)
			mousestate[i].button[MOUSE_WHEEL_UP] = 0x80;
		else if (mousestate[i].button[MOUSE_WHEEL_UP] && !mouse_wu)
			mousestate[i].button[MOUSE_WHEEL_UP] = 0;

		if (!mousestate[i].button[MOUSE_WHEEL_DOWN] && mouse_wd)
			mousestate[i].button[MOUSE_WHEEL_DOWN] = 0x80;
		else if (mousestate[i].button[MOUSE_WHEEL_DOWN] && !mouse_wd)
			mousestate[i].button[MOUSE_WHEEL_DOWN] = 0;

		if (!mousestate[i].button[MOUSE_WHEEL_LEFT] && mouse_wl)
			mousestate[i].button[MOUSE_WHEEL_LEFT] = 0x80;
		else if (mousestate[i].button[MOUSE_WHEEL_LEFT] && !mouse_wl)
			mousestate[i].button[MOUSE_WHEEL_LEFT] = 0;

		if (!mousestate[i].button[MOUSE_WHEEL_RIGHT] && mouse_wr)
			mousestate[i].button[MOUSE_WHEEL_RIGHT] = 0x80;
		else if (mousestate[i].button[MOUSE_WHEEL_RIGHT] && !mouse_wr)
			mousestate[i].button[MOUSE_WHEEL_RIGHT] = 0;
	}
}

void retro_osd_interface::process_lightgun_state(running_machine &machine)
{
	unsigned i, j;

	if (lightgun_mode == RETRO_SETTING_LIGHTGUN_MODE_DISABLED)
		return;

	for (i = 0; i < RETRO_MAX_PLAYERS; i++)
	{
		int16_t gun_x, gun_y;
		bool offscreen = false;
		bool reload    = false;

		for (j = 0; j < RETRO_MAX_LIGHTGUN_BUTTONS; j++)
			lightgunstate[i].button[j] = 0;

		if (lightgun_mode == RETRO_SETTING_LIGHTGUN_MODE_POINTER)
		{
			gun_x = input_state_cb(i, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
			gun_y = input_state_cb(i, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

			// handle pointer presses
			// use multi-touch to support different button inputs
			int touch_count[8];
			touch_count[i] = input_state_cb(i, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_COUNT);
			if (touch_count[i] > 0 && touch_count[i] <= 4)
				lightgunstate[i].button[touch_count[i]-1] = 0x80;
		}
		else
		{
			gun_x = input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X);
			gun_y = input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y);

			offscreen = input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN);
			reload    = input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || reload)
				lightgunstate[i].button[LIGHTGUN_TRIGGER] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A))
				lightgunstate[i].button[LIGHTGUN_AUX_A] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B))
				lightgunstate[i].button[LIGHTGUN_AUX_B] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_C))
				lightgunstate[i].button[LIGHTGUN_AUX_C] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START))
				lightgunstate[i].button[LIGHTGUN_START] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT))
				lightgunstate[i].button[LIGHTGUN_SELECT] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_DPAD_UP))
				lightgunstate[i].button[LIGHTGUN_DPAD_UP] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_DPAD_DOWN))
				lightgunstate[i].button[LIGHTGUN_DPAD_DOWN] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_DPAD_LEFT))
				lightgunstate[i].button[LIGHTGUN_DPAD_LEFT] = 0x80;

			if (input_state_cb(i, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_DPAD_RIGHT))
				lightgunstate[i].button[LIGHTGUN_DPAD_RIGHT] = 0x80;
		}

		lightgunstate[i].x = gun_x * 2;
		lightgunstate[i].y = gun_y * 2;

		// Place the cursor at a corner of the screen designated by "Lightgun offscreen position" when the cursor touches a min/max value
		// The LIGHTGUN_RELOAD input will fire a shot at the designated offscreen position
		// Free position is untouched when not offscreen, and goes to bottom right when offscreen, otherwise offscreen reloading is impossible
		// The reload feature of some games fails at the top-left corner
		if (offscreen)
		{
			if (lightgun_offscreen_mode == RETRO_SETTING_LIGHTGUN_OFFSCREEN_MODE_TOP_LEFT)
			{
				lightgunstate[i].x = -65535;
				lightgunstate[i].y = -65535;
			}
			else
			{
				lightgunstate[i].x = 65535;
				lightgunstate[i].y = 65535;
			}
		}
		else if (!offscreen && reload)
		{
			if (lightgun_offscreen_mode == RETRO_SETTING_LIGHTGUN_OFFSCREEN_MODE_TOP_LEFT)
			{
				lightgunstate[i].x = -65535;
				lightgunstate[i].y = -65535;
			}
			else if (lightgun_offscreen_mode == RETRO_SETTING_LIGHTGUN_OFFSCREEN_MODE_BOTTOM_RIGHT)
			{
				lightgunstate[i].x = 65535;
				lightgunstate[i].y = 65535;
			}
		}
	}
}



namespace osd {

//============================================================
//  retro_keyboard_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_keyboard_device : public event_based_device<KeyPressEventArgs>
{
public:
	retro_keyboard_device(std::string &&name, std::string &&id, input_module &module)
		: event_based_device(std::move(name), std::move(id), module)
	{
	}

	virtual void reset() override
	{
		memset(retro_key_state, 0, sizeof(retro_key_state));
		memset(retro_key_event_state, 0, sizeof(retro_key_event_state));
	}

	virtual void configure(input_device &device) override
	{
		int i = 0;
		do {
			device.add_item(
				keyboard_table[i].mame_key_name,
				std::string_view(),
				keyboard_table[i].mame_key,
				generic_button_get_state<std::uint8_t>,
				&retro_key_state[keyboard_table[i].retro_key_name]);
			i++;
		} while (keyboard_table[i].retro_key_name != -1);
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args)
	{
	}
};

//============================================================
//  keyboard_input_retro - retro keyboard input module
//============================================================

class keyboard_input_retro : public retro_input_module<retro_keyboard_device>
{
private:

public:
	keyboard_input_retro()
		: retro_input_module(OSD_KEYBOARDINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_input_module<retro_keyboard_device>::input_init(machine);

		create_device<retro_keyboard_device>(DEVICE_CLASS_KEYBOARD, "RetroKeyboard0", "RetroKeyboard0");

		memset(retro_key_state, 0, sizeof(retro_key_state));
		memset(retro_key_event_state, 0, sizeof(retro_key_event_state));

		m_global_inputs_enabled = true;
	}

	bool handle_input_event(void) override
	{
		if (!input_enabled())
			return false;
		return true;
	}
};



//============================================================
//  retro_mouse_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_mouse_device : public event_based_device<KeyPressEventArgs>
{
public:
	retro_mouse_device(std::string &&name, std::string &&id, input_module &module)
		: event_based_device(std::move(name), std::move(id), module)
	{
	}

	virtual void poll(bool relative_reset) override
	{
		event_based_device::poll(relative_reset);
	}

	virtual void reset() override
	{
		for (int i = 0; i < RETRO_MAX_PLAYERS; i++)
		{
			mousestate[i].x = fb_width / 2;
			mousestate[i].y = fb_height / 2;

			for (int j = 0; j < RETRO_MAX_MOUSE_BUTTONS; j++)
				mousestate[i].button[j] = 0;
		}
	}

	virtual void configure(input_device &device) override
	{
		input_device::assignment_vector assignments;

		device.add_item(
			"X",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_XAXIS),
			generic_axis_get_state<std::int32_t>,
			&mousestate[mouse_count].x);
        device.add_item(
			"Y",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_YAXIS),
			generic_axis_get_state<std::int32_t>,
			&mousestate[mouse_count].y);

        for (int button = 0; button < RETRO_MAX_MOUSE_BUTTONS; button++)
        {
			mousestate[mouse_count].button[button] = 0;

			device.add_item(
				mouse_button_name[button],
				std::string_view(),
				static_cast<input_item_id>(ITEM_ID_BUTTON1 + button),
				generic_button_get_state<std::int32_t>,
				&mousestate[mouse_count].button[button]);
		}

		if (mouse_count == 0)
		{
			assignments.emplace_back(IPT_UI_SELECT, SEQ_TYPE_STANDARD, input_seq(MOUSECODE_BUTTON1_INDEXED(mouse_count)));
			assignments.emplace_back(IPT_UI_BACK, SEQ_TYPE_STANDARD, input_seq(MOUSECODE_BUTTON2_INDEXED(mouse_count)));
			assignments.emplace_back(IPT_UI_CLEAR, SEQ_TYPE_STANDARD, input_seq(MOUSECODE_BUTTON3_INDEXED(mouse_count)));
			assignments.emplace_back(IPT_UI_PAGE_UP, SEQ_TYPE_STANDARD, input_seq(MOUSECODE_BUTTON6_INDEXED(mouse_count)));
			assignments.emplace_back(IPT_UI_PAGE_DOWN, SEQ_TYPE_STANDARD, input_seq(MOUSECODE_BUTTON7_INDEXED(mouse_count)));

			device.set_default_assignments(std::move(assignments));
		}

		mouse_count++;
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args)
	{
	}
};

//============================================================
//  mouse_input_retro - retro mouse input module
//============================================================

class mouse_input_retro : public retro_input_module<retro_mouse_device>
{
private:

public:
	mouse_input_retro()
		: retro_input_module(OSD_MOUSEINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_input_module<retro_mouse_device>::input_init(machine);

		if (!input_enabled() || !options()->mouse())
			return;

		char defname[32];

		mouse_count = 0;

		for (int i = 0; i < RETRO_MAX_PLAYERS; i++)
		{
			sprintf(defname, "RetroMouse%d", i);
			create_device<retro_mouse_device>(DEVICE_CLASS_MOUSE, defname, defname);

			mousestate[i].x = fb_width / 2;
			mousestate[i].y = fb_height / 2;
		}

		m_global_inputs_enabled = true;
	}

	bool handle_input_event(void) override
	{
		if (!input_enabled() || !options()->mouse())
			return false;
		return true;
	}
};



//============================================================
//  retro_joystick_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_joystick_device : public event_based_device<KeyPressEventArgs>, protected joystick_assignment_helper
{
public:
	retro_joystick_device(std::string &&name, std::string &&id, input_module &module)
		: event_based_device(std::move(name), std::move(id), module)
	{
	}

	virtual void poll(bool relative_reset)
	{
		event_based_device::poll(relative_reset);
	}

	virtual void reset() override
	{
		memset(&joystickstate, 0, sizeof(joystickstate));
	}

	virtual void configure(osd::input_device &device)
	{
		// track item IDs for setting up default assignments
		input_device::assignment_vector assignments;
		input_item_id axis_ids[AXIS_TOTAL];
		input_item_id switch_ids[SWITCH_TOTAL];
		std::fill(std::begin(switch_ids), std::end(switch_ids), ITEM_ID_INVALID);

		// axes
		axis_ids[AXIS_LX] = device.add_item(
			"LX",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_XAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystickstate[joystick_count].a1[0]);
		axis_ids[AXIS_LY] = device.add_item(
			"LY",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_YAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystickstate[joystick_count].a1[1]);

		axis_ids[AXIS_RX] = device.add_item(
			"RX",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_RXAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystickstate[joystick_count].a2[0]);
		axis_ids[AXIS_RY] = device.add_item(
			"RY",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_RYAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystickstate[joystick_count].a2[1]);

		axis_ids[AXIS_L2] = device.add_item(
			"L2",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_RZAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystickstate[joystick_count].a3[0]);
		axis_ids[AXIS_R2] = device.add_item(
			"R2",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_ZAXIS),
			generic_axis_get_state<std::int32_t>,
			&joystickstate[joystick_count].a3[1]);

		for (int j = 0; j < 6; j++)
		{
			switch_ids[j] = device.add_item(
				joystick_button_name[button_mapping[j]],
				std::string_view(),
				(input_item_id)(ITEM_ID_BUTTON1 + j),
				generic_button_get_state<std::int32_t>,
				&joystickstate[joystick_count].button[button_mapping[j]]);

			add_button_assignment(assignments, ioport_type(IPT_BUTTON1 + j), { switch_ids[j] });
		}

		switch_ids[SWITCH_START] = device.add_item(
			joystick_button_name[RETROPAD_START],
			std::string_view(),
			ITEM_ID_START,
			generic_button_get_state<std::int32_t>,
			&joystickstate[joystick_count].button[RETROPAD_START]);
		add_button_assignment(assignments, IPT_START, { switch_ids[SWITCH_START] });

		switch_ids[SWITCH_SELECT] = device.add_item(
			joystick_button_name[RETROPAD_SELECT],
			std::string_view(),
			ITEM_ID_SELECT,
			generic_button_get_state<std::int32_t>,
			&joystickstate[joystick_count].button[RETROPAD_SELECT]);
		add_button_assignment(assignments, IPT_SELECT, { switch_ids[SWITCH_SELECT] });

		switch_ids[SWITCH_L2] = device.add_item(
			joystick_button_name[RETROPAD_L2],
			std::string_view(),
			ITEM_ID_INVALID,
			generic_button_get_state<std::int32_t>,
			&joystickstate[joystick_count].button[RETROPAD_L2]);

		switch_ids[SWITCH_R2] = device.add_item(
			joystick_button_name[RETROPAD_R2],
			std::string_view(),
			ITEM_ID_INVALID,
			generic_button_get_state<std::int32_t>,
			&joystickstate[joystick_count].button[RETROPAD_R2]);

		switch_ids[SWITCH_L3] = device.add_item(
			joystick_button_name[RETROPAD_L3],
			std::string_view(),
			ITEM_ID_BUTTON7,
			generic_button_get_state<std::int32_t>,
			&joystickstate[joystick_count].button[RETROPAD_L3]);
		add_button_assignment(assignments, IPT_BUTTON7, { switch_ids[SWITCH_L3] });

		switch_ids[SWITCH_R3] = device.add_item(
			joystick_button_name[RETROPAD_R3],
			std::string_view(),
			ITEM_ID_BUTTON8,
			generic_button_get_state<std::int32_t>,
			&joystickstate[joystick_count].button[RETROPAD_R3]);
		add_button_assignment(assignments, IPT_BUTTON8, { switch_ids[SWITCH_R3] });

		// d-pad
		switch_ids[SWITCH_DPAD_UP] = device.add_item(
			joystick_button_name[RETROPAD_PAD_UP],
			std::string_view(),
			ITEM_ID_HAT1UP,
			generic_button_get_state<std::uint8_t>,
			&joystickstate[joystick_count].button[RETROPAD_PAD_UP]);

		switch_ids[SWITCH_DPAD_DOWN] = device.add_item(
			joystick_button_name[RETROPAD_PAD_DOWN],
			std::string_view(),
			ITEM_ID_HAT1DOWN,
			generic_button_get_state<std::uint8_t>,
			&joystickstate[joystick_count].button[RETROPAD_PAD_DOWN]);

		switch_ids[SWITCH_DPAD_LEFT] = device.add_item(
			joystick_button_name[RETROPAD_PAD_LEFT],
			std::string_view(),
			ITEM_ID_HAT1LEFT,
			generic_button_get_state<std::uint8_t>,
			&joystickstate[joystick_count].button[RETROPAD_PAD_LEFT]);

		switch_ids[SWITCH_DPAD_RIGHT] = device.add_item(
			joystick_button_name[RETROPAD_PAD_RIGHT],
			std::string_view(),
			ITEM_ID_HAT1RIGHT,
			generic_button_get_state<std::uint8_t>,
			&joystickstate[joystick_count].button[RETROPAD_PAD_RIGHT]);

		joystick_count++;

		// directions, analog stick
		add_directional_assignments(
				assignments,
				axis_ids[AXIS_LX],
				axis_ids[AXIS_LY],
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID);

		// directions, d-pad
		add_directional_assignments(
				assignments,
				ITEM_ID_INVALID,
				ITEM_ID_INVALID,
				switch_ids[SWITCH_DPAD_LEFT],
				switch_ids[SWITCH_DPAD_RIGHT],
				switch_ids[SWITCH_DPAD_UP],
				switch_ids[SWITCH_DPAD_DOWN]);

		// twin stick
		add_twin_stick_assignments(
				assignments,
				axis_ids[AXIS_LX],
				axis_ids[AXIS_LY],
				axis_ids[AXIS_RX],
				axis_ids[AXIS_RY],
				switch_ids[SWITCH_DPAD_LEFT],
				switch_ids[SWITCH_DPAD_RIGHT],
				switch_ids[SWITCH_DPAD_UP],
				switch_ids[SWITCH_DPAD_DOWN],
				switch_ids[SWITCH_Y],
				switch_ids[SWITCH_A],
				switch_ids[SWITCH_X],
				switch_ids[SWITCH_B]);

		// bonus AD stick Z
		assignments.emplace_back(
				IPT_AD_STICK_Z,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NONE, axis_ids[AXIS_RY])));

		// trigger pedals
		assignments.emplace_back(
				IPT_PEDAL,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, axis_ids[AXIS_R2])));
		assignments.emplace_back(
				IPT_PEDAL2,
				SEQ_TYPE_STANDARD,
				input_seq(make_code(ITEM_CLASS_ABSOLUTE, ITEM_MODIFIER_NEG, axis_ids[AXIS_L2])));

		// button pedals
		assignments.emplace_back(
				IPT_PEDAL,
				SEQ_TYPE_INCREMENT,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_B])));
		assignments.emplace_back(
				IPT_PEDAL2,
				SEQ_TYPE_INCREMENT,
				input_seq(make_code(ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, switch_ids[SWITCH_A])));

		// UI assignments
		add_button_assignment(assignments, IPT_UI_SELECT, { switch_ids[SWITCH_START] });
		add_button_assignment(assignments, IPT_UI_SELECT, { switch_ids[SWITCH_B] });
		add_button_assignment(assignments, IPT_UI_BACK, { switch_ids[SWITCH_A] });
		add_button_assignment(assignments, IPT_UI_CLEAR, { switch_ids[SWITCH_Y] });
		add_button_assignment(assignments, IPT_UI_FOCUS_NEXT, { switch_ids[SWITCH_X] });
		add_button_assignment(assignments, IPT_UI_PAGE_UP, { switch_ids[SWITCH_L1] });
		add_button_assignment(assignments, IPT_UI_PAGE_DOWN, { switch_ids[SWITCH_R1] });
		add_button_assignment(assignments, IPT_UI_FOCUS_NEXT, { switch_ids[SWITCH_L2] });
		add_button_assignment(assignments, IPT_UI_FOCUS_PREV, { switch_ids[SWITCH_R2] });
		add_button_assignment(assignments, IPT_UI_HOME, { switch_ids[SWITCH_L3] });
		add_button_assignment(assignments, IPT_UI_END, { switch_ids[SWITCH_R3] });

		// set default assignments
		device.set_default_assignments(std::move(assignments));
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args)
	{
	}
};

//============================================================
//  joystick_input_retro - retro joystick input module
//============================================================

class joystick_input_retro : public retro_input_module<retro_joystick_device>
{
private:

public:
	joystick_input_retro()
		: retro_input_module(OSD_JOYSTICKINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_input_module<retro_joystick_device>::input_init(machine);

		char defname[32];

		joystick_count = 0;

		if (buttons_profiles)
			Input_Binding(machine);

		for (int i = 0; i < RETRO_MAX_PLAYERS; i++)
		{
 			sprintf(defname, "RetroPad%d", i);
			create_device<retro_joystick_device>(DEVICE_CLASS_JOYSTICK, defname, defname);
		}

		m_global_inputs_enabled = true;
	}

	bool handle_input_event(void) override
	{
		if (!input_enabled())
			return false;
		return true;
	}
};



//============================================================
//  retro_lightgun_device
//============================================================

// This device is purely event driven so the implementation is in the module
class retro_lightgun_device : public event_based_device<KeyPressEventArgs>
{
public:
	retro_lightgun_device(std::string &&name, std::string &&id, input_module &module)
		: event_based_device(std::move(name), std::move(id), module)
	{
	}

	void poll(bool relative_reset) override
	{
		event_based_device::poll(relative_reset);
	}

	virtual void reset() override
	{
		for (int i = 0; i < RETRO_MAX_PLAYERS; i++)
		{
			lightgunstate[i].x = fb_width / 2;
			lightgunstate[i].y = fb_height / 2;

			for (int j = 0; j < RETRO_MAX_LIGHTGUN_BUTTONS; j++)
				lightgunstate[i].button[j] = 0;
		}
	}

	virtual void configure(osd::input_device &device)
	{
		input_device::assignment_vector assignments;

		device.add_item(
			"X",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_XAXIS),
			generic_axis_get_state<std::int32_t>,
			&lightgunstate[lightgun_count].x);
		device.add_item(
			"Y",
			std::string_view(),
			static_cast<input_item_id>(ITEM_ID_YAXIS),
			generic_axis_get_state<std::int32_t>,
			&lightgunstate[lightgun_count].y);

		for (int button = 0; button < RETRO_MAX_LIGHTGUN_BUTTONS; button++)
		{
			lightgunstate[lightgun_count].button[button] = 0;

			device.add_item(
				lightgun_button_name[button],
				std::string_view(),
				(input_item_id)(ITEM_ID_BUTTON1 + button),
				generic_button_get_state<std::int32_t>,
				&lightgunstate[lightgun_count].button[button]);
		}

		// assign lightgun to AD stick
		assignments.emplace_back(IPT_AD_STICK_X, SEQ_TYPE_STANDARD, input_seq(GUNCODE_X_INDEXED(lightgun_count)));
		assignments.emplace_back(IPT_AD_STICK_Y, SEQ_TYPE_STANDARD, input_seq(GUNCODE_Y_INDEXED(lightgun_count)));

		// assign remaining lightgun buttons
		assignments.emplace_back(IPT_BUTTON3, SEQ_TYPE_STANDARD, input_seq(GUNCODE_BUTTON3_INDEXED(lightgun_count)));
		assignments.emplace_back(IPT_BUTTON4, SEQ_TYPE_STANDARD, input_seq(GUNCODE_BUTTON4_INDEXED(lightgun_count)));

		assignments.emplace_back(IPT_START, SEQ_TYPE_STANDARD, input_seq(GUNCODE_BUTTON5_INDEXED(lightgun_count)));
		assignments.emplace_back(IPT_SELECT, SEQ_TYPE_STANDARD, input_seq(GUNCODE_BUTTON6_INDEXED(lightgun_count)));

		assignments.emplace_back(IPT_JOYSTICK_UP, SEQ_TYPE_STANDARD, input_code(DEVICE_CLASS_LIGHTGUN, lightgun_count, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON7));
		assignments.emplace_back(IPT_JOYSTICK_DOWN, SEQ_TYPE_STANDARD, input_code(DEVICE_CLASS_LIGHTGUN, lightgun_count, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON8));
		assignments.emplace_back(IPT_JOYSTICK_LEFT, SEQ_TYPE_STANDARD, input_code(DEVICE_CLASS_LIGHTGUN, lightgun_count, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON9));
		assignments.emplace_back(IPT_JOYSTICK_RIGHT, SEQ_TYPE_STANDARD, input_code(DEVICE_CLASS_LIGHTGUN, lightgun_count, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, ITEM_ID_BUTTON10));

		device.set_default_assignments(std::move(assignments));

		lightgun_count++;
	}

protected:
	virtual void process_event(KeyPressEventArgs const &args)
	{
	}
};

//============================================================
//  lightgun_input_retro - retro lightgun input module
//============================================================

class lightgun_input_retro : public retro_input_module<retro_lightgun_device>
{
private:

public:
	lightgun_input_retro()
		: retro_input_module(OSD_LIGHTGUNINPUT_PROVIDER, "retro")
	{
	}

	virtual void input_init(running_machine &machine) override
	{
		retro_input_module<retro_lightgun_device>::input_init(machine);

		if (!input_enabled() || !options()->lightgun())
			return;

		char defname[32];

		lightgun_count = 0;

		for (int i = 0; i < RETRO_MAX_PLAYERS; i++)
		{
			sprintf(defname, "RetroLightgun%d", i);
			create_device<retro_lightgun_device>(DEVICE_CLASS_LIGHTGUN, defname, defname);

			lightgunstate[i].x = fb_width / 2;
			lightgunstate[i].y = fb_height / 2;
		}

		m_global_inputs_enabled = true;
	}

	bool handle_input_event(void) override
	{
		if (!input_enabled() || !options()->lightgun())
			return false;
		return true;
	}
};

} // namespace osd



void retro_osd_interface::process_events_buf()
{
	input_poll_cb();
}

void retro_osd_interface::poll_inputs(running_machine &machine)
{
	process_mouse_state(machine);
	process_keyboard_state(machine);
	process_joystick_state(machine);
	process_lightgun_state(machine);
}

MODULE_DEFINITION(MOUSEINPUT_RETRO, osd::mouse_input_retro)
MODULE_DEFINITION(KEYBOARDINPUT_RETRO, osd::keyboard_input_retro)
MODULE_DEFINITION(JOYSTICKINPUT_RETRO, osd::joystick_input_retro)
MODULE_DEFINITION(LIGHTGUNINPUT_RETRO, osd::lightgun_input_retro)
