find_package (glfw REQUIRED)
find_package (GLM REQUIRED)
find_package (OpenGL REQUIRED)
find_package (OGLP REQUIRED)

include_directories (${GLFW_INCLUDE_DIR} ${OGLP_INCLUDE_DIR} ${GLM_INCLUDE_DIR})

file (GLOB PREFIXSUM_SOURCES *.cpp)

add_executable (prefixsum ${PREFIXSUM_SOURCES})

target_link_libraries (prefixsum ${GLFW_LIBRARY} ${OPENGL_LIBRARIES})

set_property (TARGET prefixsum PROPERTY COMPILE_FLAGS -std=c++11)
