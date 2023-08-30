#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;
SDL_Renderer* renderer = NULL;

bool init();
void close();

int main(int argc, char* args[])
{
	printf("hello world!");

	if (!init())
	{
		printf("ERROR: failed to initialize\n");
		return -1;
	}

	bool quit = false;
	SDL_Event e;

	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		// main loop
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);

		for (int i = 0; i < SCREEN_HEIGHT; i++)
		{
			for (int j = 0; j < SCREEN_WIDTH / 2; j++)
			{
				SDL_RenderDrawPoint(renderer, j, i);
			}
		}

		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

		SDL_RenderPresent(renderer);
	}

	close();

	return 0;
}

bool init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("ERROR: could not initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	if ((window = SDL_CreateWindow("My awesome SDL program", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)) == NULL)
	{
		printf("ERROR: window could not be created: %s\n", SDL_GetError());
		return false;
	}

	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("ERROR: SDL_image could not initialize: %s\n", IMG_GetError());
		return false;
	}

	screen = SDL_GetWindowSurface(window);
	return true;
}

void close()
{
	SDL_FreeSurface(screen);
	screen = NULL;

	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	SDL_DestroyWindow(window);
	window = NULL;

	IMG_Quit;
	SDL_Quit();
}