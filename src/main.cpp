#include <iostream>
#include <oglp/oglp.h>
#include <GLFW/glfw3.h>
#include "PrefixSum.h"

GLFWwindow *window = NULL;
PrefixSum *prefixsum = NULL;

void initialize (void)
{
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint (GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window = glfwCreateWindow (1280, 720, "PREFIXSUM", NULL, NULL);
    if (window == NULL)
        throw std::runtime_error ("Cannot open window.");
    glfwMakeContextCurrent (window);

		gl::Init ((gl::GetProcAddressCallback) glfwGetProcAddress);

		prefixsum = new PrefixSum ();
}

void cleanup (void)
{
	if (prefixsum != NULL)
		 delete prefixsum;
	if (window != NULL)
		 glfwDestroyWindow (window);
	glfwTerminate ();
}


int main (int argc, char *argv[])
{
	if (!glfwInit ())
	{
		std::cerr << "Cannot initialize GLFW." << std::endl;
		return -1;
	}

	try {
		initialize ();

		prefixsum->Run ();

		while (!glfwWindowShouldClose (window))
		{
			prefixsum->Frame ();
			glfwSwapBuffers (window);
			glfwPollEvents ();
		}

		cleanup ();
		return 0;
	} catch (std::exception &e) {
		cleanup ();
		std::cerr << "Exception: " << e.what () << std::endl;
		return -1;
	}
}
