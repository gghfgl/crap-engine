#include "shader.h"

#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void check_compile_errors(unsigned int object, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
		      << infoLog << "\n -- --------------------------------------------------- -- "
		      << std::endl;
        }
    } else {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
		      << infoLog << "\n -- --------------------------------------------------- -- "
		      << std::endl;
        }
    }
}

void use_shader(shader *Shader) {
    GLCall(glUseProgram(Shader->ID));
}

void compile_shader(shader *Shader, const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
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

unsigned int get_uniform_location_cache(shader *Shader, const char* name) {
    if (_UNIFORM_LOCATION_CACHE.find(name) != _UNIFORM_LOCATION_CACHE.end())
	return _UNIFORM_LOCATION_CACHE[name];

    unsigned int location = glGetUniformLocation(Shader->ID, name);
    _UNIFORM_LOCATION_CACHE[name] = location;

    return location;
}

void shader_set_uniform1f(shader *Shader, const char* name, float value) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform1f(id, value);
}

void shader_set_uniform1i(shader *Shader, const char* name, int value) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform1i(id, value);
}

void shader_set_uniform2f(shader *Shader, const char* name, float x, float y) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform2f(id, x, y);
}

void shader_set_uniform2f(shader *Shader, const char* name, const glm::vec2 &value) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform2f(id, value.x, value.y);
}

void shader_set_uniform3f(shader *Shader, const char* name, float x, float y, float z) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform3f(id, x, y, z);
}

void shader_set_uniform3f(shader *Shader, const char* name, const glm::vec3 &value) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform3f(id, value.x, value.y, value.z);
}

void shader_set_uniform4f(shader *Shader, const char* name, float x, float y, float z, float w) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform4f(id, x, y, z, w);
}

void shader_set_uniform4f(shader *Shader, const char* name, const glm::vec4 &value) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniform4f(id, value.x, value.y, value.z, value.w);
}

void shader_set_uniform4fv(shader *Shader, const char* name, const glm::mat4 &matrix) {
    unsigned int id = get_uniform_location_cache(Shader, name);
    glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(matrix));
}
