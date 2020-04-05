#pragma once

#include <map>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, unsigned int> globalUniformLocationCache;

struct shader {
    unsigned int ID; 
};

std::map<std::string, shader*> _SHADERS_STORAGE;
