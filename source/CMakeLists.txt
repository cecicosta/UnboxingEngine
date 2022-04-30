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
        ./matrix.cpp
        ./meshbuffer.cpp
        ./quaternion.cpp
        Vector3d.cpp
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
        ../include/Vector3d.h)
message("Source files: ${SOURCES}")
add_library(${BINARY_FILENAME}_lib STATIC ${SOURCES})
target_sources(${BINARY_FILENAME}_lib PUBLIC ${HEADERS} ${SOURCES})
