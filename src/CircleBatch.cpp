#include "CircleBatch.h"

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

CircleBatch::CircleBatch(const std::vector<std::tuple<sf::Vector2f, float, sf::Color>>& circles, int segmentsPerCircle)
{
	_circles = circles;
	_sizePerInstance = 2 + 1 + 4; // 2 floats for position, 1 float for radius, 4 floats for color (rgba)

	// vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertexShaderSource = R"(
		#version 330 core

		uniform mat4 transform; // viewport transform

		layout (location = 0) in vec2 localPos; // a vertex of the circle geometry, from the "vertex VBO" (remember we only created one set of vertices for all circles to share)
		
		layout (location = 1) in vec2 center;	// position of the circle, from the "instance VBO"
		layout (location = 2) in float radius;	// also from "instance VBO"
		layout (location = 3) in vec4 color;	// ""

		out vec4 ourColor;

		void main()
		{
			vec2 position = localPos * radius + center; // take the vertex position, scale it by the radius, then translate by the position of the circle ("center")
			gl_Position = transform * vec4(position, 0.0, 1.0);
			ourColor = color;
		}
	)";

	// compile vertex shader
	const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
	glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, NULL);
	glCompileShader(vertexShader);
	_checkShaderCompileErrors(vertexShader);

	// fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragmentShaderSource = R"(
		#version 330 core

		out vec4 color;
		in vec4 ourColor;

		void main()
		{
			color = vec4(ourColor);
		}
	)";

	// compile fragment shader
	const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
	glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, NULL);
	glCompileShader(fragmentShader);
	_checkShaderCompileErrors(fragmentShader);

	// shader program
	_shaderProgram = glCreateProgram();
	glAttachShader(_shaderProgram, vertexShader);
	glAttachShader(_shaderProgram, fragmentShader);
	glLinkProgram(_shaderProgram);

	// check for program link errors
	int success;
	char infoLog[512];
	glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED";
	}

	// delete shaders (they're linked into our program now and no longer necessery)
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// create vertex buffer object (holds vertices, in GPU memory), for the vertices of a circle geometry
	glGenBuffers(1, &_vbo);

	// create vertex array object (holds info on how to interpret vbo (i.e. what fields the vertices have and where they are))
	glGenVertexArrays(1, &_vao);

	// Create a single set of vertices that all circles will use (but they will each apply a different transform to them)
	std::vector<GLfloat> vertices;
	for (int i = 0; i <= segmentsPerCircle; i++)
	{
		float angle = 2.0f * 3.1415926f * float(i) / float(segmentsPerCircle);
		float x = cosf(angle);
		float y = sinf(angle);
		vertices.push_back(x); 
		vertices.push_back(y);
	}
	_numVerticesForACircle = vertices.size();

	// copy vertices to vbo
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	// Set vertex attributes
	glBindVertexArray(_vao);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// create instance data (aspects of a circle geometry that differs per circle, e.g. its position, radius, color, etc)
	std::vector<GLfloat> instanceData;
	for (const auto& circle : circles)
	{
		const auto& position = std::get<0>(circle);
		float radius = std::get<1>(circle);
		const auto& color = std::get<2>(circle);

		instanceData.push_back(position.x);
		instanceData.push_back(position.y);
		instanceData.push_back(radius);
		instanceData.push_back(color.r / 255.0f);
		instanceData.push_back(color.g / 255.0f);
		instanceData.push_back(color.b / 255.0f);
		instanceData.push_back(color.a / 255.0f);
	}

	// create instance vbo
	glGenBuffers(1, &_instanceVBO);

	// Copy instance data to the instance VBO
	glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(GLfloat), instanceData.data(), GL_STATIC_DRAW);

	// Set instance attribute pointers for circle centers, radii, and colors
	glBindVertexArray(_instanceVBO);
	
	// center
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	// radius
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);

	// color
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	// unbind vbo and vao (we will bind them again when we draw)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int CircleBatch::numCircles() const
{
	return _circles.size();
}

void CircleBatch::modifyCircle(int circleIndex, const std::tuple<sf::Vector2f, float, sf::Color>& circle)
{
	// delegate to modifyCircles
	std::vector<std::tuple<sf::Vector2f, float, sf::Color>> circles;
	circles.push_back(circle);
	modifyCircles(circleIndex, circles);
}

void CircleBatch::modifyCircles(int startIndex, const std::vector<std::tuple<sf::Vector2f, float, sf::Color>>& circles)
{
	// update instance data for each circle
    std::vector<GLfloat> instanceData;
	for (const auto& circle : circles)
	{
		const auto& position = std::get<0>(circle);
		float radius = std::get<1>(circle);
		const auto& color = std::get<2>(circle);

		instanceData.push_back(position.x);
		instanceData.push_back(position.y);
		instanceData.push_back(radius);
		instanceData.push_back(color.r / 255.0f);
		instanceData.push_back(color.g / 255.0f);
		instanceData.push_back(color.b / 255.0f);
		instanceData.push_back(color.a / 255.0f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
	glBufferSubData(GL_ARRAY_BUFFER, startIndex * _sizePerInstance * sizeof(GLfloat), instanceData.size() * sizeof(GLfloat), instanceData.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// update _circles
	std::copy(circles.begin(), circles.end(), _circles.begin() + startIndex);
}

std::tuple<sf::Vector2f, float, sf::Color> CircleBatch::getCircle(int circleIndex) const
{
	return _circles[circleIndex];
}

void CircleBatch::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	//target.pushGLStates();

	// bind shader program
	glUseProgram(_shaderProgram);

	// bind vao (don't need to bind vbo because it's already bound to the vao???)
	glBindVertexArray(_vao);

	 // create view transformation matrix (based on window size)
	float width = target.getSize().x;
	float height = target.getSize().y;
	float scaleX = 2.0f / width;
	float scaleY = 2.0f / height;
	float offsetX = -1.0f;
	float offsetY = -1.0f;
	GLfloat transform[] = {
		scaleX, 0.0f, 0.0f, 0.0f,
		0.0f, scaleY, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		offsetX, offsetY, 0.0f, 1.0f
	};

	// set transformation matrix uniform
	GLuint transformLocation = glGetUniformLocation(_shaderProgram, "transform");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, transform);

	// Draw using instanced rendering
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, _numVerticesForACircle, _circles.size());

	// unbind vao
	glBindVertexArray(0);

	//target.popGLStates();
	target.resetGLStates(); // so subsequently, sfml drawings can be drawn
}

void CircleBatch::_checkShaderCompileErrors(unsigned int shader)
{
	GLint success;
	GLchar infoLog[1024];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 1024, NULL, infoLog);
		throw std::runtime_error("Shader compilation failed");
	}
}
