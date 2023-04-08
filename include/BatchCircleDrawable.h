#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

/// A drawable that can draw a batch of circle primitives (each with its own position/radius/color).
///
/// Example:
/// ~~~~~~~~~~~~~~~~
/// BatchCircleDrawable d{theCircles}; // theCircles is a vector of (position,radius,color) tuples
/// 
/// sf::RenderTarget renderTarget; // assume this is assigned
/// renderTarget.draw(&d);
/// ~~~~~~~~~~~~~~~~
/// 
/// Stores vertices/etc of the circles on the GPU. This information is always on the GPU, even
/// when not drawn yet (this makes drawing BatchCircleDrawable very fast, with the trade off of
/// always using some GPU memory).
class BatchCircleDrawable : public sf::Drawable
{
public:
	/// Construct from a vector of circles. Each circle is a (position,radius,color) tuple.
	BatchCircleDrawable(const std::vector< std::tuple<sf::Vector2f, float, sf::Color> >& circles, int segmentsPerCircle);
	virtual ~BatchCircleDrawable() = default;

protected:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	unsigned int _shaderProgram;
	unsigned int _vbo;
	unsigned int _vao;
	int _numVerticesForACircle;
	int _numCircles;

	void _checkShaderCompileErrors(unsigned int shader);
};

