#include "mesh.h"

static void allocate_mesh(mesh_t *Mesh);
static void process_node(model_t *Model, aiNode *node, const aiScene *scene);
static mesh_t* process_mesh(model_t *Model, aiMesh *mesh, const aiScene *scene);
static std::vector<texture_t> load_material_textures(model_t *Model, aiMaterial *mat, aiTextureType type, std::string typeName);
static uint32 load_texture_from_file(const char *path, const std::string &directory);

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
	glDeleteBuffers(1, &Mesh->IBO);
    }

    bool RaySphereIntersection(glm::vec3 rayOriginWorld,
			       glm::vec3 rayDirectionWorld,
			       glm::vec3 sphereCenterWorld,
			       float32 sphereRadius,
			       float32 *intersectionDistance)
    {
	// work out components of quadratic
	glm::vec3 distToSphere = rayOriginWorld - sphereCenterWorld;
	float32 b = dot( rayDirectionWorld, distToSphere );
	float32 c = dot( distToSphere, distToSphere ) - sphereRadius * sphereRadius;
	float32 b_squared_minus_c = b * b - c;

	// check for "imaginary" answer. == ray completely misses sphere
	if ( b_squared_minus_c < 0.0f ) { return false; }

	// check for ray hitting twice (in and out of the sphere)
	if ( b_squared_minus_c > 0.0f ) {
	    // get the 2 intersection distances along ray
	    float32 t_a = -b + sqrt( b_squared_minus_c );
	    float32 t_b = -b - sqrt( b_squared_minus_c );
	    *intersectionDistance = t_b;

	    // if behind viewer, throw one or both away
	    if ( t_a < 0.0 ) {
		if ( t_b < 0.0 ) { return false; }
	    } else if ( t_b < 0.0 ) {
		*intersectionDistance = t_a;
	    }

	    return true;
	}

	// check for ray hitting once (skimming the surface)
	if ( 0.0f == b_squared_minus_c ) {
	    // if behind viewer, throw away
	    float32 t = -b + sqrt( b_squared_minus_c );
	    if ( t < 0.0f ) { return false; }
	    *intersectionDistance = t;
	    return true;
	}

	// note: could also check if ray origin is inside sphere radius
	return false;
    }
}

static void allocate_mesh(mesh_t *Mesh)
{
    glGenVertexArrays(1, &Mesh->VAO);
    glGenBuffers(1, &Mesh->VBO);
    glGenBuffers(1, &Mesh->IBO);
  
    glBindVertexArray(Mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);

    if (!Mesh->Vertices.empty())
    {
	glBufferData(GL_ARRAY_BUFFER,
		     Mesh->Vertices.size() * sizeof(vertex_t),
		     &Mesh->Vertices[0], GL_STATIC_DRAW);  

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
    }

    if (!Mesh->Indices.empty())
    {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     Mesh->Indices.size() * sizeof(uint32), 
		     &Mesh->Indices[0], GL_STATIC_DRAW);
    }
    
    glBindVertexArray(0);
}

namespace model
{
    model_t* LoadFromFile(std::string const &path)
    {
	model_t *Model = new model_t;
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
	    printf("ASSIMP::Error '%s'\n", import.GetErrorString());
	    return nullptr;
	}

	Model->Directory = path.substr(0, path.find_last_of('/'));
	printf("load model from file directory: %s\n", Model->Directory.c_str()); // TODO:
	process_node(Model, scene->mRootNode, scene);

	return Model;
    }

    
    void Delete(model_t *Model)
    {
	for (auto& m : Model->Meshes)
	{
	    delete m;
	    m = nullptr;
	}

	Model->Meshes.erase(std::remove(Model->Meshes.begin(),
					Model->Meshes.end(),
					nullptr),
			    Model->Meshes.end());
	delete Model;
    }
}

static void process_node(model_t *Model, aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(uint32 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        Model->Meshes.push_back(process_mesh(Model, mesh, scene)); // TODO: array			
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

	// check if texture was loaded before
	bool skip = false;
	for(uint32 j = 0; j < Model->TexturesLoadedCache.size(); j++)
	{
	    if(std::strcmp(Model->TexturesLoadedCache[j].Path.data(), str.C_Str()) == 0)
	    {
		textures.push_back(Model->TexturesLoadedCache[j]);
		skip = true;
		break;
	    }
	}

        // if texture hasn't been loaded already, load it
	if(!skip)
	{
	    texture_t Texture;
	    Texture.Id = load_texture_from_file(str.C_Str(), Model->Directory);
	    Texture.Type = typeName;
	    Texture.Path = str.C_Str();
	    textures.push_back(Texture);
	    Model->TexturesLoadedCache.push_back(Texture);  // caching texture
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
        printf("Texture failed to load at path: '%s'\n", path);
        stbi_image_free(data);
    }

    return textureID;
}
