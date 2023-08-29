#include <stdio.h>
#include <SDL.h>

int main(int argc, char* args[])
{
	printf("hello world!");
	return SDL_Init(SDL_INIT_VIDEO);
}