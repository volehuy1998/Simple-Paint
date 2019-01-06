#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <utility>
#include <string> 
#include <iostream>
using namespace std;
using byte = unsigned char;

static SDL_Window   * window;
static SDL_Renderer * renderer;
static SDL_Texture  * texture;
static SDL_Texture  * colour_texture;
static SDL_Event event;
static SDL_Rect image_rect, color_rect;

static bool isopen = true, setcolor = false;
static int w = 800, h = 600, pitch, color_pitch;
static byte * image_pixels;
static byte * color_pixels;
static string image;

static auto replace_red   = 0xff; 
static auto replace_green = 0x0; 
static auto replace_blue  = 0x0; 

static void floodFill(int x, int y) {
	const auto r_background = image_pixels[y * pitch + x * 4 + 0]; 
	const auto g_background = image_pixels[y * pitch + x * 4 + 1]; 
	const auto b_background = image_pixels[y * pitch + x * 4 + 2]; 
	vector<pair<int, int>> childs;
	childs.push_back(make_pair(x, y));
	while (!childs.empty()) {
		auto p = childs.back();
		childs.pop_back();
		auto& target_red   = image_pixels[p.second * pitch + p.first * 4 + 0]; 
		auto& target_green = image_pixels[p.second * pitch + p.first * 4 + 1]; 
		auto& target_blue  = image_pixels[p.second * pitch + p.first * 4 + 2]; 
		if (target_red == replace_red && target_green == replace_green && target_blue == replace_blue) {
			continue;
		}
		target_red   = replace_red;
		target_green = replace_green;
		target_blue  = replace_blue;

		if (p.first - 1 >= 0) {
			auto x_child = p.first - 1;
			auto y_child = p.second;
			auto& red_child   = image_pixels[y_child * pitch + x_child * 4 + 0]; 
			auto& green_child = image_pixels[y_child * pitch + x_child * 4 + 1]; 
			auto& blue_child  = image_pixels[y_child * pitch + x_child * 4 + 2]; 
			if (red_child == r_background && green_child == g_background && blue_child == b_background) {
				childs.push_back(make_pair(x_child, y_child));
			}
		} else {
			continue;
		}
		if (p.second - 1 >= 0) {
			auto x_child = p.first;
			auto y_child = p.second - 1;
			auto& red_child   = image_pixels[y_child * pitch + x_child * 4 + 0]; 
			auto& green_child = image_pixels[y_child * pitch + x_child * 4 + 1]; 
			auto& blue_child  = image_pixels[y_child * pitch + x_child * 4 + 2]; 
			if (red_child == r_background && green_child == g_background && blue_child == b_background) {
				childs.push_back(make_pair(x_child, y_child));
			}
		} else {
			continue;
		}
		if (p.first + 1 < w) {
			auto x_child = p.first + 1;
			auto y_child = p.second;
			auto& red_child   = image_pixels[y_child * pitch + x_child * 4 + 0]; 
			auto& green_child = image_pixels[y_child * pitch + x_child * 4 + 1]; 
			auto& blue_child  = image_pixels[y_child * pitch + x_child * 4 + 2]; 
			if (red_child == r_background && green_child == g_background && blue_child == b_background) {
				childs.push_back(make_pair(x_child, y_child));
			}
		} else {
			continue;
		}
		if (p.second + 1 < h) {
			auto x_child = p.first;
			auto y_child = p.second + 1;
			auto& red_child   = image_pixels[y_child * pitch + x_child * 4 + 0]; 
			auto& green_child = image_pixels[y_child * pitch + x_child * 4 + 1]; 
			auto& blue_child  = image_pixels[y_child * pitch + x_child * 4 + 2]; 
			if (red_child == r_background && green_child == g_background && blue_child == b_background) {
				childs.push_back(make_pair(x_child, y_child));
			}
		}
	}
}

int main(int argc, char * argv[]) {
	cin >> image;
	SDL_Init(SDL_INIT_VIDEO);
	if (SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_BORDERLESS, &window, &renderer) < 0) {
		SDL_Log("SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_BORDERLESS, &window, &renderer): %s", SDL_GetError());
		return -1;
	}
	auto surface = IMG_Load((image + ".bmp").c_str());
	auto fsurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
	pitch = fsurface->pitch;
	auto ww = fsurface->w;
	auto hh = fsurface->h;
	image_rect.w = ww;
	image_rect.h = hh;
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, ww, hh);
	SDL_LockTexture(texture, nullptr, (void **)&image_pixels, &pitch);
	memcpy(image_pixels, fsurface->pixels, pitch * hh);
	SDL_UnlockTexture(texture);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(fsurface);

	surface = IMG_Load("palette.bmp");
	fsurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
	color_pitch = fsurface->pitch;
	ww = fsurface->w;
	hh = fsurface->h;
	colour_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, ww, hh);
	SDL_LockTexture(colour_texture, nullptr, (void **)&color_pixels, &color_pitch);
	memcpy(color_pixels, fsurface->pixels, color_pitch * hh);
	SDL_UnlockTexture(colour_texture);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(fsurface);

	color_rect.w = ww * 2;
	color_rect.h = h;
	image_rect.x += color_rect.w + 2;
	SDL_SetWindowSize(window, image_rect.w + color_rect.w, image_rect.h);

	SDL_Rect rect {.x = 1, .y = 1, .w = 1, .h = 1 }; 
	byte pixel[4] { 0 };
	int pitch_pixel = 1;
	
	while (isopen) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case  SDLK_q: isopen = false; break;
					} break;
				case SDL_MOUSEBUTTONDOWN:
					const auto& x = event.button.x;
					const auto& y = event.button.y;
					if (x < color_rect.w) {
						setcolor = true;
						rect.x = x;
						rect.y = y;
						SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_ABGR8888, pixel, pitch_pixel);
						replace_red   = pixel[0];
						replace_green = pixel[1];
						replace_blue  = pixel[2];
					}
					if (setcolor) {
						if (x >= image_rect.x) {
							SDL_LockTexture(texture, nullptr, (void **)&image_pixels, &pitch);
							floodFill(x - image_rect.x, y);
							SDL_UnlockTexture(texture);
						}
					}
					break;
			}
		}
		SDL_RenderCopy(renderer, texture, nullptr, &image_rect);
		SDL_RenderCopy(renderer, colour_texture, nullptr, &color_rect);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyTexture(colour_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	return 0;
}
