#include "RandomLevelGenerator.h"

#include <random>
#include <algorithm>
#include <iterator>
#include <list>
#include <stack>
#include <tuple>

//TODO: add constants for id's for walls, passageways(?) and rooms(?)

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
//TODO: this is sluggish as shit. see if you can maybe optimize this
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

void RandomLevelGenerator::CreateConnections()
{
	//create connections
	//shits ridiculous
	//y-coordinate, x-coordinate, first region id, second region id
	std::list<std::tuple<int, int, int, int>> connections;

	for (int y = 1; y < mapHeight - 1; y++)
	{
		for (int x = 1; x < mapWidth - 1; x++)
		{
			if (map[y * mapWidth + x].first == '.') continue;

			//wall vertically connects 2 spaces
			if (map[(y - 1) * mapWidth + x].first == '.' &&
				map[(y + 1) * mapWidth + x].first == '.' &&
				map[(y - 1) * mapWidth + x].second != map[(y + 1) * mapWidth + x].second)
			{
				//store in list with coordinates, and 2 region id's?
				int firstId = std::min(map[(y - 1) * mapWidth + x].second, map[(y + 1) * mapWidth + x].second);
				int secondId = std::max(map[(y - 1) * mapWidth + x].second, map[(y + 1) * mapWidth + x].second);
				connections.push_back(std::make_tuple(y, x, firstId, secondId));
			}

			//wall horizontally connects 2 spaces
			else if (map[y * mapWidth + (x - 1)].first == '.' &&
				map[y * mapWidth + (x + 1)].first == '.' &&
				map[y * mapWidth + (x - 1)].second != map[y * mapWidth + (x + 1)].second)
			{
				//store in list with coordinates, and 2 region id's?
				int firstId = std::min(map[y * mapWidth + (x - 1)].second, map[y * mapWidth + (x + 1)].second);
				int secondId = std::max(map[y * mapWidth + (x - 1)].second, map[y * mapWidth + (x + 1)].second);
				connections.push_back(std::make_tuple(y, x, firstId, secondId));
			}
		}
	}

	std::uniform_int_distribution<> connectionDistribution(1, 4); //parameterize this

	while (!connections.empty())
	{
		//shuffle the currently remaining connections
		//select a random element from the connections
		std::uniform_int_distribution<> randomDistribution(0, connections.size() - 1);
		int randomIndex = randomDistribution(gen);

		auto randomElement = connections.begin();
		std::advance(randomElement, randomIndex);
		std::tuple<int, int, int, int> randomConnection = *randomElement;

		//filter connections to only include connections that connect 2 specific regions
		std::list<std::tuple<int, int, int, int>> filteredConnections(connections);
		filteredConnections.remove_if([=](std::tuple<int, int, int, int> connection)
			{
				return std::get<2>(connection) != std::get<2>(randomConnection) ||
					   std::get<3>(connection) != std::get<3>(randomConnection);
			});

		//create a shuffled refference wrapper of the filtered list
		std::vector<std::reference_wrapper<const std::tuple<int, int, int, int>>> wrapper(filteredConnections.begin(), filteredConnections.end());
		std::shuffle(wrapper.begin(), wrapper.end(), gen);

		int nrOfConnections = connectionDistribution(gen);
		int connectionsPlaced = 0;

		auto iterator = wrapper.begin();

		while (iterator < wrapper.begin() + wrapper.size() && connectionsPlaced < nrOfConnections)
		{
			//get current tile
			std::tuple<int, int, int, int> currentConnection = *iterator;
			std::advance(iterator, 1);

			//count the nr of surrounding floor tiles
			int surroundingFloors = 0;
			for (int y = -1; y <= 1; y++)
			{
				for (int x = -1; x <= 1; x++)
				{
					//ignore diagonals
					if (x == y || x == -y) continue;

					if (map[(std::get<0>(currentConnection) + y) * mapWidth + (std::get<1>(currentConnection) + x)].first == '.')
						surroundingFloors++;
				}
			}

			if (surroundingFloors > 2) continue;

			//update tile
			map[std::get<0>(currentConnection) * mapWidth + std::get<1>(currentConnection)].first = '.';
			map[std::get<0>(currentConnection) * mapWidth + std::get<1>(currentConnection)].second = std::get<2>(currentConnection);

			connectionsPlaced++;
		}

		//take the set difference: connections \ filteredconnections
		std::list<std::tuple<int, int, int, int>> difference;
		std::set_difference(connections.begin(), connections.end(), filteredConnections.begin(), filteredConnections.end(), std::inserter(difference, difference.begin()));

		connections.clear();
		std::copy(difference.begin(), difference.end(), std::back_inserter(connections));
	}
}

void RandomLevelGenerator::TrimEnds(const int iterations)
{
	//(y,x)
	std::list<std::pair<int, int>> newWalls;

	for (int i = 0; i < iterations; i++)
	{
		newWalls.clear();

		for (int y = 0; y < mapHeight - 1; y++)
		{
			for (int x = 0; x < mapWidth - 1; x++)
			{
				if (map[y * mapWidth + x].first == '#') continue;

				int walls = 0;

				//this pattern appears quite often, maybe create a utility function for this that can evaluate lambdas?
				for (int yOffset = -1; yOffset <= 1; yOffset++)
				{
					for (int xOffset = -1; xOffset <= 1; xOffset++)
					{
						if (yOffset == xOffset || yOffset == -xOffset) continue;

						if (map[(y + yOffset) * mapWidth + (x + xOffset)].first == '#')
						{
							walls++;
						}
					}
				}

				if (walls > 2)
				{
					newWalls.push_back(std::make_pair(y, x));
				}
			}
		}

		std::for_each(newWalls.begin(), newWalls.end(), [=](std::pair<int, int> elem)
			{
				map[elem.first * mapWidth + elem.second].first = '#';
				map[elem.first * mapWidth + elem.second].second = -1;
			});
	}
}

RandomLevelGenerator::RandomLevelGenerator(int mapWidth, int mapHeight, int maxRoomWidth, int maxRoomHeight, int roomPlaceAttempts)
	:mapWidth(mapWidth), mapHeight(mapHeight), maxRoomWidth(maxRoomWidth), maxRoomHeight(maxRoomHeight), roomPlaceAttempts(roomPlaceAttempts), rd(), gen(rd())
{
	map = (std::pair<char,int> *)malloc(sizeof(std::pair<char, int>) * mapWidth * mapHeight);
}

void RandomLevelGenerator::GenerateRandomLevel(std::wstring& stringMap, int& spawnX, int& spawnY)
{
	int id = -1;

	//clear map buffer
	for (int y = 0; y < mapHeight; y++)
	{
		for (int x = 0; x < mapWidth; x++)
		{
			map[y * mapWidth + x] = std::make_pair('#', id);
		}
	}

	//place rooms randomly
	std::uniform_int_distribution<> roomHeightDistr(1, maxRoomHeight);
	std::uniform_int_distribution<> roomWidthDistr(1, maxRoomWidth);

	//hopefully x and y are not swapped lol
	std::uniform_int_distribution<> roomX(1, mapWidth - 1);
	std::uniform_int_distribution<> roomY(1, mapHeight - 1);

	id = 0;
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
	}

	int unsetId = id;
	id++;

	//merge rooms
	for (int y = 1; y < mapHeight - 1; y++)
	{
		for (int x = 1; x < mapWidth - 1; x++)
		{
			if (map[y * mapWidth + x].first == '.' && map[y * mapWidth + x].second == unsetId)
			{
				std::stack<std::pair<int, int>> stack;
				stack.emplace(std::make_pair(y, x));

				while (!stack.empty())
				{
					std::pair<int, int> currentTile = stack.top();
					stack.pop();

					if (map[currentTile.first * mapWidth + currentTile.second].second != unsetId) continue;

					map[currentTile.first * mapWidth + currentTile.second].second = id;

					if (map[(currentTile.first - 1) * mapWidth + currentTile.second].first == '.') //up
						stack.emplace(std::make_pair(currentTile.first - 1, currentTile.second));

					if (map[(currentTile.first + 1) * mapWidth + currentTile.second].first == '.') //down
						stack.emplace(std::make_pair(currentTile.first + 1, currentTile.second));

					if (map[(currentTile.first) * mapWidth + (currentTile.second - 1)].first == '.') //left
						stack.emplace(std::make_pair(currentTile.first, currentTile.second - 1));

					if (map[(currentTile.first) * mapWidth + (currentTile.second + 1)].first == '.') //right
						stack.emplace(std::make_pair(currentTile.first, currentTile.second + 1));
				}

				id++;			
			}
		}
	}

	//floodfill
	for (int y = 1; y < mapHeight - 1; y++)
	{
		for (int x = 1; x < mapWidth - 1; x++)
		{
			int xStart, xEnd, yStart, yEnd;
			GetBoundingRectangle(xStart, xEnd, yStart, yEnd, Direction::Origin);

			if (map[y * mapWidth + x].first == '#' && CanPlaceTunnel(x, y, xStart, xEnd, yStart, yEnd))
			{
				//generate passage ways
				GeneratePassageWays(x, y, id);
				id++;
			}
		}
	}

	CreateConnections();

	//trim ends
	TrimEnds(15);

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