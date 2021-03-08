#pragma once

glm::mat4 ASSET_convert_matrix_to_GLM(const aiMatrix4x4& from)
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

// ASSET LOAD TEXTURE FROM FILE
uint32 ASSET_load_texture_from_file(const std::string &path)
{
    Log::info("loading texture: '%s'\n", path.c_str());

    uint32 textureID;
    glGenTextures(1, &textureID); // TODO: OGL
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        // TODO: OGL
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            format = GL_RGBA;

        // TODO: OGL
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // TODO: OGL
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
        Log::error("failed to load texture from: %s\n", path.c_str());

    stbi_image_free(data);

    return textureID;
}

// ASSET LOAD MATERIAL TEXTURES
std::vector<GPUTexture> ASSET_load_material_textures(const std::string &directory, aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<GPUTexture> textures;
    for(uint32 i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        // check if texture was loaded before
        bool skip = false;
        for(uint32 j = 0; j < textures.size(); j++)
        {
            if(std::strcmp(textures[j].filename.data(), str.C_Str()) == 0)
            {
                skip = true;
                break;
            }
        }

        std::string filename(str.C_Str());
        std::string fullpath = directory+"/"+filename;

        // if texture hasn't been loaded already, load it
        if(!skip)
        {
            GPUTexture t;
            t.ID = ASSET_load_texture_from_file(fullpath.c_str());
            t.type = typeName;
            t.filename = str.C_Str();
            textures.push_back(t);
        }
    }

    return textures;
}

uint32 ASSET_process_animation_data(std::vector<GPUVertex> &vertices, aiMesh* mesh, const aiScene* scene)
{
    uint32 jointCount = 0;
    Joint *rootJoint = new Joint;
    for (uint32 i = 0; i < mesh->mNumBones; ++i)
    {
        if (i == 0)
        {
            rootJoint->ID = i;
            rootJoint->name = mesh->mBones[i]->mName.C_Str();

            // DEBUG:
            Log::debug("rootJoint->ID %d\n", i);
            Log::debug("rootJoint->name %s\n", rootJoint->name.c_str());

            rootJoint->animatedTransform = ASSET_convert_matrix_to_GLM(mesh->mBones[i]->mOffsetMatrix);
        } else {
            Joint joint;
            joint.ID = i;
            joint.name = mesh->mBones[i]->mName.C_Str();
            joint.animatedTransform = ASSET_convert_matrix_to_GLM(mesh->mBones[i]->mOffsetMatrix);

            // DEBUG:
            Log::debug("childJoint.ID %d\n", i);
            Log::debug("childJoint.name %s\n", joint.name.c_str());

            rootJoint->children.push_back(joint);
        }
        jointCount++;

        auto weights = mesh->mBones[i]->mWeights;
        uint32 numWeights = mesh->mBones[i]->mNumWeights;

        for (uint32 y = 0; y < numWeights; ++y)
        {
            int vertexId = weights[y].mVertexId;
            float weight = weights[y].mWeight;

            for (int w = 0; w < GPU_MAX_JOINT_INFLUENCE; ++w)
            {
                if (vertices[vertexId].jointIDs[w] < 0)
                {
                    vertices[vertexId].jointIDs[w] = i;
                    vertices[vertexId].weights[w] = weight;
                    break;
                }

                // DEBUG:
                //Log::debug("id=%d weight=%f\n", vertices[vertexId].jointIDs[w], vertices[vertexId].weights[w]);
            }
        }
    }

    return jointCount;
}

// ASSET PROCESS MESH.
GPUMesh* ASSET_process_mesh(const std::string &directory, aiMesh *mesh, const aiScene *scene)
{
    std::vector<GPUVertex> vertices;
    std::vector<uint32> indices;
    std::vector<GPUTexture> textures;

    // Vertices.
    for (uint32 i = 0; i < mesh->mNumVertices; i++)
    {
        GPUVertex vertex;
        for (uint32 i = 0; i < GPU_MAX_JOINT_INFLUENCE; i++)
        {
            vertex.jointIDs[i] = -1;
            vertex.weights[i] = 0.0f;            
        }
        
        glm::vec3 vector; 

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z; 

        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;

        vertex.normal = vector; 

        // does the mesh contain texture coordinates?
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);

        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;

        vertex.tangent = vector;

        vector.x = mesh->mBitangents[i].x;
        vector.y = mesh->mBitangents[i].y;
        vector.z = mesh->mBitangents[i].z;

        vertex.bitangent = vector;
	
        vertices.push_back(vertex);
    }

    // Indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }  

    // Materials / Textures.
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // Diffuse maps.
    std::vector<GPUTexture> diffuseMaps = ASSET_load_material_textures(directory, material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // Specular maps.
    std::vector<GPUTexture> specularMaps = ASSET_load_material_textures(directory, material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // Normal maps.
    std::vector<GPUTexture> normalMaps = ASSET_load_material_textures(directory, material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // Height maps.
    std::vector<GPUTexture> heightMaps = ASSET_load_material_textures(directory, material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // TODO: @animation
    uint32 jointCount = ASSET_process_animation_data(vertices, mesh, scene);
    
    return new GPUMesh(vertices, indices, textures, jointCount);
}

// ASSET PROCESS NODE.
void ASSET_process_node(const std::string &directory, aiNode *node, const aiScene *scene, std::vector<GPUMesh*> &meshes)
{
    for(uint32 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(ASSET_process_mesh(directory, mesh, scene));
    }

    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        ASSET_process_node(directory, node->mChildren[i], scene, meshes);
    }
}

// ASSET LOAD MESHES.
std::string ASSET_load_meshes(const std::string &directory, const std::string &filename, std::vector<GPUMesh*> &meshes)
{
    std::string fullpath = directory+"/"+filename;
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fullpath, aiProcess_Triangulate
                                             // // | aiProcess_MakeLeftHanded
                                             // // | aiProcess_PreTransformVertices
                                             | aiProcess_FlipUVs
                                             | aiProcess_JoinIdenticalVertices
                                             | aiProcess_CalcTangentSpace
                                             | aiProcess_GenSmoothNormals
                                             | aiProcess_FindInvalidData
                                             | aiProcess_ValidateDataStructure
                                             | 0);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        return importer.GetErrorString();

    ASSET_process_node(directory, scene->mRootNode, scene, meshes);

    return "";
}
