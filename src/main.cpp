#include "precompile.h"

#if 1 // TODO pass arg from compiler
#include "Plateform/win32_plateform.cpp"
#endif

#include "editor.cpp"

/* TODO GAME:
   - near / far plan boundary locked camera
*/

/* TODO Improvments:
   - learn more compiler stuff
   - learn how properly debug with MSVC
   - IMGUI check examples through all versions
*/

enum program_mode
{
    EDITOR_MODE,
    RENDER_MODE,
};

const uint32 g_Width = 1280;
const uint32 g_Height = 960;

static unsigned int g_CurrentMode = EDITOR_MODE;

int main(int argc, char *argv[])
{
    window_t *Window = new window_t;
    Window->Time = new frame_time;
    input_t *Input = new input_t;

#if 1
    win32_init_window(g_Width, g_Height, "CrapEngine", Window);
    win32_init_time(Window->Time);
    win32_init_input(Window->Win32Window, Input);
#endif

    switch(g_CurrentMode)
    {
    case EDITOR_MODE:
        RunEditorMode(g_CurrentMode, Window, Input);
        // TODO: should: create a world and save it in file
        break;
    case RENDER_MODE:
        // TODO: should: render a world from file
        break;
    default:
        // log something?
        break;
    };
    
	return 0;
}
