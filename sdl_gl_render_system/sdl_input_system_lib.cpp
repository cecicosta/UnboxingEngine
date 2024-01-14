#include "sdl_input_system_lib.h"

#include <SDL.h>

namespace {
const int L_BUTTON = 0;
const int R_BUTTON = 1;
}

namespace unboxing_engine::systems {

namespace {

inline static bool OnMouseInput(const SDL_Event& event, core_events::SCursor& cursor) {
    cursor.scrolling = 0;
    bool event_detected = false;
    switch (event.type) {
        case SDL_MOUSEMOTION:
            if (cursor.cursorState[L_BUTTON] == 1) {
                cursor.draggingX = event.motion.x - cursor.buttonPressedX;
                cursor.draggingY = event.motion.y - cursor.buttonPressedY;
            }
            cursor.x = event.motion.x;
            cursor.y = event.motion.y;
            event_detected = true;
            break;
        case SDL_MOUSEBUTTONDOWN:

            if (event.button.button == SDL_BUTTON_LEFT && !cursor.isButtonPressed) {
                cursor.cursorState[L_BUTTON] = 1;
                cursor.buttonPressedX = event.motion.x;
                cursor.buttonPressedY = event.motion.y;
            }
            if (event.button.button == SDL_BUTTON_RIGHT && !cursor.isButtonPressed) {
                cursor.cursorState[R_BUTTON] = 1;
                cursor.buttonPressedX = event.motion.x;
                cursor.buttonPressedY = event.motion.y;
            }
            cursor.isButtonPressed = true;

            if (event.type == SDL_MOUSEWHEEL) {
                cursor.scrolling = event.wheel.y;
            }
            event_detected = true;
            break;
        case SDL_MOUSEBUTTONUP:
            cursor.draggingX = 0;
            cursor.draggingY = 0;
            cursor.draggingSpeedX = 0;
            cursor.draggingSpeedY = 0;
            cursor.isButtonPressed = false;

            if (event.button.button == SDL_BUTTON_LEFT && cursor.cursorState[L_BUTTON] == 1) {
                cursor.cursorState[L_BUTTON] = 0;
            }
            if (event.button.button == SDL_BUTTON_RIGHT && cursor.cursorState[R_BUTTON] == 1) {
                cursor.cursorState[R_BUTTON] = 0;
            }
            event_detected = true;
            break;
    }

    cursor.draggingSpeedX = static_cast<float>(cursor.draggingX) * 180.f / 400.f;
    cursor.draggingSpeedY = static_cast<float>(cursor.draggingY) * 180.f / 300.f;
    return event_detected;
}

inline static bool OnKeyboardInput(const SDL_Event& event, core_events::SKeyboard& keyboard) {
    if(event.type == SDL_KEYDOWN) {
        auto keyState = SDL_GetKeyboardState(nullptr);
        keyboard.key = core_events::SKeyboard::EKey::NONE;
        if (keyState) {
            if (keyState[SDL_SCANCODE_RIGHT] == 1) {
                keyboard.key = core_events::SKeyboard::EKey::KEY_RIGHT;            
            }
            if (keyState[SDL_SCANCODE_LEFT] == 1) {
                keyboard.key = core_events::SKeyboard::EKey::KEY_LEFT;
            }
            if (keyState[SDL_SCANCODE_U] == 1) {
                keyboard.key = core_events::SKeyboard::EKey::KEY_UP;
            }
            if (keyState[SDL_SCANCODE_DOWN] == 1) {
                keyboard.key = core_events::SKeyboard::EKey::KEY_DOWN;
            }
            if (keyState[SDL_SCANCODE_S] == 1) {
                keyboard.key = core_events::SKeyboard::EKey::KEY_S;
            }
            if (keyState[SDL_SCANCODE_W] == 1) {
                keyboard.key = core_events::SKeyboard::EKey::KEY_W;
            }
            if (keyState[SDL_SCANCODE_ESCAPE] == 1) {
                keyboard.key = core_events::SKeyboard::EKey::KEY_ESC;
            }
        }

    }
    return keyboard.key != core_events::SKeyboard::EKey::NONE;
}

} //namespace


CSDLInputSystem::~CSDLInputSystem() = default;

void CSDLInputSystem::OnInput() {
    SDL_Event event;
    bool event_detected = false;
    while (SDL_PollEvent(&event)) {
        if (OnMouseInput(event, mCursor)) {
            for (auto &&listener: GetListeners<core_events::IMouseInputEvent>()) {
                listener->OnMouseInputtEvent(mCursor);
            }
        }
        if (OnKeyboardInput(event, mKeyboard)) {
            for (auto &&listener: GetListeners<core_events::IKeyboardInputEvent>()) {
                listener->OnKeyboardInputtEvent(mKeyboard);
            }
        }
    }
}


} //namespace unboxing_engine::systems
