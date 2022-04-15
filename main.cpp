#include "GLAux.h"

#include <GL/glew.h>
#include <MeshPrimitivesUtils.h>

#include <cstdio>
#include <iostream>

#include <string>

typedef float t_mat4x4[16];

static inline void mat4x4_ortho(t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar) {
#define T(a, b) (a * 4 + b)

    out[T(0, 0)] = 2.0f / (right - left);
    out[T(0, 1)] = 0.0f;
    out[T(0, 2)] = 0.0f;
    out[T(0, 3)] = 0.0f;

    out[T(1, 1)] = 2.0f / (top - bottom);
    out[T(1, 0)] = 0.0f;
    out[T(1, 2)] = 0.0f;
    out[T(1, 3)] = 0.0f;

    out[T(2, 2)] = -2.0f / (zfar - znear);
    out[T(2, 0)] = 0.0f;
    out[T(2, 1)] = 0.0f;
    out[T(2, 3)] = 0.0f;

    out[T(3, 0)] = -(right + left) / (right - left);
    out[T(3, 1)] = -(top + bottom) / (top - bottom);
    out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
    out[T(3, 3)] = 1.0f;

#undef T
}

int main(int argc, char *argv[]) {

    GLAux engine(640, 480, 32);
    engine.CreateWindow(true);

    glDisable(GL_DEPTH_TEST);

    auto cube = primitive_utils::Cube();
     engine.CreateArray(cube->vertices, cube->nvertices * 3, cube->triangles, cube->nfaces * 3);

    while (!engine.HasQuit()) {
        glClear(GL_COLOR_BUFFER_BIT);
        engine.OnInput();

        engine.camera->mTransformation = Matrix::rotationMatrix(1, vector3D(0, 1, 0)) * engine.camera->mTransformation;
        engine.camera->mTransformation = Matrix::rotationMatrix(1, vector3D(1, 0, 0)) * engine.camera->mTransformation;
        glUniformMatrix4fv(glGetUniformLocation(engine.program, "u_projection_matrix"), 1, GL_FALSE, engine.camera->mTransformation.getMatrixGL());
       // glBindVertexArray(engine.vao);

        glDrawElements(GL_TRIANGLES, cube->nfaces*3, GL_UNSIGNED_INT, nullptr);
        engine.RenderCanvas();
    }

    engine.Release();

    return 0;
}
