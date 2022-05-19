#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "chip8.h"

#define SCREEN_WIDTH 64 * 8
#define SCREEN_HEIGHT 32 * 8
#define REFRESH_RATE 200000

char font[] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

CHP chip8;
SDLapp app;

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		printf("Usage: %s <rom-path>\n", argv[0]);
		return -1;
	}

	initialise_chip8(&chip8);
	load_rom(argv[1], &chip8);

	// Seed random values
	srand(time(NULL));

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("There has been an error initialising SDL.\n%s\n", SDL_GetError());
		return -1;
	}
	
	app.window = SDL_CreateWindow(
			"CHIP-8",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH,
			SCREEN_HEIGHT,
			0
	);

	if (!app.window)
	{
		printf("There has been an error creating the window.\n%s\n", SDL_GetError());
		return -1;
	}

	app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_SOFTWARE);

	if (!app.renderer)
	{
		printf("There has been an error creating the renderer.\n%s\n", SDL_GetError());
		return -1;
	}

	// Set initial render draw color
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);

	SDL_Event e;

	int quit = 0;
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = 1;
			}
		}
            
		update(&chip8);

		SDL_RenderPresent(app.renderer);

		// Enforce FPS
		SDL_Delay(1000/REFRESH_RATE);
	}

	SDL_DestroyWindow(app.window);
	SDL_Quit();

	return 0;
}

