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
		in.close ();
		return std::string (data.begin (), data.end ());
	}
	throw std::runtime_error ("Could not read file.");
}

GLuint LoadShaderProgram (GLenum type, const std::string &src)
{
	const char *s = src.c_str ();
	GLuint program = glCreateShaderProgramv (type, 1, &s);
	if (program == 0)
	{
		GLint len;
		glGetProgramiv (program, GL_INFO_LOG_LENGTH, &len);
		std::vector<char> data;
		data.resize (len);
		glGetProgramInfoLog (program, len, NULL, &data[0]);
		throw std::runtime_error (std::string (&data[0], len));
	}
	return program;
}
