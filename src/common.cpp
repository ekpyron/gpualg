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
