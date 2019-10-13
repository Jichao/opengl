#define GL_SILENCE_DEPRECATION
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

static const char* kVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec2 pos;
    layout (location = 1) in vec2 coord;
    out vec2 textureCoord;
    void main() {
        gl_Position = vec4(pos, 0., 1.);
        textureCoord = coord;
    }
)";

static const char* kFragmentShader = R"(
    #version 330 core
    in vec2 textureCoord;
    uniform sampler2D tex0;
    out vec4 color;
    void main() {
        color = texture(tex0, textureCoord);
        // color = vec4(1., 0., 0., 1.);
    }
)";

static int buildShader()
{
    uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &kVertexShader, NULL);
    glCompileShader(vs);
    GLint succ;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &succ);
    assert(succ);

    uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &kFragmentShader, NULL);
    glCompileShader(fs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &succ);
    assert(succ);

    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    return program;
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int texture() 
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "good", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    // glViewport(0, 0, 800, 600);

    uint32_t shader = buildShader();

    uint32_t vbo, ebo, vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    float verties[] = {
        -.5, .5, 0, 1, 
        .5, .5, 1, 1,
        .5, -.5, 1, 0,
        -.5, -.5, 0, 0,
    };

    unsigned int indicies[] = {
        0, 1, 2,
        2, 3, 0,
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verties), verties, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

    int width, height, channels;
    unsigned char* data = stbi_load("container.jpg", &width, &height, &channels, 0);
    assert(data && width && height);

    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(.5, .5, .5, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}