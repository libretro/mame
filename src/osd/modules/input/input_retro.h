
//============================================================
//
//  input_retro.h - Common code used by Windows input modules
//
//============================================================

#ifndef INPUT_RETRO_H_
#define INPUT_RETRO_H_

#include "osdretro.h"


//============================================================
//  TYPEDEFS
//============================================================

typedef struct joystate_t
{
   int button[RETRO_MAX_BUTTONS];
   int a1[2];
   int a2[2];
   int a3[2];
} Joystate;

typedef struct mousestate_t
{
   int mouseBUT[4];
} Mousestate;

typedef struct lightgunstate_t
{
   int lightgunBUT[4];
} Lightgunstate;

struct KeyPressEventArgs
{
	int event_id;
	uint8_t vkey;
	uint8_t scancode;
};

struct kt_table
{
   const char *mame_key_name;
   int retro_key_name;
   input_item_id mame_key;
};

extern void retro_keyboard_event(bool, unsigned, uint32_t, uint16_t);
extern kt_table const ktable[];

extern int mouseLX[8];
extern int mouseLY[8];

extern int lightgunX[8];
extern int lightgunY[8];

extern Joystate joystate[8];

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
