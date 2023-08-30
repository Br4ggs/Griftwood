#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

int main(int argc, char* args[])
{
	printf("hello world!");
	return SDL_Init(SDL_INIT_VIDEO);
}