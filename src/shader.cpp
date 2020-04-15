#include <sstream>
#include <fstream>

#include "shader.h"

static void check_compile_shader_errors(uint32 object, std::string type);
static void compile_shader(shader_t *Shader, const char* vertexSource, const char* fragmentSource, const char* geometrySource);
static shader_t* load_shader_from_file(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, const std::string& name);
static uint32 get_uniform_location_cache(shader_t *Shader, const char* name);

// TODO: implement memory pool
namespace shader
{
    void UseProgram(shader_t *Shader)
    {
	glUseProgram(Shader->ID);
    }

    void SetUniform1f(shader_t *Shader, const char* name, float32 value)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform1f(id, value);
    }

    void SetUniform1i(shader_t *Shader, const char* name, int32 value)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform1i(id, value);
    }

    void SetUniform1iv(shader_t *Shader, const char* name, int32 *value)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform1iv(id, (GLsizei)(sizeof(value)/sizeof(value[0])), value);
    }

    void SetUniform2f(shader_t *Shader, const char* name, float32 x, float32 y)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform2f(id, x, y);
    }

    void SetUniform2f(shader_t *Shader, const char* name, const glm::vec2 &value)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform2f(id, value.x, value.y);
    }

    void SetUniform3f(shader_t *Shader, const char* name, float32 x, float32 y, float32 z)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform3f(id, x, y, z);
    }

    void SetUniform3f(shader_t *Shader, const char* name, const glm::vec3 &value)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform3f(id, value.x, value.y, value.z);
    }

    void SetUniform4f(shader_t *Shader, const char* name, float32 x, float32 y, float32 z, float32 w)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform4f(id, x, y, z, w);
    }

    void ShaderSetUniform4f(shader_t *Shader, const char* name, const glm::vec4 &value)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform4f(id, value.x, value.y, value.z, value.w);
    }

    void SetUniform4fv(shader_t *Shader, const char* name, const glm::mat4 &matrix)
    {
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(matrix));
    }
    void CompileAndCache(const char* vShaderFile,
			       const char* fShaderFile,
			       const char* gShaderFile,
			       const std::string& name,
			       glm::mat4 projectionMatrix)
    {
	shader_t *Shader = load_shader_from_file(vShaderFile, fShaderFile, gShaderFile, name);

	c_Shaders[name] = Shader; // TODO: make cache static func with memory pool update
	
	UseProgram(Shader);
	SetUniform4fv(Shader, "projection", projectionMatrix);
    }

    shader_t* GetFromCache(const std::string& name)
    {
	return c_Shaders[name];
    }

    // clearing cache
    void ClearCache()
    {
	for (auto iter : c_Shaders)
	{
	    glDeleteProgram(iter.second->ID);
	    delete iter.second;
	}

	c_UniformLocations.clear();
    }
}

static uint32 get_uniform_location_cache(shader_t *Shader, const char* name)
{
    if (c_UniformLocations.find(name) != c_UniformLocations.end())
	return c_UniformLocations[name];

    uint32 location = glGetUniformLocation(Shader->ID, name);
    c_UniformLocations[name] = location;

    return location;
}

static shader_t* load_shader_from_file(const char* vShaderFile,
			      const char* fShaderFile,
			      const char* gShaderFile,
			      const std::string& name)
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
	// If geometry shader_t path is present, also load a geometry shader_t
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
	std::cout << "ERROR::SHADER: Failed to read shader_t files" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    // 2. Now create shader_t object from source code
    shader_t *Shader = new shader_t();
    compile_shader(Shader, vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);

    return Shader;
}

static void compile_shader(shader_t *Shader,
		   const char* vertexSource,
		   const char* fragmentSource,
		   const char* geometrySource)
{
    uint32 sVertex, sFragment, gShader;
    // Vertex Shader
    sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vertexSource, NULL);
    glCompileShader(sVertex);
    check_compile_shader_errors(sVertex, "VERTEX");

    // Fragment Shader
    sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fragmentSource, NULL);
    glCompileShader(sFragment);
    check_compile_shader_errors(sFragment, "FRAGMENT");

    // If geometry shader_t source code is given, also compile geometry shader_t
    if (geometrySource != nullptr)
   
    {
        gShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gShader, 1, &geometrySource, NULL);
        glCompileShader(gShader);
        check_compile_shader_errors(gShader, "GEOMETRY");
    }

    // Shader Program
    Shader->ID = glCreateProgram();
    glAttachShader(Shader->ID, sVertex);
    glAttachShader(Shader->ID, sFragment);
    gShader = 0;
    if (geometrySource != nullptr)
        glAttachShader(Shader->ID, gShader);
    glLinkProgram(Shader->ID);
    check_compile_shader_errors(Shader->ID, "PROGRAM");

    // Delete the shader_ts as they're linked into our program now and no longer necessery
    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    if (geometrySource != nullptr)
        glDeleteShader(gShader);
}

static void check_compile_shader_errors(uint32 object, std::string type)
{
    int32 success;
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
