#pragma once

#include <windows.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_glfw.h"
#include "IMGUI/imgui_impl_opengl3.h"

#include "../../dep/src/stb_image.cpp"
#include "../../dep/src/glad.c"
#include "../../dep/src/IconsFontAwesome5.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "intrinsic.h"
#include "input.cpp"
#include "camera.cpp"
#include "shader.cpp"
// #include "texture.cpp"
#include "mesh.cpp"
#include "entity.cpp"
#include "ray.cpp"
#include "renderer.cpp"
#include "window.cpp"