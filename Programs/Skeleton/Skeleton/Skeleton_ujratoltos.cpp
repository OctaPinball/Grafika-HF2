#include "framework.h"

#define SKELETON_CPP_RELATIVE_LOCATION "../src/Skeleton.cpp"

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
    outColor = vec4(0, 0, 0, 1);
}
)";

GPUProgram gpuProgram(false);
unsigned int vao;

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

// LEADÁS ELOTT VEDD KI
#include <fstream>
#include <sstream>
#include <algorithm>

static bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// VÉGE

void onDisplay() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // LEADÁS ELOTT VEDD KI
    std::ifstream skeleton(SKELETON_CPP_RELATIVE_LOCATION);
    
    std::string newVertexSrc;
    std::string newFragmentSrc;
    std::string line;
    int state = 0;
    bool finished = false;
    while (std::getline(skeleton, line) && !finished) {
        ltrim(line);
        switch (state) {
            case 0:
                if (line == "const char *const vertexSource = R\"(") {
                    state = 1;
                }
                break;
            case 1:
                if (line == ")\";") {
                    state = 2;
                } else {
                    newVertexSrc += line + "\n";
                }
                break;
            case 2:
                if (line == "const char *const fragmentSource = R\"(") {
                    state = 3;
                }
                break;
            case 3:
                if (line == ")\";") {
                    finished = true;
                } else {
                    newFragmentSrc += line + "\n";
                }
                break;
        }
    }
    
    gpuProgram.create(newVertexSrc.c_str(), newFragmentSrc.c_str(), "outColor");
    
    // VÉGE
    
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
