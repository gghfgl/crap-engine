#pragma once

#include <map>
#include <string>

#include <glad/glad.h>

#include "texture.h"
#include "shader.h"

struct resource_manager {
    static std::map<std::string, shader*>    SHADERS_STORAGE;
    static std::map<std::string, Texture2D*> TEXTURES_STORAGE;
};
