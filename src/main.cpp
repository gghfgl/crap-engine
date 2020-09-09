#include "editor.cpp"

/* TODO GAME:
   - near / far plan boundary locked camera
*/

/* TODO Improvments:
   - learn more compiler stuff
   - learn how properly debug with MSVC
   - IMGUI check examples through all versions
*/

// -------------------------------
enum program_mode
{
    EDITOR_MODE,
    RENDER_MODE,
};

static unsigned int g_CurrentMode = EDITOR_MODE;

int main(int argc, char *argv[])
{
    switch(g_CurrentMode)
    {
    case EDITOR_MODE:
        RunEditorMode(g_CurrentMode);
        // create a world saved in file
        break;
    case RENDER_MODE:
        // Render a world from file
        break;
    default:
        // log something?
        break;
    };
    
	return 0;
}
