#ifndef COMMON_H
#define COMMON_H

#include <fstream>
#include <string>
#include <stdexcept>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <sstream>

std::string LoadFile (const std::string &filename);
GLuint LoadShaderProgram (GLenum type, const std::string &src);

#endif /* COMMON_H */
