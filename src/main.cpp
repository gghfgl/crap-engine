#include "Engine/precompile.h"

const uint32 c_Width = 1280;
const uint32 c_Height = 960;

int main(int, char**)
{
    Plateform *plateform = new Plateform(c_Width, c_Height, "crapengine_editor");

    bool running{true};
    while (running)
    {

        plateform->Window->updateTime();
        plateform->Window->pollEvents();
        if (plateform->Input->Keyboard->isPressed[keyboard::CRAP_KEY_ESCAPE])
            running = false;
    }
    
    delete plateform;
    
    return 0;
}
