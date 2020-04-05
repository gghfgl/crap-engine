#include <sstream>
#include <fstream>

#include "shader.h"

void check_compile_errors(unsigned int object, std::string type);
void compile_shader(shader *Shader, const char* vertexSource, const char* fragmentSource, const char* geometrySource);
shader* load_shader_from_file(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, const std::string& name, std::map<std::string, shader*> &ShaderStorage);
unsigned int get_uniform_location_cache(shader *Shader, const char* name);

shader* ShaderGetFromStorage(const std::string& name)
{
    return _SHADERS_STORAGE[name];
}

void ShaderUseProgram(shader *Shader)
{
    glUseProgram(Shader->ID);
}

void ShaderDeleteStorage()
{
    for (auto iter : _SHADERS_STORAGE)   
    {
        glDeleteProgram(iter.second->ID);
        delete iter.second;
    }
}

void ShaderSetUniform1f(shader *Shader, const char* name, float value)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform1f(id, value);
}

void ShaderSetUniform1i(shader *Shader, const char* name, int value)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform1i(id, value);
}

void ShaderSetUniform2f(shader *Shader, const char* name, float x, float y)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform2f(id, x, y);
}

void ShaderSetUniform2f(shader *Shader, const char* name, const glm::vec2 &value)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform2f(id, value.x, value.y);
}

void ShaderSetUniform3f(shader *Shader, const char* name, float x, float y, float z)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform3f(id, x, y, z);
}

void ShaderSetUniform3f(shader *Shader, const char* name, const glm::vec3 &value)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform3f(id, value.x, value.y, value.z);
}

void ShaderSetUniform4f(shader *Shader, const char* name, float x, float y, float z, float w)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform4f(id, x, y, z, w);
}

void ShaderSetUniform4f(shader *Shader, const char* name, const glm::vec4 &value)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform4f(id, value.x, value.y, value.z, value.w);
}

void ShaderSetUniform4fv(shader *Shader, const char* name, const glm::mat4 &matrix)
{
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(matrix));
}

// TODO: hardcoded
void CrapShortcutConstructCompileShaders(glm::mat4 projectionMatrix)
{
    shader *defaultShader = load_shader_from_file("../shaders/default.vs",
					  "../shaders/default.fs",
					  nullptr,
					  "default",
					  _SHADERS_STORAGE);
    shader *stencilShader = load_shader_from_file("../shaders/default.vs",
					  "../shaders/stencil.fs",
					  nullptr,
					  "stencil",
					  _SHADERS_STORAGE);

    ShaderUseProgram(stencilShader);
    ShaderSetUniform4fv(stencilShader, "projection", projectionMatrix);
    ShaderUseProgram(defaultShader);
    ShaderSetUniform4fv(defaultShader, "projection", projectionMatrix);
}

unsigned int get_uniform_location_cache(shader *Shader, const char* name)
{
    if (globalUniformLocationCache.find(name) != globalUniformLocationCache.end())
	return globalUniformLocationCache[name];

    unsigned int location = glGetUniformLocation(Shader->ID, name);
    globalUniformLocationCache[name] = location;

    return location;
}

shader* load_shader_from_file(const char* vShaderFile,
			      const char* fShaderFile,
			      const char* gShaderFile,
			      const std::string& name,
			      std::map<std::string, shader*> &ShaderStorage)
{
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
    ShaderStorage[name] = Shader;

    return ShaderStorage[name];
}

void compile_shader(shader *Shader,
		   const char* vertexSource,
		   const char* fragmentSource,
		   const char* geometrySource)
{
    unsigned int sVertex, sFragment, gShader;
    // Vertex Shader
    sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vertexSource, NULL);
    glCompileShader(sVertex);
    check_compile_errors(sVertex, "VERTEX");

    // Fragment Shader
    sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fragmentSource, NULL);
    glCompileShader(sFragment);
    check_compile_errors(sFragment, "FRAGMENT");

    // If geometry shader source code is given, also compile geometry shader
    if (geometrySource != nullptr)
   
    {
        gShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gShader, 1, &geometrySource, NULL);
        glCompileShader(gShader);
        check_compile_errors(gShader, "GEOMETRY");
    }

    // Shader Program
    Shader->ID = glCreateProgram();
    glAttachShader(Shader->ID, sVertex);
    glAttachShader(Shader->ID, sFragment);
    gShader = 0;
    if (geometrySource != nullptr)
        glAttachShader(Shader->ID, gShader);
    glLinkProgram(Shader->ID);
    check_compile_errors(Shader->ID, "PROGRAM");

    // Delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    if (geometrySource != nullptr)
        glDeleteShader(gShader);
}

void check_compile_errors(unsigned int object, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
	{
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
		      << infoLog << "\n -- --------------------------------------------------- -- "
		      << std::endl;
        }
    } else
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
	{
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
		      << infoLog << "\n -- --------------------------------------------------- -- "
		      << std::endl;
        }
    }
}
