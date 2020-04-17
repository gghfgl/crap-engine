#include "renderer.h"
#include "shader.h"

static void allocate_mesh(mesh_t *Mesh);
static void process_node(model_t *Model, aiNode *node, const aiScene *scene);
static mesh_t* process_mesh(model_t *Model, aiMesh *mesh, const aiScene *scene);
static std::vector<texture_t> load_material_textures(model_t *Model, aiMaterial *mat, aiTextureType type, std::string typeName);
static uint32 load_texture_from_file(const char *path, const std::string &directory);

namespace renderer
{
    renderer_t* Construct()
    {
	// // TODO: think about memory pool after Model loader
	// vertex *CubeBuffer = new vertex[globalMaxVertexCount];
	// memory_arena *Arena = new memory_arena();
	// InitMemoryArena(Arena, sizeof(vertex) * globalMaxVertexCount, (int64*)CubeBuffer);

        renderer_t* Renderer = new renderer_t;
	Renderer->WireframeMode = false;
	return Renderer;
    }

    void Delete(renderer_t *Renderer)
    {
	// ===================== platform code =====================
	// // TODO: move to mesh model etc ...
	// glDeleteVertexArrays(1, &Renderer->VAO->Id);
	// glDeleteBuffers(1, &Renderer->DebugVBO);
	// glDeleteVertexArrays(1, &Renderer->CubeVAO);
	// glDeleteBuffers(1, &Renderer->CubeVBO);
	// glDeleteBuffers(1, &Renderer->CubeIBO);
	// ========================================================

	// TODO: delete MemPool
        delete Renderer;
    }

    void NewRenderingContext(renderer_t *Renderer)
    {
	// ===================== platform code =====================
	glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// ========================================================
    }

    void ResetStats(renderer_t *Renderer)
    {
	memset(&Renderer->Stats, 0, sizeof(renderer_stats));
    }

    // TODO: use geometry shader to make wireframe effect on individual object
    void ToggleWireframeMode(renderer_t *Renderer)
    {
	Renderer->WireframeMode = !Renderer->WireframeMode;
	// ===================== platform code =====================
	glPolygonMode(GL_FRONT_AND_BACK, (Renderer->WireframeMode ? GL_FILL : GL_LINE));
	// ========================================================
    }

    void DrawLines(renderer_t *Renderer,mesh_t *Mesh, shader_t *Shader, glm::mat4 viewMatrix)
    {
	shader::UseProgram(Shader);
	//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	shader::SetUniform4fv(Shader, "view", viewMatrix);    
	shader::SetUniform4fv(Shader, "model", glm::mat4(1.0f));

	glBindVertexArray(Mesh->VAO);
	glDrawArrays(GL_LINES, 0, Mesh->VertexCount);
	Renderer->Stats.DrawCalls++;
    }

    void DrawMesh(renderer_t *Renderer,
		  mesh_t *Mesh,
		  shader_t *Shader,
		  glm::mat4 viewMatrix,
		  glm::mat4 model)

    {
	shader::UseProgram(Shader);
	shader::SetUniform4fv(Shader, "view", viewMatrix);    
	shader::SetUniform4fv(Shader, "model", model);

	uint32 diffuseNr = 1;
	uint32 specularNr = 1;
	uint32 normalNr = 1;
	uint32 heightNr = 1;

	// TODO: improve texture management
	for (uint32 i = 0; i < Mesh->Textures.size(); i++)
	{
	    glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
	    std::string number;
	    std::string name = Mesh->Textures[i].Type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to stream
            else if(name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to stream
	    else if(name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to stream

	    shader::SetUniform1i(Shader, (name + number).c_str(), i);
	    glBindTexture(GL_TEXTURE_2D, Mesh->Textures[i].Id);
	}

	glBindVertexArray(Mesh->VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)Mesh->Indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0); // good practice
	glActiveTexture(GL_TEXTURE0); // good practice
    }

    void DrawModel(renderer_t *Renderer,
		   model_t *Model,
		   shader_t *Shader,
		   glm::mat4 viewMatrix,
		   glm::mat4 model)
    {
	for (uint32 i = 0; i < Model->Meshes.size(); i++)
	    DrawMesh(Renderer, Model->Meshes[i], Shader, viewMatrix, model);
    }
}

namespace mesh
{
    mesh_t* Construct(std::vector<vertex_t> vertices,
			   std::vector<uint32> indices,
			   std::vector<texture_t> textures)
    {
	mesh_t *Mesh = new mesh_t;
	Mesh->Vertices = vertices;
	Mesh->Indices = indices;
	Mesh->Textures = textures;

	allocate_mesh(Mesh);

	return Mesh;
    }

    void Delete(mesh_t *Mesh)
    {
	glDeleteVertexArrays(1, &Mesh->VAO);
	glDeleteBuffers(1, &Mesh->VBO);
	glDeleteBuffers(1, &Mesh->EBO);

    	delete[] Mesh->Data;
    }
}

static void allocate_mesh(mesh_t *Mesh)
{
    glGenVertexArrays(1, &Mesh->VAO);
    glGenBuffers(1, &Mesh->VBO);
    glGenBuffers(1, &Mesh->EBO);
  
    glBindVertexArray(Mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);

    glBufferData(GL_ARRAY_BUFFER,
		 Mesh->Vertices.size() * sizeof(vertex_t),
		 &Mesh->Vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 Mesh->Indices.size() * sizeof(uint32), 
                 &Mesh->Indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			  (void*)0);

    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			  (void*)offsetof(vertex_t, Normal));

    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			  (void*)offsetof(vertex_t, TexCoords));

    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			  (void*)offsetof(vertex_t, Tangent));

    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
			  (void*)offsetof(vertex_t, Bitangent));

    glBindVertexArray(0);
}

namespace model
{
    model_t* LoadFromFile(std::string const &path)
    {
	model_t *Model = new model_t;
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
	    // TODO: dont used STD::COUT!!!!
	    std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
	    return nullptr;
	}

	Model->Directory = path.substr(0, path.find_last_of('/'));
	process_node(Model, scene->mRootNode, scene);

	return Model;
    }

    
    void Delete(model_t *Model)
    {
	// TODO: delete all mesh from vector
    }
}

static void process_node(model_t *Model, aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(uint32 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        Model->Meshes.push_back(process_mesh(Model, mesh, scene));			
    }
    // then do the same for each of its children
    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        process_node(Model, node->mChildren[i], scene);
    }    
}

static mesh_t* process_mesh(model_t *Model, aiMesh *mesh, const aiScene *scene)
{
    std::vector<vertex_t> vertices;
    std::vector<uint32> indices;
    std::vector<texture_t> textures;

    // Vertices
    for (uint32 i = 0; i < mesh->mNumVertices; i++)
    {
	vertex_t Vertex;
	glm::vec3 vector; 

	vector.x = mesh->mVertices[i].x;
	vector.y = mesh->mVertices[i].y;
	vector.z = mesh->mVertices[i].z; 
	Vertex.Position = vector;

	vector.x = mesh->mNormals[i].x;
	vector.y = mesh->mNormals[i].y;
	vector.z = mesh->mNormals[i].z;
	Vertex.Normal = vector; 

	if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
	{
	    glm::vec2 vec;
	    vec.x = mesh->mTextureCoords[0][i].x; 
	    vec.y = mesh->mTextureCoords[0][i].y;
	    Vertex.TexCoords = vec;
	}
	else
	    Vertex.TexCoords = glm::vec2(0.0f, 0.0f);

	vector.x = mesh->mTangents[i].x;
	vector.y = mesh->mTangents[i].y;
	vector.z = mesh->mTangents[i].z;
	Vertex.Tangent = vector;

	vector.x = mesh->mBitangents[i].x;
	vector.y = mesh->mBitangents[i].y;
	vector.z = mesh->mBitangents[i].z;
	Vertex.Bitangent = vector;
	
	vertices.push_back(Vertex);
    }

    // Indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
	aiFace face = mesh->mFaces[i];
	for(unsigned int j = 0; j < face.mNumIndices; j++)
	    indices.push_back(face.mIndices[j]);
    }  

    // Materials / Textures
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // 1. diffuse maps
    std::vector<texture_t> diffuseMaps = load_material_textures(Model,
								material,
								aiTextureType_DIFFUSE,
								"texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // 2. specular maps
    std::vector<texture_t> specularMaps = load_material_textures(Model,
								 material,
								 aiTextureType_SPECULAR,
								 "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // 3. normal maps
    std::vector<texture_t> normalMaps = load_material_textures(Model,
							       material,
							       aiTextureType_HEIGHT,
							       "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // 4. height maps
    std::vector<texture_t> heightMaps = load_material_textures(Model,
							       material,
							       aiTextureType_AMBIENT,
							       "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return mesh::Construct(vertices, indices, textures);
}

static std::vector<texture_t> load_material_textures(model_t *Model,
						     aiMaterial *mat,
						     aiTextureType type,
						     std::string typeName)
{
    std::vector<texture_t> textures;
    for(uint32 i = 0; i < mat->GetTextureCount(type); i++)
    {
	aiString str;
	mat->GetTexture(type, i, &str);
	// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
	bool skip = false;
	for(uint32 j = 0; j < Model->TexturesLoaded.size(); j++)
	{
	    if(std::strcmp(Model->TexturesLoaded[j].Path.data(), str.C_Str()) == 0)
	    {
		textures.push_back(Model->TexturesLoaded[j]);
		skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
		break;
	    }
	}
	if(!skip)
	{   // if texture hasn't been loaded already, load it
	    texture_t Texture;
	    Texture.Id = load_texture_from_file(str.C_Str(), Model->Directory);
	    Texture.Type = typeName;
	    Texture.Path = str.C_Str();
	    textures.push_back(Texture);
	    Model->TexturesLoaded.push_back(Texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
	}
    }

    return textures;
}

static uint32 load_texture_from_file(const char *path, const std::string &directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    uint32 textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
	else
	    format = GL_RGBA; // TODO: ??

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
	// TODO: REMOVE STD::COUT!!!!
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
