#include "RandomLevelGenerator.h"

#include <random>
#include <algorithm>
#include <iterator>
#include <stack>
#include <tuple>

//TODO: add constants for id's for walls, passageways(?) and rooms(?)

RandomLevelGenerator::BoundingRectangle RandomLevelGenerator::GetBoundingRectangle(const Direction dir) const
{
	BoundingRectangle rect = {};

	switch (dir)
	{
		case Direction::Up:
			rect.xStart = -1;
			rect.xEnd   = 1;
			rect.yStart = -1;
			rect.yEnd   = 0;
			break;
		case Direction::Down:
			rect.xStart = -1;
			rect.xEnd   = 1;
			rect.yStart = 0;
			rect.yEnd   = 1;
			break;
		case Direction::Left:
			rect.xStart = -1;
			rect.xEnd   = 0;
			rect.yStart = -1;
			rect.yEnd   = 1;
			break;
		case Direction::Right:
			rect.xStart = 0;
			rect.xEnd   = 1;
			rect.yStart = -1;
			rect.yEnd   = 1;
			break;
		case Direction::Origin:
			rect.xStart = -1;
			rect.xEnd = 1;
			rect.yStart = -1;
			rect.yEnd = 1;
			break;
		case Direction::Vertical:
			rect.xStart = 0;
			rect.xEnd = 0;
			rect.yStart = -1;
			rect.yEnd = -1;
			break;
		case Direction::Horizontal:
			rect.xStart = -1;
			rect.xEnd = 1;
			rect.yStart = 0;
			rect.yEnd = 0;
			break;
		default:
			rect.xStart = 0;
			rect.xEnd = 0;
			rect.yStart = 0;
			rect.yEnd = 0;
			break;
	}
	
	return rect;
}

bool RandomLevelGenerator::CanPlaceTunnel(const uint32_t x, const uint32_t y, const BoundingRectangle rect) const
{
	for (int8_t yOffset = rect.yStart; yOffset <= rect.yEnd; yOffset++)
	{
		for (int8_t xOffset = rect.xStart; xOffset <= rect.xEnd; xOffset++)
		{
			if (yOffset == 0 && xOffset == 0) continue;

			if (y + yOffset < 1 || y + yOffset > mapHeight - 2) continue;
			if (x + xOffset < 1 || x + xOffset > mapWidth - 2) continue;

			if (map[(yOffset + y) * mapWidth + (xOffset + x)].first == '.') return false;
		}
	}

	return true;
}

//precondition: (x,y) is in bounds
bool RandomLevelGenerator::CanPlaceConnection(const uint32_t x, const uint32_t y, const Direction dir) const
{
	bool ret;
	switch (dir)
	{
	case Direction::Vertical:
		ret = x > 0 && x < mapWidth - 1 &&
			  y > 1 && y < mapHeight - 2 &&
			  map[(y - 1) * mapWidth + x].first == '.' &&
			  map[(y + 1) * mapWidth + x].first == '.' &&
			  map[(y - 1) * mapWidth + x].second != map[(y + 1) * mapWidth + x].second;
		break;
	case Direction::Horizontal:
		ret = x > 1 && x < mapWidth - 2 &&
			  y > 0 && y < mapHeight - 1 &&
			  map[y * mapWidth + (x - 1)].first == '.' &&
			  map[y * mapWidth + (x + 1)].first == '.' &&
			  map[y * mapWidth + (x - 1)].second != map[y * mapWidth + (x + 1)].second;
		break;
	default:
		ret = false;
		break;
	}

	return ret;
}

template <typename T>
typename std::list<T>::iterator RandomLevelGenerator::GetRandomElement(std::list<T> &list)
{
	std::uniform_int_distribution<> randomDistribution(0, list.size() - 1);
	int randomIndex = randomDistribution(gen);
	auto iter = list.begin();
	std::advance(iter, randomIndex);
	return iter;
}

void RandomLevelGenerator::GenerateRooms(uint32_t& spawnX, uint32_t& spawnY, int32_t &id)
{
	//place rooms randomly
	std::uniform_int_distribution<> roomHeightDistr(1, maxRoomHeight);
	std::uniform_int_distribution<> roomWidthDistr(1, maxRoomWidth);

	//hopefully x and y are not swapped lol
	std::uniform_int_distribution<uint32_t> roomX(1, mapWidth - 1);
	std::uniform_int_distribution<uint32_t> roomY(1, mapHeight - 1);

	bool playerPlaced = false;
	id = 0;
	for (uint8_t i = 0; i < roomPlaceAttempts; i++)
	{
		int roomHeight = roomHeightDistr(gen);
		int roomWidth = roomWidthDistr(gen);

		uint32_t roomColumn = roomX(gen);
		uint32_t roomRow = roomY(gen);

		//TODO: just make this roomHeight not roomHeight * 2
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

		if (!playerPlaced)
		{
			spawnX = roomColumn;
			spawnY = roomRow;
			playerPlaced = true;
		}
	}

	int32_t unsetId = id;
	id++;

	//merge rooms
	for (uint32_t y = 1; y < mapHeight - 1; y++)
	{
		for (uint32_t x = 1; x < mapWidth - 1; x++)
		{
			if (map[y * mapWidth + x].first == '.' && map[y * mapWidth + x].second == unsetId)
			{
				std::stack<std::pair<uint32_t, uint32_t>> stack;
				stack.emplace(std::make_pair(y, x));

				while (!stack.empty())
				{
					std::pair<uint32_t, uint32_t> currentTile = stack.top();
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
}

//precondition: CanPlaceTunnel(x,y) == true
void RandomLevelGenerator::GeneratePassageWays(const uint32_t xOrigin, const uint32_t yOrigin, const int32_t id)
{
	//TODO: check for out of bounds

	std::list<PossiblePassageWay> possiblePassageWays;

	//check adjacent walls for viability, and if viable, add them as possible new passages
	if (yOrigin > 1)				possiblePassageWays.push_back({ xOrigin, yOrigin - 1, Direction::Up    });
	if (yOrigin < mapHeight - 2)	possiblePassageWays.push_back({ xOrigin, yOrigin + 1, Direction::Down  });
	if (xOrigin > 1)				possiblePassageWays.push_back({ xOrigin - 1, yOrigin, Direction::Left  });
	if (xOrigin > mapWidth - 2)		possiblePassageWays.push_back({ xOrigin + 1, yOrigin, Direction::Right });

	//while viable walls exist
	while (!possiblePassageWays.empty())
	{
		//take a random viable wall
		auto iter = GetRandomElement(possiblePassageWays);
		PossiblePassageWay passageWay = *iter;

		//depending on the direction of the wall, we have to check a specific rectangular area
		//if it is valid then turn this tile into a passage, and add its 3 adjacent walls as
		//viable passage ways
		BoundingRectangle rect = GetBoundingRectangle(passageWay.direction);

		if (CanPlaceTunnel(passageWay.x, passageWay.y, rect))
		{
			map[passageWay.y * mapWidth + passageWay.x].first = '.';
			map[passageWay.y * mapWidth + passageWay.x].second = id;

			//add corresponding adjacent walls
			if (rect.yStart != 0 && passageWay.y + rect.yStart > 0)
				possiblePassageWays.push_back({ passageWay.x, passageWay.y + rect.yStart, Direction::Up });

			if (rect.yEnd != 0 && passageWay.y + rect.yEnd < mapHeight - 1)
				possiblePassageWays.push_back({ passageWay.x, passageWay.y + rect.yEnd, Direction::Down });

			if (rect.xStart != 0 && passageWay.x + rect.xStart > 0)
				possiblePassageWays.push_back({ passageWay.x + rect.xStart, passageWay.y, Direction::Left });

			if (rect.xEnd != 0 && passageWay.x + rect.xEnd < mapWidth - 1)
				possiblePassageWays.push_back({ passageWay.x + rect.xEnd, passageWay.y, Direction::Right });
		}

		//remove the element from the list
		possiblePassageWays.erase(iter);
	}
}


//TODO: make this a bit clearer...
void RandomLevelGenerator::CreateConnections()
{
	//create a list of all possible connections
	std::list<PossibleConnection> connections;

	for (uint32_t y = 1; y < mapHeight - 1; y++)
	{
		for (uint32_t x = 1; x < mapWidth - 1; x++)
		{
			if (map[y * mapWidth + x].first == '.') continue;

			//wall vertically connects 2 spaces
			if (CanPlaceConnection(x, y, Direction::Vertical))
			{
				//store in list with coordinates, and 2 region id's?
				int32_t firstId = std::min(map[(y - 1) * mapWidth + x].second, map[(y + 1) * mapWidth + x].second);
				int32_t secondId = std::max(map[(y - 1) * mapWidth + x].second, map[(y + 1) * mapWidth + x].second);
				connections.push_back({ x, y, firstId, secondId });
			}

			//wall horizontally connects 2 spaces
			else if (CanPlaceConnection(x, y, Direction::Horizontal))
			{
				//store in list with coordinates, and 2 region id's?
				int32_t firstId = std::min(map[y * mapWidth + (x - 1)].second, map[y * mapWidth + (x + 1)].second);
				int32_t secondId = std::max(map[y * mapWidth + (x - 1)].second, map[y * mapWidth + (x + 1)].second);
				connections.push_back({ x, y, firstId, secondId });
			}
		}
	}

	std::uniform_int_distribution<> connectionDistribution(1, 4); //parameterize this

	//iterate over all possible connections
	while (!connections.empty())
	{
		//select a random element from the connections
		auto iter = GetRandomElement(connections);
		PossibleConnection randomConnection = *iter;

		//filter connections to only include connections that connect 2 specific regions
		std::list<PossibleConnection> filteredConnections(connections);
		filteredConnections.remove_if([=](PossibleConnection connection)
			{
				return connection.id1 != randomConnection.id1 ||
					   connection.id2 != randomConnection.id2;
			});

		//create a shuffled refference wrapper of the filtered list
		std::vector<std::reference_wrapper<PossibleConnection>> wrapper(filteredConnections.begin(), filteredConnections.end());
		std::shuffle(wrapper.begin(), wrapper.end(), gen);

		int nrOfConnections = connectionDistribution(gen);
		int connectionsPlaced = 0;

		auto iterator = wrapper.begin();
		while (iterator < wrapper.begin() + wrapper.size() && connectionsPlaced < nrOfConnections)
		{
			//get current tile
			PossibleConnection currentConnection = *iterator;
			std::advance(iterator, 1);

			//count the nr of surrounding floor tiles
			uint8_t surroundingFloors = 0;
			for (int8_t y = -1; y <= 1; y++)
			{
				for (int8_t x = -1; x <= 1; x++)
				{
					//ignore diagonals
					if (x == y || x == -y) continue;

					if (map[(currentConnection.y + y) * mapWidth + (currentConnection.x + x)].first == '.')
						surroundingFloors++;
				}
			}

			if (surroundingFloors > 2) continue;

			//update tile
			map[currentConnection.y * mapWidth + currentConnection.x].first = '.';
			map[currentConnection.y * mapWidth + currentConnection.x].second = currentConnection.id1;

			connectionsPlaced++;
		}

		//take the set difference: connections \ filteredconnections
		std::list<PossibleConnection> difference;
		std::set_difference(connections.begin(), connections.end(), filteredConnections.begin(), filteredConnections.end(), std::back_inserter(difference));

		connections.clear();
		std::copy(difference.begin(), difference.end(), std::back_inserter(connections));
	}
}

void RandomLevelGenerator::TrimEnds(const uint8_t iterations)
{
	//(y,x)
	std::list<std::pair<uint32_t, uint32_t>> newWalls;

	for (uint8_t i = 0; i < iterations; i++)
	{
		newWalls.clear();

		for (uint32_t y = 0; y < mapHeight - 1; y++)
		{
			for (uint32_t x = 0; x < mapWidth - 1; x++)
			{
				if (map[y * mapWidth + x].first == '#') continue;

				uint8_t walls = 0;

				//this pattern appears quite often, maybe create a utility function for this that can evaluate lambdas?
				for (int8_t yOffset = -1; yOffset <= 1; yOffset++)
				{
					for (int8_t xOffset = -1; xOffset <= 1; xOffset++)
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

		std::for_each(newWalls.begin(), newWalls.end(), [=](std::pair<uint32_t, uint32_t> elem)
			{
				map[elem.first * mapWidth + elem.second].first = '#';
				map[elem.first * mapWidth + elem.second].second = -1;
			});
	}
}

RandomLevelGenerator::RandomLevelGenerator(uint32_t mapWidth, uint32_t mapHeight, uint16_t maxRoomWidth, uint16_t maxRoomHeight, uint8_t roomPlaceAttempts)
	:mapWidth(mapWidth), mapHeight(mapHeight), maxRoomWidth(maxRoomWidth), maxRoomHeight(maxRoomHeight), roomPlaceAttempts(roomPlaceAttempts), rd(), gen(rd())
{
	map = (std::pair<char,int> *)malloc(sizeof(std::pair<char, int>) * mapWidth * mapHeight);
}

RandomLevelGenerator::~RandomLevelGenerator()
{
	free(map);
	map = nullptr;
}

void RandomLevelGenerator::GenerateRandomLevel(std::wstring& stringMap, uint32_t& spawnX, uint32_t& spawnY)
{
	int32_t id = -1;

	//clear map buffer
	for (uint32_t y = 0; y < mapHeight; y++)
	{
		for (uint32_t x = 0; x < mapWidth; x++)
		{
			map[y * mapWidth + x] = std::make_pair('#', id);
		}
	}

	//generate room layout
	GenerateRooms(spawnX, spawnY, id);

	//floodfill
	for (uint32_t y = 1; y < mapHeight - 1; y++)
	{
		for (uint32_t x = 1; x < mapWidth - 1; x++)
		{
			BoundingRectangle rect = GetBoundingRectangle(Direction::Origin);

			if (map[y * mapWidth + x].first == '#' && CanPlaceTunnel(x, y, rect))
			{
				//generate passage ways
				GeneratePassageWays(x, y, id);
				id++;
			}
		}
	}

	//create connections between rooms and passageways
	CreateConnections();

	//trim unnecessarily long ends of passageways
	TrimEnds(15);
	
	//transfer map over to stringmap
	stringMap.clear();
	for (uint32_t y = 0; y < mapHeight; y++)
	{
		for (uint32_t x = 0; x < mapWidth; x++)
		{
			stringMap += map[y * mapWidth + x].first;
		}
	}
}