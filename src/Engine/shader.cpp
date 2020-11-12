#include <sstream>
#include <fstream>

#include "shader.h"

static void check_compile_shader_errors(uint32 object, std::string type);
static void compile_shader(shader_t *Shader, const char* vertexSource, const char* fragmentSource, const char* geometrySource);
//static shader_t* load_shader_from_file(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, const std::string& name);
static shader_t* load_shader_from_file(const char* shaderFile);
static uint32 get_uniform_location_cache(shader_t *Shader, const char* name);

// TODO: implement memory pool
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

// void SetUniform1iv(shader_t *Shader, const char* name, int32 *value)
// {
// 	uint32 id = get_uniform_location_cache(Shader, name);
// 	glUniform1iv(id, (GLsizei)(sizeof(value)/sizeof(value[0])), value);
// }

void SetUniform1ui(shader_t *Shader, const char* name, uint32 value)
{
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform1ui(id, value);
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

void SetUniform4f(shader_t *Shader, const char* name, const glm::vec4 &value)
{
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniform4f(id, value.x, value.y, value.z, value.w);
}

void SetUniform4fv(shader_t *Shader, const char* name, const glm::mat4 &matrix)
{
	uint32 id = get_uniform_location_cache(Shader, name);
	glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(matrix));
}

// void CompileAndCache(const char* vShaderFile,
// 			       const char* fShaderFile,
// 			       const char* gShaderFile,
// 			       const std::string& name,
// 			       glm::mat4 projectionMatrix)
// {
// 	shader_t *Shader = load_shader_from_file(vShaderFile, fShaderFile, gShaderFile, name);

// 	c_Shaders[name] = Shader; // TODO: make cache static func with memory pool update
	
// 	UseProgram(Shader);
// 	SetUniform4fv(Shader, "projection", projectionMatrix);
// }

void CompileAndCacheShader(const char* shaderFile, const std::string& name, glm::mat4 projectionMatrix)
{
	shader_t *Shader = load_shader_from_file(shaderFile);

	c_Shaders[name] = Shader; // TODO: make cache static func with memory pool update
	
	UseProgram(Shader);
    SetUniform4fv(Shader, "projection", projectionMatrix);
}

shader_t* GetShaderFromCache(const std::string& name)
{
	return c_Shaders[name];
}

// clearing cache
void ClearShaderCache()
{
	for (auto iter : c_Shaders)
	{
	    glDeleteProgram(iter.second->ID);
	    free(iter.second);
	    iter.second = NULL;
	}

	c_UniformLocations.clear();
}

static uint32 get_uniform_location_cache(shader_t *Shader, const char* name)
{
    if (c_UniformLocations.find(Shader->ID) != c_UniformLocations.end()
	&& c_UniformLocations[Shader->ID].find(name) != c_UniformLocations[Shader->ID].end())
	    return c_UniformLocations[Shader->ID][name];

    uint32 location = glGetUniformLocation(Shader->ID, name);
    c_UniformLocations[Shader->ID][name] = location;

    return location;
}

//static shader_t* load_shader_from_file(const char* shaderFile, const std::string& name)
static shader_t* load_shader_from_file(const char* shaderFile)
{
    std::ifstream stream(shaderFile);
    std::string line;
    std::stringstream ss[3];
    shader_type type = shader_type::NONE;

    while (getline(stream, line))
    {
	if (line.find("#shader") != std::string::npos)
	{
	    if(line.find("vertex") != std::string::npos)
		type = shader_type::VERTEX;
	    else if(line.find("fragment") != std::string::npos)
		type = shader_type::FRAGMENT;
	    else if(line.find("geometry") != std::string::npos)
		type = shader_type::GEOMETRY;
	}
	else
	{
	    ss[(int)type] << line << '\n';
	}
    }

    shader_t *Shader = new shader_t;
    compile_shader(Shader,
		   (const char*)ss[(int)shader_type::VERTEX].str().c_str(),
		   (const char*)ss[(int)shader_type::FRAGMENT].str().c_str(),
		   (const char*)ss[(int)shader_type::GEOMETRY].str().c_str());

    return Shader;
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


// static void compile_shader(shader_t *Shader,
// 		   const char* vertexSource,
// 		   const char* fragmentSource,
// 		   const char* geometrySource)
static void compile_shader(shader_t *Shader,
		   const char* vertexSource,
		   const char* fragmentSource,
		   const char*)
{
    //uint32 sVertex, sFragment, sGeometry;
    uint32 sVertex, sFragment;
    sVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sVertex, 1, &vertexSource, NULL);
    glCompileShader(sVertex);
    check_compile_shader_errors(sVertex, "VERTEX");

    sFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sFragment, 1, &fragmentSource, NULL);
    glCompileShader(sFragment);
    check_compile_shader_errors(sFragment, "FRAGMENT");

    // if (geometrySource != nullptr)
    // {
    //     sGeometry = glCreateShader(GL_GEOMETRY_SHADER);
    //     glShaderSource(sGeometry, 1, &geometrySource, NULL);
    //     glCompileShader(sGeometry);
    //     check_compile_shader_errors(sGeometry, "GEOMETRY");
    // }

    Shader->ID = glCreateProgram();
    glAttachShader(Shader->ID, sVertex);
    glAttachShader(Shader->ID, sFragment);
    //sGeometry = 0;
    // if (geometrySource != nullptr)
    //     glAttachShader(Shader->ID, sGeometry);
    glLinkProgram(Shader->ID);
    check_compile_shader_errors(Shader->ID, "PROGRAM");

    glDeleteShader(sVertex);
    glDeleteShader(sFragment);
    // if (geometrySource != nullptr)
    //     glDeleteShader(sGeometry);
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
