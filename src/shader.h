#pragma once

#include <map>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, uint32> globalUniformLocationCache;

struct shader {
    uint32 ID; 
};

std::map<std::string, shader*> _SHADERS_STORAGE;
