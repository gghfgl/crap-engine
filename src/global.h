#pragma once

enum EXEC_MODE {
    EXIT_MODE = 0,
    EDITOR_MODE = 1,
    GAME_MODE   = 2
};

struct GlobalState
{
    EXEC_MODE currentMode = GAME_MODE;
    glm::mat4 projectionMatrix;
};
