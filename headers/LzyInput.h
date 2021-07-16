#pramga once
#include "LzyDefines.h"
// 01 01 01 01
// 10 10 10 10
typedef enum LzyKeyState
{
    LZY_KEY_STATE_NONE    = 0b00,
    LZY_KEY_STATE_RELEASE = 0b01,
    LZY_KEY_STATE_PRESS   = 0b11,
    LZY_KEY_STATE_HOLD    = 0b10
}LzyKeyState;

typedef enum LzyMouseButtonCode
{
    LZY_MOUSE_BUTTON_LEFT,
    LZY_MOUSE_BUTTON_RIGHT,
    LZY_MOUSE_BUTTON_MIDDLE,
    LZY_MOUSE_BUTTON_4,
    LZY_MOUSE_BUTTON_5,
    LZY_MOUSE_BUTTON_CODE_RANGE
}LzyMouseButtonCode;

typedef enum LzyKeyCode
{
    LZY_KEY_CODE_A,
    LZY_KEY_CODE_B,
    LZY_KEY_CODE_C,
    LZY_KEY_CODE_D,
    LZY_KEY_CODE_E,
    LZY_KEY_CODE_F,
    LZY_KEY_CODE_G,
    LZY_KEY_CODE_H,
    LZY_KEY_CODE_I,
    LZY_KEY_CODE_J,
    LZY_KEY_CODE_K,
    LZY_KEY_CODE_L,
    LZY_KEY_CODE_M,
    LZY_KEY_CODE_N,
    LZY_KEY_CODE_O,
    LZY_KEY_CODE_P,
    LZY_KEY_CODE_Q,
    LZY_KEY_CODE_R,
    LZY_KEY_CODE_S,
    LZY_KEY_CODE_T,
    LZY_KEY_CODE_U,
    LZY_KEY_CODE_V,
    LZY_KEY_CODE_W,
    LZY_KEY_CODE_X,
    LZY_KEY_CODE_Y,
    LZY_KEY_CODE_Z,
    LZY_KEY_CODE_SPACE,
    LZY_KEY_CODE_RETURN,
    LZY_KEY_CODE_ESCAPE,
    LZY_KEY_CODE_LSHIFT,
    LZY_KEY_CODE_RSHIFT,
    LZY_KEY_CODE_LCTRL,
    LZY_KEY_CODE_RCTRL,
    LZY_KEY_CODE_LALT,
    LZY_KEY_CODE_RALT,
    LZY_KEY_CODE_TAB,
    LZY_KEY_CODE_1,
    LZY_KEY_CODE_2,
    LZY_KEY_CODE_3,
    LZY_KEY_CODE_4,
    LZY_KEY_CODE_5,
    LZY_KEY_CODE_6,
    LZY_KEY_CODE_7,
    LZY_KEY_CODE_8,
    LZY_KEY_CODE_9,
    LZY_KEY_CODE_0,
    LZY_KEY_CODE_F1,
    LZY_KEY_CODE_F2,
    LZY_KEY_CODE_F3,
    LZY_KEY_CODE_F4,
    LZY_KEY_CODE_F5,
    LZY_KEY_CODE_F6,
    LZY_KEY_CODE_F7,
    LZY_KEY_CODE_F8,
    LZY_KEY_CODE_F9,
    LZY_KEY_CODE_F10,
    LZY_KEY_CODE_F11,
    LZY_KEY_CODE_F12,
    LZY_KEY_CODE_PERIOD,
    LZY_KEY_CODE_COMMA,
    LZY_KEY_CODE_SLASH,
    LZY_KEY_CODE_BACKSLASH,
    LZY_KEY_CODE_UP,
    LZY_KEY_CODE_DOWN,
    LZY_KEY_CODE_LEFT,
    LZY_KEY_CODE_RIGHT,
    LZY_KEY_CODE_RANGE
}LzyKeyCode;

b8 lzy_input_init();
void lzy_input_set_key_state(LzyKeyCode keyCode, LzyKeyState keyState);
void lzy_input_set_mouse_button_state(LzyMouseButtonCode buttonCode, LzyKeyState keyState);
void lzy_input_step();

LAPI LzyKeyState lzy_input_get_key_state(LzyKeyCode keyCode);
LAPI LzyKeyState lzy_input_get_mouse_button_state(LzyMouseButtonCode buttonCode);
//TODO: handle scroll
