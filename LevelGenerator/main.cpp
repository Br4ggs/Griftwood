#include <iostream>
#include <stdio.h>
#include <string>

#include "RandomLevelGenerator.h"

const uint32_t mapWidth = 40;
const uint32_t mapHeight = 40;

RandomLevelGenerator levelGenerator(mapWidth, mapHeight, 5, 5, 25);

void PrintMap(std::wstring map, uint32_t mapWidth, uint32_t mapHeight)
{
	for (uint32_t y = 0; y < mapHeight; y++)
	{
		std::wcout << map.substr((uint64_t)y * mapWidth, mapWidth) << std::endl;
	}
}

void main(int argc, char* args[])
{
	std::wstring map;

	uint32_t spawnX = 0;
	uint32_t spawnY = 0;

	levelGenerator.GenerateRandomLevel(map, spawnX, spawnY);

	map.at(spawnY * mapWidth + spawnX) = '@';

	PrintMap(map, mapWidth, mapHeight);
}