#define GL_SILENCE_DEPRECATION
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "spdlog/spdlog.h"
#include "stb_image.h"

static float alpha = 0.2;
unsigned int shader;

static const char* kVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec2 pos;
    layout (location = 1) in vec2 coord;
    out vec2 textCoord;
    uniform mat4 matrix;
    void main() {
        gl_Position = matrix * vec4(pos, 0., 1.);
        textCoord = coord;
    }
)";

static const char* kFragmentShader = R"(
    #version 330 core
    in vec2 textCoord;
    uniform sampler2D tex0;
    uniform sampler2D tex1;
    uniform float alpha;
    out vec4 color;
    void main() {
        color = mix(texture(tex0, textCoord), texture(tex1, textCoord), alpha);
    }
)";

// static const char* kFragmentShader = R"(
//     #version 330 core
//     in vec2 textCoord;
//     uniform sampler2D tex0;
//     uniform sampler2D tex1;
//     out vec4 color;
//     void main() {
//         color = mix(texture(tex0, textCoord),
//         texture(tex1, vec2(1 - textCoord.x, textCoord.y)), 0.2);
//     }
// )";

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        alpha -= 0.1;
    } else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        alpha += 0.1;
    }
    alpha = std::max<float>(alpha, .0);
    alpha = std::min<float>(alpha, 1.0);
    glUseProgram(shader);
    glUniform1f(glGetUniformLocation(shader, "alpha"), alpha);
}

static int buildShader() {
    uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &kVertexShader, NULL);
    glCompileShader(vs);
    GLint succ;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &succ);
    if (!succ) {
        char infoLog[512] = {0};
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        spdlog::critical(infoLog);
    }
    assert(succ);

    succ = false;
    uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &kFragmentShader, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &succ);
    if (!succ) {
        char infoLog[512] = {0};
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        spdlog::critical(infoLog);
    }
    assert(succ);

    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    return program;
}

static void framebuffer_size_callback(GLFWwindow* window, int width,
                                      int height) {
    glViewport(0, 0, width, height);
}

int texture() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "good", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    shader = buildShader();

    uint32_t vbo, ebo, vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    float verties[] = {
        -.5, .5, 0, 1, .5, .5, 1, 1, .5, -.5, 1, 0, -.5, -.5, 0, 0,
    };

    unsigned int indicies[] = {
        0, 1, 2, 2, 3, 0,
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verties), verties, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies,
                 GL_STATIC_DRAW);

    uint32_t texture;
    {
        int width, height, channels;
        unsigned char* data =
            stbi_load("container.jpg", &width, &height, &channels, 0);
        assert(data && width && height);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    uint32_t texture1;
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        unsigned char* data =
            stbi_load("awesomeface.png", &width, &height, &channels, 0);
        assert(data && width && height);

        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    glUseProgram(shader);
    glUniform1i(glGetUniformLocation(shader, "tex0"), 0);
    glUniform1i(glGetUniformLocation(shader, "tex1"), 1);
    glUniform1f(glGetUniformLocation(shader, "alpha"), .2);

    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) {

        glClearColor(.5, .5, .5, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture1);

        glUseProgram(shader);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(.5, -.5, .0));
        trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0, 0, 1));
        glUniformMatrix4fv(glGetUniformLocation(shader, "matrix"), 1, GL_FALSE,
                           glm::value_ptr(trans));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        trans = glm::mat4(1.0f);
        float scale = sin((float)glfwGetTime());
        trans = glm::scale(trans, glm::vec3(scale, scale, 1));
        trans = glm::translate(trans, glm::vec3(-.5, .5, .0));
        glUniformMatrix4fv(glGetUniformLocation(shader, "matrix"), 1, GL_FALSE,
                           glm::value_ptr(trans));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}