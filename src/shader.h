#pragma once

#include <string>
#include <unordered_map>

std::unordered_map<std::string, unsigned int> globalUniformLocationCache;

struct shader {
    unsigned int ID; 
};
