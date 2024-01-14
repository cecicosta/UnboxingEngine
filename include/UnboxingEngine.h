#pragma once

#include <unordered_map>

#include "Camera.h"
#include "IEngine.h"
#include "texture.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "UVector.h"
#include "EventDispatcher.h"
#include "systems/CollisionSystem.h"
#include "systems/IRenderSystem.h"
#include "systems/IInputSystem.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>
#include <optional>
#include <iostream>

#define L_BUTTON 0
#define R_BUTTON 1

class SDL_Surface;
union SDL_Event;
class SDL_Rect;
class SDL_Window;

namespace unboxing_engine {
class CMeshBuffer;
class CSceneComposite;

class CCore : public IEngine
    , public CEventDispatcher
    , public UListener<systems::ICollisionEvent, core_events::IMouseInputEvent, core_events::IKeyboardInputEvent> {

public:
    CCore(uint32_t width, uint32_t height, uint32_t bpp);
    ~CCore() override = default;

    /// <summary>
    ///Create new window and setup view
    /// </summary>
    void CreateWindow();

    /// <summary>
    ///Initialize engine subsystems
    /// </summary>
    void Start() override;

    /// <summary>
    /// 
    /// </summary>
    void Run() override;

    /// <summary>
    /// 
    /// </summary>
    /// <param name="listener"></param>
    void RegisterEventListener(UListener<>& listener);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="listener"></param>
    void UnregisterEventListener(const UListener<>& listener);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="sceneComposite"></param>
    void RegisterSceneElement(CSceneComposite & sceneComposite);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="sceneComposite"></param>
    void UnregisterSceneElement(const CSceneComposite &sceneComposite);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="id"></param>
    /// <returns></returns>
    CSceneComposite *GetSceneElement(int id) const;

    /// <summary>
    ///Destroy opengl context, window and finish SDL subsystems
    /// </summary>
    void Release();

    /// <summary>
    /// 
    /// </summary>
    /// <param name="c1"></param>
    /// <param name="c2"></param>
    /// <param name="result"></param>
    void OnCollisionEvent(const IColliderComponent &c1, const IColliderComponent &c2, const algorithms::SCollisionResult<float, 3> &result) override; 

    /// From IMouseInputEvent 
    void OnMouseInputtEvent(const core_events::SCursor &cursor) override;

    // From IKeyboardInputEvent
    void OnKeyboardInputtEvent(const core_events::SKeyboard &keyboardState) override;
private:
    ///Register object to interact with the basic engine systems throught its existing components
    //void WritePendingRenderData();
    ///Rendering routines
    void Render() override;
    ///Start rendering
    void RenderCanvas();
    ///Capture and process user inputs
    void OnInput() override;

    ///Method used for debug purposes. Allows to control the camera and fly around the 3D space.
    ///The control keys used are wasd for movement and mouse for direction.
    void UpdateFlyingController(const core_events::SCursor& cursor);

    ///Captures the window y button click
    [[nodiscard]] bool HasQuit() const { return quit; }

    std::unique_ptr<Camera> camera;
    std::uint32_t BPP = 32;

    ///
    std::unordered_map<int, CSceneComposite*> mRenderQueue;
    std::vector<CSceneComposite*> mPendingWriteQueue;

    //Handle objects with a ICollisionComponent registered
    systems::CollisionSystem mCollisionSystem;
    std::unique_ptr<systems::IRenderSystem> mRenderSystem;
    std::unique_ptr<systems::IInputSystem> mInputSystem;

    bool quit = false;
};

}