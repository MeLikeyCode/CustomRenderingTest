#include <iostream>

#include <GL/glew.h>

#include <SFML/Graphics.hpp>

#include "BatchCircleDrawable.h"

int main()
{
    // create a window, which also creates an opengl context.
    // an opengl context is bascially a state machine that holds all the state for opengl.
    // for example, the current shader program, the current vertex array object, etc.
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Custom Physics");

    // must initialize glew *after* context has been created
    glewInit();

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(0, 0);

    sf::Text fps;
    sf::Font font;
    font.loadFromFile("Roboto-Regular.ttf");
    fps.setFont(font);
    fps.setCharacterSize(100);
    fps.setFillColor(sf::Color::White);
    fps.setOutlineColor(sf::Color::Black);
    fps.setOutlineThickness(40);
    fps.setPosition(10, 10);

    // create random circles
    int xmax = window.getSize().x;
    int ymax = window.getSize().y;
    int xmin = 0;
    int ymin = 0;
    int rmax = 50;
    int rmin = 0;

    std::vector< std::tuple<sf::Vector2f, float, sf::Color> > circles;
    for (int i = 0; i < 50000; ++i)
    {
		circles.push_back(std::make_tuple(sf::Vector2f(xmin + rand() % xmax, ymin + rand() % ymax), rmin + rand() % rmax, sf::Color(rand() % 255, rand() % 255, rand() % 255,rand() % 255)));
	}
    BatchCircleDrawable customDrawable(circles,100);

    window.setActive(true);

    int numFrames = 0;
    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized)
                window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
        }

        window.clear();

        //window.draw(shape);
        window.draw(customDrawable);
        window.draw(fps);
        window.display();

        numFrames += 1;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps.setString(std::to_string(numFrames));
			numFrames = 0;
			clock.restart();
		}
    }

    return 0;
}