#include "input.h"

void Delete(input_t *InputState)
{
    delete InputState->KeyboardEvent;
    delete InputState->MouseEvent;
    delete InputState;
}

void UpdateMouseOffsets(mouse_e *MouseEvent)
{
    if (MouseEvent->LeftButtonFirstClick)
    {
        MouseEvent->LastX = MouseEvent->PosX;
        MouseEvent->LastY = MouseEvent->PosY;
        MouseEvent->LeftButtonFirstClick = false;
    }

    MouseEvent->OffsetX = (float32)(MouseEvent->PosX - MouseEvent->LastX);
    MouseEvent->OffsetY = (float32)(MouseEvent->LastY - MouseEvent->PosY);

    MouseEvent->LastX = MouseEvent->PosX;
    MouseEvent->LastY = MouseEvent->PosY;
}

float32 GetMouseScrollOffsetY(mouse_e *MouseEvent)
{
    float32 rValue = (float32)MouseEvent->ScrollOffsetY;
    MouseEvent->ScrollOffsetY = 0.0f;

    return rValue;
}
