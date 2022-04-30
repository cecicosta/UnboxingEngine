project(UnboxingEngineSrc)

set(BINARY_FILENAME unboxing_engine_src)
include_directories(
        ../include
)
set(SOURCES
        ${SOURCES}
        ./Camera.cpp
        ./mathUtil.cpp
        ./matrix.cpp
        ./meshbuffer.cpp
        ./quaternion.cpp
        ./Composite.cpp
        ./vector3D.cpp
        )
set(HEADERS
        ${HEADERS}
        ../include/Camera.h
        ../include/IEngine.h
        ../include/EventDispatcher.h
        ../include/CoreEvents.h
        ../include/mathUtil.h
        ../include/MeshPrimitivesUtils.h
        ../include/matrix.h
        ../include/meshbuffer.h
        ../include/quaternion.h
        ../include/CustomTypes.h
        ../include/Composite.h
        ../include/texture.h
        ../include/vector3D.h)

add_library(${BINARY_FILENAME}_lib SHARED)
target_sources(${BINARY_FILENAME}_lib PUBLIC ${HEADERS} ${SOURCES})
