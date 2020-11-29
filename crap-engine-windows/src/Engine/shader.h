#pragma once

#include <string>
#include <map>
#include <unordered_map>

enum shader_type
{
    NONE = -1,
    VERTEX = 0,
    FRAGMENT = 1,
    GEOMETRY = 2
};

struct shader_t {
    uint32 ID; 
};

std::map<std::string, shader_t*> c_Shaders;
std::unordered_map<uint32, std::unordered_map<std::string, uint32>> c_UniformLocations;
