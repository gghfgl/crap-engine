#include "Engine/precompile.h"
#include "precompile.h"
#include "global.h"

#include "editor.cpp"
#include "game.cpp"

const uint32 c_Width = 1440;
const uint32 c_Height = 900;

int main(int, char**)
{
    Plateform *plateform = new Plateform(c_Width, c_Height, "crapengine v0.0.1");
    
    GlobalState gs;

    while (gs.currentMode != EXIT_MODE)
    {
        if (gs.currentMode == EDITOR_MODE)
            RunEditor(plateform->window, plateform->input, plateform->info, &gs);

        if (gs.currentMode == GAME_MODE)
            RunGame(plateform->window, plateform->input, plateform->info, &gs);
    }

    delete plateform;
    
    return 0;
}
