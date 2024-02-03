#pragma once

#include <string>
#include <random>

//how is map data going to be stored in the future?

class RandomLevelGenerator
{
public:
	RandomLevelGenerator(int mapWidth, int mapHeight, int maxRoomWidth, int maxRoomHeight, int roomPlaceAttempts);

	void GenerateRandomLevel(std::wstring &map, int &spawnX, int &spawnY);

private:
	enum Direction
	{
		Up,
		Down,
		Left,
		Right,
		Origin,
	};

	const int mapWidth;
	const int mapHeight;
	const int maxRoomWidth;
	const int maxRoomHeight;
	int roomPlaceAttempts;

	std::random_device rd;
	std::mt19937 gen;

	//not the actual map but is used during construction
	std::pair<char, int> *map;

	int generateLayout();
	int placeRooms();
	int floodfillMaze();

	//TODO: is there a more clever way to store the direction offsets in the direction enum?
	void GetBoundingRectangle(int& xStart, int& xEnd, int& yStart, int& yEnd, Direction dir);
	bool CanPlaceTunnel(int x, int y, int xStart, int xEnd, int yStart, int yEnd);
	void GeneratePassageWays(int x, int y, int id);
};

