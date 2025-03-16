#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i = 0;
  int w = 400;
  int h = 300;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for(i = 0; i < w * h; i++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t screen = inl(VGACTL_ADDR);
  uint32_t wid = screen >> 16, hei = screen & 0xffff;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = wid, .height = hei,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h; // x：绘制的水平起始点 y：绘制的垂直起始点 w：绘制的矩形宽度 h：绘制的矩形高度
  if(!ctl->sync && (w == 0 || h == 0)) return;
  else outl(SYNC_ADDR, 1);
  
  uint32_t *pixels = ctl->pixels; // pixels：绘制的矩形内所有像素点的颜色
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  for(int i = y; i < y + h; i++){
    for(int j = x; j < x + w; j++){
      fb[i * screen_w + j] = pixels[w * (i - y) + (j - x)];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
