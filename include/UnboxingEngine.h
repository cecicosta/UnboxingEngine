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
    , public UListener<systems::ICollisionEvent> {
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
private:
    void WriteGeometryData(systems::SRenderContextHandle &renderContext, unboxing_engine::CMeshBuffer &meshBuffer);
    ///Register object to interact with the basic engine systems throught its existing components
    void WritePendingRenderData();
    ///Rendering routines
    void Render() override;
    ///Start rendering
    void RenderCanvas();
    ///Capture and process user inputs
    void OnInput() override;

    void CreateView() const;
    void CreateBasicShader(const char* vertexShaderSrc, const char* fragmentShaderSrc);
    
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

    /// <summary>
    /// 
    /// </summary>
    /// <param name="context"></param>
    void ReleaseRenderData(systems::SRenderContextHandle &context);

    ///Retrieve and log opengl errors
    static void GetError() ;

    ///Captures the window y button click
    [[nodiscard]] bool HasQuit() const { return quit; }

    std::unique_ptr<Camera> camera;
    std::uint32_t BPP = 32;

    ///Basic shader handler
    std::unique_ptr<systems::SShaderHandle> program;
    ///
    std::vector<systems::SRenderContextHandle> mRenderQueue;
    std::vector<systems::SRenderContextHandle> mPendingWriteQueue;

    //SDL input event handling
    SDL_Event event{};

    core_events::SCursor mCursor;

    //Handle objects with a ICollisionComponent registered
    systems::CollisionSystem mCollisionSystem;
    std::unique_ptr<systems::IRenderSystem> mRenderSystem;

    ///Keyboard input attributes
    std::uint8_t const *keyState = nullptr;

    bool quit = false;
};

}