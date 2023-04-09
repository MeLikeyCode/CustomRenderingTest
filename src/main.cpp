#include <GL/glew.h> // must be included before any opengl headers (sfml includes opengl headers, so we must include glew before sfml)
#include <SFML/Graphics.hpp>

#include "BatchCircleDrawable.h" // my own custom batch circle renderer (uses instancing)

/// Get a random integer in the range [from, to] (both inclusive)
int randomInt(int from, int to)
{
    return from + rand() % (to - from + 1);
}

/// Create a BatchCircleDrawable containing a bunch of random circles.
BatchCircleDrawable createBatchCircleDrawable(int numCircles, int xmin, int xmax, int ymin, int ymax, int rmin, int rmax, int numSegmentsPerCircle)
{
	std::vector<std::tuple<sf::Vector2f, float, sf::Color>> circles;
    for (int i = 0; i < numCircles; i++)
    {
        int x = randomInt(xmin, xmax);
        int y = randomInt(ymin, ymax);
        int r = randomInt(rmin, rmax);
		circles.push_back(std::make_tuple(sf::Vector2f(x, y), r, sf::Color(rand() % 255, rand() % 255, rand() % 255, rand() % 255)));
	}
    BatchCircleDrawable customBatchDrawable(circles, numSegmentsPerCircle);

    return customBatchDrawable;
}

/// Create a bunch of sf::CircleShapes.
std::vector<sf::CircleShape> createSfmlCircles(int numCircles, int xmin, int xmax, int ymin, int ymax, int rmin, int rmax, int numSegmentsPerCircle)
{
	std::vector<sf::CircleShape> sfmlCircles;
	for (int i = 0; i < numCircles; i++)
  {
        int x = randomInt(xmin, xmax);
        int y = randomInt(ymin, ymax);
        int r = randomInt(rmin, rmax);
		sf::CircleShape circle(r, numSegmentsPerCircle);
		circle.setFillColor(sf::Color(rand() % 255, rand() % 255, rand() % 255, rand() % 255));
		circle.setPosition(x, y);
		sfmlCircles.push_back(circle);
	}

	return sfmlCircles;
}

/// Create FPS counter
sf::Text createFpsCounterText(const sf::Font& font)
{
	sf::Text fps;
	fps.setFont(font);
	fps.setCharacterSize(100);
	fps.setFillColor(sf::Color::White);
	fps.setOutlineColor(sf::Color::Black);
	fps.setOutlineThickness(40);
	fps.setPosition(10, 10);

	return fps;
}

int main()
{
    // create a sfml window, which also creates an opengl context.
    // an opengl context is bascially a state machine that holds all the state for opengl.
    // for example, the current shader program, the current vertex array object, etc.
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Custom Batch Rendering (Instancing)");

    // must initialize glew *after* context has been created
    glewInit();

    // fps counter
    sf::Font font;
    font.loadFromFile("Roboto-Regular.ttf");
    sf::Text fps = createFpsCounterText(font);

    // create (don't draw yet) the circles
    int xmax = window.getSize().x;
    int ymax = window.getSize().y;
    int xmin = 0;
    int ymin = 0;
    int rmax = 5;
    int rmin = 1;
    int numCircles = 300000;
    int numSegmentsPerCircle = 100;

    // we can either use my custom batch renderer, or just use sfml's built in circle renderer, so we create circles for both
	BatchCircleDrawable customBatchDrawable = createBatchCircleDrawable(numCircles, xmin, xmax, ymin, ymax, rmin, rmax, numSegmentsPerCircle);
	std::vector<sf::CircleShape> sfmlCircles = createSfmlCircles(numCircles, xmin, xmax, ymin, ymax, rmin, rmax, numSegmentsPerCircle);
    
    // this is what determines which will be rendered (my own batch renderer or sfmls)
    bool useCustomRenderer = true; // if false, use sfml renderer

    window.setActive(true); // make the window's opengl context the current context (opengl commands operate on a current context)

    int numFrames = 0; // number of frames that have passed
    sf::Clock clock; // time that has passed

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

        if (useCustomRenderer)
        {
            // draw circles using custom batch (instancing) renderer
            window.draw(customBatchDrawable);
        }
        else {
            // draw circles using sfml
            for (auto& circle : sfmlCircles)
            {
                window.draw(circle);
            }
        }
        window.draw(fps);
        
        window.display();

        // update fps counter
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