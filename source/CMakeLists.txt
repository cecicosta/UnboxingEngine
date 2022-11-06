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
        TransformComponent.cpp
        SceneNode.cpp
        SceneComposite.cpp
        collisionutil.cpp
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
        ../include/TransformComponent.h
        ../include/SceneComposite.h
        ../include/SceneNode.h
        ../include/texture.h
        ../include/collisionutil.h)

message("Source files: ${SOURCES}")
add_library(${BINARY_FILENAME}_lib STATIC ${SOURCES})
target_sources(${BINARY_FILENAME}_lib PUBLIC ${HEADERS} ${SOURCES})
