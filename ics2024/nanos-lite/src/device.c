#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

static bool has_key = 0;
static int sbsize = 0;


static int screen_w, screen_h;
static size_t screen_size;


size_t serial_write(const void *buf, size_t offset, size_t len) {
  size_t i;
  for(i = 0; i < len; i++) {
    putch(((char *)buf)[i]); // write the data to the serial port
  }
  return i;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode != AM_KEY_NONE) {
      return snprintf(buf, len, "k%c %s\n", ev.keydown ? 'd' : 'u', keyname[ev.keycode]);
    }
  return 0;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return snprintf(buf, len, "WIDTH :%d\nHEIGHT:%d\n", screen_w, screen_h);
}


size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int width = cfg.width;
  len /= sizeof(uint32_t);
  offset /= sizeof(uint32_t);
  int y = offset / width;
  int x = offset % width;
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len, 1, true);
  return len;
}


void init_device() {
  Log("Initializing devices...");
  ioe_init();
  has_key = io_read(AM_INPUT_CONFIG).present;
  if (has_key)
    Log("Input device has been detected!");
  AM_GPU_CONFIG_T information = io_read(AM_GPU_CONFIG);
  screen_w = information.width;
  screen_h = information.height;
  screen_size = screen_w * screen_h * sizeof(uint32_t);
  Log("Initializing Screen, size: %d x %d\n", screen_w, screen_h);
  sbsize = io_read(AM_AUDIO_CONFIG).bufsize;
  Log("Initializing Audio, buffer size: %d\n", sbsize);
}
