#pragma once

#include <iostream>
#include <filesystem>
#include <stdarg.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
// TODO: @animation
//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

// TODO: kind of meh ...
#include "../../dep/src/glad.c"
#include "../../dep/src/stb_image.cpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
// TODO: @animation
//#include <assimp/quaternion.h>
//#include <assimp/vector3.h>
//#include <assimp/matrix4x4.h>

#include "internal.h"
#include "plateform.cpp"

#include "camera.cpp"
#include "shader.cpp"
#include "model.cpp"
#include "entity.cpp"
#include "ray.cpp"
#include "renderer.cpp"
