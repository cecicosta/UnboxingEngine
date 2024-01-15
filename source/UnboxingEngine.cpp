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

#if !BUILD_WITH_EXTERNAL_LIBS
#include "sdl_gl_render_system_lib.h"
#include "sdl_input_system_lib.h"
#else 
#include "dummie_libs/sdl_gl_render_system_lib.h"
#include "dummie_libs/sdl_input_system_lib.h"
#endif


namespace unboxing_engine {


CCore::CCore(uint32_t width, uint32_t height, uint32_t bpp)
    : camera(std::make_unique<Camera>(width, height, 70.0f, 1.f, 1.f))
    , BPP(bpp) 
    , mRenderSystem(std::make_unique<systems::COpenGLRenderSystem>(*camera))
    , mInputSystem(std::make_unique<systems::CSDLInputSystem>()) {
    mCollisionSystem.RegisterListener(*this);
    RegisterEventListener(mCollisionSystem);
    RegisterEventListener(*mRenderSystem);

    mInputSystem->RegisterListener(dynamic_cast<UListener<core_events::IKeyboardInputEvent>&>(*this));
}

void CCore::Start() {
    if (!mRenderSystem->Initialize()) {
        std::cout << "Render system failed to initialize.\n" << std::endl;    
        quit = true;
        return;
    }
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

        //WritePendingRenderData();

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
            render->OnRender();
        }
    }

    for (auto &&listener: GetListeners<core_events::IPostRenderListener>()) {
        listener->OnPostRender();
    }
}

void CCore::OnInput() {
    mInputSystem->OnInput();
}

void CCore::UpdateFlyingController(const core_events::SCursor& cursor) {
    Vector3f velocity;
    Vector3f rotation;
    
    rotation.x = cursor.draggingX * 180 / camera->mWidth - cursor.draggingSpeedX;
    rotation.y = cursor.draggingY * 180 / camera->mHeight - cursor.draggingSpeedY;

    camera->FPSCamera(velocity, rotation);
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

void CCore::OnMouseInputtEvent(const core_events::SCursor &cursor) {
    UpdateFlyingController(cursor);
}

void unboxing_engine::CCore::OnKeyboardInputtEvent(const core_events::SKeyboard &keyboardState) {
    if(keyboardState.key == core_events::SKeyboard::EKey::KEY_ESC) {
        quit = true;
    }
}

void CCore::RegisterSceneElement(CSceneComposite &sceneComposite) {
    if (auto collider = sceneComposite.GetComponent<IColliderComponent>()) {
        mCollisionSystem.RegisterCollider(*collider);
    }
    if (auto listener = dynamic_cast<UListener<>*>(&sceneComposite)) {
        RegisterEventListener(*listener);
    }

    if (auto render = sceneComposite.GetComponent<IRenderComponent>()) {
        render->OnInitialize(*mRenderSystem);
    }

    if(auto inputListener = dynamic_cast<UListener<core_events::IMouseInputEvent>*>(&sceneComposite)) {
        mInputSystem->RegisterListener(*inputListener);
    }
    if(auto inputListener = dynamic_cast<UListener<core_events::IKeyboardInputEvent>*>(&sceneComposite)) {
        mInputSystem->RegisterListener(*inputListener);
    }

    mRenderQueue.try_emplace(sceneComposite.id, &sceneComposite);
}

void CCore::UnregisterSceneElement(const CSceneComposite &sceneComposite) {
    if (auto listener = dynamic_cast<const UListener<>*>(&sceneComposite)) {
        UnregisterEventListener(*listener);
    }

    if (auto collider = sceneComposite.GetComponent<IColliderComponent>()) {
        mCollisionSystem.UnregisterCollider(*collider);
    }

    if (auto it = mRenderQueue.find(sceneComposite.id); it != mRenderQueue.end()) {
        if(auto renderComponent = it->second->GetComponent<IRenderComponent>()) {
            renderComponent->ReleaseRenderContext();
        }
        mRenderQueue.erase(it);
    }
    
    if(auto inputListener = dynamic_cast<const UListener<core_events::IMouseInputEvent>*>(&sceneComposite)) {
        mInputSystem->UnregisterListener(*inputListener);
    }
    if(auto inputListener = dynamic_cast<const UListener<core_events::IKeyboardInputEvent>*>(&sceneComposite)) {
        mInputSystem->UnregisterListener(*inputListener);
    }
}

CSceneComposite *CCore::GetSceneElement(int id) const {
    auto it = mRenderQueue.find(id);
    if (it != mRenderQueue.end()) {
        return it->second;
    }
    return nullptr;
}

void CCore::RegisterEventListener(UListener<> &listener) {
    RegisterListener(listener);
}

void CCore::UnregisterEventListener(const UListener<>& listener) {
    UnregisterListener(listener);
}

}