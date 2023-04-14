#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

/// A drawable that can draw a batch of circle primitives (each with its own position/radius/color).
/// This class represents circles as a tuple of (position, radius, color).
///
/// Example:
/// ~~~~~~~~~~~~~~~~
/// CircleBatch d{theCircles}; // theCircles is a vector of (position,radius,color) tuples
/// 
/// sf::RenderTarget renderTarget; // assume this is assigned
/// renderTarget.draw(&d);
/// ~~~~~~~~~~~~~~~~
/// 
/// Stores vertices/etc of the circles on the GPU as well as the CPU. 
/// This information is always on the GPU, even
/// when not drawn yet (this makes drawing CircleBatch very fast, with the trade off of
/// always using some GPU memory).
class CircleBatch : public sf::Drawable
{
public:
	/// Construct from a vector of circles. Each circle is a (position,radius,color) tuple.
	CircleBatch(const std::vector< std::tuple<sf::Vector2f, float, sf::Color> >& circles, int segmentsPerCircle);
	
	virtual ~CircleBatch() = default;

	/// Get the number of circles in the CircleBatch.
	int numCircles() const;

	/// Modify a circle in the CircleBatch.
	/// 
	/// Modifying the circles in a CircleBatch is slow (b/c circles are stored on the GPU). In particular,
	/// if you need to modify a lot of circles, it's better to use modifyCircles() instead (which will do
	/// the modification's in a batch, i.e. less cpu->gpu round trips).
	void modifyCircle(int circleIndex, const std::tuple<sf::Vector2f, float, sf::Color>& circle);

	/// Modify a range of circles in the CircleBatch, starting at startIndex and ending at (startIndex + circles.size()).
	void modifyCircles(int startIndex, const std::vector< std::tuple<sf::Vector2f, float, sf::Color> >& circles);

	/// Get the circle at the given index.
	std::tuple<sf::Vector2f, float, sf::Color> getCircle(int circleIndex) const;

protected:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	std::vector< std::tuple<sf::Vector2f, float, sf::Color> > _circles;
	unsigned int _shaderProgram;
	unsigned int _vbo;
	unsigned int _vao;
	unsigned int _instanceVBO;
	int _sizePerInstance;
	int _numVerticesForACircle;

	void _checkShaderCompileErrors(unsigned int shader);
};

