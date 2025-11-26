enum class PollStatus {
    NONE,
    QUIT,
    KEYDOWN,
    KEYUP,
    MOUSEBUTTONDOWN,
    MOUSEBUTTONUP,
    MOUSEMOTION,
    MOUSEWHEEL,
    WINDOWEVENT
};

enum class KeyCode {
    UNKNOWN,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
    SPACE, ENTER, ESCAPE, TAB, BACKSPACE,
    LEFT, RIGHT, UP, DOWN,
    LSHIFT, RSHIFT, LCTRL, RCTRL
    // Add more as needed
};

enum class MouseButton {
    LEFT,
    MIDDLE,
    RIGHT,
    UNKNOWN
};

struct KeyEvent {
    KeyCode key = KeyCode::UNKNOWN;
    bool repeat = false;
};

struct MouseButtonEvent {
    MouseButton button = MouseButton::UNKNOWN;
    int x = 0;
    int y = 0;
    int clicks = 0;
};

struct MouseMotionEvent {
    int x = 0;
    int y = 0;
    int xrel = 0;
    int yrel = 0;
};

struct PollEvent {
    PollStatus type = PollStatus::NONE;
    KeyEvent key = {};
    MouseButtonEvent mouseButton = {};
    MouseMotionEvent mouseMotion = {};
};