#pragma once

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// TODO: kind of meh ...
#include "../../dep/src/stb_image.cpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "internal.h"
#include "plateform.cpp"

#include "camera.cpp"
#include "shader.cpp"
#include "entity.cpp"
#include "ray.cpp"
#include "renderer.cpp"
