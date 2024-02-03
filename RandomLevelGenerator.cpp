#include "RandomLevelGenerator.h"

#include <random>
#include <list>
#include <tuple>

//TODO: add none direction to enum and single boundary check function

void RandomLevelGenerator::GetBoundingRectangle(int &xStart, int &xEnd, int &yStart, int &yEnd, Direction dir)
{
	switch (dir)
	{
		case Direction::Up:
			xStart = -1;
			xEnd   = 1;
			yStart = -1;
			yEnd   = 0;
			break;
		case Direction::Down:
			xStart = -1;
			xEnd   = 1;
			yStart = 0;
			yEnd   = 1;
			break;
		case Direction::Left:
			xStart = -1;
			xEnd   = 0;
			yStart = -1;
			yEnd   = 1;
			break;
		case Direction::Right:
			xStart = 0;
			xEnd   = 1;
			yStart = -1;
			yEnd   = 1;
			break;
		case Direction::Origin:
			xStart = -1;
			xEnd = 1;
			yStart = -1;
			yEnd = 1;
			break;
		default:
			xStart = 0;
			xEnd = 0;
			yStart = 0;
			yEnd = 0;

	}
}

bool RandomLevelGenerator::CanPlaceTunnel(int x, int y, int xStart, int xEnd, int yStart, int yEnd)
{
	for (int yOffset = yStart; yOffset <= yEnd; yOffset++)
	{
		for (int xOffset = xStart; xOffset <= xEnd; xOffset++)
		{
			if (yOffset == 0 && xOffset == 0) continue;

			if (y + yOffset < 1 || y + yOffset > mapHeight - 2) continue;

			if (x + xOffset < 1 || x + xOffset > mapWidth - 2) continue;

			if (map[(yOffset + y) * mapWidth + (xOffset + x)].first == '.') return false;
		}
	}

	return true;
}

//precondition: CanPlaceTunnel(x,y) == true
void RandomLevelGenerator::GeneratePassageWays(int xOrigin, int yOrigin, int id)
{
	std::list<std::tuple<int, int, Direction>> possiblePassageWays;
	//turn the current tile (x,y) int a passage
	//check the up/down/left and right adjacent walls, and if viable add them to the stack
	//viable walls

	//check adjacent walls for viability, and if viable, add them as possible new passages
	if (yOrigin - 1 > 0)				possiblePassageWays.push_back(std::make_tuple(yOrigin - 1, xOrigin, Direction::Up));
	if (yOrigin + 1 < mapHeight - 1)	possiblePassageWays.push_back(std::make_tuple(yOrigin + 1, xOrigin, Direction::Down));
	if (xOrigin - 1 > 0)				possiblePassageWays.push_back(std::make_tuple(yOrigin, xOrigin - 1, Direction::Left));
	if (xOrigin + 1 > mapWidth - 1)		possiblePassageWays.push_back(std::make_tuple(yOrigin, xOrigin + 1, Direction::Right));

	//while viable walls exist
	while (!possiblePassageWays.empty())
	{
		//take a random viable wall
		std::uniform_int_distribution<> randomDistribution(0, possiblePassageWays.size() - 1);
		int randomIndex = randomDistribution(gen);

		auto randomElement = possiblePassageWays.begin();
		std::advance(randomElement, randomIndex);

		std::tuple<int, int, Direction> passageWay = *randomElement;

		int xCurrent = std::get<1>(passageWay);
		int yCurrent = std::get<0>(passageWay);

		//depending on the direction of the wall, we have to check a specific rectangular area
		//if it is valid then turn this tile into a passage, and add its 3 adjacent walls as
		//viable passage ways
		int xStart, xEnd, yStart, yEnd;
		GetBoundingRectangle(xStart, xEnd, yStart, yEnd, std::get<2>(passageWay));

		if (CanPlaceTunnel(xCurrent, yCurrent, xStart, xEnd, yStart, yEnd))
		{
			map[yCurrent * mapWidth + xCurrent].first = '.';
			map[yCurrent * mapWidth + xCurrent].second = id;

			//add corresponding adjacent walls
			if (yStart != 0 && yCurrent + yStart > 0)
				possiblePassageWays.push_back(std::make_tuple(yCurrent + yStart, xCurrent, Direction::Up));

			if (yEnd != 0 && yCurrent + yEnd < mapHeight - 1)
				possiblePassageWays.push_back(std::make_tuple(yCurrent + yEnd, xCurrent, Direction::Down));

			if (xStart != 0 && xCurrent + xStart > 0)
				possiblePassageWays.push_back(std::make_tuple(yCurrent, xCurrent + xStart, Direction::Left));

			if (xEnd != 0 && xCurrent + xEnd < mapWidth - 1)
				possiblePassageWays.push_back(std::make_tuple(yCurrent, xCurrent + xEnd, Direction::Right));
		}

		//remove the element from the list
		possiblePassageWays.erase(randomElement);
	}
}

RandomLevelGenerator::RandomLevelGenerator(int mapWidth, int mapHeight, int maxRoomWidth, int maxRoomHeight, int roomPlaceAttempts)
	:mapWidth(mapWidth), mapHeight(mapHeight), maxRoomWidth(maxRoomWidth), maxRoomHeight(maxRoomHeight), roomPlaceAttempts(roomPlaceAttempts), rd(), gen(rd())
{
	map = (std::pair<char,int> *)malloc(sizeof(std::pair<char, int>) * mapWidth * mapHeight);
}

void RandomLevelGenerator::GenerateRandomLevel(std::wstring& stringMap, int& spawnX, int& spawnY)
{
	//clear map buffer
	for (int y = 0; y < mapHeight; y++)
	{
		for (int x = 0; x < mapWidth; x++)
		{
			map[y * mapWidth + x] = std::make_pair('#', 0);
		}
	}

	//place rooms randomly
	std::uniform_int_distribution<> roomHeightDistr(1, maxRoomHeight);
	std::uniform_int_distribution<> roomWidthDistr(1, maxRoomWidth);

	//hopefully x and y are not swapped lol
	std::uniform_int_distribution<> roomX(1, mapWidth - 1);
	std::uniform_int_distribution<> roomY(1, mapHeight - 1);

	int id = 1;
	for (int i = 0; i < roomPlaceAttempts; i++)
	{
		int roomHeight = roomHeightDistr(gen);
		int roomWidth = roomWidthDistr(gen);

		uint16_t roomColumn = roomX(gen);
		uint16_t roomRow = roomY(gen);

		//todo: just make this roomHeight not roomHeight * 2
		//i was fucking high when i wrote this aight
		for (int y = -roomHeight; y <= roomHeight; y++)
		{
			int yOffset = y + roomRow;

			if (yOffset < 1 || yOffset >= mapHeight - 1) continue;

			for (int x = -roomWidth; x <= roomWidth; x++)
			{
				int xOffset = x + roomColumn;
				
				if (xOffset < 1 || xOffset >= mapWidth - 1) continue;

				map[yOffset * mapWidth + xOffset].first = '.';
				map[yOffset * mapWidth + xOffset].second = id;
			}
		}

		id++;
	}

	//floodfill
	for (int y = 1; y < mapHeight; y++)
	{
		for (int x = 1; x < mapWidth; x++)
		{
			int xStart, xEnd, yStart, yEnd;
			GetBoundingRectangle(xStart, xEnd, yStart, yEnd, Direction::Origin);

			if (map[y * mapWidth + x].first == '#' && CanPlaceTunnel(x, y, xStart, xEnd, yStart, yEnd))
			{
				//generate passage ways
				GeneratePassageWays(x, y, id);
			}
		}
	}

	//merge rooms

	//create connections

	//trim ends

	//remove unnecessary walls
	
	stringMap.clear();

	for (int y = 0; y < mapHeight; y++)
	{
		for (int x = 0; x < mapWidth; x++)
		{
			stringMap += map[y * mapWidth + x].first;
		}

		stringMap += L"\n";
	}
}