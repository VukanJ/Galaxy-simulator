#include "Galaxy.h"

Galaxy::Galaxy(unsigned int amount, sf::Vector2i screen)
{
	Root = nullptr;
	velocities.reserve(amount);
	forces.reserve(amount);
	for (int i = 0; i < amount; i++)
		add(sf::Vector2i(rand() % screen.x, rand() % screen.y));

	
	for (int i = -200; i < 200; i+=5){
		for (int j = -200; j < 200; j+=5){
		//	add(sf::Vector2i(j+400,i+300));
			//velocities.back().x = -6000;
		//	add(sf::Vector2i(j + 400, i + 300));
			//velocities.back().x = (rand() % 15000) -7500;
			//velocities.back().y = (rand() % 15000) -7500;
		}
	}
	for (int i = 0; i <4000; i++){
		float phi = rand() % 10000 * (2 * M_PI * 10000);
		int r = rand() % 300;
		add(sf::Vector2i((int)r*cos(phi)+400, (int)r*sin(phi)+400));
		velocities.back() = sf::Vector2f(-(0.09*r*r+2000) * sin(phi),(0.09*r*r+2000) * cos(phi));
	}
}

Galaxy::~Galaxy()
{
	clear(Root);
}

void Galaxy::add(sf::Vector2i pos)
{
	stars.append(sf::Vertex(sf::Vector2f(pos.x, pos.y), sf::Color::White));
	insert(stars.getVertexCount() - 1);
	velocities.push_back(sf::Vector2f(0, 0));
	forces.push_back(sf::Vector2f(0, 0));
}

void Galaxy::clear(StarNode* star)
{
	if (star == nullptr)return;
	for (int c = 0; c < 4; clear(star->quads[c++]));
	delete star;
}

void Galaxy::update(float timestep)
{
	InfoString.clear();
	InfoString += "Stars: " + to_string((int)stars.getVertexCount()) + '\n';
	for (auto& f : forces){ f = sf::Vector2f(0, 0); }
	calcForce(Root, timestep);
	step(timestep);
	clear(Root);
	Root->mass = 1;
	Root = nullptr;
	for (int i = 0; i < stars.getVertexCount(); i++)
		insert(i);
	//std::cout << time(0) << ' ' <<Root->_1<< ' ' <<Root->_2<< ' ' <<Root->_3<< ' ' <<Root->_4<< std::endl;
}

void Galaxy::render(sf::RenderWindow& window,sf::Shader& glow)
{
	window.draw(stars,&glow);
	//window.draw(DebugCOM);
	
	//drawBoxes(window,Root);
}

void Galaxy::drawBoxes(sf::RenderWindow& window, StarNode* node)
{
	sf::RectangleShape rect;
	rect.setOutlineColor(sf::Color(255, 255, 255, 30));
	rect.setOutlineThickness(1);
	rect.setFillColor(sf::Color::Transparent);
	for (int i = 0; i < 4; i++){
		if (node->quads[i] != nullptr)
			drawBoxes(window, node->quads[i]);
		else{
			rect.setPosition(node->region.x, node->region.y);
			rect.setSize(sf::Vector2f(node->region.z - 1, node->region.z - 1));
			window.draw(rect);
		}
	}
}

Galaxy::StarNode::StarNode(unsigned int index)
{
	this->index = index;
	quads[0] = quads[1] = quads[2] = quads[3] = nullptr;
	mass = 1;
	leaf = true;
}

Galaxy::StarNode::StarNode(sf::Vector3f reg, unsigned int index)
{
	this->index = index;
	quads[0] = quads[1] = quads[2] = quads[3] = nullptr;
	mass = 1;
	leaf = true;
	region = reg;
}

void Galaxy::insert(unsigned int index)
{
	if (Root == nullptr){
		sf::FloatRect box = stars.getBounds();
		Root = new StarNode(0);
		Root->com.x = stars[0].position.x;
		Root->com.y = stars[0].position.y;
		Root->region = sf::Vector3f(box.left, box.top, std::max(box.width, box.height));
	}
	else
		insert(Root, index, 0,true);
}

void Galaxy::insert(StarNode* node, unsigned int index, unsigned int depth, bool incmass)
{
	if (depth > maxdepth) return;
	if (node->leaf){
		node->leaf = false;
		depth++;
		
		insert(node, node->index, depth,false);
		insert(node, index, depth,true);
	}
	else { // Update COM, insert star
		depth++;
		node->com = (node->com*node->mass + stars[index].position) / (node->mass+1);
		if (incmass)node->mass += 1.f;
		int reg = getRegion(node->region, stars[index].position);
		if (node->quads[reg] == nullptr){
			node->quads[reg] = new StarNode(subregion(node->region, reg), index);
			node->quads[reg]->com = stars[index].position;
		}
		else{
			insert(node->quads[reg], index, depth,true);
			node->quads[reg]->leaf = false;
		}
	}
	
}

void Galaxy::calcForce(StarNode* start, float timestep)
{
	// "Force == Acceleration"
	if (start == nullptr)return; // No stars
	// Search individual stars and calculate forces
	for (int reg = 0; reg < 4; reg++)
		calcForce(start->quads[reg], timestep);
	if (start->leaf){
		calcForce(start, Root); // Start calculating at root
		velocities[start->index] += forces[start->index] * timestep;
	}
	if (forces[start->index].x == 0){
		InfoString[0] = 'E';
	}
}

void Galaxy::calcForce(StarNode* ref, StarNode* other)
{ // Incoming target body ref
	// other 
	static int callcount = 0;
	if (other == nullptr)return;
	StarNode** otherQuads = other->quads;
	for (int q = 0; q < 4; q++){
		if (otherQuads[q] != nullptr && otherQuads[q] != ref && ref != nullptr){
			if (otherQuads[q]->leaf){
				forces[ref->index] += newtonGravity(stars[ref->index].position,
													stars[otherQuads[q]->index].position, 1.f);
			}
			else{
				// Star cluster
				if (otherQuads[q]->region.z / vabs(stars[ref->index].position - otherQuads[q]->com)<Theta){
					// Cluster is far away
					// Calculate force between cluster and star
					forces[ref->index] += newtonGravity(stars[ref->index].position,
														otherQuads[q]->com, otherQuads[q]->mass);
				}
				else{
					// Cluster is nearby => Repeat procedure for clustered stars
					for (int reg = 0; reg < 4; reg++)
						calcForce(ref, otherQuads[q]->quads[reg]);
				}
			}
		}
	}
}

sf::Vector2f Galaxy::newtonGravity(sf::Vector2f target, sf::Vector2f other, float mass)
{
	float r = vabs(other - target);
	sf::Vector2f r1r2 = other - target;
	return r1r2*mass*(_G_ / ((float)(pow(r, 2))));
}

void Galaxy::step(float timestep)
{
	for (int i = 0; i < stars.getVertexCount(); i++)
		stars[i].position += velocities.at(i)*timestep;
}

sf::Vector3f Galaxy::subregion(sf::Vector3f region, unsigned char q)
{ // q in {0,1,2,3}
	switch (q){
		case 0: break;
		case 1: region.x += region.z / 2.f; break;
		case 2: region.y += region.z / 2.f; break;
		case 3: region.x += region.z / 2.f; region.y += region.z / 2.f; break;
		default: std::cerr << "Invalid use of subregion" << std::endl;
	}
	region.z /= 2.f;
	return region;
}

short Galaxy::getRegion(sf::Vector3f region, sf::Vector2f pos)
{
	if (pos.x > region.x + region.z / 2.f){
		if (pos.y > region.y + region.z / 2.f)
			return 3;
		else return 1;
	}
	else{
		if (pos.y > region.y + region.z / 2.f)
			return 2;
		else return 0;
	}
}

static float vabs(sf::Vector2f vec)
{
	return sqrt(vec.x*vec.x + vec.y*vec.y);
}

static sf::Vector2f norm(sf::Vector2f v)
{
	float a = vabs(v);
	return sf::Vector2f(v.x / a, v.y / a);
}

sf::Vector2f operator+(sf::Vector2f v1, sf::Vector2f v2)
{
	return sf::Vector2f(v1.x + v2.x, v1.y + v2.y);
}

sf::Vector2f operator-(sf::Vector2f v1, sf::Vector2f v2)
{
	return sf::Vector2f(v1.x - v2.x, v1.y - v2.y);
}

sf::Vector2f operator*(sf::Vector2f v, float c)
{
	return sf::Vector2f(c*v.x, c*v.y);
}

sf::Vector2f operator+(sf::Vector2f v, float c)
{
	return sf::Vector2f(c + v.x, c + v.y);
}

sf::Vector2f operator-(sf::Vector2f v, float c)
{
	return sf::Vector2f(v.x - c, v.y - c);
}

sf::Vector2f operator/(sf::Vector2f v, float c)
{
	return sf::Vector2f(v.x / c, v.y / c);
}