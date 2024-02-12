#ifndef RANDOMLEVELGENERATOR_H
#define RANDOMLEVELGENERATOR_H

#include <string>
#include <random>
#include <list>
#include <iterator>

//how is map data going to be stored in the future?

class RandomLevelGenerator
{
public:
	RandomLevelGenerator(uint32_t mapWidth, uint32_t mapHeight, uint16_t maxRoomWidth, uint16_t maxRoomHeight, uint8_t roomPlaceAttempts);
	~RandomLevelGenerator();

	void GenerateRandomLevel(std::wstring &map, uint32_t &spawnX, uint32_t &spawnY);

private:

	/// <summary>
	/// TODO
	/// </summary>
	enum Direction
	{
		Up,
		Down,
		Left,
		Right,
		Origin,
		Vertical, //for connection checking: up and down neighbours
		Horizontal, //for connection checking: left and right neighbours
	};

	/// <summary>
	/// TODO
	/// </summary>
	struct BoundingRectangle
	{
		int8_t xStart;
		int8_t xEnd;
		int8_t yStart;
		int8_t yEnd;
	};

	/// <summary>
	/// TODO: helper struct
	/// </summary>
	static struct PossiblePassageWay
	{
		uint32_t x;
		uint32_t y;
		Direction direction;
	};

	/// <summary>
	/// TODO: helper struct
	/// </summary>
	static struct PossibleConnection
	{
		uint32_t x;
		uint32_t y;
		int32_t id1;
		int32_t id2;

		//this is required in order for set_difference
		bool operator<(const PossibleConnection& other) const {
			return y < other.y || (y == other.y && x < other.x);
		}
	};

	const uint32_t mapWidth;
	const uint32_t mapHeight;
	const uint16_t maxRoomWidth;
	const uint16_t maxRoomHeight;
	const uint8_t roomPlaceAttempts;

	std::random_device rd; //cant make this const?
	std::mt19937 gen;

	//not the actual map but is used during construction
	std::pair<char, int32_t> *map;

	//TODO: is there a more clever way to store the direction offsets in the direction enum?

	/// <summary>
	/// 
	/// </summary>
	/// <param name="dir"></param>
	/// <returns></returns>
	BoundingRectangle GetBoundingRectangle(const Direction dir) const;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="rect"></param>
	/// <returns></returns>
	bool CanPlaceTunnel(const uint32_t x, const uint32_t y, const BoundingRectangle rect) const;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="dir"></param>
	/// <returns></returns>
	bool CanPlaceConnection(const uint32_t x, const uint32_t y, const Direction dir) const;

	template <typename T>
	typename std::list<T>::iterator GetRandomElement(std::list<T> &list);

	void GeneratePassageWays(const uint32_t x, const uint32_t y, const int32_t id);
	void CreateConnections();
	void TrimEnds(const uint8_t iterations);
};

#endif