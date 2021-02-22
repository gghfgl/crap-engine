#pragma once

#include <string>
#include <map>
#include <unordered_map>


enum ShaderType
{
    NONE     = -1,
    VERTEX   =  0,
    FRAGMENT =  1,
    GEOMETRY =  2
};

struct Shader {
    ~Shader();
    void UseProgram();
    void CompileSources(const char* vertexSource,
                         const char* fragmentSource,
                         const char* geometrySource);

    void SetUniform1f(const char* name, float32 value);
    void SetUniform1i(const char* name, int32 value);
    void SetUniform1ui(const char* name, uint32 value);
    void SetUniform2f(const char* name, float32 x, float32 y);
    void SetUniform2f(const char* name, const glm::vec2 &value);
    void SetUniform3f(const char* name, float32 x, float32 y, float32 z);
    void SetUniform3f(const char* name, const glm::vec3 &value);
    void SetUniform4f(const char* name, float32 x, float32 y, float32 z, float32 w);
    void SetUniform4f(const char* name, const glm::vec4 &value);
    void SetUniform4fv(const char* name, const glm::mat4 &matrix);
    
    uint32 ID;
    std::unordered_map<std::string, uint32> UniformLocations;

private:
    void check_compile_errors(uint32 object, std::string type);
    uint32 get_uniform_location(const char* name);
};

struct ShaderCache
{
    ~ShaderCache();
    int32 CompileShadersFromDirectory(const char* directory, glm::mat4 projectionMatrix);
    Shader* GetShader(const std::string& name);
    
    std::map<std::string, Shader*> Shaders;

private:
    Shader* load_shader_from_file(const char* filepath);
};
