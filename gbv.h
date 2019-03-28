#ifndef __GBV_H__
#define __GBV_H__

#define GBV_VERSION_MAJOR 1
#define GBV_VERSION_MINOR 0
#define GBV_VERSION_PATCH 0

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

#define GBV_TILE_WIDTH         8
#define GBV_TILE_HEIGHT        8
#define GBV_TILE_PITCH         2
#define GBV_TILE_SIZE          (GBV_TILE_PITCH * GBV_TILE_HEIGHT)

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

/*****************************/
/*** I/O control registers ***/
/*****************************/

/* lcd control */
extern gbv_io gbv_io_lcdc;

/* 
  palettes:
    - defined as 4 packed colors at 2 bits per color
    - for objects, 00 means transparent
*/
extern gbv_io gbv_io_bgp;
extern gbv_io gbv_io_obp0;
extern gbv_io gbv_io_obp1;

/* bg scroll registers */
extern gbv_io gbv_io_scx;
extern gbv_io gbv_io_scy;

/* wnd position */
extern gbv_io gbv_io_wx;
extern gbv_io gbv_io_wy;

/*****************************/
/************ API ************/
/*****************************/

/* version information */
void gbv_get_version(int * maj, int * min, int * patch);

/* initialize system, provide GBV_HW_MEMORY_SIZE (64k) of memory */
void gbv_init(void * memory);

/* utility function for lcd control */
void gbv_lcdc_set(gbv_lcdc_flag flag);
void gbv_lcdc_reset(gbv_lcdc_flag flag);

/* return raw pointers for data specification */
gbv_u8 * ggbv_get_rom_data();
gbv_u8 * gbv_get_bg_map_data1();
gbv_u8 * gbv_get_bg_map_data2();
gbv_u8 * gbv_get_oam_data();

gbv_u8   * gbv_get_tile_data();
gbv_tile * gbv_get_tile(gbv_u8 tile_id);

/* render all data to target buffer */
void gbv_render(void * render_buffer, gbv_render_mode mode, gbv_palette * palette);

#endif