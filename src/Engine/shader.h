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
    void useProgram();
    void compileSources(const char* vertexSource,
                         const char* fragmentSource,
                         const char* geometrySource);

    void setUniform1f(const char* name, float32 value);
    void setUniform1i(const char* name, int32 value);
    void setUniform1ui(const char* name, uint32 value);
    void setUniform2f(const char* name, float32 x, float32 y);
    void setUniform2f(const char* name, const glm::vec2 &value);
    void setUniform3f(const char* name, float32 x, float32 y, float32 z);
    void setUniform3f(const char* name, const glm::vec3 &value);
    void setUniform4f(const char* name, float32 x, float32 y, float32 z, float32 w);
    void setUniform4f(const char* name, const glm::vec4 &value);
    void setUniform4fv(const char* name, const glm::mat4 &matrix);
    
    uint32 ID;
    std::unordered_map<std::string, uint32> UniformLocations;

private:
    void check_compile_errors(uint32 object, std::string type);
    uint32 get_uniform_location(const char* name);
};

struct ShaderCache
{
    ~ShaderCache();
    void compileAndAddShader(const char* filepath, const std::string& name, glm::mat4 projectionMatrix);
    Shader* getShader(const std::string& name);
    
    std::map<std::string, Shader*> Shaders;

private:
    Shader* load_shader_from_file(const char* filepath);
};
