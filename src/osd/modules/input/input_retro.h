
//============================================================
//
//  input_retro.h - Common code used by Windows input modules
//
//============================================================

#ifndef INPUT_RETRO_H_
#define INPUT_RETRO_H_

#include "osdretro.h"

enum
{
	SWITCH_B,           // button bits
	SWITCH_A,
	SWITCH_Y,
	SWITCH_X,
	SWITCH_L1,
	SWITCH_R1,
	SWITCH_L3,
	SWITCH_R3,
	SWITCH_START,
	SWITCH_SELECT,

	SWITCH_DPAD_UP,     // D-pad bits
	SWITCH_DPAD_DOWN,
	SWITCH_DPAD_LEFT,
	SWITCH_DPAD_RIGHT,

	SWITCH_L2,          // for arcade stick/pad with LT/RT buttons
	SWITCH_R2,

	SWITCH_TOTAL
};

enum
{
	AXIS_L2,            // half-axes for triggers
	AXIS_R2,

	AXIS_LSX,           // full-precision axes
	AXIS_LSY,
	AXIS_RSX,
	AXIS_RSY,

	AXIS_TOTAL
};

enum
{
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,
	MOUSE_4,
	MOUSE_5,
	MOUSE_WHEEL_UP,
	MOUSE_WHEEL_DOWN,
	MOUSE_WHEEL_LEFT,
	MOUSE_WHEEL_RIGHT,

	MOUSE_BUTTONS_TOTAL
};

enum
{
	LIGHTGUN_TRIGGER,
	LIGHTGUN_AUX_A,
	LIGHTGUN_AUX_B,
	LIGHTGUN_AUX_C,
	LIGHTGUN_START,
	LIGHTGUN_SELECT,
	LIGHTGUN_DPAD_UP,
	LIGHTGUN_DPAD_DOWN,
	LIGHTGUN_DPAD_LEFT,
	LIGHTGUN_DPAD_RIGHT,

	LIGHTGUN_BUTTONS_TOTAL
};

#define RETRO_MAX_PLAYERS 8
#define RETRO_MAX_JOYPAD_BUTTONS SWITCH_TOTAL
#define RETRO_MAX_MOUSE_BUTTONS MOUSE_BUTTONS_TOTAL
#define RETRO_MAX_LIGHTGUN_BUTTONS LIGHTGUN_BUTTONS_TOTAL

//============================================================
//  TYPEDEFS
//============================================================

typedef struct joystate_t
{
	int button[RETRO_MAX_JOYPAD_BUTTONS];
	int a1[2];
	int a2[2];
	int a3[2];
} Joystate;

typedef struct mousestate_t
{
	int x;
	int y;
	int button[RETRO_MAX_MOUSE_BUTTONS];
} Mousestate;

typedef struct lightgunstate_t
{
	int x;
	int y;
	int button[RETRO_MAX_LIGHTGUN_BUTTONS];
} Lightgunstate;

struct KeyPressEventArgs
{
	int event_id;
	uint8_t vkey;
	uint8_t scancode;
};

struct keyboard_table_t
{
	const char *mame_key_name;
	int retro_key_name;
	input_item_id mame_key;
};

extern void retro_keyboard_event(bool, unsigned, uint32_t, uint16_t);

template <typename Info>
class retro_input_module : public input_module_impl<Info, retro_osd_interface>
{
protected:
	bool  m_global_inputs_enabled;

public:
	retro_input_module(const char *type, const char *name)
		: input_module_impl<Info, retro_osd_interface>(type, name),
			m_global_inputs_enabled(true)
	{
	}

	bool input_enabled() const { return m_global_inputs_enabled; }

	virtual bool handle_input_event(void)
	{
		return false;
	}

protected:
};

#endif
