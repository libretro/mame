#ifdef __GNUC__
#include <unistd.h>
#endif
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "osdepend.h"

#include "emu.h"
#include "emuopts.h"
#include "render.h"
#include "ui/uimain.h"
#include "uiinput.h"
#include "drivenum.h"
#include "../frontend/mame/mame.h"

#include "libretro.h"
#include "libretro_shared.h"
#include "libretro_core_options.h"

/* forward decls / externs / prototypes */

extern void retro_finish();
extern void retro_main_loop();

int RLOOP          = 1;
int retro_pause    = 0;
int SHIFTON        = -1;
char RPATH[512];
bool first_run     = true;
bool audio_ready   = false;
bool retro_load_ok = false;
bool libretro_supports_bitmasks = false;
static bool libretro_supports_option_categories = false;
bool libretro_supports_ff_override = false;
bool libretro_ff_enabled = false;

int fb_width       = 640;
int fb_height      = 480;
int max_width      = 720;
int max_height     = 720;
float retro_aspect = (float)4.0f / (float)3.0f;
float view_aspect  = 1.0f;
float sample_rate  = 48000.0f;
float retro_fps    = 60.0f;
float sound_timer  = 50.0f; /* default STREAMS_UPDATE_ATTOTIME, changed later to `retro_fps` */
int video_changed  = 0;
int screen_configured = 0;

static bool draw_this_frame = true;
static int cpu_overclock = 100;

const char *retro_save_directory;
const char *retro_system_directory;
const char *retro_content_directory;

//FIXME: re-add way to handle 16/32 bit
#ifdef M16B
uint16_t videoBuffer[4096*3072];
#define LOG_PIXEL_BYTES 1
#else
unsigned int videoBuffer[4096*3072];
#define LOG_PIXEL_BYTES 2*1
#endif

/* FIXME: re-add way to handle OGL  */
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "retroogl.c"
#endif

void extract_basename(char *buf, const char *path, size_t size)
{
   char *ext = NULL;
   const char *base = strrchr(path, '/');

   if (!base)
      base = strrchr(path, '\\');
   if (!base)
      base = path;

   if (*base == '\\' || *base == '/')
      base++;

   strncpy(buf, base, size - 1);
   buf[size - 1] = '\0';

   ext = strrchr(buf, '.');
   if (ext)
      *ext = '\0';
}

void extract_directory(char *buf, const char *path, size_t size)
{
   char *base = NULL;

   strncpy(buf, path, size - 1);
   buf[size - 1] = '\0';

   base = strrchr(buf, '/');

   if (!base)
      base = strrchr(buf, '\\');

   if (base)
      *base = '\0';
   else
      buf[0] = '\0';

   base = strrchr(buf, '\"');
   if (base)
      strncpy(buf, base + 1, size - 1);
}

retro_log_printf_t log_cb = NULL;
retro_environment_t environ_cb = NULL;
retro_input_state_t input_state_cb = NULL;
retro_input_poll_t input_poll_cb = NULL;
retro_video_refresh_t video_cb = NULL;
retro_audio_sample_batch_t audio_batch_cb = NULL;

void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }

/* Audio output buffer */
static struct {
   int16_t *data;
   int32_t size;
   int32_t capacity;
} output_audio_buffer = {NULL, 0, 0};

static void ensure_output_audio_buffer_capacity(int32_t capacity)
{
   if (capacity <= output_audio_buffer.capacity) {
      return;
   }

   output_audio_buffer.data = (int16_t*)realloc(output_audio_buffer.data, capacity * sizeof(*output_audio_buffer.data));
   output_audio_buffer.capacity = capacity;
   log_cb(RETRO_LOG_DEBUG, "Output audio buffer capacity set to %d\n", capacity);
}

static void init_output_audio_buffer(int32_t capacity)
{
   output_audio_buffer.data = NULL;
   output_audio_buffer.size = 0;
   output_audio_buffer.capacity = 0;
   ensure_output_audio_buffer_capacity(capacity);
}

static void free_output_audio_buffer()
{
   free(output_audio_buffer.data);
   output_audio_buffer.data = NULL;
   output_audio_buffer.size = 0;
   output_audio_buffer.capacity = 0;
}

static void upload_output_audio_buffer()
{
   if (!audio_ready)
   {
      unsigned samples = (sample_rate / retro_fps);
      memset(output_audio_buffer.data + output_audio_buffer.size, 0, samples * sizeof(*output_audio_buffer.data));
      output_audio_buffer.size += samples;
   }
   audio_batch_cb(output_audio_buffer.data, output_audio_buffer.size / 2);
   output_audio_buffer.size = 0;

   audio_ready = false;
}

void retro_audio_queue(const int16_t *data, int32_t samples)
{
   if ((samples < 1) || retro_pause)
      return;

   if (output_audio_buffer.capacity - output_audio_buffer.size < samples)
      ensure_output_audio_buffer_capacity((output_audio_buffer.capacity + samples) * 1.5);

   memcpy(output_audio_buffer.data + output_audio_buffer.size, data, samples * sizeof(*output_audio_buffer.data));
   output_audio_buffer.size += samples;

   audio_ready = true;
}

/* LED interface */
static retro_set_led_state_t led_state_cb = NULL;
static unsigned int retro_led_state[2] = {0};

#define CDD_READ        0x0100
#define CDD_READY       0x0400
#define CDD_DATA        0x1000
#define CDD_SCSI        0x2000

int CDD_status = CDD_READY;

static void retro_led_interface(void)
{
   /* 0: Power
    * 1: CD */

   unsigned int led_state[2] = {0};
   unsigned int l            = 0;

   led_state[0] = (!retro_pause) ? 1 : 0;
   led_state[1] = (CDD_status & CDD_READ) ? 1 : 0;

   for (l = 0; l < sizeof(led_state)/sizeof(led_state[0]); l++)
   {
      if (retro_led_state[l] != led_state[l])
      {
         retro_led_state[l] = led_state[l];
         led_state_cb(l, led_state[l]);
      }
   }

   if (CDD_status & CDD_SCSI)
      CDD_status &= ~(CDD_READ | CDD_DATA);
}

void retro_fastforwarding(bool enabled)
{
   struct retro_fastforwarding_override ff_override;
   bool frontend_ff_enabled = false;

   if (!libretro_supports_ff_override)
      return;

   environ_cb(RETRO_ENVIRONMENT_GET_FASTFORWARDING, &frontend_ff_enabled);
   if (enabled && frontend_ff_enabled)
      return;

   ff_override.fastforward    = enabled;
   ff_override.inhibit_toggle = enabled;
   libretro_ff_enabled        = enabled;

   environ_cb(RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE, &ff_override);
}

static int ff_counter_on  = 0;
static int ff_counter_off = 0;
static void retro_autoloadfastforwarding(void)
{
   if (!libretro_supports_ff_override)
      return;

   if (     autoloadfastforward
         && !mame_machine_manager::instance()->machine()->video().fastforward())
   {
      int ff             = -1;
      int drive_led      = CDD_status & CDD_READ && CDD_status & CDD_DATA;

      if (!drive_led && libretro_ff_enabled)
      {
         int ff_max = (CDD_status & CDD_SCSI) ? 10 : 0;
         ff_counter_on = 0;
         ff_counter_off++;
         if (ff_counter_off > ff_max)
            ff = 0;
      }
      else if (drive_led && !libretro_ff_enabled)
      {
         ff_counter_off = 0;
         ff_counter_on++;
         if (ff_counter_on > 0)
            ff = 1;
      }
      else
      {
         ff_counter_off = ff_counter_on = 0;
         ff = -2;
      }

      if (ff > -1)
         retro_fastforwarding((ff > 1) ? false : (ff) ? true : false);
#if 0
      if (ff > -1)
         printf("CD FF:%2d led:%d - on:%3d off:%3d\n",
            ff, drive_led, ff_counter_on, ff_counter_off);
#endif
   }
}

static const struct retro_controller_description default_controllers[] =
{
   { "RetroPad", RETRO_DEVICE_JOYPAD },
   { "Keyboard", RETRO_DEVICE_KEYBOARD },
   { "None", RETRO_DEVICE_NONE },
   { NULL, 0 }
};

static void retro_set_inputs(void)
{
   const struct retro_controller_info ports[] =
   {
      { default_controllers, sizeof(default_controllers) / sizeof(default_controllers[0]) },
      { NULL, 0 }
   };

   environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

void retro_set_environment(retro_environment_t cb)
{
   struct retro_led_interface led_interface;
   bool option_categories = false;

   environ_cb = cb;

   libretro_set_core_options(environ_cb, &option_categories);
   libretro_supports_option_categories |= option_categories;

   retro_set_inputs();

   bool support_no_game = true;
   environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &support_no_game);

   if (environ_cb(RETRO_ENVIRONMENT_GET_LED_INTERFACE, &led_interface))
      if (led_interface.set_led_state && !led_state_cb)
         led_state_cb = led_interface.set_led_state;
}

static void update_runtime_variables(bool startup)
{
   // Skip at startup if using default speed, because
   // games like 'dragoona' use a CPU timing boothack
   if (startup && cpu_overclock == 100)
      return;

   // Update CPU Overclock
   if (mame_machine_manager::instance() != NULL && mame_machine_manager::instance()->machine() != NULL)
   {
      device_enumerator iter(mame_machine_manager::instance()->machine()->root_device());
      for (device_t &device : iter)
      {
         if (dynamic_cast<cpu_device *>(&device) != nullptr)
         {
            cpu_device* firstcpu = downcast<cpu_device *>(&device);
            firstcpu->set_clock_scale((float)cpu_overclock * 0.01f);
            break;
         }
      }
   }
}

static void check_variables(void)
{
   struct retro_variable var = {0};

   var.key   = CORE_NAME "_joystick_deadzone";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      strcpy(joystick_deadzone, var.value);
   }

   var.key   = CORE_NAME "_joystick_saturation";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      strcpy(joystick_saturation, var.value);
   }

   var.key   = CORE_NAME "_joystick_threshold";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      strcpy(joystick_threshold, var.value);
   }

   var.key   = CORE_NAME "_mame_4way_enable";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      mame_4way_enable = true;
      if (!strcmp(var.value, "disabled"))
         mame_4way_enable = false;
      if (!strcmp(var.value, "4way"))
         sprintf(mame_4way_map, "%s", "s8.4s8.44s8.4445");
      if (!strcmp(var.value, "strict"))
         sprintf(mame_4way_map, "%s", "ss8.sss8.4sss8.44s5.4445");
      if (!strcmp(var.value, "qbert"))
         sprintf(mame_4way_map, "%s", "4444s8888.4444s8888.444458888.444555888.ss5.222555666.222256666.2222s6666.2222s6666");
   }

   var.key   = CORE_NAME "_buttons_profiles";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         buttons_profiles = false;
      if (!strcmp(var.value, "enabled"))
         buttons_profiles = true;
   }

   var.key   = CORE_NAME "_mouse_enable";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         mouse_enable = false;
      if (!strcmp(var.value, "enabled"))
         mouse_enable = true;
   }

   var.key   = CORE_NAME "_lightgun_mode";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "touchscreen"))
         lightgun_mode = RETRO_SETTING_LIGHTGUN_MODE_POINTER;
      else if (!strcmp(var.value, "lightgun"))
         lightgun_mode = RETRO_SETTING_LIGHTGUN_MODE_LIGHTGUN;
      else
         lightgun_mode = RETRO_SETTING_LIGHTGUN_MODE_DISABLED;
   }

   var.key   = CORE_NAME "_lightgun_offscreen_mode";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "free"))
         lightgun_offscreen_mode = RETRO_SETTING_LIGHTGUN_OFFSCREEN_MODE_FREE;
      else if (!strcmp(var.value, "fixed (top left)"))
         lightgun_offscreen_mode = RETRO_SETTING_LIGHTGUN_OFFSCREEN_MODE_TOP_LEFT;
      else
         lightgun_offscreen_mode = RETRO_SETTING_LIGHTGUN_OFFSCREEN_MODE_BOTTOM_RIGHT;
   }

   var.key   = CORE_NAME "_rotation_mode";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "libretro"))
         rotation_mode = ROTATION_MODE_LIBRETRO;
      else if (!strcmp(var.value, "internal"))
         rotation_mode = ROTATION_MODE_INTERNAL;
      else if (!strcmp(var.value, "tate-rol"))
         rotation_mode = ROTATION_MODE_TATE_ROL;
      else if (!strcmp(var.value, "tate-ror"))
         rotation_mode = ROTATION_MODE_TATE_ROR;
      else
         rotation_mode = ROTATION_MODE_NONE;
   }

   var.key   = CORE_NAME "_alternate_renderer";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      bool alternate_renderer_prev = alternate_renderer;

      if (!strcmp(var.value, "disabled"))
         alternate_renderer = false;
      if (!strcmp(var.value, "enabled"))
         alternate_renderer = true;

      if (alternate_renderer != alternate_renderer_prev)
         video_changed = 2;
   }

   var.key   = CORE_NAME "_altres";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (alternate_renderer)
      {
         char *pch;
         char str[100];
         int width          = 640;
         int height         = 480;

         snprintf(str, sizeof(str), "%s", var.value);

         pch = strtok(str, "x");
         if (pch)
            width = strtoul(pch, NULL, 0);

         pch = strtok(NULL, "x");
         if (pch)
            height = strtoul(pch, NULL, 0);

         if (width != fb_width || height != fb_height)
         {
            fb_width      = width;
            fb_height     = height;
            video_changed = 2;

            /* Must use SET_SYSTEM_AV_INFO when max is not enough */
            if (fb_width > max_width || fb_height > max_height)
            {
               max_width     = fb_width;
               max_height    = fb_height;
               video_changed = 1;
            }
         }
      }
   }

   var.key   = CORE_NAME "_cpu_overclock";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      cpu_overclock = 100;
      if (strcmp(var.value, "default"))
         cpu_overclock = atoi(var.value);
   }

   var.key   = CORE_NAME "_autoloadfastforward";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         autoloadfastforward = false;
      else
         autoloadfastforward = true;
   }

   var.key   = CORE_NAME "_thread_mode";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         thread_mode = 1;
      else
         thread_mode = 0;
   }

   var.key   = CORE_NAME "_cheats_enable";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         cheats_enable = false;
      if (!strcmp(var.value, "enabled"))
         cheats_enable = true;
   }

   var.key   = CORE_NAME "_throttle";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         throttle_enable = false;
      if (!strcmp(var.value, "enabled"))
         throttle_enable = true;
   }

   var.key   = CORE_NAME "_boot_to_bios";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         boot_to_bios_enable = true;
      if (!strcmp(var.value, "disabled"))
         boot_to_bios_enable = false;
   }

   var.key   = CORE_NAME "_boot_to_osd";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         boot_to_osd_enable = true;
      if (!strcmp(var.value, "disabled"))
         boot_to_osd_enable = false;
   }

   var.key = CORE_NAME "_read_config";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         read_config_enable = false;
      if (!strcmp(var.value, "enabled"))
         read_config_enable = true;
   }

   var.key   = CORE_NAME "_write_config";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         write_config_enable = false;
      if (!strcmp(var.value, "enabled"))
         write_config_enable = true;
   }

   var.key   = CORE_NAME "_mame_paths_enable";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         mame_paths_enable = true;
      if (!strcmp(var.value, "disabled"))
         mame_paths_enable = false;
   }

   var.key   = CORE_NAME "_saves";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "game"))
         game_specific_saves_enable = true;
      if (!strcmp(var.value, "system"))
         game_specific_saves_enable = false;
   }

   var.key   = CORE_NAME "_auto_save";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "disabled"))
         auto_save_enable = false;
      if (!strcmp(var.value, "enabled"))
         auto_save_enable = true;
   }


   var.key   = CORE_NAME "_softlists_enable";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         softlist_enable = true;
      if (!strcmp(var.value, "disabled"))
         softlist_enable = false;
   }

   var.key   = CORE_NAME "_softlists_auto_media";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "enabled"))
         softlist_auto = true;
      if (!strcmp(var.value, "disabled"))
         softlist_auto = false;
   }

   var.key   = CORE_NAME "_media_type";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      sprintf(mediaType,"-%s",var.value);
   }
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));

   info->library_name     = "MAME";
   info->library_version  = build_version;
   info->valid_extensions = "cmd|zip|7z";
   info->need_fullpath    = true;
   info->block_extract    = true;
}

void update_geometry(void)
{
   struct retro_system_av_info av_info;
   av_info.geometry.base_width   = fb_width;
   av_info.geometry.base_height  = fb_height;
   av_info.geometry.aspect_ratio = retro_aspect;
   environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
   video_changed = 0;
}

void update_av_info(void)
{
   struct retro_system_av_info av_info;
   retro_get_system_av_info(&av_info);
   environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &av_info);
   video_changed = 0;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.base_width   = fb_width;
   info->geometry.base_height  = fb_height;
   info->geometry.aspect_ratio = retro_aspect;

   info->geometry.max_width    = max_width;
   info->geometry.max_height   = max_height;

   info->timing.fps            = retro_fps;
   info->timing.sample_rate    = sample_rate;
}

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   (void)level;
   va_list va;
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

void retro_init(void)
{
   const char *system_dir  = NULL;
   const char *content_dir = NULL;
   const char *save_dir    = NULL;

//FIXME: re-add way to handle 16/32 bit
#ifdef M16B
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
#else
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
#endif

   struct retro_log_callback log;
   log_cb = fallback_log;
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir) && system_dir)
   {
      /* if defined, use the system directory */
      retro_system_directory = system_dir;
   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY, &content_dir) && content_dir)
   {
      /* if defined, use the content directory */
      retro_content_directory = content_dir;
   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save_dir) && save_dir)
   {
      /* If save directory is defined use it,
       * otherwise use system directory. */
      retro_save_directory = *save_dir ? save_dir : retro_system_directory;
   }
   else
   {
      /* make retro_save_directory the same,
       * in case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY
       * is not implemented by the frontend. */
      retro_save_directory = retro_system_directory;
   }

   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      log_cb(RETRO_LOG_ERROR, "pixel format not supported\n");
      exit(0);
   }

   #define input_descriptor_macro(c) \
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Joy Left" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Joy Right" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Joy Up" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Joy Down" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "X" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "Y" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "L" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      "R" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,     "L2" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3,     "L3" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,     "R2" },\
      { c, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3,     "R3" },\
      { c, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X, "Left Stick X" },\
      { c, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y, "Left Stick Y" },\
      { c, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X, "Right Stick X" },\
      { c, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y, "Right Stick Y" },

   struct retro_input_descriptor input_descriptors[] =
   {
      input_descriptor_macro(0)
      input_descriptor_macro(1)
      input_descriptor_macro(2)
      input_descriptor_macro(3)
      input_descriptor_macro(4)
      input_descriptor_macro(5)
      input_descriptor_macro(6)
      input_descriptor_macro(7)
      { 0 },
   };
   #undef input_descriptor_macro
   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, input_descriptors);

   if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
      libretro_supports_bitmasks = true;

   if (environ_cb(RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE, NULL))
      libretro_supports_ff_override = true;

   static struct retro_keyboard_callback keyboard_callback = {retro_keyboard_event};
   environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &keyboard_callback);

   memset(videoBuffer, 0, sizeof(videoBuffer));
   init_output_audio_buffer(2048);

   retro_pause = 0;

   log_cb(RETRO_LOG_INFO, "------------------------\n");
   log_cb(RETRO_LOG_INFO, "MAME %s\n", build_version);
   log_cb(RETRO_LOG_INFO, "------------------------\n");
}

void retro_deinit(void)
{
   free_output_audio_buffer();
   if (retro_load_ok)
      retro_finish();
}

void retro_reset(void)
{
   mame_reset = 1;
}

void retro_run(void)
{
   bool updated = false;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
   {
      check_variables();
      update_runtime_variables(false);
   }

   if (!retro_pause)
      retro_main_loop();
   RLOOP = 1;

   /* Automatic loading fast-forward */
   if (autoloadfastforward)
      retro_autoloadfastforwarding();

   /* LED interface */
   if (led_state_cb)
      retro_led_interface();

   if (first_run)
   {
      /* Skip drawing the first frame due to a gray border */
      first_run       = false;
      draw_this_frame = false;
   }

//FIXME: re-add way to handle OGL
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
   do_glflush();
#else
   if (draw_this_frame)
      video_cb(videoBuffer, fb_width, fb_height, fb_width << LOG_PIXEL_BYTES);
   else
      video_cb(NULL, fb_width, fb_height, fb_width << LOG_PIXEL_BYTES);
#endif
   upload_output_audio_buffer();

   if (video_changed == 1)
      update_av_info();
   else if (video_changed == 2)
      update_geometry();
}

bool retro_load_game(const struct retro_game_info *info)
{
   retro_load_ok = false;

   check_variables();

//FIXME: re-add way to handle OGL
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#if defined(HAVE_OPENGLES)
   hw_render.context_type = RETRO_HW_CONTEXT_OPENGLES2;
#else
   hw_render.context_type = RETRO_HW_CONTEXT_OPENGL;
#endif
   hw_render.context_reset = context_reset;
   hw_render.context_destroy = context_destroy;
   /*
   hw_render.depth = true;
   hw_render.stencil = true;
   hw_render.bottom_left_origin = true;
   */
   if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
      return false;
#endif

   g_rom_dir[0] = '\0';
   RPATH[0]     = '\0';

   if (info)
   {
      extract_directory(g_rom_dir, info->path, sizeof(g_rom_dir));
      strcpy(RPATH, info->path);
   }

   int res = mmain2(1, RPATH);

   /* Force success with empty content */
   if (!RPATH[0])
      res = 0;

   if (res != 0)
   {
      /* Must wait a bit for failure to finish properly */
      log_cb(RETRO_LOG_DEBUG, "%s osd_sleep\n", __func__);
      osd_sleep(osd_ticks_per_second());
      log_cb(RETRO_LOG_DEBUG, "%s osd_sleep done\n", __func__);
      return false;
   }

   retro_load_ok = true;
   update_runtime_variables(true);

   return true;
}

void retro_unload_game(void)
{
   if (     mame_machine_manager::instance() != NULL
         && mame_machine_manager::instance()->machine() != NULL
         && mame_machine_manager::instance()->machine()->options().autosave()
         && (mame_machine_manager::instance()->machine()->system().flags & MACHINE_SUPPORTS_SAVE) != 0)
	  mame_machine_manager::instance()->machine()->immediate_save("auto");

   if (retro_pause == 0)
      retro_pause = -1;
}

/* Stubs */
size_t retro_serialize_size(void)
{
   if (     mame_machine_manager::instance() != NULL
	      && mame_machine_manager::instance()->machine() != NULL
	      && ram_state::get_size(mame_machine_manager::instance()->machine()->save()) > 0)
      return ram_state::get_size(mame_machine_manager::instance()->machine()->save());

   return 0;
}
bool retro_serialize(void *data, size_t size)
{
   if (     mame_machine_manager::instance() != NULL
	      && mame_machine_manager::instance()->machine() != NULL
	      && ram_state::get_size(mame_machine_manager::instance()->machine()->save()) > 0)
      return (mame_machine_manager::instance()->machine()->save().write_buffer((u8*)data, size) == STATERR_NONE);

   return false;
}
bool retro_unserialize(const void *data, size_t size)
{
   if (     mame_machine_manager::instance() != NULL
         && mame_machine_manager::instance()->machine() != NULL
         &&	ram_state::get_size(mame_machine_manager::instance()->machine()->save()) > 0)
      return (mame_machine_manager::instance()->machine()->save().read_buffer((u8*)data, size) == STATERR_NONE);

   return false;
}

unsigned retro_get_region (void) { return RETRO_REGION_NTSC; }

void *find_mame_bank_base(offs_t start, address_space &space)
{
   for (auto &bank : mame_machine_manager::instance()->machine()->memory().banks())
      // if ( bank.second->addrstart() == start)
         return bank.second->base();
   return NULL;
}

void *retro_get_memory_data(unsigned type)
{
   void *best_match1 = NULL;
   void *best_match2 = NULL;
   void *best_match3 = NULL;
   int space_index   = 0;

   /* Eventually the RA cheat system can be updated to accommodate multiple memory
    * locations, but for now this does a pretty good job for MAME since most of the machines
    * have a single primary RAM segment that is marked read/write as AMH_RAM.
    *
    * This will find a best match based on certain qualities of the address_map_entry objects.
    */
   if (     type == RETRO_MEMORY_SYSTEM_RAM
         && mame_machine_manager::instance() != NULL
         && mame_machine_manager::instance()->machine() != NULL)
   {
      memory_interface_enumerator iter(mame_machine_manager::instance()->machine()->root_device());
      for (device_memory_interface &memory : iter)
      {
         for (space_index = 0; space_index < memory.num_spaces(); space_index++)
         {
            if (memory.has_space(space_index))
            {
               auto &space = memory.space(space_index);
               for (address_map_entry &entry : space.map()->m_entrylist)
               {
                  if (entry.m_read.m_type == AMH_RAM)
                  {
                     if (entry.m_write.m_type == AMH_RAM)
                     {
                        if (entry.m_share == NULL)
                           best_match1 = find_mame_bank_base(entry.m_addrstart, space);
                        else
                           best_match2 = find_mame_bank_base(entry.m_addrstart, space);
                     }
                     else
                        best_match3 = find_mame_bank_base(entry.m_addrstart, space);
                  }
               }
            }
         }
      }
   }
   return (best_match1 != NULL ? best_match1 : (best_match2 != NULL) ? best_match2 : best_match3);
}

size_t retro_get_memory_size(unsigned type)
{
   size_t best_match1 = 0;
   size_t best_match2 = 0;
   size_t best_match3 = 0;
   int space_index    = 0;

   if (     type == RETRO_MEMORY_SYSTEM_RAM
         && mame_machine_manager::instance() != NULL
         && mame_machine_manager::instance()->machine() != NULL)
   {
      memory_interface_enumerator iter(mame_machine_manager::instance()->machine()->root_device());
      for (device_memory_interface &memory : iter)
      {
         for (space_index = 0; space_index < memory.num_spaces(); space_index++)
         {
            if (memory.has_space(space_index))
            {
               auto &space = memory.space(space_index);
               for (address_map_entry &entry : space.map()->m_entrylist)
               {
                  if (entry.m_read.m_type == AMH_RAM)
                  {
                     if (entry.m_write.m_type == AMH_RAM)
                     {
                        if (entry.m_share == NULL)
                           best_match1 = entry.m_addrend - entry.m_addrstart + 1;
                        else
                           best_match2 = entry.m_addrend - entry.m_addrstart + 1;
                     }
                     else
                        best_match3 = entry.m_addrend - entry.m_addrstart + 1;
                  }
               }
            }
         }
      }
   }

   return (best_match1 != 0 ? best_match1 : (best_match2 != 0) ? best_match2 : best_match3);
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) { return false; }
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned unused, bool unused1, const char* unused2) {}
void retro_set_controller_port_device(unsigned in_port, unsigned device) {}

void *retro_get_fb_ptr(void)
{
   return videoBuffer;
}

void retro_frame_draw_enable(bool enable)
{
   draw_this_frame = enable;
}
