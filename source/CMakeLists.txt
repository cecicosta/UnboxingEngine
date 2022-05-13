project(UnboxingEngineSrc)

set(BINARY_FILENAME unboxing_engine_src)
include_directories(
        ../include
)
set(SOURCES
        ${SOURCES}
        ./Composite.cpp
        ./Camera.cpp
        ./mathUtil.cpp
        Matrix.cpp
        ./meshbuffer.cpp
        Quaternion.cpp
        Vector3Df.cpp
        ../include/TransformComponent.h)
set(HEADERS
        ${HEADERS}
        ../include/Camera.h
        ../include/IEngine.h
        ../include/EventDispatcher.h
        ../include/CoreEvents.h
        ../include/mathUtil.h
        ../include/MeshPrimitivesUtils.h
        ../include/Matrix.h
        ../include/meshbuffer.h
        ../include/Quaternion.h
        ../include/CustomTypes.h
        ../include/Composite.h
        ../include/texture.h
        ../include/Vector3Df.h)
message("Source files: ${SOURCES}")
add_library(${BINARY_FILENAME}_lib STATIC ${SOURCES})
target_sources(${BINARY_FILENAME}_lib PUBLIC ${HEADERS} ${SOURCES})
