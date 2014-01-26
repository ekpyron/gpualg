find_package (glfw REQUIRED)
find_package (GLM REQUIRED)
find_package (OpenGL REQUIRED)
find_package (GLEW REQUIRED)

include_directories (${GLFW_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${GLM_INCLUDE_DIR})

file (GLOB GPUALG_SOURCES *.cpp)

add_executable (gpualg ${GPUALG_SOURCES})

target_link_libraries (gpualg ${GLFW_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY})

set_property (TARGET gpualg PROPERTY COMPILE_FLAGS -std=c++11)
