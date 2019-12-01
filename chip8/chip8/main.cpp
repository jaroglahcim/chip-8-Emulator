#include <stdio.h>
#include "chip8.h"
#include "SDL.h"

struct Display
{
	SDL_Window* window;
	SDL_Renderer *renderer;
	SDL_Event* event;
	int displayWidth;
	int displayHeight;
	int modifier;
	bool quit;
	Display(int modifier)
	{
		quit = false;
		window = NULL;
		renderer = NULL;
		event = new SDL_Event();
		this->modifier = modifier;
		displayWidth = SCREEN_WIDTH * modifier;
		displayHeight = SCREEN_HEIGHT * modifier;
	}
};

Chip8 myChip8;
Display* myDisplay;

void setupGraphics()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		fprintf(stderr, "SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
		return;
	}
	myDisplay->window = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		myDisplay->displayWidth, myDisplay->displayHeight, SDL_WINDOW_SHOWN);
	if (myDisplay->window == NULL)
	{
		fprintf(stderr, "Window could not be created. SDL_Error: %s\n", SDL_GetError());
		return;
	}
	myDisplay->renderer = SDL_CreateRenderer(myDisplay->window, -1, 0);

	SDL_SetRenderDrawColor(myDisplay->renderer, 0, 0, 0, 255);
	SDL_RenderClear(myDisplay->renderer);

	SDL_RenderPresent(myDisplay->renderer);
	//SDL_Delay(1000);

	//SDL_DestroyWindow(myDisplay->window);
	//SDL_Quit();
}

void handleInput()
{
	if (SDL_PollEvent(myDisplay->event)) {
		/* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
		switch (myDisplay->event->type) {
			case SDL_QUIT:
				myDisplay->quit = true;
				break;
			case SDL_KEYDOWN:
				switch (myDisplay->event->key.keysym.sym) {
					case SDLK_1: myChip8.key[0x1] = 1; break;
					case SDLK_2: myChip8.key[0x2] = 1; break;
					case SDLK_3: myChip8.key[0x3] = 1; break;
					case SDLK_4: myChip8.key[0xC] = 1; break;
					case SDLK_q: myChip8.key[0x4] = 1; break;
					case SDLK_w: myChip8.key[0x5] = 1; break;
					case SDLK_e: myChip8.key[0x6] = 1; break;
					case SDLK_r: myChip8.key[0xD] = 1; break;
					case SDLK_a: myChip8.key[0x7] = 1; break;
					case SDLK_s: myChip8.key[0x8] = 1; break;
					case SDLK_d: myChip8.key[0x9] = 1; break;
					case SDLK_f: myChip8.key[0xE] = 1; break;
					case SDLK_z: myChip8.key[0xA] = 1; break;
					case SDLK_x: myChip8.key[0x0] = 1; break;
					case SDLK_c: myChip8.key[0xB] = 1; break;
					case SDLK_v: myChip8.key[0xF] = 1; break;
				}
				//printf("Key press detected\n");
				break;

			case SDL_KEYUP:
				switch (myDisplay->event->key.keysym.sym) {
					case SDLK_1: myChip8.key[0x1] = 0; break;
					case SDLK_2: myChip8.key[0x2] = 0; break;
					case SDLK_3: myChip8.key[0x3] = 0; break;
					case SDLK_4: myChip8.key[0xC] = 0; break;
					case SDLK_q: myChip8.key[0x4] = 0; break;
					case SDLK_w: myChip8.key[0x5] = 0; break;
					case SDLK_e: myChip8.key[0x6] = 0; break;
					case SDLK_r: myChip8.key[0xD] = 0; break;
					case SDLK_a: myChip8.key[0x7] = 0; break;
					case SDLK_s: myChip8.key[0x8] = 0; break;
					case SDLK_d: myChip8.key[0x9] = 0; break;
					case SDLK_f: myChip8.key[0xE] = 0; break;
					case SDLK_z: myChip8.key[0xA] = 0; break;
					case SDLK_x: myChip8.key[0x0] = 0; break;
					case SDLK_c: myChip8.key[0xB] = 0; break;
					case SDLK_v: myChip8.key[0xF] = 0; break;
				}
				//printf("Key release detected\n");
				break;

			default:
				break;
		}
	}
}

void drawGraphics()
{
	SDL_SetRenderDrawColor(myDisplay->renderer, 0, 0, 0, 255);
	SDL_RenderClear(myDisplay->renderer);
	SDL_SetRenderDrawColor(myDisplay->renderer, 255, 255, 255, 255);
	int modifier = myDisplay->modifier;
	SDL_Rect rect;
	rect.h = rect.w = modifier;
	for (int x = 0; x < 64; ++x)
	{
		for (int y = 0; y < 32; ++y)
		{
			if (myChip8.gfx[(y * 64) + x] == 1) {
				//SDL_RenderDrawPoint(myDisplay->renderer, x * modifier + i, y * modifier + j);
				rect.x = x * modifier;
				rect.y = y * modifier;
				SDL_RenderFillRect(myDisplay->renderer, &rect);
			}
		}
	}
	SDL_RenderPresent(myDisplay->renderer);
}

void emulationFrame() {
	// Emulate one cycle
	for(int cycles = 0; cycles < 9; cycles++)
		myChip8.emulateCycle();
	// If the draw flag is set, update the screen
	if (myChip8.drawFlag)
	{
		//myChip8.debugRender();
		drawGraphics();
	}

	// Store key press state (Press and Release)
	handleInput();
	myChip8.drawFlag = false;
	myChip8.timersTick();
}

int main(int argc, char** argv) {
	myDisplay = new Display(10);
	// Set up render system and register input callbacks
	setupGraphics();
	//setupInput();

	// Initialize the Chip8 system and load the game into the memory  
	myChip8.initialize();                                              //myChip8.loadGame("pong2.c8");
	if (argc < 2)
	{
		fprintf(stderr, "No file specified.\n");
		return 1;
	}
	myChip8.loadGame(argv[1]);
	int frameStart, frameTime, frameDelay = 1000 / 60;
	// Emulation loop
	for (;!(myDisplay->quit);)
	{
		frameStart = SDL_GetTicks();
		emulationFrame();
		frameTime = SDL_GetTicks() - frameStart;
		if (frameDelay > frameTime)
			SDL_Delay(frameDelay - frameTime);
		//printf("%d\n", frameTime);
	}
	SDL_DestroyWindow(myDisplay->window);
	SDL_Quit();
	return 0;
}