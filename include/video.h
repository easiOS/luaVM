#ifndef H_VIDEO
#define H_VIDEO

#include <stdint.h>

typedef enum {
  FILL,
  LINE
} vdrawmode_t;

typedef struct {
  uint8_t r,g,b,a;
} __attribute__((packed)) rgb_t;

void vinit(int64_t width, int64_t height, int64_t bpp, int64_t pitch, uint64_t addr);
void vdestroy();
void vplot(int64_t x, int64_t y);
void vd_circle(int x0, int y0, int radius);
void vsetcol(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
rgb_t vgetcol();
void vcls();
void vd_print(int64_t x, int64_t y, const char* str, int64_t* xe, int64_t* ye);
void vd_printl(int64_t x, int64_t y, const char* str, int xl, int yl);
void vd_line(int64_t x1, int64_t y1, int64_t x2, int64_t y2);
void vd_triangle(vdrawmode_t drawmode, int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t x3, int64_t y3);
void vd_rectangle(vdrawmode_t drawmode, int64_t x, int64_t y, int64_t w, int64_t h);
int64_t vgetw();
int64_t vgeth();
void vswap();
void vd_bitmap16(uint16_t* bitmap, int64_t x, int64_t y, int64_t h);
void vd_bitmap32(uint32_t* bitmap, int64_t x, int64_t y, int64_t h);
void vplot_nb(int64_t x, int64_t y);
int vwidth(void);
int vheight(void);
uint32_t* bitmap16_to32(uint32_t* dest, uint16_t* src);
void vd_print32(int64_t x, int64_t y, const char* str, int64_t* xe, int64_t* ye);

#endif
