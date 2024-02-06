#include <stdio.h>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>

#include <SDL.h>
#include <SDL_image.h>

#include "RandomLevelGenerator.h"

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

RandomLevelGenerator levelGenerator(40, 40, 5, 5, 25);

std::wstring map;

//SDL constants
const Uint8* keystates = SDL_GetKeyboardState(NULL);

//SDL stuff
SDL_Window* window = NULL;
SDL_Surface* screen = NULL;
SDL_Renderer* renderer = NULL;

//used sprites
SDL_Surface* wallTexture = NULL;
SDL_Surface* floorTexture = NULL;
SDL_Surface* ceilingTexture = NULL;

SDL_Surface* barrelSprite = NULL;
SDL_Surface* pillarSprite = NULL;

struct Object
{
	float x;
	float y;
	SDL_Surface* sprite;
};

std::vector<Object> objects;

bool init();
void close();

void set_pixel(SDL_Surface* surface, int x, int y, Uint8 r, Uint8 g, Uint8 b);
void set_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel);
SDL_Surface* loadSurface(std::string path);
void sortSprites(std::vector<int>& order, std::vector<float>& distance, int count);

void render();

//drawing each pixel individually is super slow
int main(int argc, char* args[])
{
	if (!init())
	{
		printf("ERROR: failed to initialize\n");
		return -1;
	}

	//BUGBUG: something weird going on with this map, check spawn position
	uint16_t spawnX = 0;
	uint16_t spawnY = 0;

	levelGenerator.GenerateRandomLevel(map, (uint16_t &)spawnX, (uint16_t &)spawnY);

	playerX = spawnX + 0.5f;
	playerY = spawnY + 0.5f;

	//create map
	//map += L"################";
	//map += L"#..............#";
	//map += L"#.##.#.#.#.#.#.#";
	//map += L"#.#............#";
	//map += L"#..............#";
	//map += L"#..............#";
	//map += L"#......#.#......";
	//map += L"#...............";
	//map += L"#......#.#......";
	//map += L"#..............#";
	//map += L"#..............#";
	//map += L"#..............#";
	//map += L"#...........####";
	//map += L"#..............#";
	//map += L"#..............#";
	//map += L"################";

	//create objects inside map
	objects =
	{
		{2.5, 6.5, pillarSprite},
		{4.5, 6.5, pillarSprite},
		{2.5, 8.5, pillarSprite},
		{4.5, 8.5, pillarSprite},
		{14.0, 14.0, barrelSprite},
		{10.0, 14.0, barrelSprite},
		{9.0, 14.0, barrelSprite}
	};

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
		//TODO: later using GPU rendering
		//SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		//SDL_RenderClear(renderer);
		
		SDL_FillRect(screen, NULL, 0xFFFFFFFF);

		render(); //TODO: screen as a parameter

		//SDL_BlitSurface(blueWallSprite, NULL, screen, NULL);

		//SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);
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

	//TODO: later using GPU rendering
	//if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL)
	//{
	//	printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
	//	return false;
	//}

	//SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("ERROR: SDL_image could not initialize: %s\n", IMG_GetError());
		return false;
	}

	screen = SDL_GetWindowSurface(window);

	//load in all necessary textures
	wallTexture = loadSurface("./sprites/masonwall_64px.png");
	floorTexture = loadSurface("./sprites/seaweed_64px.png");
	ceilingTexture = loadSurface("./sprites/stonewall_64px.png");

	barrelSprite = loadSurface("./sprites/barrel.png");
	pillarSprite = loadSurface("./sprites/pillar.png");

	//SDL_PixelFormat* format = screen->format;
	return true;
}

void close()
{
	SDL_FreeSurface(screen);
	screen = NULL;

	//SDL_DestroyRenderer(renderer);
	//renderer = NULL;

	SDL_DestroyWindow(window);
	window = NULL;

	IMG_Quit;
	SDL_Quit();
}

//Blue component mask:
//00000000000000000000000011111111
//Green component mask:
//00000000000000001111111100000000
//Red component mask:
//00000000111111110000000000000000
//guessing it has no alpha component?
void set_pixel(SDL_Surface* surface, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	//Convert the pixels to 32 bit
	Uint32* pixels = (Uint32*)surface->pixels;

	Uint32 pixel = b | (g << 8) | (r << 16);

	//Set the pixel
	pixels[(y * surface->w) + x] = pixel;
}

void set_pixel(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
	//Convert the pixels to 32 bit
	Uint32* pixels = (Uint32*)surface->pixels;

	//Set the pixel
	pixels[(y * surface->w) + x] = pixel;
}

SDL_Surface* loadSurface(std::string path)
{
	SDL_Surface* optimizedSurface = NULL;
	SDL_Surface* loadedSurface;
	if ((loadedSurface = IMG_Load(path.c_str())) == NULL)
	{
		printf("ERROR: unable to load image: %s, SDL_image Error: %s\n", path.c_str(), IMG_GetError());
		return NULL;
	}
	
	if ((optimizedSurface = SDL_ConvertSurface(loadedSurface, screen->format, 0)) == NULL)
	{
		printf("ERROR: unable to optimize image: %s, SDL_image Error: %s\n", path.c_str(), SDL_GetError());
		return NULL;
	}

	return optimizedSurface;
}

void sortSprites(std::vector<int>& order, std::vector<float>& distance, int count)
{
	std::vector<std::pair<double, int>> sprites(count);
	for (int i = 0; i < count; i++)
	{
		sprites[i].first = distance[i];
		sprites[i].second = order[i];
	}

	std::sort(sprites.begin(), sprites.end());

	for (int i = 0; i < count; i++)
	{
		distance[i] = sprites[count - i - 1].first;
		order[i] = sprites[count - i - 1].second;
	}
}

void render()
{
	//unit vector of player looking direction
	float dirX = std::sinf(playerA);
	float dirY = std::cosf(playerA);

	//another unit vector perpendicular to the direction vector (camera plane)
	float perpX = std::sinf(playerA + (3.14159f * 0.5f));
	float perpY = std::cosf(playerA + (3.14159f * 0.5f));

	//vertical position of the camera
	float posZ = 0.5f * SCREEN_HEIGHT;

	double zBuffer[SCREEN_WIDTH] = { 0 };

	//render the ceiling and floor
	//TODO: research and improve
	for (int y = SCREEN_HEIGHT - 1; y >= SCREEN_HEIGHT * 0.5f; y--)
	{	
		//2 vectors for leftmost and rightmost ray
		float vector1X = dirX - perpX;
		float vector1Y = dirY - perpY;
		float vector2X = dirX + perpX;
		float vector2Y = dirY + perpY;

		//the current y-position, relative to the center of the screen (the horizon)
		int p = y - SCREEN_HEIGHT * 0.5f;

		//horizontal distance from the camera to the floor for the current row?
		float rowDistance = posZ / p;

		float floorStepX = rowDistance * (vector2X - vector1X) / (float)SCREEN_WIDTH;
		float floorStepY = rowDistance * (vector2Y - vector1Y) / (float)SCREEN_WIDTH;

		//coordinates in world-space of leftmost column, which we will update
		float floorX = playerX + rowDistance * vector1X;
		float floorY = playerY + rowDistance * vector1Y;

		for (int x = 0; x < SCREEN_WIDTH; x++)
		{
			//texture coordinates, mask with (texHeight - 1) in case of overflow?
			int texX = (int)(floorTexture->w * (floorX - std::floor(floorX))) & (floorTexture->h - 1);
			int texY = (int)(floorTexture->h * (floorY - std::floor(floorY))) & (floorTexture->h - 1);
			Uint32* spritePixels = (Uint32*)floorTexture->pixels;
			Uint32 pixel = spritePixels[(texY * floorTexture->w) + texX];
			set_pixel(screen, x, y, pixel);

			texX = (int)(ceilingTexture->w * (floorX - std::floor(floorX))) & (ceilingTexture->h - 1);
			texY = (int)(ceilingTexture->h * (floorY - std::floor(floorY))) & (ceilingTexture->h - 1);
			spritePixels = (Uint32*)ceilingTexture->pixels;
			pixel = spritePixels[(texY * ceilingTexture->w) + texX];
			pixel = (pixel >> 1) & 8355711; // make a bit darker;
			set_pixel(screen, x, SCREEN_HEIGHT - y - 1, pixel);

			floorX += floorStepX;
			floorY += floorStepY;
		}
	}

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
		Uint8 r = 0;
		Uint8 g = 0;
		Uint8 b = 0;

		double wallX;
		if (side == 0) //use y-coordinate
		{
			wallX = playerY + perpWallDist * vectorY;
			//set draw color to green
			//SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
			r = 0x00;
			g = 0xFF;
			b = 0x00;
		}
		else //use x-coordinate
		{
			wallX = playerX + perpWallDist * vectorX;
			//set draw color to red
			//SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
			r = 0xFF;
			g = 0x00;
			b = 0x00;
		}

		//wallX gotta be normalized
		wallX -= std::floor(wallX);

		int texX = int(wallX * double(wallTexture->w));

		double step = 1.0 * wallTexture->h / lineheight;

		double texPos = (ceiling - SCREEN_HEIGHT / 2 + lineheight / 2) * step;
		for (int y = ceiling; y <= floor; y++)
		{
			//mask with (texHeight - 1) in case of overflow?
			int texY = (int)texPos & (wallTexture->h - 1);
			texPos += step;

			Uint32* spritePixels = (Uint32*)wallTexture->pixels;
			
			Uint32 pixel = spritePixels[(texY * wallTexture->w) + texX];
			if (side == 1)
			{
				//make pixel darker for Y sides of the maze by a color shift
				pixel = (pixel >> 1) & 8355711;
			}

			set_pixel(screen, x, y, pixel);

			//if (perpWallDist < maxDepth)
			//{
				//float texY = texPos;
				//Draw(x, y, spriteWall->SampleGlyph(texX, texY), spriteWall->SampleColour(texX, texY));
				//texPos += step;
				//draw wall
				//SDL_RenderDrawPoint(renderer, x, y);

			//}
			//else //primarily doing this to prevent any weird rendering due to rounding errors from far distances
			//{
			//	//Draw(x, y, PIXEL_SOLID, FG_BLACK);
			//	//draw black
			//}
		}

		//update zbuffer using perpendicular wall distance
		zBuffer[x] = perpWallDist;
	}

	//render objects
	//sort objects by distance
	std::vector<int> objectOrder;
	std::vector<float> objectDistance;

	for (int i = 0; i < objects.size(); i++)
	{
		objectOrder.push_back(i);
		//application of pythagoras theorem, we do not need the square root to sort using distances
		objectDistance.push_back((playerX - objects[i].x) * (playerX - objects[i].x) + (playerY - objects[i].y) * (playerY - objects[i].y));
	}

	sortSprites(objectOrder, objectDistance, objects.size());

	for (int i = 0; i < objects.size(); i++)
	{
		Object* object = &objects[objectOrder[i]];

		//object position relative to player
		float objectX = object->x - playerX;
		float objectY = object->y - playerY;

		//calculate determinant using camera matrix
		// [ perpX dirX ] -1                                       [ dirY   -dirX ]
		// [            ]       =  1/(perpX*dirY-dirX*perpY) *     [              ]
		// [ perpY dirY ]                                          [ -perpY perpX ]

		float invDeterminant = 1 / (perpX * dirY - dirX * perpY);

		//see above matrix for second term?
		float transformX = invDeterminant * (dirY * objectX - dirX * objectY);
		float transformY = invDeterminant * (-perpY * objectX + perpX * objectY);

		//?
		int spriteScreenX = int((SCREEN_WIDTH * 0.5f) * (1 + transformX / transformY));

		//calculate height of the sprite on screen
		//using transformY instead of real distance prevents fisheye?
		int spriteHeight = abs(int(SCREEN_HEIGHT / transformY));
		int drawStartY = (-spriteHeight + SCREEN_HEIGHT) * 0.5f;
		if (drawStartY < 0) drawStartY = 0;
		int drawEndY = (spriteHeight + SCREEN_HEIGHT) * 0.5f;
		if (drawEndY >= SCREEN_HEIGHT) drawEndY = SCREEN_HEIGHT - 1;

		//object aspect ratio
		float objectAspectRatio = (float)object->sprite->w / (float)object->sprite->h;

		//calculate width of sprite on screen
		int spriteWidth = spriteHeight * objectAspectRatio;
		int drawStartX = -spriteWidth * 0.5f + spriteScreenX;
		if (drawStartX < 0) drawStartX = 0;
		int drawEndX = spriteWidth * 0.5f + spriteScreenX;
		if (drawEndX >= SCREEN_WIDTH) drawEndX = SCREEN_WIDTH - 1;

		for (int x = drawStartX; x < drawEndX; x++)
		{
			int texX = int(256 * (x - (-spriteWidth * 0.5f + spriteScreenX)) * object->sprite->w / spriteWidth) / 256;

			if (transformY > 0 && x > 0 && x < SCREEN_WIDTH && transformY < zBuffer[x])
			{
				for (int y = drawStartY; y < drawEndY; y++)
				{
					int d = y * 256 - SCREEN_HEIGHT * 128 + spriteHeight * 128;
					int texY = ((d * object->sprite->h) / spriteHeight) / 256;

					//look up texture using texX and texY
					Uint32* spritePixels = (Uint32*)object->sprite->pixels;
					Uint32 pixel = spritePixels[(texY * object->sprite->w) + texX];

					//if pixel is not black (transparent color) then draw it
					if ((pixel & 0x00FFFFFF) != 0)
					{
						set_pixel(screen, x, y, pixel);
					}
				}
			}
		}
	}
}