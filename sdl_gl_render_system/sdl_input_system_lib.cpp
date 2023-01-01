#include "sdl_input_system_lib.h"

#include <SDL.h>

namespace {
const int L_BUTTON = 0;
const int R_BUTTON = 1;
}

unboxing_engine::systems::CSDLInputSystem::~CSDLInputSystem() {
}

void unboxing_engine::systems::CSDLInputSystem::OnInput() {
    if (OnMouseInput()) {
        for (auto &&listener: GetListeners<core_events::IMouseInputEvent>()) {
            listener->OnMouseInputtEvent(mCursor);
        }
    }
    if (OnKeyboardInput()) {
        for (auto &&listener: GetListeners<core_events::IMouseInputEvent>()) {
            listener->OnMouseInputtEvent(mCursor);
        }
    }
}

bool unboxing_engine::systems::CSDLInputSystem::OnMouseInput() {
    SDL_Event event;
    bool event_detected = false;
    mCursor.scrolling = 0;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                if (mCursor.cursorState[L_BUTTON] == 1) {
                    mCursor.draggingX = event.motion.x - mCursor.buttonPressedX;
                    mCursor.draggingY = event.motion.y - mCursor.buttonPressedY;
                }
                mCursor.x = event.motion.x;
                mCursor.y = event.motion.y;
                event_detected = true;
                break;
            case SDL_MOUSEBUTTONDOWN:

                if (event.button.button == SDL_BUTTON_LEFT && !mCursor.isButtonPressed) {
                    mCursor.cursorState[L_BUTTON] = 1;
                    mCursor.buttonPressedX = event.motion.x;
                    mCursor.buttonPressedY = event.motion.y;
                }
                if (event.button.button == SDL_BUTTON_RIGHT && !mCursor.isButtonPressed) {
                    mCursor.cursorState[R_BUTTON] = 1;
                    mCursor.buttonPressedX = event.motion.x;
                    mCursor.buttonPressedY = event.motion.y;
                }
                mCursor.isButtonPressed = true;

                if (event.type == SDL_MOUSEWHEEL) {
                    mCursor.scrolling = event.wheel.y;
                }
                event_detected = true;
                break;
            case SDL_MOUSEBUTTONUP:
                mCursor.draggingX = 0;
                mCursor.draggingY = 0;
                mCursor.draggingSpeedX = 0;
                mCursor.draggingSpeedY = 0;
                mCursor.isButtonPressed = false;

                if (event.button.button == SDL_BUTTON_LEFT && mCursor.cursorState[L_BUTTON] == 1) {
                    mCursor.cursorState[L_BUTTON] = 0;
                }
                if (event.button.button == SDL_BUTTON_RIGHT && mCursor.cursorState[R_BUTTON] == 1) {
                    mCursor.cursorState[R_BUTTON] = 0;
                }
                event_detected = true;
                break;
            case SDL_QUIT:
                quit = true;
        }
    }

    mCursor.draggingSpeedX = static_cast<float>(mCursor.draggingX) * 180.f / 400.f;
    mCursor.draggingSpeedY = static_cast<float>(mCursor.draggingY) * 180.f / 300.f;
    return event_detected;
}

bool unboxing_engine::systems::CSDLInputSystem::OnKeyboardInput() {
    keyState = SDL_GetKeyboardState(nullptr);
    mKeyboard.key = core_events::SKeyboard::EKey::NONE;
    if (keyState) {
        if (keyState[SDLK_RIGHT]) {
            mKeyboard.key = core_events::SKeyboard::EKey::KEY_RIGHT;            
        }
        if (keyState[SDLK_LEFT]) {
            mKeyboard.key = core_events::SKeyboard::EKey::KEY_LEFT;
        }
        if (keyState[SDLK_UP]) {
            mKeyboard.key = core_events::SKeyboard::EKey::KEY_UP;
        }
        if (keyState[SDLK_DOWN]) {
            mKeyboard.key = core_events::SKeyboard::EKey::KEY_DOWN;
        }
        if (keyState[SDLK_o]) {
            mKeyboard.key = core_events::SKeyboard::EKey::KEY_S;
        }
        if (keyState[SDLK_i]) {
            mKeyboard.key = core_events::SKeyboard::EKey::KEY_W;
        }
    }

    if (keyState[SDLK_ESCAPE]) {
        mKeyboard.key = core_events::SKeyboard::EKey::KEY_ESC;
    }
    return mKeyboard.key != core_events::SKeyboard::EKey::NONE;
}
