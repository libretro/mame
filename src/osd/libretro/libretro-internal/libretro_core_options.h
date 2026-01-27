#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

#include <libretro_shared.h>

#ifndef HAVE_NO_LANGEXTRA
#include "libretro_core_options_intl.h"
#endif

/*
 ********************************
 * VERSION: 2.0
 ********************************
 *
 * - 2.0: Add support for core options v2 interface 
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Definitions
 ********************************
*/


/*
 ********************************
 * Core Option Definitions
 ********************************
*/

/* RETRO_LANGUAGE_ENGLISH */

/* Default language:
 * - All other languages must include the same keys and values
 * - Will be used as a fallback in the event that frontend language
 *   is not available
 * - Will be used as a fallback for any missing entries in
 *   frontend language definition */

struct retro_core_option_v2_category option_cats_us[] = {
   {
      "system",
      "System",
      "Configure general MAME related options."
   },
   {
      "video",
      "Video",
      "Configure video options."
   },
   {
      "input",
      "Input",
      "Configure input options."
   },
   {
      "hacks",
      "Emulation Hacks",
      "Configure emulation hack options."
   },
   { NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_us[] = {
   {
      CORE_NAME "_thread_mode",
      "Thread Mode",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled"
   },
   {
      CORE_NAME "_cheats_enable",
      "Cheats",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_throttle",
      "Throttle",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_boot_to_bios",
      "Boot to BIOS",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_boot_to_osd",
      "Boot to OSD",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_read_config",
      "Read Configuration",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_write_config",
      "Write Configuration",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_mame_paths_enable",
      "MAME INI Paths",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_saves",
      "Save State Naming",
      NULL,
      "",
      NULL,
      "system",
      {
         { "game",   "Game" },
         { "system", "System" },
         { NULL,  NULL },
      },
      "game"
   },
   {
      CORE_NAME "_auto_save",
      "Auto Save/Load States",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_softlists_enable",
      "Softlists",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled"
   },
   {
      CORE_NAME "_softlists_auto_media",
      "Softlist Automatic Media Type",
      NULL,
      "",
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled"
   },
   {
      CORE_NAME "_media_type",
      "Media Type",
      NULL,
      "",
      NULL,
      "system",
      {
         { "cart", NULL },
         { "cass", NULL },
         { "cdrm", NULL },
         { "flop", NULL },
         { "hard", NULL },
         { "prin", NULL },
         { "rom",  NULL },
         { "serl", NULL },
         { NULL, NULL },
      },
      "rom"
   },
   {
      CORE_NAME "_joystick_deadzone",
      "Joystick Deadzone",
      NULL,
      "",
      NULL,
      "input",
      {
         { "0.00", NULL },
         { "0.05", NULL },
         { "0.10", NULL },
         { "0.15", NULL },
         { "0.20", NULL },
         { "0.25", NULL },
         { "0.30", NULL },
         { "0.35", NULL },
         { "0.40", NULL },
         { "0.45", NULL },
         { "0.50", NULL },
         { "0.55", NULL },
         { "0.60", NULL },
         { "0.65", NULL },
         { "0.70", NULL },
         { "0.75", NULL },
         { "0.80", NULL },
         { "0.85", NULL },
         { "0.90", NULL },
         { "0.95", NULL },
         { "1.00", NULL },
         { NULL, NULL },
      },
      "0.15"
   },
   {
      CORE_NAME "_joystick_saturation",
      "Joystick Saturation",
      NULL,
      "",
      NULL,
      "input",
      {
         { "0.05", NULL },
         { "0.10", NULL },
         { "0.15", NULL },
         { "0.20", NULL },
         { "0.25", NULL },
         { "0.30", NULL },
         { "0.35", NULL },
         { "0.40", NULL },
         { "0.45", NULL },
         { "0.50", NULL },
         { "0.55", NULL },
         { "0.60", NULL },
         { "0.65", NULL },
         { "0.70", NULL },
         { "0.75", NULL },
         { "0.80", NULL },
         { "0.85", NULL },
         { "0.90", NULL },
         { "0.95", NULL },
         { "1.00", NULL },
         { NULL, NULL },
      },
      "0.85"
   },
   {
      CORE_NAME "_joystick_threshold",
      "Joystick Threshold",
      NULL,
      "",
      NULL,
      "input",
      {
         { "0.05", NULL },
         { "0.10", NULL },
         { "0.15", NULL },
         { "0.20", NULL },
         { "0.25", NULL },
         { "0.30", NULL },
         { "0.35", NULL },
         { "0.40", NULL },
         { "0.45", NULL },
         { "0.50", NULL },
         { "0.55", NULL },
         { "0.60", NULL },
         { "0.65", NULL },
         { "0.70", NULL },
         { "0.75", NULL },
         { "0.80", NULL },
         { "0.85", NULL },
         { "0.90", NULL },
         { "0.95", NULL },
         { "1.00", NULL },
         { NULL, NULL },
      },
      "0.30"
   },
   {
      CORE_NAME "_mame_4way_enable",
      "Joystick 4-way Simulation",
      NULL,
      "",
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "4way",     NULL },
         { "strict",   NULL },
         { "qbert",    NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_buttons_profiles",
      "Profile Buttons Per Game",
      NULL,
      "",
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_mouse_enable",
      "Mouse",
      NULL,
      "",
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled"
   },
   {
      CORE_NAME "_lightgun_mode",
      "Lightgun Mode",
      NULL,
      "",
      NULL,
      "input",
      {
         { "none",        "None" },
         { "lightgun",    "Lightgun" },
         { "touchscreen", "Touchscreen" },
         { NULL, NULL },
      },
      "none"
   },
   {
      CORE_NAME "_lightgun_offscreen_mode",
      "Lightgun Offscreen Position",
      NULL,
      "",
      NULL,
      "input",
      {
         { "free",                 "Free" },
         { "fixed (top left)",     "Fixed (Top Left)" },
         { "fixed (bottom right)", "Fixed (Bottom Right)" },
         { NULL, NULL },
      },
      "free"
   },
   {
      CORE_NAME "_rotation_mode",
      "Screen Rotation Mode",
      NULL,
      "",
      NULL,
      "video",
      {
         { "libretro", "Libretro" },
         { "internal", "Internal" },
         { "tate-rol", "TATE-ROL" },
         { "tate-ror", "TATE-ROR" },
         { "none",     "None" },
         { NULL, NULL },
      },
      "libretro"
   },
   {
      CORE_NAME "_alternate_renderer",
      "Alternate Renderer",
      NULL,
      "",
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_altres",
      "Alternate Renderer Resolution",
      NULL,
      "",
      NULL,
      "video",
      {
         { "640x360",   NULL },
         { "640x480",   NULL },
         { "800x450",   NULL },
         { "800x600",   NULL },
         { "960x540",   NULL },
         { "960x720",   NULL },
         { "1024x576",  NULL },
         { "1024x768",  NULL },
         { "1280x720",  NULL },
         { "1280x960",  NULL },
         { "1600x900",  NULL },
         { "1600x1200", NULL },
         { "1440x1080", NULL },
         { "1920x1080", NULL },
         { "1920x1440", NULL },
         { "2560x1440", NULL },
         { "2880x2160", NULL },
         { "3840x2160", NULL },
         { NULL, NULL },
      },
      "640x480"
   },
   {
      CORE_NAME "_cpu_overclock",
      "Main CPU Overclock %",
      NULL,
      "",
      NULL,
      "hacks",
      {
         { "default", "Default (100%)" },
         { "10",  NULL },
         { "11",  NULL },
         { "12",  NULL },
         { "13",  NULL },
         { "14",  NULL },
         { "15",  NULL },
         { "16",  NULL },
         { "17",  NULL },
         { "18",  NULL },
         { "19",  NULL },
         { "20",  NULL },
         { "21",  NULL },
         { "22",  NULL },
         { "23",  NULL },
         { "24",  NULL },
         { "25",  NULL },
         { "26",  NULL },
         { "27",  NULL },
         { "28",  NULL },
         { "29",  NULL },
         { "30",  NULL },
         { "31",  NULL },
         { "32",  NULL },
         { "33",  NULL },
         { "34",  NULL },
         { "35",  NULL },
         { "36",  NULL },
         { "37",  NULL },
         { "38",  NULL },
         { "39",  NULL },
         { "40",  NULL },
         { "41",  NULL },
         { "42",  NULL },
         { "43",  NULL },
         { "44",  NULL },
         { "45",  NULL },
         { "46",  NULL },
         { "47",  NULL },
         { "48",  NULL },
         { "49",  NULL },
         { "50",  NULL },
         { "51",  NULL },
         { "52",  NULL },
         { "53",  NULL },
         { "54",  NULL },
         { "55",  NULL },
         { "60",  NULL },
         { "65",  NULL },
         { "70",  NULL },
         { "75",  NULL },
         { "80",  NULL },
         { "85",  NULL },
         { "90",  NULL },
         { "95",  NULL },
         { "100", NULL },
         { "105", NULL },
         { "110", NULL },
         { "115", NULL },
         { "120", NULL },
         { "125", NULL },
         { "130", NULL },
         { "135", NULL },
         { "140", NULL },
         { "145", NULL },
         { "150", NULL },
         { "155", NULL },
         { "160", NULL },
         { "165", NULL },
         { "170", NULL },
         { "175", NULL },
         { "180", NULL },
         { "185", NULL },
         { "190", NULL },
         { "195", NULL },
         { "200", NULL },
         { "205", NULL },
         { "210", NULL },
         { "215", NULL },
         { "220", NULL },
         { "225", NULL },
         { "230", NULL },
         { "235", NULL },
         { "240", NULL },
         { "245", NULL },
         { "250", NULL },
         { "255", NULL },
         { "260", NULL },
         { "265", NULL },
         { "270", NULL },
         { "275", NULL },
         { "280", NULL },
         { "285", NULL },
         { "290", NULL },
         { "295", NULL },
         { "300", NULL },
         { "305", NULL },
         { "310", NULL },
         { "315", NULL },
         { "320", NULL },
         { "325", NULL },
         { "330", NULL },
         { "335", NULL },
         { "340", NULL },
         { "345", NULL },
         { "350", NULL },
         { "355", NULL },
         { "360", NULL },
         { "365", NULL },
         { "370", NULL },
         { "375", NULL },
         { "380", NULL },
         { "385", NULL },
         { "390", NULL },
         { "395", NULL },
         { "400", NULL },
         { NULL, NULL },
      },
      "default"
   },
   {
      CORE_NAME "_cpu_sound_overclock",
      "Sound CPU Overclock %",
      NULL,
      "",
      NULL,
      "hacks",
      {
         { "default", "Default (100%)" },
         { "10",  NULL },
         { "11",  NULL },
         { "12",  NULL },
         { "13",  NULL },
         { "14",  NULL },
         { "15",  NULL },
         { "16",  NULL },
         { "17",  NULL },
         { "18",  NULL },
         { "19",  NULL },
         { "20",  NULL },
         { "21",  NULL },
         { "22",  NULL },
         { "23",  NULL },
         { "24",  NULL },
         { "25",  NULL },
         { "26",  NULL },
         { "27",  NULL },
         { "28",  NULL },
         { "29",  NULL },
         { "30",  NULL },
         { "31",  NULL },
         { "32",  NULL },
         { "33",  NULL },
         { "34",  NULL },
         { "35",  NULL },
         { "36",  NULL },
         { "37",  NULL },
         { "38",  NULL },
         { "39",  NULL },
         { "40",  NULL },
         { "41",  NULL },
         { "42",  NULL },
         { "43",  NULL },
         { "44",  NULL },
         { "45",  NULL },
         { "46",  NULL },
         { "47",  NULL },
         { "48",  NULL },
         { "49",  NULL },
         { "50",  NULL },
         { "51",  NULL },
         { "52",  NULL },
         { "53",  NULL },
         { "54",  NULL },
         { "55",  NULL },
         { "60",  NULL },
         { "65",  NULL },
         { "70",  NULL },
         { "75",  NULL },
         { "80",  NULL },
         { "85",  NULL },
         { "90",  NULL },
         { "95",  NULL },
         { "100", NULL },
         { "105", NULL },
         { "110", NULL },
         { "115", NULL },
         { "120", NULL },
         { "125", NULL },
         { "130", NULL },
         { "135", NULL },
         { "140", NULL },
         { "145", NULL },
         { "150", NULL },
         { "155", NULL },
         { "160", NULL },
         { "165", NULL },
         { "170", NULL },
         { "175", NULL },
         { "180", NULL },
         { "185", NULL },
         { "190", NULL },
         { "195", NULL },
         { "200", NULL },
         { "205", NULL },
         { "210", NULL },
         { "215", NULL },
         { "220", NULL },
         { "225", NULL },
         { "230", NULL },
         { "235", NULL },
         { "240", NULL },
         { "245", NULL },
         { "250", NULL },
         { "255", NULL },
         { "260", NULL },
         { "265", NULL },
         { "270", NULL },
         { "275", NULL },
         { "280", NULL },
         { "285", NULL },
         { "290", NULL },
         { "295", NULL },
         { "300", NULL },
         { "305", NULL },
         { "310", NULL },
         { "315", NULL },
         { "320", NULL },
         { "325", NULL },
         { "330", NULL },
         { "335", NULL },
         { "340", NULL },
         { "345", NULL },
         { "350", NULL },
         { "355", NULL },
         { "360", NULL },
         { "365", NULL },
         { "370", NULL },
         { "375", NULL },
         { "380", NULL },
         { "385", NULL },
         { "390", NULL },
         { "395", NULL },
         { "400", NULL },
         { NULL, NULL },
      },
      "default"
   },
   {
      CORE_NAME "_autoloadfastforward",
      "Automatic Load Fast-Forward",
      NULL,
      "Experimental feature to automatically fast-forward during CD access.\nWorks with:\n- Generic SCSI\n- Sega CD and Neo Geo CD",
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      CORE_NAME "_coin_limit",
      "Coin Limit",
      NULL,
      "",
      NULL,
      "hacks",
      {
         { "0", "disabled" },
         { "1", NULL },
         { "2", NULL },
         { "3", NULL },
         { "4", NULL },
         { "5", NULL },
         { "6", NULL },
         { "7", NULL },
         { "8", NULL },
         { "9", NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "0"
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};

struct retro_core_options_v2 options_us = {
   option_cats_us,
   option_defs_us
};

/*
 ********************************
 * Language Mapping
 ********************************
*/

#ifndef HAVE_NO_LANGEXTRA
struct retro_core_options_v2 *options_intl[RETRO_LANGUAGE_LAST] = {
   &options_us,      /* RETRO_LANGUAGE_ENGLISH */
   NULL,             /* RETRO_LANGUAGE_JAPANESE */
   NULL,             /* RETRO_LANGUAGE_FRENCH */
   NULL,             /* RETRO_LANGUAGE_SPANISH */
   NULL,             /* RETRO_LANGUAGE_GERMAN */
   NULL,             /* RETRO_LANGUAGE_ITALIAN */
   NULL,             /* RETRO_LANGUAGE_DUTCH */
   NULL,             /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
   NULL,             /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
   NULL,             /* RETRO_LANGUAGE_RUSSIAN */
   NULL,             /* RETRO_LANGUAGE_KOREAN */
   NULL,             /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
   NULL,             /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
   NULL,             /* RETRO_LANGUAGE_ESPERANTO */
   NULL,             /* RETRO_LANGUAGE_POLISH */
   NULL,             /* RETRO_LANGUAGE_VIETNAMESE */
   NULL,             /* RETRO_LANGUAGE_ARABIC */
   NULL,             /* RETRO_LANGUAGE_GREEK */
   NULL,             /* RETRO_LANGUAGE_TURKISH */
   NULL,             /* RETRO_LANGUAGE_SLOVAK */
   NULL,             /* RETRO_LANGUAGE_PERSIAN */
   NULL,             /* RETRO_LANGUAGE_HEBREW */
   NULL,             /* RETRO_LANGUAGE_ASTURIAN */
   NULL,             /* RETRO_LANGUAGE_FINNISH */
   NULL,             /* RETRO_LANGUAGE_INDONESIAN */
   NULL,             /* RETRO_LANGUAGE_SWEDISH */
   NULL,             /* RETRO_LANGUAGE_UKRAINIAN */
   NULL,             /* RETRO_LANGUAGE_CZECH */
   NULL,             /* RETRO_LANGUAGE_CATALAN_VALENCIA */
   NULL,             /* RETRO_LANGUAGE_CATALAN */
   NULL,             /* RETRO_LANGUAGE_BRITISH_ENGLISH */
   NULL,             /* RETRO_LANGUAGE_HUNGARIAN */
};
#endif

/*
 ********************************
 * Functions
 ********************************
*/

/* Handles configuration/setting of core options.
 * Should be called as early as possible - ideally inside
 * retro_set_environment(), and no later than retro_load_game()
 * > We place the function body in the header to avoid the
 *   necessity of adding more .c files (i.e. want this to
 *   be as painless as possible for core devs)
 */

INLINE void libretro_set_core_options(retro_environment_t environ_cb,
      bool *categories_supported)
{
   unsigned version  = 0;
#ifndef HAVE_NO_LANGEXTRA
   unsigned language = 0;
#endif

   if (!environ_cb || !categories_supported)
      return;

   *categories_supported = false;

   if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
      version = 0;

   if (version >= 2)
   {
#ifndef HAVE_NO_LANGEXTRA
      struct retro_core_options_v2_intl core_options_intl;

      core_options_intl.us    = &options_us;
      core_options_intl.local = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
          (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
         core_options_intl.local = options_intl[language];

      *categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL,
            &core_options_intl);
#else
      *categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2,
            &options_us);
#endif
   }
   else
   {
      size_t i, j;
      size_t option_index              = 0;
      size_t num_options               = 0;
      struct retro_core_option_definition
            *option_v1_defs_us         = NULL;
#ifndef HAVE_NO_LANGEXTRA
      size_t num_options_intl          = 0;
      struct retro_core_option_v2_definition
            *option_defs_intl          = NULL;
      struct retro_core_option_definition
            *option_v1_defs_intl       = NULL;
      struct retro_core_options_intl
            core_options_v1_intl;
#endif
      struct retro_variable *variables = NULL;
      char **values_buf                = NULL;

      /* Determine total number of options */
      while (true)
      {
         if (option_defs_us[num_options].key)
            num_options++;
         else
            break;
      }

      if (version >= 1)
      {
         /* Allocate US array */
         option_v1_defs_us = (struct retro_core_option_definition *)
               calloc(num_options + 1, sizeof(struct retro_core_option_definition));

         /* Copy parameters from option_defs_us array */
         for (i = 0; i < num_options; i++)
         {
            struct retro_core_option_v2_definition *option_def_us = &option_defs_us[i];
            struct retro_core_option_value *option_values         = option_def_us->values;
            struct retro_core_option_definition *option_v1_def_us = &option_v1_defs_us[i];
            struct retro_core_option_value *option_v1_values      = option_v1_def_us->values;

            option_v1_def_us->key           = option_def_us->key;
            option_v1_def_us->desc          = option_def_us->desc;
            option_v1_def_us->info          = option_def_us->info;
            option_v1_def_us->default_value = option_def_us->default_value;

            /* Values must be copied individually... */
            while (option_values->value)
            {
               option_v1_values->value = option_values->value;
               option_v1_values->label = option_values->label;

               option_values++;
               option_v1_values++;
            }
         }

#ifndef HAVE_NO_LANGEXTRA
         if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
             (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH) &&
             options_intl[language])
            option_defs_intl = options_intl[language]->definitions;

         if (option_defs_intl)
         {
            /* Determine number of intl options */
            while (true)
            {
               if (option_defs_intl[num_options_intl].key)
                  num_options_intl++;
               else
                  break;
            }

            /* Allocate intl array */
            option_v1_defs_intl = (struct retro_core_option_definition *)
                  calloc(num_options_intl + 1, sizeof(struct retro_core_option_definition));

            /* Copy parameters from option_defs_intl array */
            for (i = 0; i < num_options_intl; i++)
            {
               struct retro_core_option_v2_definition *option_def_intl = &option_defs_intl[i];
               struct retro_core_option_value *option_values           = option_def_intl->values;
               struct retro_core_option_definition *option_v1_def_intl = &option_v1_defs_intl[i];
               struct retro_core_option_value *option_v1_values        = option_v1_def_intl->values;

               option_v1_def_intl->key           = option_def_intl->key;
               option_v1_def_intl->desc          = option_def_intl->desc;
               option_v1_def_intl->info          = option_def_intl->info;
               option_v1_def_intl->default_value = option_def_intl->default_value;

               /* Values must be copied individually... */
               while (option_values->value)
               {
                  option_v1_values->value = option_values->value;
                  option_v1_values->label = option_values->label;

                  option_values++;
                  option_v1_values++;
               }
            }
         }

         core_options_v1_intl.us    = option_v1_defs_us;
         core_options_v1_intl.local = option_v1_defs_intl;

         environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_v1_intl);
#else
         environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, option_v1_defs_us);
#endif
      }
      else
      {
         /* Allocate arrays */
         variables  = (struct retro_variable *)calloc(num_options + 1,
               sizeof(struct retro_variable));
         values_buf = (char **)calloc(num_options, sizeof(char *));

         if (!variables || !values_buf)
            goto error;

         /* Copy parameters from option_defs_us array */
         for (i = 0; i < num_options; i++)
         {
            const char *key                        = option_defs_us[i].key;
            const char *desc                       = option_defs_us[i].desc;
            const char *default_value              = option_defs_us[i].default_value;
            struct retro_core_option_value *values = option_defs_us[i].values;
            size_t buf_len                         = 3;
            size_t default_index                   = 0;

            values_buf[i] = NULL;

            if (desc)
            {
               size_t num_values = 0;

               /* Determine number of values */
               while (true)
               {
                  if (values[num_values].value)
                  {
                     /* Check if this is the default value */
                     if (default_value)
                        if (strcmp(values[num_values].value, default_value) == 0)
                           default_index = num_values;

                     buf_len += strlen(values[num_values].value);
                     num_values++;
                  }
                  else
                     break;
               }

               /* Build values string */
               if (num_values > 0)
               {
                  buf_len += num_values - 1;
                  buf_len += strlen(desc);

                  values_buf[i] = (char *)calloc(buf_len, sizeof(char));
                  if (!values_buf[i])
                     goto error;

                  strcpy(values_buf[i], desc);
                  strcat(values_buf[i], "; ");

                  /* Default value goes first */
                  strcat(values_buf[i], values[default_index].value);

                  /* Add remaining values */
                  for (j = 0; j < num_values; j++)
                  {
                     if (j != default_index)
                     {
                        strcat(values_buf[i], "|");
                        strcat(values_buf[i], values[j].value);
                     }
                  }
               }
            }

            variables[option_index].key   = key;
            variables[option_index].value = values_buf[i];
            option_index++;
         }

         /* Set variables */
         environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
      }

error:
      /* Clean up */

      if (option_v1_defs_us)
      {
         free(option_v1_defs_us);
         option_v1_defs_us = NULL;
      }

#ifndef HAVE_NO_LANGEXTRA
      if (option_v1_defs_intl)
      {
         free(option_v1_defs_intl);
         option_v1_defs_intl = NULL;
      }
#endif

      if (values_buf)
      {
         for (i = 0; i < num_options; i++)
         {
            if (values_buf[i])
            {
               free(values_buf[i]);
               values_buf[i] = NULL;
            }
         }

         free(values_buf);
         values_buf = NULL;
      }

      if (variables)
      {
         free(variables);
         variables = NULL;
      }
   }
}

#ifdef __cplusplus
}
#endif

#endif
