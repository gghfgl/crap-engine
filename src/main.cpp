#include "Engine/precompile.h"

#include "editor.cpp"

const uint32 c_Width = 1680;
const uint32 c_Height = 1050;

int main(int, char**)
{
    Plateform *plateform = new Plateform(c_Width, c_Height, "crapengine_editor");

    RunEditorMode(plateform->window, plateform->input, plateform->info);    

    delete plateform;
    
    return 0;
}
