#include "LzyInput.h"
#include "LzyMemory.h"

typedef struct LzyInputState
{
    u8 keys[LZY_KEY_CODE_RANGE / 4 + 1];
    u8 mouseButtons[LZY_MOUSE_BUTTON_CODE_RANGE / 4 + 1];
}LzyInputState;


global b8 bInputIsInitialized = false;
global LzyInputState inputState = {};

b8 lzy_input_init()
{
    lzy_memzero(&inputState, sizeof(inputState));
    
    bInputIsInitialized = true;
    return true;
}

void lzy_input_set_key_state(LzyKeyCode keyCode, LzyKeyState keyState)
{
    u16 uMajIndex = keyCode / 4;
    u8 uMinIndex  = (keyCode & 3) * 2;
    
    inputState.keys[uMajIndex] &= ~(0b11 << uMinIndex);
    inputState.keys[uMajIndex] |= keyState << uMinIndex;
}

void lzy_input_set_mouse_button_state(LzyMouseButtonCode buttonCode, LzyKeyState state)
{
    u16 uMajIndex = buttonCode / 4;
    u8 uMinIndex  = (buttonCode & 3) * 2;
    
    inputState.mouseButtons[uMajIndex] &= ~(0b11 << uMinIndex);
    inputState.mouseButtons[uMajIndex] |= keyState << uMinIndex;
}

LzyKeyState lzy_input_get_key_state(LzyKeyCode keyCode)
{
    u16 uMajIndex = keyCode / 4;
    u8 uMinIndex  = (keyCode & 3) * 2;
    
    return inputState.keys[uMajIndex] & (0b11 << uMinIndex);
}

LzyKeyState lzy_input_get_mouse_button_state(LzyKeyCode buttonCode)
{
    u16 uMajIndex = buttonCode / 4;
    u8 uMinIndex  = (buttonCode & 3) * 2;
    
    return inputState.mouseButtons[uMajIndex] & (0b11 << uMinIndex); 
}

void lzy_input_step()
{
    
    for(LzyKeyCode i = 0; i < LZY_KEY_CODE_RANGE / 4 + 1; i++)
    {
        inputState.keys[i] &= 0b10101010;
    }
    
    
    for(LzyKeyCode i = 0; i < LZY_MOUSE_BUTTON_CODE_RANGE / 4 + 1; i++)
    {
        inputState.mouseButtons[i] &= 0b10101010;
    }
    
}