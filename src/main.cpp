#include "Engine/precompile.h"
#include "precompile.h"

#include "editor.cpp"

const uint32 c_Width = 1440;
const uint32 c_Height = 900;

int main(int, char**)
{
    Plateform *plateform = new Plateform(c_Width, c_Height, "crapengine 0.0.1");
    
    Run(plateform->window, plateform->input, plateform->info);    

    delete plateform;
    
    return 0;
}
