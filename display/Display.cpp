//
// Created by SnowNF on 2024/5/28.
//

#include "glad.h"
#include "Display.h"
#include <GLFW/glfw3.h>
#include "shared/log_helper.h"
#include "shared/network.h"
#include "shared/shared.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <cstring>
#include <lz4.h>

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
    unsigned int ID{};

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

    void setInt(const std::string &name, GLuint value) const {
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

void Display::run(int _width, int _height, int _texW, int _texH) {
    width = _width;
    height = _height;
    texW = _texW;
    texH = _texH;
    // ------------------------------
    glfwSetErrorCallback(error_callback);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(width, height, "Test", nullptr, nullptr);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    sharedThread = glfwCreateWindow(1, 1, "Shared", nullptr, window);
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

    // build and compile our shader program
    // ------------------------------------
    Shader displayShader("../display/display.vert", "../display/display.frag");
    Shader diffShader("../display/differ.vert", "../display/differ.frag");

    int indices[] = {0, 1, 2, 0, 1, 3};
    // pos uv
#if !TEXTURE_DIRECT_RENDERING
    float verts[] = {-1, -1, 0, 1,
                     1, 1, 1, 0,
                     1, -1, 1, 1,
                     -1, 1, 0, 0
    };
#else
    float verts[] = {-1, -1, 0, 0,
                     1, 1, 1, 1,
                     1, -1, 1, 0,
                     -1, 1, 0, 1
    };
#endif
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

    MeshData meshData = {sizeof(indices), indices, sizeof(verts), verts};
    GLuint vao;
    glGenVertexArrays(1, &vao);
    Mesh mesh(meshData, vao);

    glfwGetFramebufferSize(window, &width, &height);
    printf("%s %s %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));

#if ENABLE_PBO
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pbo);
    GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    auto size = (int64_t) (texH * texW * sizeof(uint32_t));
    glBufferStorage(GL_PIXEL_UNPACK_BUFFER, size, nullptr, flags);
    texData = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, flags);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
#endif
    pixelPtrAvailable(this);
#if !TEXTURE_DIRECT_RENDERING
    renderTarget = new RenderTarget(texW, texH);
#endif
    GLint diffSrcLocation = glGetUniformLocation(diffShader.ID, "src");
    GLint diffBack0Location = glGetUniformLocation(diffShader.ID, "back0");
    GLint diffBack1Location = glGetUniformLocation(diffShader.ID, "back1");
    GLint diffOutIndexLocation = glGetUniformLocation(diffShader.ID, "outIndex");

    GLint dispSrc0Location = glGetUniformLocation(displayShader.ID, "src0");
    GLint dispSrc1Location = glGetUniformLocation(displayShader.ID, "src1");
    GLint dispSrcIndexLocation = glGetUniformLocation(displayShader.ID, "srcIndex");

    bool destIsZero = true;
    while (!glfwWindowShouldClose(window)) {
#if !TEXTURE_DIRECT_RENDERING
        renderTarget->bind();
        {
            diffShader.use();
//            glClearColor(1.0f, 1.0f, 0.f, 1.0f);
//            glClear(GL_COLOR_BUFFER_BIT);
            glViewport(0, 0, texW, texH);

            glUniform1i(diffSrcLocation, (int) tex);
            glActiveTexture(GL_TEXTURE0 + tex);
            glBindTexture(GL_TEXTURE_2D, tex);

            int back0 = (int) renderTarget->getTextureId(0);
            glUniform1i(diffBack0Location, back0);
            glActiveTexture(GL_TEXTURE0 + back0);
            glBindTexture(GL_TEXTURE_2D, back0);

            int back1 = (int) renderTarget->getTextureId(1);
            glUniform1i(diffBack1Location, back1);
            glActiveTexture(GL_TEXTURE0 + back1);
            glBindTexture(GL_TEXTURE_2D, back1);

            glUniform1i(diffOutIndexLocation, destIsZero ? 0 : 1);

            mesh.draw();
        }
        renderTarget->restore();
#endif
        {
            displayShader.use();
//            glClearColor(0.f, 0.f, 1.f, 1.0f);
//            glClear(GL_COLOR_BUFFER_BIT);
            glViewport(0, 0, width, height);
#if !TEXTURE_DIRECT_RENDERING

            int src0 = (int) renderTarget->getTextureId(0);
            glUniform1i(dispSrc0Location, src0);
            glActiveTexture(GL_TEXTURE0 + src0);
            glBindTexture(GL_TEXTURE_2D, src0);

            int src1 = (int) renderTarget->getTextureId(1);
            glUniform1i(dispSrc1Location, src1);
            glActiveTexture(GL_TEXTURE0 + src1);
            glBindTexture(GL_TEXTURE_2D, src1);

            glUniform1i(dispSrcIndexLocation, destIsZero ? 0 : 1);
#else
            glUniform1i(dispSrc1Location, 0);
            glUniform1i(dispSrc0Location, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
#endif
            mesh.draw();
        }

        destIsZero = !destIsZero;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
#if !TEXTURE_DIRECT_RENDERING
    renderTarget->close();
    delete renderTarget;
#endif
}

void Display::terminate() {
    glfwTerminate();
}

void Display::setPixelPtrAvailable(std::function<void(Display *)> available) {
    this->pixelPtrAvailable = available;
}

static bool madeContext = false;

#if ENABLE_PBO

uint32_t *Display::getPixelPtr() {
    return (uint32_t *) (texData);
}

void Display::uploadTex() {
    if (!madeContext) {
        glfwMakeContextCurrent(sharedThread);
        glDebugMessageCallback(reinterpret_cast<GLDEBUGPROC>(debugOutput), nullptr);
        madeContext = true;
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    glFinish();
}
#else

void Display::uploadTex(void *pixel) {
    if (!madeContext) {
        glfwMakeContextCurrent(sharedThread);
        glDebugMessageCallback(reinterpret_cast<GLDEBUGPROC>(debugOutput), nullptr);
        madeContext = true;
        glBindTexture(GL_TEXTURE_2D, tex);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixel);
    glFinish();
}

void Display::uploadTex(int x, int y, int w, int h, void *pixel) {
    if (!madeContext) {
        glfwMakeContextCurrent(sharedThread);
        glDebugMessageCallback(reinterpret_cast<GLDEBUGPROC>(debugOutput), nullptr);
        madeContext = true;
        glBindTexture(GL_TEXTURE_2D, tex);
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_BGRA, GL_UNSIGNED_BYTE, pixel);
    glFinish();
}

#endif

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

#include <random>

std::random_device rd;//非确定性随机数生成器
std::mt19937 gen(rd()); //使用Mersenne twister算法随机数生成器
std::uniform_int_distribution<> distrib(0, UINT32_MAX); //随机均匀分布[1,6]区间

// [0,1]
float nRandom() {
    return (float) distrib(gen) / (float) UINT32_MAX;
}

int multiply(float a, int b) {
    return a * b;
}

uint8_t rand8() {
    return nRandom() * 255;
}

static std::chrono::steady_clock::time_point start;
static float frameCount = 0;

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG

#include "../shared/stb_image_write.h"

#endif
#if USE_JPEG_TURBO

#include <turbojpeg.h>

#endif

int main() {
    Display display{};
    int texW = 1024;
    int texH = 768;
    TheClient client("127.0.0.1", 37385);
    int sock = client.cConnect();
    IMAGE_TYPE type{};
    mw_read_all(sock, (char *) (&type), sizeof(IMAGE_TYPE));
    texW = type.w;
    texH = type.h;
    display.setPixelPtrAvailable([sock, type, texH, texW](Display *dpy) {
        std::thread([dpy, sock, type, texH, texW] {
            bool first = true;
            void *tmp = calloc(sizeof(uint32_t), texW * texH);
            void *tex = calloc(sizeof(uint32_t), texW * texH);
#if ENABLE_DEBUG
            uint32_t *pixel = (uint32_t *) calloc(sizeof(uint32_t), type.w * type.h);
            int oCount = 0;
#endif

#if USE_JPEG_TURBO
            tjhandle tj;
#endif

            while (true) {
                if (first) {
#if USE_JPEG_TURBO
                    tj = tjInitDecompress();
#endif
                    mw_read_all(sock, (char *) (tmp), type.size);
                    first = false;
                } else {
                    mw_read_all(sock, (char *) (&type), sizeof(IMAGE_TYPE));
                    mw_read_all(sock, (char *) (tmp), type.size);
                }
#if !USE_JPEG_TURBO
                int r = LZ4_decompress_safe((const char *) tmp, (char *) (tex), type.size,
                                            texW * texH * sizeof(uint32_t));
                if (r < 0) {
                    assert(0);
                }
#else
                int result = tjDecompress2(tj, (const uint8_t *) (tmp), type.size,
                                           (uint8_t *) (tex), 0, 0, 0,
                                           TJPF_BGRA, 0);
                if (result != 0) {
                    assert(0);
                }
#endif

#if ENABLE_DEBUG
                auto count = type.w * type.h;
                auto *src = (uint32_t *) tex;
                uint32_t writePixel = 0;
                for (int i = 0; i < count; ++i) {
                    uint8_t rgba[4] = {};
                    memcpy(&rgba, &src[i], sizeof(uint32_t));
                    if (rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0 && rgba[3] == 0) {
                        continue;
                    }
                    pixel[i] = src[i];
                    writePixel++;
                }
                fprintf(stderr, "Write %d pixels\n", writePixel);
                oCount++;
                if (oCount % 10 == 0) {
                    std::string name = ("out" + std::to_string(oCount) + ".bmp");
                    fprintf(stderr, "Out %s\n", name.c_str());
                    stbi_write_bmp(name.c_str(), type.w, type.h, 4, pixel);
                }
#endif

#if !TEXTURE_DIRECT_RENDERING
                dpy->uploadTex(tex);
#else
                dpy->uploadTex(type.x, type.y, type.w, type.h, tex);
                fprintf(stderr, "%d %d %d %d\n", type.x, type.y, type.w, type.h);
#endif
                frameCount++;
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                float sec = (float) ms.count() / 1000.f;
                if (sec > 5) {
                    float frame_rate = frameCount / sec;
                    start = end;
                    mw_debug("FPS: %f", frame_rate);
                    frameCount = 0;
                }
            }
        }).detach();
    });
    display.run(texW / 2, texH / 2, texW, texH);
    display.terminate();
}