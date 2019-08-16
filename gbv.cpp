#include "gbv.h"

#define OBJ_NULL 0xff
#define MAX_OBJECTS_PER_SCANLINE 10

#define GBV_MIN(a, b) ((a) < (b)) ? (a) : (b)
#define GBV_MAX(a, b) ((a) > (b)) ? (a) : (b)
#define GBV_SPRITE_MARGIN_LEFT 8
#define GBV_SPRITE_MARGIN_TOP 16

/* module globals */
GBV_API gbv_io gbv_io_lcdc = 0;
GBV_API gbv_io gbv_io_bgp  = 0;
GBV_API gbv_io gbv_io_obp0 = 0;
GBV_API gbv_io gbv_io_obp1 = 0;
GBV_API gbv_io gbv_io_scx  = 0;
GBV_API gbv_io gbv_io_scy  = 0;
GBV_API gbv_io gbv_io_wx   = 0;
GBV_API gbv_io gbv_io_wy   = 0;

/* internal data pointers */
static gbv_u8 * gbv_mem;
static gbv_u8 * gbv_tile_data;
static gbv_u8 * gbv_tile_map0;
static gbv_u8 * gbv_tile_map1;
static gbv_obj_char * gbv_oam_data;

/* internal functions */
static gbv_u8 get_color(gbv_u8 idx, gbv_u8 pal) {
	gbv_u8 color = (pal >> (2 * idx)) & 0x03;
	return color;
}

static gbv_u8 get_pal_idx_from_tile_row(gbv_u8 row[2], gbv_u8 x) {
	gbv_u8 pal_idx = ((row[0] >> (7 - x)) & 0x01) | ((row[1] >> (7 - x) & 0x01) << 1);
	return pal_idx;
}

static gbv_u8 get_pixel_from_tile_row(gbv_u8 row[2], gbv_u8 x, gbv_u8 pal) {
	gbv_u8 pal_idx = get_pal_idx_from_tile_row(row, x);
	gbv_u8 color = get_color(pal_idx, pal);
	return color;
}

static void fill_memory(void * buffer, gbv_u16 size, gbv_u8 value) {
	gbv_u16 size8 = size / 8;
	gbv_u16 rem = size % 8;
	unsigned long long * buffer8 = (unsigned long long*)buffer;
	if (size8) {
		unsigned long long value8 = (unsigned long long)value;
		value8 |= (value8 << 56) | (value8 << 48) | (value8 << 40) | (value8 << 32) | (value8 << 24) | (value8 << 16) | (value8 << 8);
		for (gbv_u16 i = 0; i < size8; i++) {
			buffer8[i] = value8;
		}
	}
	if (rem) {
		gbv_u8 * buffer_rem = (gbv_u8*)(buffer8 + size8);
		for (gbv_u8 i = 0; i < rem; i++) {
			buffer_rem[i] = value;
		}
	}
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
	gbv_mem       = (gbv_u8*)memory;
	gbv_tile_data = gbv_mem + 0x8000;
	gbv_tile_map0 = gbv_mem + 0x9800;
	gbv_tile_map1 = gbv_mem + 0x9C00;
	gbv_oam_data  = (gbv_obj_char*)(gbv_mem + 0xFE00);
}

void gbv_lcdc_set(gbv_lcdc_flag flag) {
	gbv_io_lcdc = gbv_io_lcdc | flag;
}

void gbv_lcdc_reset(gbv_lcdc_flag flag) {
	gbv_io_lcdc = gbv_io_lcdc & ~flag;
}

gbv_u8 * gbv_get_rom_data() {
	return gbv_mem;
}

gbv_u8 * gbv_get_tile_map0() {
	return gbv_tile_map0;
}

gbv_u8 * gbv_get_tile_map1() {
	return gbv_tile_map1;
}

gbv_u8 * gbv_get_tile_data() {
	return gbv_tile_data;
}

gbv_tile * gbv_get_tile(gbv_u8 tile_id) {
	return (gbv_tile*)gbv_tile_data + tile_id;
}

void gbv_transfer_oam_data(gbv_obj_char objs[GBV_OBJ_COUNT]) {
	for (int i = 0; i < GBV_OBJ_COUNT; i++) {
		gbv_oam_data[i] = objs[i];
	}
}

gbv_u8 * get_tile_from_tilemap(gbv_u8 x, gbv_u8 y, gbv_lcdc_flag map_select) {
	gbv_u8 *tile_map = (gbv_io_lcdc & map_select) ? gbv_tile_map1 : gbv_tile_map0;
	gbv_u8 tile_id = tile_map[GBV_BG_TILES_X * y + x];

	tile_id = (gbv_io_lcdc & GBV_LCDC_BG_DATA_SELECT) ? (~tile_id + 1) : tile_id;

	gbv_u8 * tile_data = (gbv_io_lcdc & GBV_LCDC_BG_DATA_SELECT) ? gbv_tile_data + 0x800 : gbv_tile_data;
	gbv_u8 * tile = tile_data + GBV_TILE_SIZE * tile_id;

	return tile;
}

void gbv_render(void * render_buffer, gbv_render_mode mode, gbv_palette * palette) {
	gbv_u8 * buffer = (gbv_u8*)render_buffer;
	if (gbv_io_lcdc & GBV_LCDC_CTRL) {
		if (gbv_io_lcdc & GBV_LCDC_BG_ENABLE) {
			for (gbv_u8 ty = 0; ty < GBV_BG_TILES_Y; ty++) {
				for (gbv_u8 tx = 0; tx < GBV_BG_TILES_X; tx++) {
					gbv_u8 * tile = get_tile_from_tilemap(tx, ty, GBV_LCDC_BG_MAP_SELECT);
					for (gbv_u8 y = 0; y < GBV_TILE_HEIGHT; y++) {
						gbv_u8 * row = tile + GBV_TILE_PITCH * y;
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
			/* window enabled */
			for (gbv_u8 ty = 0; ty < GBV_BG_TILES_Y; ty++) {
				for (gbv_u8 tx = 0; tx < GBV_BG_TILES_X; tx++) {
					gbv_u8 * tile = get_tile_from_tilemap(tx, ty, GBV_LCDC_WND_MAP_SELECT);
					for (gbv_u8 y = 0; y < GBV_TILE_HEIGHT; y++) {
						gbv_u8 * row = tile + GBV_TILE_PITCH * y;
						for (gbv_u8 x = 0; x < GBV_TILE_WIDTH; x++) {
							gbv_u16 screen_x = (GBV_TILE_WIDTH * tx + x) + gbv_io_wx - GBV_TILE_WIDTH;
							gbv_u16 screen_y = (GBV_TILE_HEIGHT * ty + y) + gbv_io_wy;
							/* clipping */
							if (screen_x < GBV_SCREEN_WIDTH && screen_y < GBV_SCREEN_HEIGHT) {
								gbv_u8 color = get_pixel_from_tile_row(row, x, gbv_io_bgp);
								gbv_u16 index = screen_y * GBV_SCREEN_WIDTH + screen_x;
								buffer[index] = palette->colors[color];
							}
						}
					}
				}
			}
		}
		if (gbv_io_lcdc & GBV_LCDC_OBJ_ENABLE) {
			/* TODO: 8x16 mode */
			/* render per scanline */
			for (gbv_u8 y = 0; y < GBV_SCREEN_HEIGHT; y++) {
				/* search oam */
				gbv_u8 obj_count = 0;
				gbv_u8 objs[MAX_OBJECTS_PER_SCANLINE];
				for (gbv_u8 idx = 0; idx < GBV_OBJ_COUNT; idx++) {
					gbv_obj_char *obj = gbv_oam_data + idx;
					if (obj->y <= y + GBV_SPRITE_MARGIN_TOP  && obj->y + 8 > y + GBV_SPRITE_MARGIN_TOP) {
						if (obj_count < MAX_OBJECTS_PER_SCANLINE) {
							objs[obj_count++] = idx;
						}
					}
				}
				if (obj_count) {
					/* objs array contains all sprites that get hit by this scanline */
					/* array is sorted by ascending indices */
					for (gbv_u8 idx = obj_count; idx > 0; idx--) {
						gbv_obj_char *obj = gbv_oam_data + objs[idx - 1];
						gbv_tile *tile = gbv_get_tile(obj->id);
						if (obj->x < GBV_SPRITE_MARGIN_LEFT + GBV_SCREEN_WIDTH) {
							gbv_u8 min_x, max_x;
							min_x = (obj->x < GBV_SPRITE_MARGIN_LEFT) ? GBV_SPRITE_MARGIN_LEFT - obj->x : 0;
							/* TODO: yuk */
							if (min_x >= GBV_SPRITE_MARGIN_LEFT + GBV_SCREEN_WIDTH) {
								max_x = min_x;
							}
							else {
								max_x = (obj->x + GBV_TILE_WIDTH > GBV_SPRITE_MARGIN_LEFT + GBV_SCREEN_WIDTH) ? GBV_SPRITE_MARGIN_LEFT + GBV_SCREEN_WIDTH - obj->x : GBV_TILE_WIDTH;
							}
							/* TODO: handle flipH/flipV */
							gbv_u8 src_y = GBV_SPRITE_MARGIN_TOP + y - obj->y;
							for (gbv_u8 src_x = min_x; src_x < max_x; src_x++) {
								gbv_u8 pal_idx = get_pal_idx_from_tile_row(tile->data[src_y], src_x);
								if(pal_idx != 0x00) {
									gbv_u8 color = get_color(pal_idx, (obj->attr & GBV_OBJ_ATTR_PALETTE_SELECT) ? gbv_io_obp1 : gbv_io_obp0);
									gbv_u8 dst_x, dst_y;
									dst_x = obj->x + src_x - GBV_SPRITE_MARGIN_LEFT;
									dst_y = y;
									gbv_u16 offset = dst_y * GBV_SCREEN_WIDTH + dst_x;
									if (!(obj->attr & GBV_OBJ_ATTR_PRIORITY_FLAG) || (obj->attr & GBV_OBJ_ATTR_PRIORITY_FLAG && buffer[offset] == 0x00)) {
										buffer[offset] = palette->colors[color];
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else {
		/* LCD is off, clear screen */
		for (gbv_u16 i = 0; i < GBV_SCREEN_SIZE; i++) {
			buffer[i] = 0xFF;
		}
	}
}