#pragma once

#include <unordered_map>

#include "SDL_events.h"
#include "SDL_timer.h"
#include "SDL_video.h"

#include "Camera.h"
#include "IEngine.h"
#include "texture.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "UVector.h"
#include "algorithms/CollisionAlgorithms.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

#include "EventDispatcher.h"
#include "systems/CollisionSystem.h"


#define L_BUTTON 0
#define R_BUTTON 1

class SDL_Surface;
union SDL_Event;
class SDL_Rect;
class SDL_Window;

namespace unboxing_engine {
class CMeshBuffer;
class CSceneComposite;

class CCore : public IEngine, public CEventDispatcher, public systems::ICollisionEventLitener {
    ///Rendering opengl buffer handlers
    struct SRenderContext {
        explicit SRenderContext(CSceneComposite & sceneComposite);
        std::uint32_t vao = -1;//Refers to the whole render context, including geometry, shaders and parameters
        std::uint32_t vbo = -1;//Buffer handler for geometry
        std::uint32_t ebo = -1;//Buffer handler for geometry vertex indices
        CSceneComposite *mSceneComposite;
    };

public:
    CCore(uint32_t width, uint32_t height, uint32_t bpp);
    ~CCore() override = default;
    std::unique_ptr<Camera> camera;

    ///Create new window and setup view
    void CreateWindow();

    ///Initialize engine subsystems
    void Start() override;
    ///Run loop
    void Run() override;

    ///
    void RegisterEventListener(UListener<>& listener);
    ///
    void UnregisterEventListener(const UListener<>& listener);
    ///
    void RegisterSceneElement(CSceneComposite & sceneComposite);
    void UnregisterSceneElement(const CSceneComposite &sceneComposite);
    CSceneComposite *GetSceneElement(int id) const;
    ///Register object to interact with the basic engine systems throught its existing components
    void WritePendingRenderData();

    ///Destroy opengl context, window and finish SDL subsystems
    void Release();

    void OnCollisionEvent(const IColliderComponent &c1, const IColliderComponent &c2, const algorithms::SCollisionResult<float, 3> &result) override; 

private:
    ///Rendering routines
    void Render() override;
    ///Start rendering
    void RenderCanvas();
    ///Capture and process user inputs
    void OnInput() override;

    void CreateView() const;
    void CreateBasicShader();
    
    ///Load texture into the engine resources manager
    Texture *LoadTexture(char *filename);
    ///Convert um SDL_surface to opengl texture
    Texture *CreateTextureFromSurface(SDL_Surface *sdlSurface);

    ///Make a surface composition
    ///@param x horizontal left-to-right coordinate on @origin surface where @destine will be applied.
    ///@param y vertical top-to-bottom coordinate on @origin surface where @destine will be applied.
    ///@param origin existing surface to where @destine will be applied at the coordinates specified by @x and @y.
    ///@param destine existing surface to be applied at the specified coordinates into @origin.
    void ApplySurface(int x, int y, SDL_Surface *origin, SDL_Surface *destine, SDL_Rect *clip);
    ///Loads an image without alpha channel
    SDL_Surface *LoadSurface(char *nome);
    ///Loads an image with alpha channel
    SDL_Surface *LoadSurfaceAlpha(char *nome);

    ///Create an array of rect clips for splitting a surface into equally spaced sprites
    ///@param rows number or rows in which the sprite sheet is divided
    ///@param rows number or columns in which the sprite sheet is divided
    ///@param width of each the sprite. Applied to all sprites clips
    ///@param height of each sprite. Applied to all sprites clips
    SDL_Rect *CreateSurfaceClips(int rows, int cols, int width, int height);

    ///Method used for debug purposes. Allows to control the camera and fly around the 3D space.
    ///The control keys used are wasd for movement and mouse for direction.
    void UpdateFlyingController();

    ///
    void ReleaseRenderData(SRenderContext &context);

    ///Retrieve and log opengl errors
    static void GetError() ;

    ///Captures the window y button click
    [[nodiscard]] bool HasQuit() const { return quit; }

    ///Basic shader handler
    std::uint32_t program = -1;
    ///
    std::vector<SRenderContext> mRenderQueue;
    std::vector<SRenderContext> mPendingWriteQueue;

    //SDL input event handling
    SDL_Event event{};

    core_events::SCursor mCursor;

    //Handle objects with a ICollisionComponent registered
    systems::CollisionSystem mCollisionSystem;

    ///Keyboard input attributes
    std::uint8_t const *keyState = nullptr;

    ///Window and opengl handlers
    SDL_Window *mWindow = nullptr;
    SDL_GLContext mGLContext = nullptr;

    bool quit = false;
    std::uint32_t BPP = 32;
};

}