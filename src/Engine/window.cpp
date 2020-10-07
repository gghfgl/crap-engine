#include "window.h"

void Delete(window_t *Window)
{
    delete Window->Time;
    delete Window;
}

void ToggleDebug(window_t *Window)
{
    Window->Debug = !Window->Debug;
}
