#pragma once

#include <string>
#include <map>
#include <unordered_map>

struct shader_t {
    uint32 ID; 
};

std::map<std::string, shader_t*> c_Shaders;
std::unordered_map<std::string, uint32> c_UniformLocations;
