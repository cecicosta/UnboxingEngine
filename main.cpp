#include "CatUnboxerEngine.h"

#include <GL/glew.h>
#include <MeshPrimitivesUtils.h>


int main(int argc, char *argv[]) {

    CCatUnboxerEngine engine(640, 480, 32);
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
