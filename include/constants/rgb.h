#ifndef GUARD_RGB_H
#define GUARD_RGB_H

#define GET_R(color) ((color) & 31)
#define GET_G(color) (((color) >> 5) & 31)
#define GET_B(color) (((color) >> 10) & 31)

#define RGB(r, g, b)  ((r) | ((g) << 5) | ((b) << 10))
#define RGB2(r, g, b) (((b) << 10) | ((g) << 5) | (r))
#define _RGB(r, g, b) ((((b) & 31) << 10) + (((g) & 31) << 5) + ((r) & 31))
#define RGB2GBA(r, g, b) (((r >> 3) & 31) | (((g >> 3) & 31) << 5) | (((b >> 3) & 31) << 10))

#define RGB_ALPHA       (1 << 15)
#define IS_ALPHA(color) ((color) & RGB_ALPHA)

#define RGB_BLACK      RGB(0, 0, 0)
#define RGB_WHITE      RGB(31, 31, 31)
#define RGB_RED        RGB(31, 0, 0)
#define RGB_GREEN      RGB(0, 31, 0)
#define RGB_BLUE       RGB(0, 0, 31)
#define RGB_PURPLE     RGB(24, 0, 24)
#define RGB_YELLOW     RGB(31, 31, 0)
#define RGB_MAGENTA    RGB(31, 0, 31)
#define RGB_CYAN       RGB(0, 31, 31)
#define RGB_WHITEALPHA (RGB_WHITE | RGB_ALPHA)

#define RGB_LIME_GREEN      RGB2GBA(222, 230, 49)
#define RGB_DARK_BLUE       RGB2GBA(46, 68, 130)
#define RGB_ORANGE          RGB2GBA(250, 144, 98)
#define RGB_LIGHT_YELLOW    RGB2GBA(255, 242, 204)

#endif // GUARD_RGB_H
