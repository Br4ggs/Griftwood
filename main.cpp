#include <stdio.h>
#include <string>
#include <cmath>

#include <SDL.h>
#include <SDL_image.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Game constants
const int mapHeight = 16;
const int mapWidth = 16;

const float FOV = 3.14159 / 4.0;
const float maxDepth = 12.0f;

//Player constants
const float walkSpeed = 5.0f;

//Player position
float playerX = 8.5f;
float playerY = 7.5f;
float playerA = 0.0f;

std::wstring map;

//SDL constants
const Uint8* keystates = SDL_GetKeyboardState(NULL);

//SDL stuff
SDL_Window* window = NULL;
SDL_Surface* screen = NULL;
SDL_Renderer* renderer = NULL;

bool init();
void close();

void render();

int main(int argc, char* args[])
{
	map += L"################";
	map += L"#..............#";
	map += L"#.##.#.#.#.#.#.#";
	map += L"#.#............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#......#.#......";
	map += L"#...............";
	map += L"#......#.#......";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#...........####";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	if (!init())
	{
		printf("ERROR: failed to initialize\n");
		return -1;
	}

	bool quit = false;
	SDL_Event e;

	Uint32 ticksCount = 0;
	while (!quit)
	{
		//TODO: fix performance?
		//calculate delta time
		float deltaTime = (SDL_GetTicks() - ticksCount) / 1000.0f;
		ticksCount = SDL_GetTicks();

		//TODO: input code for moving player
		//also TODO: only process one input event per loop iteration
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		//rotation
		if (keystates[SDL_SCANCODE_Q])
		{
			playerA -= 1.0f * deltaTime;
		}
		if (keystates[SDL_SCANCODE_E])
		{
			playerA += 1.0f * deltaTime;
		}

		//forward/backward
		if (keystates[SDL_SCANCODE_W])
		{
			playerX += sinf(playerA) * walkSpeed * deltaTime; //shouldn't these be the other way around?
			playerY += cosf(playerA) * walkSpeed * deltaTime;

			if (map[(int)playerY * mapWidth + (int)playerX] == '#')
			{
				playerX -= sinf(playerA) * walkSpeed * deltaTime;
				playerY -= cosf(playerA) * walkSpeed * deltaTime;
			}
		}
		if (keystates[SDL_SCANCODE_S])
		{
			playerX -= sinf(playerA) * walkSpeed * deltaTime;
			playerY -= cosf(playerA) * walkSpeed * deltaTime;

			if (map[(int)playerY * mapWidth + (int)playerX] == '#')
			{
				playerX += sinf(playerA) * walkSpeed * deltaTime;
				playerY += cosf(playerA) * walkSpeed * deltaTime;
			}
		}

		//strafing
		if (keystates[SDL_SCANCODE_D])
		{
			playerX += cosf(playerA) * walkSpeed * deltaTime; //shouldn't these be the other way around?
			playerY -= sinf(playerA) * walkSpeed * deltaTime;

			if (map[(int)playerY * mapWidth + (int)playerX] == '#')
			{
				playerX -= cosf(playerA) * walkSpeed * deltaTime;
				playerY += sinf(playerA) * walkSpeed * deltaTime;
			}
		}
		if (keystates[SDL_SCANCODE_A])
		{
			playerX -= cosf(playerA) * walkSpeed * deltaTime;
			playerY += sinf(playerA) * walkSpeed * deltaTime;

			if (map[(int)playerY * mapWidth + (int)playerX] == '#')
			{
				playerX += cosf(playerA) * walkSpeed * deltaTime;
				playerY -= sinf(playerA) * walkSpeed * deltaTime;
			}
		}


		// main loop
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

		SDL_RenderClear(renderer);

		render();

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

void render()
{
	//unit vector of player looking direction
	float dirX = std::sinf(playerA);
	float dirY = std::cosf(playerA);

	//another unit vector perpendicular to the direction vector
	float perpX = std::sinf(playerA + (3.14159f * 0.5f));
	float perpY = std::cosf(playerA + (3.14159f * 0.5f));

	//half the screen height? forgot what this was for
	//float posZ = 0.5f * SCREEN_HEIGHT;

	//render the walls
	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		//value ranging from -1...1 which will be used for the raycasting
		float cameraX = 2 * x / (float)SCREEN_WIDTH - 1;

		//the actual raycast vector
		float vectorX = dirX + perpX * cameraX;
		float vectorY = dirY + perpY * cameraX;

		//current map tile the player is in
		int mapX = (int)playerX;
		int mapY = (int)playerY;

		//DDA (Digital Differential Analysis) stuff
		//if this stuff gets complicated i'd recommend looking at https://lodev.org/cgtutor/raycasting.html

		//initial length of raycast from current position to next x or y side, respectively
		float sideDistX;
		float sideDistY;

		//distance the ray has to travel to go from 1 x-side to the next x-side, or from 1 y-side to the next y-side, respectively
		float deltaDistX = (vectorX == 0) ? 1e30 : std::abs(1 / vectorX);
		float deltaDistY = (vectorY == 0) ? 1e30 : std::abs(1 / vectorY);

		int stepX;
		int stepY;

		bool hitWall = false;
		int side;

		//configure step direction and sideDistX and Y
		if (vectorX < 0)
		{
			stepX = -1;
			sideDistX = (playerX - mapX) * deltaDistX;
		}
		else
		{
			stepX = 1;
			sideDistX = (mapX + 1 - playerX) * deltaDistX;
		}
		if (vectorY < 0)
		{
			stepY = -1;
			sideDistY = (playerY - mapY) * deltaDistY;
		}
		else
		{
			stepY = 1;
			sideDistY = (mapY + 1 - playerY) * deltaDistY;
		}

		//"march" forward from ray origin untill we hit a wall
		while (!hitWall)
		{
			//TODO: prevent infinite while loop?
			if (sideDistX < sideDistY)
			{
				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			}
			else
			{
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			}

			if (map[mapY * mapWidth + mapX] == '#')
			{
				hitWall = true;
			}
		}

		//distance between the hit point of the wall and the perpendicular camera plane
		float perpWallDist;
		if (side == 0)
		{
			perpWallDist = (sideDistX - deltaDistX);
		}
		else
		{
			perpWallDist = (sideDistY - deltaDistY);
		}

		//calculate the height of the line of the wall
		int lineheight = (int)(SCREEN_HEIGHT / perpWallDist);

		//calculate lowest and highest pixel to fill in current "stripe"
		int ceiling = -lineheight * 0.5f + SCREEN_HEIGHT * 0.5f;
		if (ceiling < 0) ceiling = 0;

		int floor = lineheight * 0.5f + SCREEN_HEIGHT * 0.5f;
		if (floor >= SCREEN_HEIGHT) floor = SCREEN_HEIGHT - 1;

		//float wallX; //sample coordinate for texture, normalized
		if (side == 0) //use y-coordinate
		{
			//wallX = playerY + perpWallDist * vectorY;
			//set draw color to green
			SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
		}
		else //use x-coordinate
		{
			//wallX = playerX + perpWallDist * vectorX;
			//set draw color to red
			SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
		}

		for (int y = ceiling; y <= floor; y++)
		{
			//if (perpWallDist < maxDepth)
			//{
				//float texY = texPos;
				//Draw(x, y, spriteWall->SampleGlyph(texX, texY), spriteWall->SampleColour(texX, texY));
				//texPos += step;
				//draw wall
				SDL_RenderDrawPoint(renderer, x, y);

			//}
			//else //primarily doing this to prevent any weird rendering due to rounding errors from far distances
			//{
			//	//Draw(x, y, PIXEL_SOLID, FG_BLACK);
			//	//draw black
			//}
		}
	}
}