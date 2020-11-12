#pragma once

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../../dep/src/precompile.h"

#include "internal.h"
#include "plateform.cpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// TODO: move to dedicated dependency file
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "camera.cpp"
#include "shader.cpp"
// #include "texture.cpp"
#include "mesh.cpp"
#include "entity.cpp"
#include "ray.cpp"
#include "renderer.cpp"
