#ifndef __ROMIMAGE_H__
#define __ROMIMAGE_H__

#define ROMIMAGE(w) ((romimage_t*)((w)->priv))

typedef struct {
    int16_t w, h;
    char filename[16];
    pixel_t *fb;
} romimage_t;

#endif
