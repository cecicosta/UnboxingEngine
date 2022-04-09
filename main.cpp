#include <cmath>
#include <iostream>

#include "GLAux.h"
#include "SDL_timer.h"

#include <GL/glew.h>

void drawTriangle() {
    GLAux::vertices = std::vector<float>({-0.5f, -0.5f, 0.0f,
                                          0.5f, -0.5f, 0.0f,
                                          0.0f, 0.5f, 0.0f});
}

int main(int arg, char *argv[]) {

    GLAux::Init(true);

    drawTriangle();
    GLAux::CreateArray();

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    while (!GLAux::quit) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GLAux::Controller();

        glUseProgram(GLAux::program);
        glBindVertexArray(GLAux::vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(GLAux::screen);
        SDL_Delay(1);
    }
    GLAux::ReleaseCurrentGLContext();
    return 0;
}
