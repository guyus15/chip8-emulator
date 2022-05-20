#ifndef CHIP8_HEADER
#define CHIP8_HEADER

#include <SDL2/SDL.h>

#define MEMORY_SIZE 4096
#define STACK_SIZE 16
#define V_SIZE 16

unsigned char font[80];
char pixel_coords[64][32];

typedef struct 
{
	// Program counter and stack pointer
	unsigned short PC;
	unsigned short SP;

	unsigned short stack[STACK_SIZE];
	unsigned char memory[MEMORY_SIZE];

	// General purpose registers
	unsigned char V[V_SIZE];
	// Delay timer and sound timer
	unsigned char DT;
	unsigned char ST;

	// Index register
	unsigned short I;
} CHP;

typedef struct
{
	SDL_Window *window;
	SDL_Renderer *renderer;
} SDLapp;

void initialise_chip8(CHP *chip8);

void load_rom(const char* rom_path, CHP *chip8);

void draw_square(unsigned int x, unsigned int y);

unsigned short fetch(CHP *chip8);

void decode(unsigned short opcode, CHP *chip8);

void update(CHP *chip8);

void draw_pixel(unsigned int x, unsigned int y);

#endif
