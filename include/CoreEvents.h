#pragma once

#include <EventDispatcher.h>

namespace unboxing_engine::core_events {

struct SCursor {
    //Cursor input attributes
    int x = 0;
    int y = 0;
    int draggingX = 0;
    int draggingY = 0;
    float draggingSpeedX = 0;
    float draggingSpeedY = 0;
    int buttonPressedX = 0;
    int buttonPressedY = 0;
    bool isButtonPressed = false;
    int scrolling = -1;
    int cursorState[3]{0, 0, 0};
};

class IStartListener {
public:
    virtual ~IStartListener() = default;
    virtual void OnStart() = 0;
};
class IUpdateListener {
public:
    virtual ~IUpdateListener() = default;
    virtual void OnUpdate() = 0;
};
class IPreRenderListener {
public:
    virtual ~IPreRenderListener() = default;
    virtual void OnPreRender() = 0;
};
class IPostRenderListener {
public:
    virtual ~IPostRenderListener() = default;
    virtual void OnPostRender() = 0;
};
class IReleaseListener {
public:
    virtual ~IReleaseListener() = default;
    virtual void OnRelease() = 0;
};
class IMouseInputEvent {
public:
    virtual ~IMouseInputEvent() = default;
    virtual void OnMouseInputtEvent(const SCursor &cursor) = 0;
};

class ICoreEventsListener : public UListener<IStartListener, IUpdateListener, IPreRenderListener, IPostRenderListener, IReleaseListener> {};
}// namespace unboxing_engine::core_events