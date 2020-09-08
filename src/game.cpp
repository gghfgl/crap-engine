glm::mat4* GenerateTerrainModelMatrices(uint32 squareSideLenght)
{
    glm::mat4 *modelMatrices;
    modelMatrices = new glm::mat4[squareSideLenght * squareSideLenght];    
    glm::vec3 size = { 1.0f, 1.0f, 1.0f };
    float posX = 0.0f;

    uint32 index = 0;
    for (uint32 i = 0; i < squareSideLenght; i++)
    {
        float posZ = -size.z;
        for (uint32 y = 0; y < squareSideLenght; y++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, { posX, 0.0f, posZ });
            model = glm::scale(model, glm::vec3(1.0f));
            model = glm::rotate(model, glm::radians(0.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrices[index] = model;

            posZ -= size.z;
            index++;
        }
        posX += size.x;
    }

    return modelMatrices;
}
