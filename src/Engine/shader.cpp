#include <sstream>
#include <fstream>

#include "shader.h"

Shader::~Shader()
{
    this->UniformLocations.clear();
}

void Shader::useProgram()
{
    glUseProgram(this->ID);
}

void Shader::setUniform1f(const char* name, float32 value)
{
    uint32 id = this->get_uniform_location(name);
    glUniform1f(id, value);
}

void Shader::setUniform1i(const char* name, int32 value)
{
    uint32 id = this->get_uniform_location(name);
    glUniform1i(id, value);
}

// void Shader::setUniform1iv(const char* name, int32 *value)
// {
// 	uint32 id = this->get_uniform_location(name);
// 	glUniform1iv(id, (GLsizei)(sizeof(value)/sizeof(value[0])), value);
// }

void Shader::setUniform1ui(const char* name, uint32 value)
{
    uint32 id = this->get_uniform_location(name);
    glUniform1ui(id, value);
}

void Shader::setUniform2f(const char* name, float32 x, float32 y)
{
    uint32 id = this->get_uniform_location(name);
    glUniform2f(id, x, y);
}

void Shader::setUniform2f(const char* name, const glm::vec2 &value)
{
    uint32 id = this->get_uniform_location(name);
    glUniform2f(id, value.x, value.y);
}

void Shader::setUniform3f(const char* name, float32 x, float32 y, float32 z)
{
    uint32 id = this->get_uniform_location(name);
    glUniform3f(id, x, y, z);
}

void Shader::setUniform3f(const char* name, const glm::vec3 &value)
{
    uint32 id = this->get_uniform_location(name);
    glUniform3f(id, value.x, value.y, value.z);
}

void Shader::setUniform4f(const char* name, float32 x, float32 y, float32 z, float32 w)
{
    uint32 id = this->get_uniform_location(name);
    glUniform4f(id, x, y, z, w);
}

void Shader::setUniform4f(const char* name, const glm::vec4 &value)
{
    uint32 id = this->get_uniform_location(name);
    glUniform4f(id, value.x, value.y, value.z, value.w);
}

void Shader::setUniform4fv(const char* name, const glm::mat4 &matrix)
{
    uint32 id = this->get_uniform_location(name);
    glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::compileSources(const char* vertexSource,
                             const char* fragmentSource,
                             const char* geometrySource)
{
    //uint32 sVertex, sFragment, sGeometry;
    uint32 sVertex, sFragment;
    sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vertexSource, NULL);
    glCompileShader(sVertex);
    this->check_compile_errors(sVertex, "VERTEX");

    sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fragmentSource, NULL);
    glCompileShader(sFragment);

    this->check_compile_errors(sFragment, "FRAGMENT");

    // if (geometrySource != nullptr)
    // {
    //     sGeometry = glCreateShader(GL_GEOMETRY_SHADER);
    //     glShaderSource(sGeometry, 1, &geometrySource, NULL);
    //     glCompileShader(sGeometry);
    //     this->check_compile_errors(sGeometry, "GEOMETRY");
    // }

    this->ID = glCreateProgram();
    glAttachShader(this->ID, sVertex);
    glAttachShader(this->ID, sFragment);
    //sGeometry = 0;
    // if (geometrySource != nullptr)
    //     glAttachShader(this->ID, sGeometry);
    glLinkProgram(this->ID);

    this->check_compile_errors(this->ID, "PROGRAM");

    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    // if (geometrySource != nullptr)
    //     glDeleteShader(sGeometry);
}

uint32 Shader::get_uniform_location(const char* name)
{
    if (this->UniformLocations.find(name) != this->UniformLocations.end())
        return this->UniformLocations[name];

    uint32 location = glGetUniformLocation(this->ID, name);
    this->UniformLocations[name] = location;

    return location;
}

void Shader::check_compile_errors(uint32 object, std::string type)
{
    int32 success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            printf("| ERROR::SHADER: Compile-time error: Type: %s\n %s\n -- --------------------------------------------------- -- \n",
                   type.c_str(),
                   infoLog);
        }
    } else
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            printf("| ERROR::SHADER: Link-time error: Type: %s\n %s\n -- --------------------------------------------------- -- \n",
                   type.c_str(),
                   infoLog);
        }
    }
}

// =================================

void ShaderCache::compileAndAddShader(const char* filepath, const std::string& name, glm::mat4 projectionMatrix)
{
    Shader *shader = this->load_shader_from_file(filepath);

    this->Shaders[name] = shader;
	
    shader->useProgram();
    shader->setUniform4fv("projection", projectionMatrix);
}

ShaderCache::~ShaderCache()
{
    for (auto iter : this->Shaders)
    {
        glDeleteProgram(iter.second->ID);
        delete iter.second;
        iter.second = NULL;
    }
}

Shader* ShaderCache::getShader(const std::string& name)
{
    return this->Shaders[name];
}

Shader* ShaderCache::load_shader_from_file(const char* filepath)
{
    std::ifstream stream(filepath);
    std::string line;
    std::stringstream ss[3];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if(line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if(line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
            else if(line.find("geometry") != std::string::npos)
                type = ShaderType::GEOMETRY;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    Shader *shader = new Shader;
    shader->compileSources((const char*)ss[(int)ShaderType::VERTEX].str().c_str(),
                            (const char*)ss[(int)ShaderType::FRAGMENT].str().c_str(),
                            (const char*)ss[(int)ShaderType::GEOMETRY].str().c_str());

    return shader;
}

// static shader_t* load_shader_from_file(const char* vShaderFile,
// 			      const char* fShaderFile,
// 			      const char* gShaderFile,
// 			      const std::string& name)
// {
//     // 1. Retrieve the vertex/fragment source code from filePath
//     std::string vertexCode;
//     std::string fragmentCode;
//     std::string geometryCode;
//     try
   
//     {
// 	// Open files
// 	std::ifstream vertexShaderFile(vShaderFile);
// 	std::ifstream fragmentShaderFile(fShaderFile);
// 	std::stringstream vShaderStream, fShaderStream;
// 	// Read file's buffer contents into streams
// 	vShaderStream << vertexShaderFile.rdbuf();
// 	fShaderStream << fragmentShaderFile.rdbuf();
// 	// close file handlers
// 	vertexShaderFile.close();
// 	fragmentShaderFile.close();
// 	// Convert stream into string
// 	vertexCode = vShaderStream.str();
// 	fragmentCode = fShaderStream.str();
// 	// If geometry shader_t path is present, also load a geometry shader_t
// 	if (gShaderFile != nullptr)
       
// 	{
// 	    std::ifstream geometryShaderFile(gShaderFile);
// 	    std::stringstream gShaderStream;
// 	    gShaderStream << geometryShaderFile.rdbuf();
// 	    geometryShaderFile.close();
// 	    geometryCode = gShaderStream.str();
// 	}
//     }
//     catch (std::exception e)
//     {
//         printf("ERROR::SHADER: Failed to read shader_t files\n");
//     }
//     const char* vShaderCode = vertexCode.c_str();
//     const char* fShaderCode = fragmentCode.c_str();
//     const char* gShaderCode = geometryCode.c_str();
//     // 2. Now create shader_t object from source code
//     shader_t *Shader = new shader_t;
//     compile_shader(Shader, vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);

//     return Shader;
// }
