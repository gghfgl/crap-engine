#include "precompile.h"

#include "editor.cpp"

/* TODO GAME:
   - near / far plan boundary locked camera
*/

/* TODO Improvments:
   - learn more compiler stuff
   - learn how properly debug with MSVC
   - IMGUI check examples through all versions
*/

// enum program_mode
// {
//     EDITOR_MODE,
//     RENDER_MODE,
// };

const uint32 c_Width = 1280;
const uint32 c_Height = 960;

//static unsigned int g_CurrentMode = EDITOR_MODE;

int main(int argc, char *argv[])
{
    Plateform *plateform = new Plateform(c_Width, c_Height, "crapengine_editor");

    // switch(g_CurrentMode)
    // {
    // case EDITOR_MODE:
    RunEditorMode(plateform->Window, plateform->Input);
    //     // TODO: should: create a world and save it in file
    //     break;
    // case RENDER_MODE:
    //     // TODO: should: render a world from file
    //     break;
    // default:
    //     // log something?
    //     break;
    // };

    bool running{true};
    while (running)
    {

        plateform->Window->updateTime();
	    plateform->Window->pollEvents();
		if (plateform->Input->KeyboardEvent->isPressed[keyboard::CRAP_KEY_ESCAPE])
		    running = false;
    }
    
    delete plateform;
    
	return 0;
}
