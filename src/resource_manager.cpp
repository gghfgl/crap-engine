#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "stb_image.h"

// Instantiate static variables
// std::map<std::string, Texture2D*>    resource_manager::Textures;
std::map<std::string, shader*> resource_manager::ShadersStorage;

shader* get_shader(const std::string& name) {
    return resource_manager::ShadersStorage[name];
}

void clear_resources() {
    // (Properly) delete all shaders
    for (auto iter : resource_manager::ShadersStorage)
    {
        glDeleteProgram(iter.second->ID);
        delete iter.second;
    }

    // // (Properly) delete all textures
    // for (auto iter : TexturesStorage)
    // {
    //     glDeleteTextures(1, &iter.second->ID);
    //     delete iter.second;
    // }
}

shader* load_shader_from_file(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try
    {
        // Open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // If geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr)
        {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception e)
    {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    // 2. Now create shader object from source code
    shader *Shader = new shader();
    compile_shader(Shader, vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    return Shader;
}

shader* load_shader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, const std::string& name) {
    resource_manager::ShadersStorage[name] = load_shader_from_file(vShaderFile, fShaderFile, gShaderFile);
    return resource_manager::ShadersStorage[name];
}

// Texture2D* ResourceManager::LoadTexture(const char* file, GLboolean alpha, const std::string& name) {
//     Textures[name] = loadTextureFromFile(file, alpha);
//     return Textures[name];
// }

// Texture2D* ResourceManager::GetTexture(const std::string& name) {
//     return Textures[name];
// }

// Texture2D* ResourceManager::loadTextureFromFile(const char* file, GLboolean alpha)
// {
//     // Create Texture object
//     Texture2D * texture = new Texture2D();
//     if (alpha)
//     {
//         texture->Internal_Format = GL_RGBA;
//         texture->Image_Format    = GL_RGBA;
//     }
//     // Load image
//     int width, height;
//     unsigned char* image = stbi_load(file, &width, &height, nullptr, texture->Image_Format == GL_RGBA ? STBI_rgb_alpha : STBI_rgb);
//     // Now generate texture
//     texture->Generate(width, height, image);
//     // And finally free image data
//     stbi_image_free(image);
//     return texture;
// }
