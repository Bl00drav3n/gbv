#include "gbv.h"

/* module globals */
gbv_io gbv_io_lcdc = 0;
gbv_io gbv_io_bgp  = 0;
gbv_io gbv_io_obp0 = 0;
gbv_io gbv_io_obp1 = 0;
gbv_io gbv_io_scx  = 0;
gbv_io gbv_io_scy  = 0;
gbv_io gbv_io_wx   = 0;
gbv_io gbv_io_wy   = 0;

/* internal data pointers */
static gbv_u8 * gbv_mem;
static gbv_u8 * gbv_tile_data;
static gbv_u8 * gbv_bg_map_data1;
static gbv_u8 * gbv_bg_map_data2;
static gbv_u8 * gbv_oam_data;

/* internal functions */
static gbv_u8 get_pixel_from_tile_row(gbv_u8 * row, gbv_u8 x, gbv_u8 pal) {
	gbv_u8 pal_idx = ((row[0] >> (7 - x)) & 0x01) | ((row[1] >> (7 - x) & 0x01) << 1);
	gbv_u8 color = (pal >> (2 * pal_idx)) & 0x03;
	return color;
}

/* API functions */
void gbv_get_version(int * maj, int * min, int * patch) {
	if (maj) {
		*maj = GBV_VERSION_MAJOR;
	}
	if (min) {
		*min = GBV_VERSION_MINOR;
	}
	if (patch) {
		*patch = GBV_VERSION_PATCH;
	}
}

void gbv_init(void * memory) {
	gbv_mem          = (gbv_u8*)memory;
	gbv_tile_data    = gbv_mem + 0x8000;
	gbv_bg_map_data1 = gbv_mem + 0x9800;
	gbv_bg_map_data2 = gbv_mem + 0x9C00;
	gbv_oam_data     = gbv_mem + 0xFE00;
}

void gbv_lcdc_set(gbv_lcdc_flag flag) {
	gbv_io_lcdc = gbv_io_lcdc | flag;
}

void gbv_lcdc_reset(gbv_lcdc_flag flag) {
	gbv_io_lcdc = gbv_io_lcdc & ~flag;
}

gbv_u8 * ggbv_get_rom_data() {
	return gbv_mem;
}

gbv_u8 * gbv_get_bg_map_data1() {
	return gbv_bg_map_data1;
}

gbv_u8 * gbv_get_bg_map_data2() {
	return gbv_bg_map_data2;
}

gbv_u8 * gbv_get_oam_data() {
	return gbv_oam_data;
}

gbv_u8 * gbv_get_tile_data() {
	return gbv_tile_data;
}

gbv_tile * gbv_get_tile(gbv_u8 tile_id) {
	return (gbv_tile*)gbv_tile_data + tile_id;
}

void gbv_render(void * render_buffer, gbv_render_mode mode, gbv_palette * palette) {
	gbv_u8 * buffer = (gbv_u8*)render_buffer;
	gbv_u8 * bg_map = (gbv_io_lcdc & GBV_LCDC_BG_MAP_SELECT) ? gbv_bg_map_data2 : gbv_bg_map_data1;
	gbv_u8 * bg_tile_data = (gbv_io_lcdc & GBV_LCDC_BG_DATA_SELECT) ? gbv_tile_data + 0x800 : gbv_tile_data;
	if (gbv_io_lcdc & GBV_LCDC_BG_ENABLE) {
		for (gbv_u8 ty = 0; ty < GBV_BG_TILES_Y; ty++) {
			for (gbv_u8 tx = 0; tx < GBV_BG_TILES_X; tx++) {
				gbv_u8 tile_id = bg_map[GBV_BG_TILES_X * ty + tx];
				tile_id = (gbv_io_lcdc & GBV_LCDC_BG_DATA_SELECT) ? tile_id + 0x80 : tile_id;
				gbv_u8 * tile_data = gbv_tile_data + GBV_TILE_SIZE * tile_id;
				for (gbv_u8 y = 0; y < GBV_TILE_HEIGHT; y++) {
					gbv_u8 * row = tile_data + GBV_TILE_PITCH * y;
					for (gbv_u8 x = 0; x < GBV_TILE_WIDTH; x++) {
						gbv_u8 dest_x = (GBV_TILE_WIDTH * tx + x) - gbv_io_scx;
						gbv_u8 dest_y = (GBV_TILE_HEIGHT * ty + y) - gbv_io_scy;
						/* clipping */
						if (dest_x < GBV_SCREEN_WIDTH && dest_y < GBV_SCREEN_HEIGHT) {
							gbv_u8 color = get_pixel_from_tile_row(row, x, gbv_io_bgp);
							gbv_u16 index = dest_y * GBV_SCREEN_WIDTH + dest_x;
							buffer[index] = palette->colors[color];
						}
					}
				}
			}
		}
	}
	else {
		/* background is off, clear screen */
		for (gbv_u16 i = 0; i < GBV_SCREEN_SIZE; i++) {
			buffer[i] = 0xFF;
		}
	}
	if (gbv_io_lcdc & GBV_LCDC_WND_ENABLE) {
		/* TODO (similar to BG) */
	}
	if (gbv_io_lcdc & GBV_LCDC_OBJ_ENABLE) {
		/* TODO */
	}
}