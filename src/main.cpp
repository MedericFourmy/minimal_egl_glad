#include <glad/glad.h>
#include <EGL/egl.h>

#include <iostream>

int main(int argc, char const *argv[])
{
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;

    ////////////////
    // INITIAL SETUP
    // Get EGL display
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
    {
        std::cerr << "Failed to get EGL display" << std::endl;
        return false;
    }

    // Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor))
    {
        std::cerr << "Failed to initialize EGL" << std::endl;
        return false;
    }
    std::cout << "EGL major.minor: " << major << "." << minor << std::endl;

    // Bind OpenGL API
    if (!eglBindAPI(EGL_OPENGL_API))
    {
        std::cerr << "Failed to bind OpenGL API" << std::endl;
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
        return false;
    }

    // Choose EGL configuration
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE};

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(display, config_attribs, &config, 1, &num_configs) ||
        num_configs == 0)
    {
        std::cerr << "Failed to choose EGL configuration" << std::endl;
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
        return false;
    }

    // Create EGL context with OpenGL 3.3 Core Profile
    EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE};

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT)
    {
        std::cerr << "Failed to create EGL context" << std::endl;
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
        return false;
    }

    // Create a minimal pbuffer surface (since we don't need actual rendering output)
    EGLint surface_attribs[] = {
        EGL_WIDTH, 640,
        EGL_HEIGHT, 480,
        EGL_NONE};

    surface = eglCreatePbufferSurface(display, config, surface_attribs);
    if (surface == EGL_NO_SURFACE)
    {
        std::cerr << "Failed to create EGL surface" << std::endl;
        eglDestroyContext(display, context);
        context = EGL_NO_CONTEXT;
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
        return false;
    }

    // Make context current to initialize GLEW
    if (!eglMakeCurrent(display, surface, surface, context))
    {
        std::cerr << "Failed to make EGL context current" << std::endl;
        eglDestroySurface(display, surface);
        surface = EGL_NO_SURFACE;
        eglDestroyContext(display, context);
        context = EGL_NO_CONTEXT;
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
        return false;
    }

    if (eglGetCurrentContext() == EGL_NO_CONTEXT)
    {
        fprintf(stderr, "No valid OpenGL context\n");
    }

    // Initialize GLAD
    if (!gladLoadGL())
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, surface);
        surface = EGL_NO_SURFACE;
        eglDestroyContext(display, context);
        context = EGL_NO_CONTEXT;
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
        return false;
    }
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    /////////////////////////////////
    // Draw a triangle on EGL surface
    /////////////////////////////////

    eglMakeCurrent(display, surface, surface, context);

    // Create and bind a vertex array object (VAO)
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Define triangle vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    // Create and bind vertex buffer
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Create and compile shaders
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Clear and draw
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Read pixels and save to file
    unsigned char* pixels = new unsigned char[640 * 480 * 4];
    glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    FILE* output = fopen("triangle.bmp", "wb");
    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 32,0};
    unsigned int filesize = 54 + 640 * 480 * 4;
    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);
    bmpinfoheader[ 4] = (unsigned char)(640    );
    bmpinfoheader[ 5] = (unsigned char)(640>> 8);
    bmpinfoheader[ 6] = (unsigned char)(640>>16);
    bmpinfoheader[ 7] = (unsigned char)(640>>24);
    bmpinfoheader[ 8] = (unsigned char)(480    );
    bmpinfoheader[ 9] = (unsigned char)(480>> 8);
    bmpinfoheader[10] = (unsigned char)(480>>16);
    bmpinfoheader[11] = (unsigned char)(480>>24);
    fwrite(bmpfileheader, 1, 14, output);
    fwrite(bmpinfoheader, 1, 40, output);
    fwrite(pixels, 1, 640 * 480 * 4, output);
    fclose(output);
    delete[] pixels;

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    return 0;
}
