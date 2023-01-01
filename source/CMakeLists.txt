project(UnboxingEngineSrc)

set(BINARY_FILENAME unboxing_engine_src)
include_directories(
        ../include
)
set(SOURCES
        ${SOURCES}
        internal_components/RenderComponent.cpp
        internal_components/TransformComponent.cpp
        internal_components/BoundingBox2DColliderComponent.cpp
        internal_components/SegmentColliderComponent.cpp
        internal_components/RayColliderComponent.cpp
        Camera.cpp
        MeshBuffer.cpp
        Quaternion.cpp
        UVector.cpp
        SceneNode.cpp
        SceneComposite.cpp
        BoundingBox.cpp
        BoundingBox2D.cpp
        algorithms/CollisionAlgorithms.cpp
        systems/CollisionSystem.cpp
        #collisionutil.cpp
)
set(HEADERS
        ${HEADERS}
        ../include/internal_components/IRenderComponent.h
        ../include/internal_components/RenderComponent.h
        ../include/internal_components/IColliderComponent.h 
        ../include/internal_components/BoundingBox2DColliderComponent.h
        ../include/internal_components/SegmentColliderComponent.h
        ../include/internal_components/RayColliderComponent.h
        ../include/internal_components/TransformComponent.h
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
        ../include/UVector.h
        ../include/SceneComposite.h
        ../include/SceneNode.h
        ../include/texture.h
        ../include/BoundingBox.h
        ../include/material.h
        ../include/BoundingBox2D.h
        ../include/algorithms/CollisionAlgorithms.h
        ../include/systems/CollisionSystem.h
        ../include/systems/IRenderSystem.h
        ../include/systems/IInputSystem.h
        #../include/collisionutil.h
        )

message("Source files: ${SOURCES}")
add_library(${BINARY_FILENAME}_lib STATIC ${HEADERS} ${SOURCES})
target_sources(${BINARY_FILENAME}_lib PUBLIC ${HEADERS} ${SOURCES})
