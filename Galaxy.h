#define _USE_MATH_DEFINES
#ifndef GALAXY_H
#define GALAXY_H
#include <iostream>
#include <typeinfo>
#include <vector>
#include <cmath>
#include <string>
#include <SFML/Graphics.hpp>

using namespace std;

const unsigned int maxdepth = 1000; // maximum treedepth
static float Theta = 1; // Accuracy (0=>Newton N-Body Simulation)
const float _G_ = 10000; // Gravitational constant

class Galaxy
{
private:
	class StarNode
	{
	public:
		StarNode(unsigned int index);
		StarNode(sf::Vector3f region, unsigned int index);
		sf::Vector3f region; // [x,y,WIDTH]
		sf::Vector2f com; // [x,y]
		unsigned int index;  // Pointer to pixel
		bool leaf;
		float mass;
		StarNode* quads[4];
	}*Root;
	sf::VertexArray stars; // Stores pixels
	std::vector<sf::Vector2f> velocities, forces;
	sf::Vector3f subregion(sf::Vector3f region, unsigned char q);
	void drawBoxes(sf::RenderWindow&, StarNode* node);
	short getRegion(sf::Vector3f region, sf::Vector2f pos);
	void insert(unsigned int index);
	void insert(StarNode* node, unsigned int index, unsigned int depth, bool incmass);
	void calcForce(StarNode* root, float timestep);
	void calcForce(StarNode*, StarNode*); // Barnes Hut Algorithm
	void step(float); // Moves stars according to velocity
	inline sf::Vector2f newtonGravity(sf::Vector2f target, sf::Vector2f, float);

	// DEBUG
	sf::VertexArray DebugCOM;
public:
	string InfoString;
	Galaxy(unsigned int amount, sf::Vector2i screen);
	~Galaxy();
	void add(sf::Vector2i pos);
	void update(float timestep);
	void render(sf::RenderWindow&, sf::Shader& glow);
	void clear(StarNode*); // Deletes Galaxy
};

static float vabs(sf::Vector2f);
static sf::Vector2f norm(sf::Vector2f);

static ostream& operator<<(ostream& stream, sf::Vector2f& v)
{
	printf("[%f,%f]", v.x,v.y);
	return stream;
}

/*
Subdivision pattern
_____________
|     |     |
|  0  |  1  |
|_____|_____|
|     |     |
|  2  |  3  |
|_____|_____|

*/

#endif