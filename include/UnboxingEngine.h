#pragma once

#include <unordered_map>

#include "SDL_events.h"
#include "SDL_timer.h"
#include "SDL_video.h"

#include "Camera.h"
#include "IEngine.h"
#include "Vector3d.h"
#include "matrix.h"
#include "quaternion.h"
#include "sphere.h"
#include "texture.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

#include <EventDispatcher.h>

#define L_BUTTON 0
#define R_BUTTON 1

class SDL_Surface;
union SDL_Event;
class SDL_Rect;
class SDL_Window;
class CMeshBuffer;

namespace unboxing_engine {

class CCore : public IEngine, public CEventDispatcher {
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
    void UnregisterEventListener(UListener<>& listener);
    ///
    void RegisterSceneElement(const CMeshBuffer& mesh);
    ///Writes objects geometry to be rendered
    void WritePendingRenderData();

    ///Destroy opengl context, window and finish SDL subsystems
    void Release();
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

    ///Retrieve and log opengl errors
    static void GetError() ;

    ///Captures the window y button click
    [[nodiscard]] bool HasQuit() const { return quit; }

    ///Basic shader handler
    std::uint32_t program = -1;

    ///Rendering opengl buffer handlers
    struct SRenderContext {
        explicit SRenderContext(const CMeshBuffer& mesh) : mMeshBuffer(&mesh) {}
        std::uint32_t vao = -1; //Refers to the whole render context, including geometry, shaders and parameters
        std::uint32_t vbo = -1; //Buffer handler for geometry
        std::uint32_t ebo = -1; //Buffer handler for geometry vertex indices
        const CMeshBuffer *mMeshBuffer = nullptr;
    };
    std::vector<SRenderContext> mRenderQueue;
    std::vector<SRenderContext> mPendingWriteQueue;

    //SDL input event handling
    SDL_Event event{};

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
    SCursor mCursor;

    ///Keyboard input attributes
    std::uint8_t const *keyState = nullptr;

    ///Window and opengl handlers
    SDL_Window *mWindow = nullptr;
    SDL_GLContext mGLContext = nullptr;

    bool quit = false;
    std::uint32_t BPP = 32;
};

}