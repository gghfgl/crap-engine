#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h" // TODO remove
#include "renderer.h"

// DEBUG: OPENGL_DEBUG ===================================================================
void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError())
    {
	std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
	return false;
    }

    return true;
}
// DEBUG: OPENGL_DEBUG ===================================================================

unsigned int texture1, texture2;
// world space positions of our cubes
glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f, -10.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

void init_renderer_data(renderer *Renderer) {
    // // TODO: hardcoded
    // float vertices[] = {
    //     -0.5f, -0.5f, -0.5f, 
    // 	0.5f, -0.5f, -0.5f,  
    // 	0.5f,  0.5f, -0.5f,  
    // 	0.5f,  0.5f, -0.5f,  
    //     -0.5f,  0.5f, -0.5f, 
    //     -0.5f, -0.5f, -0.5f, 
    // };


    
    // // Configure VAO/VBO
    // unsigned int VBO;
    // glGenVertexArrays(1, &this->cubeVAO);
    // glGenBuffers(1, &VBO);

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindVertexArray(this->cubeVAO);
    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);

    // TODO: implement IBO!
    
    float vertices[] = {
        -10.5f, -0.0f, -10.5f,  0.0f, 0.0f,
         10.5f, -0.0f, -10.5f,  1.0f, 0.0f,
         10.5f,  0.0f, -10.5f,  1.0f, 1.0f,
         10.5f,  0.0f, -10.5f,  1.0f, 1.0f,
        -10.5f,  0.0f, -10.5f,  0.0f, 1.0f,
        -10.5f, -0.0f, -10.5f,  0.0f, 0.0f,

        -10.5f, -0.0f,  10.5f,  0.0f, 0.0f,
         10.5f, -0.0f,  10.5f,  1.0f, 0.0f,
         10.5f,  0.0f,  10.5f,  1.0f, 1.0f,
         10.5f,  0.0f,  10.5f,  1.0f, 1.0f,
        -10.5f,  0.0f,  10.5f,  0.0f, 1.0f,
        -10.5f, -0.0f,  10.5f,  0.0f, 0.0f,

        -10.5f,  0.0f,  10.5f,  1.0f, 0.0f,
        -10.5f,  0.0f, -10.5f,  1.0f, 1.0f,
        -10.5f, -0.0f, -10.5f,  0.0f, 1.0f,
        -10.5f, -0.0f, -10.5f,  0.0f, 1.0f,
        -10.5f, -0.0f,  10.5f,  0.0f, 0.0f,
        -10.5f,  0.0f,  10.5f,  1.0f, 0.0f,

         10.5f,  0.0f,  10.5f,  1.0f, 0.0f,
         10.5f,  0.0f, -10.5f,  1.0f, 1.0f,
         10.5f, -0.0f, -10.5f,  0.0f, 1.0f,
         10.5f, -0.0f, -10.5f,  0.0f, 1.0f,
         10.5f, -0.0f,  10.5f,  0.0f, 0.0f,
         10.5f,  0.0f,  10.5f,  1.0f, 0.0f,

        -10.5f, -0.0f, -10.5f,  0.0f, 1.0f,
         10.5f, -0.0f, -10.5f,  1.0f, 1.0f,
         10.5f, -0.0f,  10.5f,  1.0f, 0.0f,
         10.5f, -0.0f,  10.5f,  1.0f, 0.0f,
        -10.5f, -0.0f,  10.5f,  0.0f, 0.0f,
        -10.5f, -0.0f, -10.5f,  0.0f, 1.0f,

        -10.5f,  0.0f, -10.5f,  0.0f, 1.0f,
         10.5f,  0.0f, -10.5f,  1.0f, 1.0f,
         10.5f,  0.0f,  10.5f,  1.0f, 0.0f,
         10.5f,  0.0f,  10.5f,  1.0f, 0.0f,
        -10.5f,  0.0f,  10.5f,  0.0f, 0.0f,
        -10.5f,  0.0f, -10.5f,  0.0f, 1.0f
    };
    
    unsigned int VBO;
    GLCall(glGenVertexArrays(1, &Renderer->cubeVAO));
    GLCall(glGenBuffers(1, &VBO));

    GLCall(glBindVertexArray(Renderer->cubeVAO));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // position attribute
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
    GLCall(glEnableVertexAttribArray(0));
    // texture coord attribute
    GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
    GLCall(glEnableVertexAttribArray(1));


    // load and create a texture 
    // -------------------------
    // texture 1
    // ---------
    GLCall(glGenTextures(1, &texture1));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture1));
    // set the texture wrapping parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    // set texture filtering parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("../assets/container.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
    // ---------
    GLCall(glGenTextures(1, &texture2));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture2));
    // set the texture wrapping parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    // set texture filtering parameters
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    // load image, create texture and generate mipmaps
    data = stbi_load("../assets/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    Renderer->shader->Use();
    Renderer->shader->SetInteger("texture1", 0);
    Renderer->shader->SetInteger("texture2", 1);    
}

renderer* renderer_construct(Shader *shader) {
    renderer* Result = new renderer();
    Result->shader = shader;
    init_renderer_data(Result);

    return Result;
}

void delete_renderer(renderer *Renderer) {
    glDeleteVertexArrays(1, &Renderer->cubeVAO);
    delete Renderer;
}

void renderer_draw(renderer *Renderer, camera *Camera, glm::vec3 position, glm::vec3 color) {

    // // world transformation
    // this->shader->Use();
    // glm::mat4 view = camera.GetViewMatrix();
    // this->shader->SetMatrix4("view", view);

    // glm::mat4 model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(position));
    // // TODO: test rotation hardcoded
    // // float angle = 35.0f;
    // // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    // // model = glm::scale(model, glm::vec3(size, 1.0f)); // Last scale
    // this->shader->SetMatrix4("model", model);
    // // Color
    // this->shader->SetVector3f("objectColor", color);
    //  //glActiveTexture(GL_TEXTURE0);
    // //texture->Bind();    
    // glBindVertexArray(this->cubeVAO);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    // //glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    Renderer->shader->Use();
    glm::mat4 view = get_camera_view_matrix(Camera);
    Renderer->shader->SetMatrix4("view", view);

    glBindVertexArray(Renderer->cubeVAO);
    for (unsigned int i = 0; i < 1; i++){
	// calculate the model matrix for each object and pass it to shader before drawing
	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	model = glm::translate(model, cubePositions[i]);
	float angle = 20.0f * i;
	model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        Renderer->shader->SetMatrix4("model", model);

	glDrawArrays(GL_TRIANGLES, 0, 36);
    }

}
