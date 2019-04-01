#include "gbv.h"
#include <SDL2/SDL.h>

#define WINDOW_SCALE  3
#define WINDOW_WIDTH  (WINDOW_SCALE * GBV_SCREEN_WIDTH)
#define WINDOW_HEIGHT (WINDOW_SCALE * GBV_SCREEN_HEIGHT)

static void sdl_graceful_exit(const char *fmt) {
	fprintf(stderr, fmt, SDL_GetError());
	SDL_Quit();
	exit(1);
}

int main(int argc, char *argv[]) {
	/* platform setup */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		sdl_graceful_exit("Error initializing SDL video: %s\n");
	}
	SDL_Window * window = SDL_CreateWindow("GBV Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		sdl_graceful_exit("Error creating window: %s\n");
	}
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		sdl_graceful_exit("Error creating renderer: %s\n");
	}
	SDL_Texture * framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, GBV_SCREEN_WIDTH, GBV_SCREEN_HEIGHT);
	if (!framebuffer) {
		sdl_graceful_exit("Error creating framebuffer: %s\n");
	}

	SDL_version sdl_ver;	
	SDL_GetVersion(&sdl_ver);
	fprintf(stdout, "Initialized SDL version %d.%d.%d\n", sdl_ver.major, sdl_ver.minor, sdl_ver.patch);
	fprintf(stdout, "  platform:         %s\n", SDL_GetPlatform());
	fprintf(stdout, "  video driver:     %s\n", SDL_GetCurrentVideoDriver());
	fprintf(stdout, "  framebuffer size: %dx%d\n", GBV_SCREEN_WIDTH, GBV_SCREEN_HEIGHT);
	fprintf(stdout, "  window size:      %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
	fprintf(stdout, "  scale:            %dx\n", WINDOW_SCALE);

	/* gbv version */
	int maj, min, patch;
	gbv_get_version(&maj, &min, &patch);
	fprintf(stdout, "\nusing GBV version %d.%d.%d\n", maj, min, patch);

	/* gb setup */
	unsigned char gbmem[GBV_HW_MEMORY_SIZE] = {};
	unsigned char video_mem[GBV_SCREEN_SIZE] = {};
	gbv_init(&gbmem);

	/* enable lcd */
	gbv_lcdc_set(GBV_LCDC_CTRL);

	/* enable bg */
	gbv_lcdc_set(GBV_LCDC_BG_ENABLE);

	/* set up 4 color 2bpp palette */
	gbv_io_bgp = 0xE4;

	/* set up test bg map 1 */
	gbv_u8 * map_data0 = gbv_get_tile_map0();
	for (int i = 0; i < GBV_BG_TILES_Y; i++) {
		for (int j = 0; j < GBV_BG_TILES_X; j++) {
			map_data0[GBV_BG_TILES_X * i + j] = 0;
		}
	}

	/* set up test bg map 2 */
	gbv_u8 * map_data1 = gbv_get_tile_map1();
	for (int i = 0; i < GBV_BG_TILES_Y; i++) {
		for (int j = 0; j < GBV_BG_TILES_X; j++) {
			map_data1[GBV_BG_TILES_X * i + j] = 1;
		}
	}

	/* tile 0 */
	gbv_tile * tile0 = gbv_get_tile(0);
	tile0->data[0][0] = 0xC3; tile0->data[0][1] = 0x43;
	tile0->data[1][0] = 0x81; tile0->data[1][1] = 0x81;
	tile0->data[6][0] = 0x18; tile0->data[6][1] = 0x18;
	tile0->data[7][0] = 0x3C; tile0->data[7][1] = 0x3C;
	
	/* tile 1 */
	gbv_tile * tile1 = gbv_get_tile(1);
	tile1->data[0][0] = 0xFF; tile1->data[0][1] = 0xFF;
	tile1->data[1][0] = 0x81; tile1->data[1][1] = 0x81;
	tile1->data[2][0] = 0x81; tile1->data[2][1] = 0x81;
	tile1->data[3][0] = 0x81; tile1->data[3][1] = 0x81;
	tile1->data[4][0] = 0x81; tile1->data[4][1] = 0x81;
	tile1->data[5][0] = 0x81; tile1->data[5][1] = 0x81;
	tile1->data[6][0] = 0x81; tile1->data[6][1] = 0x81;
	tile1->data[7][0] = 0xFF; tile1->data[7][1] = 0xFF;
	
	/* main loop */
	int running = 1;
	while (running) {
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				switch (evt.key.keysym.sym) {
				case SDLK_1:
					gbv_lcdc_reset(GBV_LCDC_BG_MAP_SELECT);
					break;
				case SDLK_2:
					gbv_lcdc_set(GBV_LCDC_BG_MAP_SELECT);
					break;
				case SDLK_UP:
					gbv_io_scy -= 1;
					break;
				case SDLK_DOWN:
					gbv_io_scy += 1;
					break;
				case SDLK_RIGHT:
					gbv_io_scx += 1;
					break;
				case SDLK_LEFT:
					gbv_io_scx -= 1;
					break;
				case SDLK_RETURN:
				case SDLK_RETURN2:
					if (gbv_io_lcdc & GBV_LCDC_BG_ENABLE)
						gbv_lcdc_reset(GBV_LCDC_BG_ENABLE);
					else
						gbv_lcdc_set(GBV_LCDC_BG_ENABLE);
					break;
				} break;
			}
		}

		/* render gbv state to 8bit per pixel buffer */
		gbv_palette palette = { 0xFF, 0xAA, 0x55, 0x00 };
		gbv_render(video_mem, GBV_RENDER_MODE_GRAYSCALE_8, &palette);

		/* transfer data to framebuffer */
		void *video_mem_rgba;
		int video_mem_pitch;
		SDL_LockTexture(framebuffer, 0, &video_mem_rgba, &video_mem_pitch);
		for (int y = 0; y < GBV_SCREEN_HEIGHT; y++) {
			for (int x = 0; x < GBV_SCREEN_WIDTH; x++) {
				unsigned char *src_px = video_mem + GBV_SCREEN_WIDTH * y + x;
				unsigned char *dst_px = (unsigned char*)video_mem_rgba + y * video_mem_pitch + 4 * x;
				dst_px[0] = 0xFF;
				dst_px[1] = src_px[0];
				dst_px[2] = src_px[0];
				dst_px[3] = src_px[0];
			}
		}
		SDL_UnlockTexture(framebuffer);

		/* transfer framebuffer to screen */
		SDL_Rect dest = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
		SDL_RenderCopy(renderer, framebuffer, 0, &dest);
		SDL_RenderPresent(renderer);

		/* delay 20 ms */
		SDL_Delay(20);
	}
	SDL_Quit();

	return 0;
}