#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int fdsb = -1;
static int fdsbctl = -1;

static int canvas_x = 0, canvas_y = 0; // canvas position
static int screen_w = 0, screen_h = 0; // screen size

static uint32_t time_start = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return ((uint32_t)tv.tv_sec) * 1000 + tv.tv_usec / 1000 - time_start;
}

int NDL_PollEvent(char *buf, int len) {
  if (!buf) return 0;
  int ret = read(evtdev, buf, len);
  assert(ret != -1 && ret < len);
  return ret;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%d %d", screen_w, screen_h);
    write(fbctl, buf, len);
    while (1) {
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  } else {
    if (*w == 0 || *w > screen_w) *w = screen_w;
    if (*h == 0 || *h > screen_h) *h = screen_h;
    canvas_x = (screen_w - *w) / 2;
    canvas_y = (screen_h - *h) / 2;
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  for (int i = 0; i < h; i++) {
    lseek(fbdev, ((canvas_y + y + i) * screen_w + x + canvas_x) * 4, SEEK_SET);
    write(fbdev, &pixels[i * w], w * 4);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  int spec[3] = {freq, channels, samples};
  write(fdsbctl, spec, sizeof(spec));
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return write(fdsb, buf, len);
}

int NDL_QueryAudio() {
  char buf[16];
  read(fdsbctl, buf, sizeof(buf));
  return atoi(buf);
}

static void init_event() {
  evtdev = open("/dev/events", O_RDONLY);
  assert(evtdev != -1);
}

static void init_display() {
  fbdev = open("/dev/fb", O_RDWR);
  assert(fbdev != -1);
  int dispinfo = open("/proc/dispinfo", O_RDONLY);
  assert(dispinfo != -1);
  char buf[64];
  char *tmp;
  int nread = read(dispinfo, buf, sizeof(buf));
  assert(nread > 0 && nread < sizeof(buf));
  for (tmp = strtok(buf, "\n"); tmp; tmp = strtok(NULL, "\n")) {
    if (sscanf(tmp, " WIDTH : %d", &screen_w) == 1) continue;
    if (sscanf(tmp, " HEIGHT : %d", &screen_h) == 1) continue;
    tmp += strspn(tmp, " \t");
    if (tmp[0] == '#' || tmp[0] == '\0') continue;
    fprintf(stderr, "/proc/dispinfo: syntax error near '%s'\n", tmp);
    assert(0);
  }
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  } else {
    init_event();
    init_display();
  }
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_start = ((uint32_t)tv.tv_sec) * 1000 + tv.tv_usec / 1000;
  return 0;
}

void NDL_Quit() {
  if (!getenv("NWM_APP")) {
    close(evtdev);
    close(fbdev);
    close(fdsb);
  }
}
