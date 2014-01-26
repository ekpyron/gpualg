#include "GpuAlg.h"
#include "PrefixSum.h"
#include "RadixSort.h"

GLFWwindow *window = NULL;
GpuAlg *gpualg = NULL;

void initialize (int &argc, char **&argv)
{
    const std::unordered_map<std::string, std::function<GpuAlg*(void)>> algos = {
    		{ "prefixsum", [] { return new PrefixSum (); } },
    		{ "radixsort", [] { return new RadixSort (); } }
    };

    if (argc > 2)
    	throw std::runtime_error ("Usage: gpualg [algorithm]");
    std::string algoname = (argc > 1) ? argv[1] : "prefixsum";
    auto algo = algos.find (algoname);
    if (algo == algos.end ())
    	throw std::runtime_error ("Invalid algorithm");

    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint (GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window = glfwCreateWindow (1280, 720, "GPUALG", NULL, NULL);
    if (window == NULL)
        throw std::runtime_error ("Cannot open window.");
    glfwMakeContextCurrent (window);

    glewExperimental = true;
    if (glewInit () != GLEW_OK)
    	throw std::runtime_error ("Cannot initialize GLEW.");
    glGetError ();

	gpualg = algo->second ();
}

void cleanup (void)
{
	if (gpualg != NULL)
		 delete gpualg;
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
		initialize (argc, argv);
		gpualg->Run ();
		cleanup ();
		return 0;
	} catch (std::exception &e) {
		cleanup ();
		std::cerr << "Exception: " << e.what () << std::endl;
		return -1;
	}
}
