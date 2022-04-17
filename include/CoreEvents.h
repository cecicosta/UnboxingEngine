#pragma once

#include <EventDispatcher.h>
//
// Created by Cecilia Costa on 17/04/2022.
//

namespace unboxing_engine::core_events {
    class IStartListener {
    public:
        ~IStartListener() = default;
        virtual void OnStart() = 0;
    };
    class IUpdateListener {
    public:
        ~IUpdateListener() = default;
        virtual void OnUpdate() = 0;
    };
    class IInputListener {
    public:
        ~IInputListener() = default;
        virtual void OnInput() = 0;
    };
    class IPreRenderListener {
    public:
        ~IPreRenderListener() = default;
        virtual void OnPreRender() = 0;
    };
    class IPostRenderListener {
    public:
        ~IPostRenderListener() = default;
        virtual void OnPostRender() = 0;
    };
    class IReleaseListener {
    public:
        ~IReleaseListener() = default;
        virtual void OnRelease() = 0;
    };

    class ICoreEventsListener : public IListener<IStartListener, IUpdateListener, IInputListener, IPreRenderListener, IPostRenderListener, IReleaseListener> {};
}// namespace unboxing_engine::core_events