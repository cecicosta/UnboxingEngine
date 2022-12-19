project(UnboxingEngineSrc)

set(BINARY_FILENAME unboxing_engine_src)
include_directories(
        ../include
)
set(SOURCES
        ${SOURCES}
        Camera.cpp
        MeshBuffer.cpp
        Quaternion.cpp
        UVector.cpp
        SceneNode.cpp
        SceneComposite.cpp
        BoundingBox.cpp
        BoundingBox2D.cpp
        internal_components/RenderComponent.cpp
        internal_components/TransformComponent.cpp

        #collisionutil.cpp
        #../include/internal_components/IColliderComponent.h 
        #../include/internal_components/BoundingBoxColliderComponent.h 
        #internal_components/IBoundingBoxColliderComponent.cpp
)
set(HEADERS
        ${HEADERS}
        ../include/Camera.h
        ../include/IEngine.h
        ../include/EventDispatcher.h
        ../include/CoreEvents.h
        ../include/MeshPrimitivesUtils.h
        ../include/Matrix.h
        ../include/MathUtils.h
        ../include/MeshBuffer.h
        ../include/Quaternion.h
        ../include/CustomTypes.h
        ../include/IComposite.h
        ../include/UVector.h
        ../include/internal_components/TransformComponent.h
        ../include/SceneComposite.h
        ../include/SceneNode.h
        ../include/texture.h
        ../include/BoundingBox.h
        ../include/material.h
        ../include/BoundingBox2D.h
        #../include/collisionutil.h
        )

message("Source files: ${SOURCES}")
add_library(${BINARY_FILENAME}_lib STATIC ${SOURCES} "internal_components/RenderComponent.cpp" "systems/CollisionSystem.cpp")
target_sources(${BINARY_FILENAME}_lib PUBLIC ${HEADERS} ${SOURCES})
