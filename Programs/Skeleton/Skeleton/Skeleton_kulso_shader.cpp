#include "framework.h"

const char *const vertexSource = R"(
#version 330
precision highp float;

layout(location = 0) in vec4 position;

void main() {
    gl_Position = position;
}
)";

const char *const fragmentSource = R"(
#version 330
precision highp float;

out vec4 outColor;

void main() {
    outColor = vec4(1, 0, 0, 1);
}
)";

GPUProgram gpuProgram(false);
unsigned int vao;
int frame = 0;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

// LEADAS ELOTT VEDD KI
#include <fstream>
#include <sstream>
// VEGE

void onDisplay() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // LEADAS ELOTT VEDD KI
    
    std::string newVertexSrc;
    std::string newFragmentSrc;
    std::string line;
    std::ifstream vfile("vertex.vert");
    while (std::getline(vfile, line)) {
        newVertexSrc += line + "\n";
    }
    vfile.close();
    std::ifstream ffile("fragment.frag");
    while (std::getline(ffile, line)) {
        newFragmentSrc += line + "\n";
    }
    ffile.close();
    
    GPUProgram gpuProgram(false);
    gpuProgram.create(newVertexSrc.c_str(), newFragmentSrc.c_str(), "outColor");
    
    // VEGE
    
    gpuProgram.setUniform((float)frame, "frame");
    frame++;
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glutSwapBuffers();
    glutPostRedisplay();
}

void onKeyboard(unsigned char key, int pX, int pY) {
}

void onKeyboardUp(unsigned char key, int pX, int pY) {
}

void onMouseMotion(int pX, int pY) {
}

void onMouse(int button, int state, int pX, int pY) {
}

void onIdle() {
}
