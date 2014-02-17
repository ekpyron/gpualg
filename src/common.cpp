#include "common.h"
#include <iostream>

std::string LoadFile (const std::string &filename)
{
	std::ifstream in (filename, std::ios::in);
	if (in)
	{
		std::vector<char> data;
		in.seekg (0, std::ios::end);
		data.resize (in.tellg ());
		in.seekg (0, std::ios::beg);
		in.read (&data[0], data.size ());
		data.resize (in.gcount ());
		in.close ();
		return std::string (data.begin (), data.end ());
	}
	throw std::runtime_error ("Could not read file.");
}

GLuint LoadShaderProgram (GLenum type, const std::string &src)
{
	GLuint program = glCreateProgram ();

	GLuint shader = glCreateShader (type);
	const char *s = src.c_str ();
	glShaderSource (shader, 1, &s, NULL);
	glCompileShader (shader);

	GLint v;
	glGetShaderiv (shader, GL_COMPILE_STATUS, &v);
	if (v != GL_TRUE)
	{
		GLint len;
		glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &len);
		std::vector<char> data;
		data.resize (len);
		glGetShaderInfoLog (shader, len, NULL, &data[0]);
		throw std::runtime_error (std::string (&data[0], len));
	}

	glAttachShader (program, shader);
	glLinkProgram (program);

	glGetProgramiv (program, GL_LINK_STATUS, &v);
	if (v != GL_TRUE)
	{
		GLint len;
		glGetProgramiv (program, GL_INFO_LOG_LENGTH, &len);
		std::vector<char> data;
		data.resize (len);
		glGetProgramInfoLog (program, len, NULL, &data[0]);
		throw std::runtime_error (std::string (&data[0], len));
	}

	glDetachShader (program, shader);
	glDeleteShader (shader);

	return program;
}
