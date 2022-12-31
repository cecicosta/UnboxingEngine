#include "UnboxingEngine.h"

#include "SceneComposite.h"
#include <Camera.h>
#include <CoreEvents.h>
#include <MeshBuffer.h>

#include <algorithm>
#include <cstdio>
#include <iostream>

#include "internal_components/IColliderComponent.h"
#include "internal_components/RenderComponent.h"

#include "sdl_gl_render_system_lib.h"

namespace unboxing_engine {

static const char *vertex_shader_source =
    "#version 150 core\n"
    "in vec3 i_position;\n"
    "uniform vec4 color;\n"
    "out vec4 v_color;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_color = color;\n"
    "    gl_Position = u_projection_matrix * vec4( i_position.x, i_position.y, i_position.z, 1.0 );\n"
    "}\n";

static const char *fragment_shader_source =
    "#version 150\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";

CCore::CCore(uint32_t width, uint32_t height, uint32_t bpp)
    : camera(std::make_unique<Camera>(width, height, 70.0f, 1.f, 1.f))
    , BPP(bpp) 
    , mRenderSystem(std::make_unique<systems::COpenGLRenderSystem>()) {
    mCollisionSystem.RegisterListener(*this);
    RegisterEventListener(mCollisionSystem);
    RegisterEventListener(*mRenderSystem);
}

void CCore::Start() {
    if (!mRenderSystem->Initialize(camera->mWidth, camera->mHeight)) {

    }
    program = mRenderSystem->CompileShader(vertex_shader_source, fragment_shader_source);
    for (auto l: GetListeners<core_events::IStartListener>()) {
        l->OnStart();
    }
}
void CCore::Run() {
    while (!HasQuit()) {
        OnInput();

        for (auto &&listener: GetListeners<core_events::IUpdateListener>()) {
            listener->OnUpdate();
        }

        WritePendingRenderData();

        Render();
    }
}
void CCore::Render() {
    for (auto &&listener: GetListeners<core_events::IPreRenderListener>()) {
        listener->OnPreRender();
    }

    for (auto &&data: mRenderQueue) {
        auto sceneComposite = GetSceneElement(data.second->id);
        if (auto render = sceneComposite->GetComponent<IRenderComponent>()) {
            render->Render(*mRenderSystem);
        }
    }

    for (auto &&listener: GetListeners<core_events::IPostRenderListener>()) {
        listener->OnPostRender();
    }
}

void CCore::OnInput() {
    keyState = SDL_GetKeyboardState(nullptr);

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
                for (auto&& listener : GetListeners<core_events::IMouseInputEvent>()) {
                    listener->OnMouseInputtEvent(mCursor);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:

                if (CCore::event.button.button == SDL_BUTTON_LEFT && !mCursor.isButtonPressed) {
                    mCursor.cursorState[L_BUTTON] = 1;
                    mCursor.buttonPressedX = event.motion.x;
                    mCursor.buttonPressedY = event.motion.y;
                }
                if (CCore::event.button.button == SDL_BUTTON_RIGHT && !mCursor.isButtonPressed) {
                    mCursor.cursorState[R_BUTTON] = 1;
                    mCursor.buttonPressedX = event.motion.x;
                    mCursor.buttonPressedY = event.motion.y;
                }
                mCursor.isButtonPressed = true;

                if (CCore::event.type == SDL_MOUSEWHEEL) {
                    mCursor.scrolling = CCore::event.wheel.y;
                }
                for (auto &&listener: GetListeners<core_events::IMouseInputEvent>()) {
                    listener->OnMouseInputtEvent(mCursor);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                mCursor.draggingX = 0;
                mCursor.draggingY = 0;
                mCursor.draggingSpeedX = 0;
                mCursor.draggingSpeedY = 0;
                mCursor.isButtonPressed = false;

                if (CCore::event.button.button == SDL_BUTTON_LEFT && mCursor.cursorState[L_BUTTON] == 1) {
                    mCursor.cursorState[L_BUTTON] = 0;
                }
                if (CCore::event.button.button == SDL_BUTTON_RIGHT && mCursor.cursorState[R_BUTTON] == 1) {
                    mCursor.cursorState[R_BUTTON] = 0;
                }
                for (auto &&listener: GetListeners<core_events::IMouseInputEvent>()) {
                    listener->OnMouseInputtEvent(mCursor);
                }
                break;
            case SDL_QUIT:
                quit = true;
        }
    }

    mCursor.draggingSpeedX = static_cast<float>(mCursor.draggingX) * 180.f / 400.f;
    mCursor.draggingSpeedY = static_cast<float>(mCursor.draggingY) * 180.f / 300.f;

    if (keyState[SDLK_ESCAPE])
        quit = true;
}

void CCore::UpdateFlyingController() {
    Vector3f velocity;
    Vector3f rotation;
    if (keyState) {
        if (keyState[SDLK_RIGHT]) {
            velocity.x = -1;
        }
        if (keyState[SDLK_LEFT]) {
            velocity.x = 1;
        }
        if (keyState[SDLK_UP]) {
            velocity.y = -1;
        }
        if (keyState[SDLK_DOWN]) {
            velocity.y = 1;
        }
        if (keyState[SDLK_o]) {
            velocity.z = 1;
        }
        if (keyState[SDLK_i]) {
            velocity.z = -1;
        }
    }

    rotation.x = mCursor.draggingX * 180 / 400 - mCursor.draggingSpeedX;
    rotation.y = mCursor.draggingY * 180 / 300 - mCursor.draggingSpeedY;

    camera->FPSCamera(velocity, rotation);
}

Texture *CCore::LoadTexture(char *filename) {

    GLuint retval;
    SDL_Surface *sdlimage;
    void *raw;
    int w, h, bpp;
    Uint8 *srcPixel, *dstPixel;
    Uint32 truePixel;
    GLenum errorCode;

    //Pode substituir aki a forma de carregar a surface
    sdlimage = LoadSurfaceAlpha(filename);


    if (!sdlimage) {
        FILE *f;
        f = fopen("debug.txt", "w");
        fprintf(f, "SDL_Image load error: %s\n", IMG_GetError());
        fclose(f);
        return nullptr;
    }

    w = sdlimage->w;
    h = sdlimage->h;

    raw = (void *) malloc(w * h * 4);
    dstPixel = (Uint8 *) raw;

    SDL_LockSurface(sdlimage);

    bpp = sdlimage->format->BytesPerPixel;

    for (auto i = h - 1; i >= 0; i--) {
        for (auto j = 0; j < w; j++) {
            srcPixel = (Uint8 *) sdlimage->pixels + i * sdlimage->pitch + j * bpp;
            switch (bpp) {
                case 1:
                    truePixel = *srcPixel;
                    break;

                case 2:
                    truePixel = *(Uint16 *) srcPixel;
                    break;

                case 3:
                    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                        truePixel = srcPixel[0] << 16 | srcPixel[1] << 8 | srcPixel[2];
                    } else {
                        truePixel = srcPixel[0] | srcPixel[1] << 8 | srcPixel[2] << 16;
                    }
                    break;

                case 4:
                    truePixel = *(Uint32 *) srcPixel;
                    break;

                default:
                    printf("Image bpp of %d unusable\n", bpp);
                    SDL_UnlockSurface(sdlimage);
                    SDL_FreeSurface(sdlimage);
                    free(raw);
                    return nullptr;
            }

            SDL_GetRGBA(truePixel, sdlimage->format, &(dstPixel[0]), &(dstPixel[1]), &(dstPixel[2]), &(dstPixel[3]));
            dstPixel++;
            dstPixel++;
            dstPixel++;
            dstPixel++;
        }
    }

    SDL_UnlockSurface(sdlimage);
    SDL_FreeSurface(sdlimage);

    while (glGetError()) { ; }

    glGenTextures(1, &retval);
    glBindTexture(GL_TEXTURE_2D, retval);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    errorCode = glGetError();
    if (errorCode != 0) {
        if (errorCode == GL_OUT_OF_MEMORY) {
            printf("Out of texture memory!\n");
        }

        glDeleteTextures(1, &retval);
        free(raw);
        return nullptr;
    }

    //gluBuild2DMipmaps( GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (Uint8 *)raw);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (Uint8 *) raw);
    glGenerateMipmap(retval);
    auto *texture = new Texture();

    texture->imageData = (int *) raw;
    texture->bpp = bpp;
    texture->width = w;
    texture->height = h;
    texture->texID = retval;
    texture->type = GL_RGBA;

    errorCode = glGetError();
    if (errorCode != 0) {
        if (errorCode == GL_OUT_OF_MEMORY) {
            printf("Out of texture memory!\n");
        }

        glDeleteTextures(1, &retval);
        free(raw);
        return nullptr;
    }

    return texture;
}

Texture *CCore::CreateTextureFromSurface(SDL_Surface *sdlSurface) {

    GLuint retval;
    void *raw;
    int w, h, i, j, bpp;
    Uint8 *srcPixel, *dstPixel;
    Uint32 truePixel;
    GLenum errorCode;

    if (!sdlSurface) {
        printf("SDL_Image load error: %s\n", IMG_GetError());
        return nullptr;
    }

    w = sdlSurface->w;
    h = sdlSurface->h;

    raw = (void *) malloc(w * h * 4);
    dstPixel = (Uint8 *) raw;

    SDL_LockSurface(sdlSurface);

    bpp = sdlSurface->format->BytesPerPixel;

    for (i = h - 1; i >= 0; i--) {
        for (j = 0; j < w; j++) {
            srcPixel = (Uint8 *) sdlSurface->pixels + i * sdlSurface->pitch + j * bpp;
            switch (bpp) {
                case 1:
                    truePixel = *srcPixel;
                    break;

                case 2:
                    truePixel = *(Uint16 *) srcPixel;
                    break;

                case 3:
                    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                        truePixel = srcPixel[0] << 16 | srcPixel[1] << 8 | srcPixel[2];
                    } else {
                        truePixel = srcPixel[0] | srcPixel[1] << 8 | srcPixel[2] << 16;
                    }
                    break;

                case 4:
                    truePixel = *(Uint32 *) srcPixel;
                    break;

                default:
                    printf("Image bpp of %d unusable\n", bpp);
                    SDL_UnlockSurface(sdlSurface);
                    SDL_FreeSurface(sdlSurface);
                    free(raw);
                    return nullptr;
            }

            SDL_GetRGBA(truePixel, sdlSurface->format, &(dstPixel[0]), &(dstPixel[1]), &(dstPixel[2]), &(dstPixel[3]));
            dstPixel++;
            dstPixel++;
            dstPixel++;
            dstPixel++;
        }
    }

    SDL_UnlockSurface(sdlSurface);
    SDL_FreeSurface(sdlSurface);

    while (glGetError()) { ; }

    glGenTextures(1, &retval);
    glBindTexture(GL_TEXTURE_2D, retval);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    errorCode = glGetError();
    if (errorCode != 0) {
        if (errorCode == GL_OUT_OF_MEMORY) {
            printf("Out of texture memory!\n");
        }

        glDeleteTextures(1, &retval);
        free(raw);
        return nullptr;
    }

    //gluBuild2DMipmaps( GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (Uint8 *)raw);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (Uint8 *) raw);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);


    Texture *texture = (Texture *) malloc(sizeof(Texture));

    texture->imageData = (int *) raw;
    texture->bpp = bpp;
    texture->width = w;
    texture->height = h;
    texture->texID = retval;
    texture->type = GL_RGBA;

    errorCode = glGetError();
    if (errorCode != 0) {
        if (errorCode == GL_OUT_OF_MEMORY) {
            printf("Out of texture memory!\n");
        }

        glDeleteTextures(1, &retval);
        free(raw);
        return nullptr;
    }

    return texture;
}

void CCore::ApplySurface(int x, int y, SDL_Surface *origem, SDL_Surface *destino, SDL_Rect *clip = NULL) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    //Aplica a Imagem na mWindow.
    SDL_BlitSurface(origem, clip, destino, &offset);
}

SDL_Surface *CCore::LoadSurface(char *nome) {
    auto *newImage = IMG_Load(nome);
    auto *newImageOptimized = SDL_ConvertSurface(newImage, newImage->format, 0);

    //Define a color key
    Uint32 colorkey = SDL_MapRGB(newImageOptimized->format, 0, 0xFF, 0xFF);
    SDL_SetColorKey(newImageOptimized, SDL_TRUE, colorkey);

    SDL_FreeSurface(newImage);
    return newImageOptimized;
}

SDL_Surface *CCore::LoadSurfaceAlpha(char *nome) {
    auto *newImage = IMG_Load(nome);
    auto *newImageOptimized = SDL_ConvertSurfaceFormat(newImage, SDL_PIXELFORMAT_RGBA8888, 0);

    SDL_FreeSurface(newImage);

    return newImageOptimized;
}

SDL_Rect *CCore::CreateSurfaceClips(int linhas, int colunas, int width, int height) {
    int i, j, cont = 0;
    SDL_Rect *clip = nullptr;
    clip = (SDL_Rect *) malloc((linhas * colunas) * sizeof(SDL_Rect));
    for (i = 0; i < linhas; i++)
        for (j = 0; j < colunas; j++) {
            clip[cont].x = j * width;
            clip[cont].y = i * height;
            clip[cont].w = width;
            clip[cont++].h = height;
        }

    return clip;
}

void CCore::Release() {
    mRenderSystem.reset();
    for (auto &&listener: GetListeners<core_events::IReleaseListener>()) {
        listener->OnRelease();
    }
}

void CCore::OnCollisionEvent(const IColliderComponent &c1, const IColliderComponent &c2, const algorithms::SCollisionResult<float, 3> &result) {
    if (auto c1Composite = GetSceneElement(c1.GetSceneComposite()->id); auto listener = dynamic_cast<systems::IIntersectsEvent *>(c1Composite)) {
        listener->OnIntersects();
    }
   
    if (auto c2Composite = GetSceneElement(c2.GetSceneComposite()->id); auto listener = dynamic_cast<systems::IIntersectsEvent *>(c2Composite)) {
        listener->OnIntersects();
    }
}

void CCore::WritePendingRenderData() {
    for (auto &&data: mPendingWriteQueue) {
        auto render = data->GetComponent<IRenderComponent>();
        if (!render) {
            continue;
        }
        // For now, render are obligated to have a CMeshBuffer
        auto meshBuffer = render->GetMeshBuffer();
        renderContext.renderBufferHandle = mRenderSystem->WriteRenderBufferData(meshBuffer);
        mRenderQueue.emplace_back(renderContext);
    }
    mPendingWriteQueue.clear();
}

void CCore::ReleaseRenderData(systems::SRenderContextHandle &context) {
    mRenderSystem->EraseRenderBufferData(*context.renderBufferHandle);
}

void CCore::RegisterSceneElement(CSceneComposite &sceneComposite) {
    if (auto collider = sceneComposite.GetComponent<IColliderComponent>()) {
        mCollisionSystem.RegisterCollider(*collider);
    }
    if (auto listener = dynamic_cast<UListener<> *>(&sceneComposite)) {
        RegisterEventListener(*listener);
    }
    mPendingWriteQueue.emplace_back(SRenderContext(sceneComposite));
}

void CCore::UnregisterSceneElement(const CSceneComposite &sceneComposite) {
    if (auto collider = sceneComposite.GetComponent<IColliderComponent>()) {
        mCollisionSystem.UnregisterCollider(*collider);
    }

    if (auto listener = dynamic_cast<const UListener<>*>(&sceneComposite)) {
        UnregisterEventListener(*listener);
    }

    auto findRenderContext = [&sceneComposite](const SRenderContext &context) {
        if (context.mSceneComposite->id == sceneComposite.id) {
            return true;
        }
        return false;
    };
    {
        auto it = std::find_if(mPendingWriteQueue.begin(), mPendingWriteQueue.end(), findRenderContext);
        if (it != mPendingWriteQueue.end()) {
            mPendingWriteQueue.erase(it);
        }
    }
    {
        auto it = std::find_if(mRenderQueue.begin(), mRenderQueue.end(), findRenderContext);
        if (it != mRenderQueue.end()) {
            ReleaseRenderData(*it);
            mRenderQueue.erase(it);
        }
    }
}

CSceneComposite *CCore::GetSceneElement(int id) const {
    auto it = std::find_if(mRenderQueue.begin(), mRenderQueue.end(), [id](const SRenderContext &composite) {
        return composite.mSceneComposite->id == id;
    });
    if (it != mRenderQueue.end()) {
        return it->mSceneComposite;
    }
    return nullptr;
}

void CCore::RegisterEventListener(UListener<> &listener) {
    RegisterListener(listener);
}

void CCore::UnregisterEventListener(const UListener<>& listener) {
    UnregisterListener(listener);
}

//TODO: Find a better way to store and retrieve the components. The render cannot be retrieved from its common interface with the current method
CCore::SRenderContext::SRenderContext(CSceneComposite &sceneComposite)
    : mSceneComposite(&sceneComposite) {}
}//namespace unboxing_engine
