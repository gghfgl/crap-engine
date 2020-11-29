#pragma once

#include <iostream>

//#include <windows.h>

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

#include "Engine/internal.h"
#include "Engine/camera.cpp"
#include "Engine/shader.cpp"
// #include "Engine/texture.cpp"
#include "Engine/mesh.cpp"
#include "Engine/entity.cpp"
#include "Engine/ray.cpp"
#include "Engine/renderer.cpp"

#include "Plateform/plateform.cpp"
