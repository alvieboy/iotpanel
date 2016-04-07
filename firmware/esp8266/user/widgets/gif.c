#include "widget.h"
#include "gif.h"
#include "smallfs.h"
#include "ets_sys.h"
#include "osapi.h"
#include "alloc.h"
#include <string.h>
#include "protos.h"

#define DEFAULT_DELAY 10

LOCAL void * ICACHE_FLASH_ATTR gif_bitmap_create(int width, int height)
{
    return os_calloc(width * height,1);
}


LOCAL void ICACHE_FLASH_ATTR gif_bitmap_set_opaque(void *bitmap, bool opaque)
{
    (void) opaque;  /* unused */
    (void) bitmap;  /* unused */
 //   assert(bitmap);
}


LOCAL bool ICACHE_FLASH_ATTR gif_bitmap_test_opaque(void *bitmap)
{
    (void) bitmap;  /* unused */
 //   assert(bitmap);
    return false;
}


LOCAL unsigned char *ICACHE_FLASH_ATTR gif_bitmap_get_buffer(void *bitmap)
{
    //assert(bitmap);
    return bitmap;
}


LOCAL void ICACHE_FLASH_ATTR gif_bitmap_destroy(void *bitmap)
{
    //assert(bitmap);
    os_free(bitmap);
}


LOCAL void ICACHE_FLASH_ATTR gif_bitmap_modified(void *bitmap)
{
    (void) bitmap;  /* unused */
    //assert(bitmap);
    return;
}

LOCAL gif_bitmap_callback_vt gif_callbacks = {
    gif_bitmap_create,
    gif_bitmap_destroy,
    gif_bitmap_get_buffer,
    gif_bitmap_set_opaque,
    gif_bitmap_test_opaque,
    gif_bitmap_modified
};

LOCAL void ICACHE_FLASH_ATTR gif_redraw(widget_t *w, int x, int y, gfxinfo_t *gfx)
{
    gif_t *r = GIF(w);

    if ((r->w<0) || (r->h<0) || (r->fb==NULL) || (r->cframe<0))
        return;

    pixel_t *pix = &gfx->fb[x];
    pix += y * gfx->stride;
    pixel_t *src = &r->fb[r->cframe][0];
    int cx,cy;
    for (cy=0;cy<r->h;cy++) {
        for (cx=0;cx<r->w;cx++) {
            // TODO: Clip
            pix[cx] = *src;
            src++;
        }
        pix += gfx->stride;
    }
    if (r->delay<=0) {
        r->cframe++;
        if(r->cframe  == r->frames) {
            r->cframe = 0;
        }
        r->delay = DEFAULT_DELAY;
    } else {
        r->delay--;
    }
}

LOCAL int loadgif(gif_t *w, struct smallfsfile *file)
{
    gif_animation gif;
    size_t size;
    gif_result code;
    unsigned int i;

    /* create our gif animation */
    gif_create(&gif, &gif_callbacks);

    size = smallfsfile__getSize(file);
    unsigned char *data = (unsigned char*)os_malloc(size);

    /* load file into memory */
    smallfsfile__read(file, data, size);

    /* begin decoding */
    do {
        code = gif_initialise(&gif, size, data);
        if (code != GIF_OK && code != GIF_WORKING) {
            //warning("gif_initialise", code);
            os_printf("Cannot initialise, code %d\n",code);
        }
    } while (code != GIF_OK);
    os_printf("GIF loaded OK");

    // Found. Allocate and link
    w->w = gif.width;
    w->h = gif.height;

    unsigned asize = w->w * w->h;

    w->cframe = -1;
    w->frames = gif.frame_count;
    w->fb = os_malloc( sizeof(void*) * w->frames );

    for (i = 0; i != gif.frame_count; i++) {
        unsigned int row, col;
        unsigned char *image;

        w->fb[i] = (pixel_t*)os_malloc(asize);

        code = gif_decode_frame(&gif, i);
        if (code != GIF_OK)
            os_printf("gif_decode_frame", code);

        os_printf("colour_table_size %d\n", gif.colour_table_size);
        os_printf("global_colours %d\n", gif.global_colours);
        os_printf("loop_count %d\n", gif.loop_count);
        os_printf("# frame %u:\n", i);
        image = (unsigned char *) gif.frame_image;
        for (row = 0; row != gif.height; row++) {
            for (col = 0; col != gif.width; col++) {
                size_t z = (row * gif.width + col) * 1;
                w->fb[i][row*gif.width + col] = image[z];
            }
        }
    }

    /* clean up */
    gif_finalise(&gif);
    os_free(data);
    w->cframe = 0;

    return 0;
}

LOCAL int ICACHE_FLASH_ATTR gif_set_filename(widget_t *w, const char *v)
{
    gif_t *r = GIF(w);

    if (r->fb != NULL) {
        void *oldfb = r->fb;
        r->fb = NULL;
        os_free(oldfb);
    }


    // Open it up.
    struct smallfsfile file;
    strncpy(r->filename,v,sizeof(r->filename));

    if (smallfs__start() == 0) {
        if (smallfs__open(smallfs__getfs(), &file, r->filename)==0) {
            if (smallfsfile__valid(&file)) {
                loadgif(r,&file);
               // smallfsfile__read(&file, r->fb, size);
            } else {
                os_free(r->fb);
                r->fb = NULL;
                os_printf("Cannot open file %s!!!\n", r->filename);
            }
        } else {
            os_printf("Cannot open file %s!!!\n", r->filename);
        }
        smallfs__end(smallfs__getfs());
    } else {
        os_printf("Could not open smallfs!!!\n");
    }

    if (r->fb == NULL) {
        r->filename[0] = '\0';
        return -1;
    }
    return 0;
}

LOCAL void *ICACHE_FLASH_ATTR gif_new(void*what)
{
    gif_t *r = os_calloc(sizeof(gif_t),1);
    r->fb = NULL;
    r->w = -1;
    r->h = -1;
    r->cframe = -1;
    r->delay = DEFAULT_DELAY;
    r->filename[0] = '\0';
    return r;
}

LOCAL void ICACHE_FLASH_ATTR gif_destroy(void*what)
{
    gif_t *r = GIF( (widget_t*)what);
    if (r->fb) {
        os_free(r->fb);
    }
    os_free(what);
}

STRING_GETTER( gif_t, filename );

static property_t properties[] = {
    { 1, T_STRING,"filename",   SETTER(gif_set_filename), GETTER(gif_t_get_filename) },
    END_OF_PROPERTIES
};

widgetdef_t gif_widget = {
    .name = "gif",
    .properties = properties,
    .alloc = &gif_new,
    .redraw = &gif_redraw,
    .destroy = &gif_destroy
};
