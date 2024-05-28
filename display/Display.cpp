//
// Created by SnowNF on 2024/5/28.
//

#include "glad.h"
#include "Display.h"
#include <GLFW/glfw3.h>
#include "log_helper.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>

void APIENTRY debugOutput(GLenum source,
                          GLenum type,
                          GLuint id,
                          GLenum severity,
                          GLsizei length,
                          const GLchar *message,
                          void *userParam) {
    // 忽略一些不重要的错误/警告代码
//    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cout << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cout << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cout << "Source: Other";
            break;
    }
    std::cout << std::endl;

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "Type: Other";
            break;
    }
    std::cout << std::endl;

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "Severity: notification";
            break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

class Shader {
public:
    unsigned int ID;

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char *vertexPath, const char *fragmentPath) {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure &e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << vertexPath << " " << fragmentPath
                      << std::endl;
            abort();
        }
        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, nullptr);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, nullptr);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }

    // activate the shader
    // ------------------------------------------------------------------------
    void use() const {
        glUseProgram(ID);
    }

    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
    }

    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }

    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }


    void setVec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, const std::string &type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog
                          << "\n -- --------------------------------------------------- -- " << std::endl;
                abort();
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog
                          << "\n -- --------------------------------------------------- -- " << std::endl;
                abort();
            }
        }
    }
};

class RenderTarget {
    GLuint m_FBO{};
    unsigned int rbo{};
    // 一个用来存放上个pass的内容
#define ATTACHMENT_NUM 1
    GLuint m_AttachTexIds[ATTACHMENT_NUM] = {};
    const GLenum attachments[ATTACHMENT_NUM] = {
            GL_COLOR_ATTACHMENT1,
    };
    GLint defaultFrameBuffer = GL_NONE;
    int w;
    int h;
public:
    RenderTarget(int width, int height) : w(width), h(height) {
        std::cout << "new RenderTarget w: " << w << " h: " << h << std::endl;
        //生成帧缓冲区对象
        glGenFramebuffers(1, &m_FBO);
        bind();

        glGenTextures(ATTACHMENT_NUM, static_cast<GLuint *>(m_AttachTexIds));
        for (int i = 0; i < ATTACHMENT_NUM; ++i) {
            glBindTexture(GL_TEXTURE_2D, m_AttachTexIds[i]);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, m_AttachTexIds[i], 0);
        }
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        // use a single renderbuffer object for both a depth AND stencil buffer.
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        // now actually attach it
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  rbo);
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        restore();
    }

    static int getTextureCount() {
        return ATTACHMENT_NUM;
    }

    void getTextureIds(GLuint *ids) {
        for (int i = 0; i < ATTACHMENT_NUM; ++i) {
            ids[i] = m_AttachTexIds[i];
        }
    }

    void bind() {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glDrawBuffers(ATTACHMENT_NUM, attachments);
    }

    void restore() const {
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    }

    void close() const {
        for (int i = 0; i < ATTACHMENT_NUM; ++i) {
            glDeleteTextures(1, &m_AttachTexIds[i]);
        }
        glDeleteFramebuffers(1, &m_FBO);
        glDeleteRenderbuffers(1, &rbo);
    }
};

class Mesh {

public:
    unsigned int VBO{}, EBO{};
    uint64_t elementCount = 0;
    GLuint vao;

    Mesh(MeshData data, GLuint vao) : vao(vao) {
        glBindVertexArray(vao);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, data.sizeOfVert, data.verts, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.sizeOfIndices, data.indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void *) 0);
        glEnableVertexAttribArray(0);

        elementCount = data.sizeOfIndices / sizeof(int);
    }

    void draw() const {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLint) elementCount, GL_UNSIGNED_INT, nullptr);
    }

    void free() {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
};

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

void Display::init(int _width, int _height, int _texW, int _texH) {
    width = _width;
    height = _height;
    texW = _texW;
    texH = _texH;
    // ------------------------------
    glfwSetErrorCallback(error_callback);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(width, height, "Test", nullptr, nullptr);
//    sharedThread = glfwCreateWindow(1, 1, "Shared", nullptr, window);
//    glfwHideWindow(sharedThread);
    glfwSetWindowUserPointer(window, this);
    if (window == nullptr) {
        glfwTerminate();
        mw_fatal("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        mw_fatal("Failed to initialize GLAD");
    }

    glDebugMessageCallback(reinterpret_cast<GLDEBUGPROC>(debugOutput), nullptr);
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader shader("../display/display.vert", "../display/display.frag");

    int indices[] = {0, 1, 2, 0, 1, 3};
    // pos uv
    float verts[] = {-1, -1, 0, 1,
                     1, 1, 1, 0,
                     1, -1, 1, 1,
                     -1, 1, 0, 0
    };

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    MeshData meshData = {sizeof(indices), indices, sizeof(verts), verts};
    GLuint vao;
    glGenVertexArrays(1, &vao);
    Mesh mesh(meshData, vao);

    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
//    auto renderTarget = new RenderTarget(width, height);
//    renderTarget->close();
//    delete renderTarget;
    printf("%s %s %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));
    shader.use();
    shader.setInt("tex", 0);

//    std::thread([this] {
//        printf("wait tex upload\n");
//        std::this_thread::sleep_for(std::chrono::seconds(3));
    printf("do tex upload\n");
    auto *pix = (uint32_t *) calloc(texW * texH, sizeof(uint32_t));
    double all = texW * texH;
    for (uint32_t j = 0; j < texW * texH; ++j) {
        if (j < all * 0.25) {
            uint8_t p[4] = {255, 0, 0, 255};
            memcpy(&pix[j], p, sizeof(uint32_t));
        } else if (j < all * 0.5) {
            uint8_t p[4] = {0, 255, 0, 255};
            memcpy(&pix[j], p, sizeof(uint32_t));
        } else if (j < all * 0.75) {
            uint8_t p[4] = {0, 0, 255, 255};
            memcpy(&pix[j], p, sizeof(uint32_t));
        } else {
            uint8_t p[4] = {0, 0, 0, 0};
            memcpy(&pix[j], p, sizeof(uint32_t));
        }
    }
    uploadTex(pix);
    free(pix);
//    }).detach();


    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        mesh.draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Display::terminate() {
    glfwTerminate();
}

void Display::uploadTex(void *pixels) {
//    glfwMakeContextCurrent(sharedThread);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Display::framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    auto *p = (Display *) (glfwGetWindowUserPointer(window));
    p->setSize(width, height);
}

void Display::setSize(int _width, int _height) {
    this->width = _width;
    this->height = _height;
    glViewport(0, 0, width, height);
}


int main() {
    Display display{};
    display.init(1024, 768, 1024, 768);
    display.terminate();
}