#pragma once

class IEngine {
public:
    virtual ~IEngine() = default;

    ///Initialize engine subsystems
    virtual void Start() = 0;
    ///Run loop
    virtual void Run() = 0;
    ///Rendering routines
    virtual void Render() = 0;
    ///Capture and process user inputs
    virtual void OnInput() = 0;
};
