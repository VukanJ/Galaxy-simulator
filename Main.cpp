#include <iostream>
#include <SFML/Graphics.hpp>
#include <ctime>

#include "Galaxy.h"

using namespace std;

const unsigned int SIZEX = 800, SIZEY = 800;

void loadShader(sf::Shader& shader)
{
	if (!shader.loadFromFile("glow_frag.glsl", sf::Shader::Fragment)){
		cerr << "Missing shader: glow_frag.glsl\n";
		cin.ignore();
		exit(1);
	}
}

int main(int argc, char* argv[])
{
	srand(static_cast<unsigned int>(time(0)));

	sf::RenderWindow window(sf::VideoMode(SIZEX, SIZEY), "Galaxy");
	window.setFramerateLimit(60);

	Galaxy galaxy(1, sf::Vector2i(800, 800));

	sf::View view;

	float timestep = 1.f / 2000.f;
	// Load Info Text
	sf::Font font;
	if (!font.loadFromFile("Orbitron-Regular.ttf")){
		cout << "Error: lucan.ttf missing in Program dir\n";
		return 1;
	}
	sf::Clock clock;
	sf::Text infoText;
	infoText.setFont(font);
	infoText.setString("INFOTEXT");
	infoText.setCharacterSize(20);
	float fpsTime = 0;
	//

	sf::Shader glow;
	loadShader(glow);
	sf::Shader::bind(&glow);

	while (window.isOpen()){
		sf::Event ev;
		fpsTime = clock.restart().asSeconds();
		while (window.pollEvent(ev)){
			switch (ev.type){
			case sf::Event::Closed:window.close(); break;
			case sf::Event::MouseButtonPressed:
				galaxy.add(sf::Mouse::getPosition(window));
			case sf::Event::KeyPressed:
				switch (ev.key.code){
				case sf::Keyboard::Up:view.move(0, -10); break;
				case sf::Keyboard::Down:view.move(0, 10); break;
				case sf::Keyboard::Left:view.move(-10, 0); break;
				case sf::Keyboard::Right:view.move(10, 0); break;
				case sf::Keyboard::Add:view.zoom(0.9f); break;
				case sf::Keyboard::Subtract:view.zoom(1.1f); break;
				case sf::Keyboard::Escape:window.close(); break;
				case sf::Keyboard::N: timestep *= .9f; break;
				case sf::Keyboard::M: timestep *= 1.1f; break;
				case sf::Keyboard::V: Theta *= .9f; break;
				case sf::Keyboard::B: Theta *= 1.1f; break;
				}
				break;
			}
		}
		infoText.setPosition(view.getCenter().x - SIZEX / 2.f - 50.f, view.getCenter().y - SIZEY / 2.f - 50.f);
		window.setView(view);
		window.clear(sf::Color::Black);
		galaxy.update(timestep);
		galaxy.render(window,glow);
		infoText.setString("FPS " + to_string((int)(1. / fpsTime)) + "\nTimestep "+
			to_string(timestep)+
			"\nTheta " + to_string(Theta) + '\n'
			+ galaxy.InfoString);
		window.draw(infoText);
		window.display();
	}
	sf::Shader::bind(NULL);
	return 0;
}
