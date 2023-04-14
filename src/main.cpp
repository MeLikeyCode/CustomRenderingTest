#include <GL/glew.h> // must be included before any opengl headers (sfml includes opengl headers, so we must include glew before sfml)
#include <SFML/Graphics.hpp>

#include "CircleBatch.h" // my own custom batch circle renderer (uses instancing)

/// Get a random integer in the range [from, to] (both inclusive)
int randomInt(int from, int to)
{
    return from + rand() % (to - from + 1);
}

/// Get a random float in the range [from, to] (both inclusive)
float randomFloat(float from, float to) {
    return from + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (to - from)));
}

/// Create a CircleBatch containing a bunch of random circles.
CircleBatch createBatchCircleDrawable(int numCircles, int xmin, int xmax, int ymin, int ymax, int rmin, int rmax, int numSegmentsPerCircle)
{
	std::vector<std::tuple<sf::Vector2f, float, sf::Color>> circles;
    for (int i = 0; i < numCircles; i++)
    {
        int x = randomInt(xmin, xmax);
        int y = randomInt(ymin, ymax);
        int r = randomInt(rmin, rmax);
		circles.push_back(std::make_tuple(sf::Vector2f(x, y), r, sf::Color(rand() % 255, rand() % 255, rand() % 255, rand() % 255)));
	}
    CircleBatch customBatchDrawable(circles, numSegmentsPerCircle);

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

/// Move each circle individually (i.e. for each cirlce, issue one GPU command to modify its position data)
void moveCirclesIndividually(CircleBatch& circleBatch, int numCirclesToMove, float moveAmount) {
    for (size_t i = 0; i < numCirclesToMove; i++)
    {
        const auto circle = circleBatch.getCircle(i);
        float dx = randomFloat(-1, 1) * moveAmount;
        float dy = randomFloat(-1, 1) * moveAmount;
        float newX = std::get<0>(circle).x + dx;
        float newY = std::get<0>(circle).y + dy;
        auto newCircle = std::make_tuple(sf::Vector2f(newX, newY), std::get<1>(circle), std::get<2>(circle));
        circleBatch.modifyCircle(i, newCircle);
    }
}

/// Move the circles in batch (i.e. issue a single GPU command to modify the position data of all the circles).
void moveCirclesBatch(CircleBatch& circleBatch, int numCirclesToMove, float moveAmount) {
    
    std::vector<std::tuple<sf::Vector2f, float, sf::Color>> circles;
    for (size_t i = 0; i < numCirclesToMove; i++)
    {
        const auto circle = circleBatch.getCircle(i);
        float dx = randomFloat(-1, 1) * moveAmount;
        float dy = randomFloat(-1, 1) * moveAmount;
        float newX = std::get<0>(circle).x + dx;
        float newY = std::get<0>(circle).y + dy;
        auto newCircle = std::make_tuple(sf::Vector2f(newX, newY), std::get<1>(circle), std::get<2>(circle));
        circles.push_back(newCircle);
    }
    circleBatch.modifyCircles(0,circles);
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
	CircleBatch customBatchDrawable = createBatchCircleDrawable(numCircles, xmin, xmax, ymin, ymax, rmin, rmax, numSegmentsPerCircle);
	std::vector<sf::CircleShape> sfmlCircles = createSfmlCircles(numCircles, xmin, xmax, ymin, ymax, rmin, rmax, numSegmentsPerCircle);
    
    // this is what determines which will be rendered (my own batch renderer or sfmls)
    bool useCustomRenderer = true; // if false, use sfml renderer

    window.setActive(true); // make the window's opengl context the current context (opengl commands operate on a current context)

    int framesPassed = 0; // number of frames that have passed since we updated the fps counter
    sf::Clock fpsClock; // time that has passed since the last time we updated the fps counter

    sf::Clock frameClock; // time since last frame

    while (window.isOpen())
    {
        // handle all events in event queue
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized)
                window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
        }

        // update
        float dt = frameClock.restart().asSeconds();
        float moveSpeed = 100; // pixels per second
        //float moveAmount = moveSpeed * dt;
        float moveAmount = 2;
        int numCirclesToMove = customBatchDrawable.numCircles();
        //int numCirclesToMove = 5000;
        //moveCirclesIndividually(customBatchDrawable, numCirclesToMove, moveAmount);
        moveCirclesBatch(customBatchDrawable, numCirclesToMove, moveAmount);

        // render
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
        framesPassed += 1;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps.setString(std::to_string(framesPassed));
            framesPassed = 0;
            fpsClock.restart();
        }
    }

    return 0;
}