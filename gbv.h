#ifndef __GBV_H__
#define __GBV_H__

/* in case you're compiling this as a library */
#ifndef GBV_API
#define GBV_API
#endif

#define GBV_VERSION_MAJOR 1
#define GBV_VERSION_MINOR 0
#define GBV_VERSION_PATCH 3

#define GBV_TILE_MEMORY_SIZE   6144
#define GBV_BG_MAP_MEMORY_SIZE 1024
#define GBV_OAM_MEMORY_SIZE    160
#define GBV_HW_MEMORY_SIZE     (64 * 1024)

#define GBV_SCREEN_WIDTH       160
#define GBV_SCREEN_HEIGHT      144
#define GBV_SCREEN_SIZE        (GBV_SCREEN_WIDTH * GBV_SCREEN_HEIGHT)

#define GBV_BG_TILES_X         32
#define GBV_BG_TILES_Y         32
#define GBV_BG_TILE_COUNT      (GBV_BG_TILES_X * GBV_BG_TILES_Y)

#define GBV_WND_TILES_X        20
#define GBV_WND_TILES_Y        18
#define GBV_WND_TILE_COUNT     (GBV_WND_TILES_X * GBV_WND_TILES_Y)

#define GBV_TILE_WIDTH         8
#define GBV_TILE_HEIGHT        8
#define GBV_TILE_PITCH         2
#define GBV_TILE_SIZE          (GBV_TILE_PITCH * GBV_TILE_HEIGHT)

#define GBV_OBJ_COUNT          40
#define GBV_OBJ_SIZE           (4 * GBV_OBJ_COUNT)

typedef char           gbv_s8;
typedef short          gbv_s16;
typedef unsigned char  gbv_u8;
typedef unsigned short gbv_u16;

typedef unsigned char  gbv_io;

typedef enum {
	GBV_RENDER_MODE_GRAYSCALE_8,	// 0-255
} gbv_render_mode;

typedef struct {
	gbv_u8 colors[4];
} gbv_palette;

typedef enum {
	GBV_LCDC_BG_ENABLE = 0x01,       /* enable bg display */
	GBV_LCDC_OBJ_ENABLE = 0x02,      /* enable obj display */
	GBV_LCDC_OBJ_SIZE_SELECT = 0x04, /* enable 8x16 obj mode */
	GBV_LCDC_BG_MAP_SELECT = 0x08,   /* bg tile map display select */
	GBV_LCDC_BG_DATA_SELECT = 0x10,  /* bg & wnd tile data select */
	GBV_LCDC_WND_ENABLE = 0x20,      /* enable wnd display */
	GBV_LCDC_WND_MAP_SELECT = 0x40,  /* wnd tile map display select */
	GBV_LCDC_CTRL = 0x80,            /* enable lcd */
} gbv_lcdc_flag;

typedef struct {
	gbv_u8 data[8][2];
} gbv_tile;

typedef struct {
	gbv_u8 y;
	gbv_u8 x;
	gbv_u8 id;
	gbv_u8 attr;
} gbv_obj_char;

/*****************************/
/*** I/O control registers ***/
/*****************************/

/* lcd control */
extern GBV_API gbv_io gbv_io_lcdc;

/* 
  palettes:
    - defined as 4 packed colors at 2 bits per color
    - for objects, 00 means transparent
*/
extern GBV_API gbv_io gbv_io_bgp;
extern GBV_API gbv_io gbv_io_obp0;
extern GBV_API gbv_io gbv_io_obp1;

/* bg scroll registers */
extern GBV_API gbv_io gbv_io_scx;
extern GBV_API gbv_io gbv_io_scy;

/* wnd position */
extern GBV_API gbv_io gbv_io_wx;
extern GBV_API gbv_io gbv_io_wy;

/*****************************/
/************ API ************/
/*****************************/

/* version information */
extern GBV_API void gbv_get_version(int * maj, int * min, int * patch);

/* initialize system, provide GBV_HW_MEMORY_SIZE (64k) of memory */
extern GBV_API void gbv_init(void * memory);

/* utility function for lcd control */
extern GBV_API void gbv_lcdc_set(gbv_lcdc_flag flag);
extern GBV_API void gbv_lcdc_reset(gbv_lcdc_flag flag);

/* return raw pointers for data specification */
extern GBV_API gbv_u8 * ggbv_get_rom_data();
extern GBV_API gbv_u8 * gbv_get_tile_map0();
extern GBV_API gbv_u8 * gbv_get_tile_map1();

extern GBV_API gbv_u8   * gbv_get_tile_data();
extern GBV_API gbv_tile * gbv_get_tile(gbv_u8 tile_id);

/* copy GBV_OBJ_SIZE bytes of data to OAM memory */
extern GBV_API void gbv_transfer_oam_data(gbv_obj_char objs[GBV_OBJ_COUNT]);

/* render all data to target buffer */
extern GBV_API void gbv_render(void * render_buffer, gbv_render_mode mode, gbv_palette * palette);

#endif