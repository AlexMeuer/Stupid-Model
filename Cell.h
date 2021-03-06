#ifndef _CELL_H
#define _CELL_H
#include <SFML/Graphics/RectangleShape.hpp>

namespace sf {
	class RenderWindow;
}

using namespace std;

struct Cell {
	bool hasBug;
	bool hasPredator;

	Cell();
};

struct HabitatCell : public Cell {
	float foodAvailability;
	float maxFoodProductionRate;
	sf::RectangleShape m_shape;


	HabitatCell( int xPos, int yPos, float MaxFoodProductionRate = 0.01f, float FoodAvailability = 0.0f );

	//Returns available food in cell
	float ProduceFood();
	void Draw(sf::RenderWindow &w) const;
};
#endif