#pragma once

#define CRAP_KEY_UNKNOWN -1 
#define CRAP_KEY_SPACE 32 
#define CRAP_KEY_APOSTROPHE 39
#define CRAP_KEY_COMMA 44
#define CRAP_KEY_MINUS 45
#define CRAP_KEY_PERIOD 46
#define CRAP_KEY_SLASH 47
#define CRAP_KEY_0 48 
#define CRAP_KEY_1 49 
#define CRAP_KEY_2 50 
#define CRAP_KEY_3 51 
#define CRAP_KEY_4 52 
#define CRAP_KEY_5 53 
#define CRAP_KEY_6 54 
#define CRAP_KEY_7 55 
#define CRAP_KEY_8 56 
#define CRAP_KEY_9 57 
#define CRAP_KEY_SEMICOLON 59
#define CRAP_KEY_EQUAL 61
#define CRAP_KEY_A 65 
#define CRAP_KEY_B 66 
#define CRAP_KEY_C 67 
#define CRAP_KEY_D 68 
#define CRAP_KEY_E 69 
#define CRAP_KEY_F 70 
#define CRAP_KEY_G 71 
#define CRAP_KEY_H 72 
#define CRAP_KEY_I 73 
#define CRAP_KEY_J 74 
#define CRAP_KEY_K 75 
#define CRAP_KEY_L 76 
#define CRAP_KEY_M 77 
#define CRAP_KEY_N 78 
#define CRAP_KEY_O 79 
#define CRAP_KEY_P 80 
#define CRAP_KEY_Q 81 
#define CRAP_KEY_R 82 
#define CRAP_KEY_S 83 
#define CRAP_KEY_T 84 
#define CRAP_KEY_U 85 
#define CRAP_KEY_V 86 
#define CRAP_KEY_W 87 
#define CRAP_KEY_X 88 
#define CRAP_KEY_Y 89 
#define CRAP_KEY_Z 90 
#define CRAP_KEY_LEFT_BRACKET 91
#define CRAP_KEY_BACKSLASH 92
#define CRAP_KEY_RIGHT_BRACKET 93
#define CRAP_KEY_GRAVE_ACCENT 96
#define CRAP_KEY_WORLD_1 161
#define CRAP_KEY_WORLD_2 162
#define CRAP_KEY_ESCAPE 256 
#define CRAP_KEY_ENTER 257 
#define CRAP_KEY_TAB 258 
#define CRAP_KEY_BACKSPACE 259 
#define CRAP_KEY_INSERT 260 
#define CRAP_KEY_DELETE 261 
#define CRAP_KEY_RIGHT 262 
#define CRAP_KEY_LEFT 263 
#define CRAP_KEY_DOWN 264 
#define CRAP_KEY_UP 265 
#define CRAP_KEY_PAGE_UP 266 
#define CRAP_KEY_PAGE_DOWN 267 
#define CRAP_KEY_HOME 268 
#define CRAP_KEY_END 269 
#define CRAP_KEY_CAPS_LOCK 280 
#define CRAP_KEY_SCROLL_LOCK 281 
#define CRAP_KEY_NUM_LOCK 282 
#define CRAP_KEY_PRINT_SCREEN 283 
#define CRAP_KEY_PAUSE 284 
#define CRAP_KEY_F1 290 
#define CRAP_KEY_F2 291 
#define CRAP_KEY_F3 292 
#define CRAP_KEY_F4 293 
#define CRAP_KEY_F5 294 
#define CRAP_KEY_F6 295 
#define CRAP_KEY_F7 296 
#define CRAP_KEY_F8 297 
#define CRAP_KEY_F9 298 
#define CRAP_KEY_F10 299 
#define CRAP_KEY_F11 300 
#define CRAP_KEY_F12 301 
#define CRAP_KEY_F13 302 
#define CRAP_KEY_F14 303 
#define CRAP_KEY_F15 304 
#define CRAP_KEY_F16 305 
#define CRAP_KEY_F17 306 
#define CRAP_KEY_F18 307 
#define CRAP_KEY_F19 308 
#define CRAP_KEY_F20 309 
#define CRAP_KEY_F21 310 
#define CRAP_KEY_F22 311 
#define CRAP_KEY_F23 312 
#define CRAP_KEY_F24 313 
#define CRAP_KEY_F25 314 
#define CRAP_KEY_KP_0 320 
#define CRAP_KEY_KP_1 321 
#define CRAP_KEY_KP_2 322 
#define CRAP_KEY_KP_3 323 
#define CRAP_KEY_KP_4 324 
#define CRAP_KEY_KP_5 325 
#define CRAP_KEY_KP_6 326 
#define CRAP_KEY_KP_7 327 
#define CRAP_KEY_KP_8 328 
#define CRAP_KEY_KP_9 329 
#define CRAP_KEY_KP_DECIMAL 330 
#define CRAP_KEY_KP_DIVIDE 331 
#define CRAP_KEY_KP_MULTIPLY 332 
#define CRAP_KEY_KP_SUBTRACT 333 
#define CRAP_KEY_KP_ADD 334 
#define CRAP_KEY_KP_ENTER 335 
#define CRAP_KEY_KP_EQUAL 336 
#define CRAP_KEY_LEFT_SHIFT 340 
#define CRAP_KEY_LEFT_CONTROL 341 
#define CRAP_KEY_LEFT_ALT 342 
#define CRAP_KEY_LEFT_SUPER 343 
#define CRAP_KEY_RIGHT_SHIFT 344 
#define CRAP_KEY_RIGHT_CONTROL 345 
#define CRAP_KEY_RIGHT_ALT 346 
#define CRAP_KEY_RIGHT_SUPER 347 
#define CRAP_KEY_MENU 348 
#define CRAP_KEY_LAST CRAP_KEY_MENU

struct keyboard_e
{
    bool IsPressed[512] = {0};
    bool IsReleased[512] = {0};
};

struct mouse_e
{
    float64  PosX, PosY;
    float64  LastX, LastY;
    float32  OffsetX, OffsetY;
    float64  ScrollOffsetX, ScrollOffsetY;
    bool     LeftButton;
    bool     LeftButtonFirstClick;  
};

struct input_t
{
    keyboard_e *KeyboardEvent;
    mouse_e    *MouseEvent;
};
