#include "BatchCircleDrawable.h"

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

BatchCircleDrawable::BatchCircleDrawable(const std::vector<std::tuple<sf::Vector2f, float, sf::Color>>& circles, int segmentsPerCircle)
{
	_numCircles = circles.size();
	_segmentsPerCircle = segmentsPerCircle;

	// vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertexShaderSource = R"(
		#version 330 core

		uniform mat4 transform; // viewport transform

		layout (location = 0) in vec2 localPos;
		layout (location = 1) in vec2 center;
		layout (location = 2) in float radius;
		layout (location = 3) in vec4 color;

		out vec4 ourColor;

		void main()
		{
			vec2 position = localPos * radius + center;
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

	// create vertex buffer object (holds vertices, in GPU memory)
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
	_numVertices = vertices.size();

	// copy vertices to vbo
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	// Set vertex attribute
	glBindVertexArray(_vao);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// create instance data
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
	GLuint instanceVBO;
	glGenBuffers(1, &instanceVBO);

	// Copy instance data to the instance VBO
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(GLfloat), instanceData.data(), GL_STATIC_DRAW);

	// Set instance attribute pointers for circle centers, radii, and colors
	glBindVertexArray(instanceVBO);
	
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

void BatchCircleDrawable::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	//target.pushGLStates();

	// bind shader program
	glUseProgram(_shaderProgram);

	// bind vao
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
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, _numVertices, _numCircles);

	// unbind vao
	glBindVertexArray(0);


	//target.popGLStates();
	target.resetGLStates();
}

void BatchCircleDrawable::_checkShaderCompileErrors(unsigned int shader)
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
