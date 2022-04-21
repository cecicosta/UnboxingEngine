#pragma once

#include <EventDispatcher.h>
//
// Created by Cecilia Costa on 17/04/2022.
//

namespace unboxing_engine::core_events {
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
    class IInputListener {
    public:
        virtual ~IInputListener() = default;
        virtual void OnInput() = 0;
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

    class ICoreEventsListener : public IListener<IStartListener, IUpdateListener, IInputListener, IPreRenderListener, IPostRenderListener, IReleaseListener> {};
}// namespace unboxing_engine::core_events