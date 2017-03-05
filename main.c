/*
TODO:
- Single vertex on the pole
- Move to CPP
- Use matrices to implement camera
- Rotate camera with mouse
*/

#include <windows.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <GL/GL.h>

#define APIENTRYP *

typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef char GLchar;

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642

typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
PFNGLGENBUFFERSPROC glGenBuffers;
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
PFNGLBINDBUFFERPROC glBindBuffer;
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
PFNGLBUFFERDATAPROC glBufferData;
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
PFNGLCREATESHADERPROC glCreateShader;
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
PFNGLSHADERSOURCEPROC glShaderSource;
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
PFNGLCOMPILESHADERPROC glCompileShader;
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
PFNGLCREATEPROGRAMPROC glCreateProgram;
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
PFNGLATTACHSHADERPROC glAttachShader;
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
PFNGLLINKPROGRAMPROC glLinkProgram;
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
PFNGLUSEPROGRAMPROC glUseProgram;
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

#define PI 3.14159265358979323846f
#define ARR_LEN(arr) (sizeof(arr)/sizeof(*arr))

LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(wnd, msg, wparam, lparam);
    }
    return 0;
}

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPSTR cmd_line, int cmd_show) {
    UNREFERENCED_PARAMETER(prev_inst);
    UNREFERENCED_PARAMETER(cmd_line);

    WNDCLASS wnd_class = {0};
    wnd_class.style = CS_HREDRAW | CS_VREDRAW;
    wnd_class.lpfnWndProc = wnd_proc;
    wnd_class.hInstance = inst;
    wnd_class.hCursor = LoadCursor(0, IDC_ARROW);
    wnd_class.lpszClassName = "Sphere GL";
    RegisterClass(&wnd_class);

    int wnd_width = 1920/2;
    int wnd_height = wnd_width;

    RECT crect = {0};
    crect.right = wnd_width;
    crect.bottom = wnd_height;

    DWORD wnd_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    AdjustWindowRect(&crect, wnd_style, 0);

    HWND wnd = CreateWindowEx(0, wnd_class.lpszClassName, "Sphere GL", wnd_style, 300, 0,
                              crect.right - crect.left, crect.bottom - crect.top,
                              0, 0, inst, 0);
    ShowWindow(wnd, cmd_show);
    UpdateWindow(wnd);

    HDC hdc = GetDC(wnd);

    float dt = 0.0f;
    float target_fps = 60.0f;
    float max_dt = 1.0f / target_fps;
    LARGE_INTEGER perfc_freq = {0};
    LARGE_INTEGER perfc = {0};
    LARGE_INTEGER prefc_prev = {0};

    QueryPerformanceFrequency(&perfc_freq);
    QueryPerformanceCounter(&perfc);

    {
        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 32;

        int pixel_format = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, pixel_format, &pfd);

        HGLRC gl_context = wglCreateContext(hdc);
        wglMakeCurrent(hdc, gl_context);

        glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
        glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
        glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
        glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
        glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
        glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
        glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
        glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
        glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
        glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
        glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
        glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    }

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

    int parallels_count = 40;
    int meridians_count = parallels_count;
    float radius = 1.0f;
    int points_count = parallels_count * meridians_count;
    int components_count = 3;
    int sphere_size = points_count * components_count * sizeof(float);
    float *sphere = malloc(sphere_size);

    int i = 0;
    for (int p = 0; p < parallels_count; ++p) {
        for (int m = 0; m < meridians_count; ++m) {
            float theta = PI * p / (parallels_count-1);
            float phi = 2 * PI * m / meridians_count;
            float x = radius * sinf(theta) * cosf(phi);
            float y = radius * sinf(theta) * sinf(phi);
            float z = radius * cosf(theta);
            sphere[i++] = x;
            sphere[i++] = y;
            sphere[i++] = z;
        }
    }

    int rect_count = parallels_count * meridians_count;
    int triangle_count = rect_count * 2;
    int indices_count = triangle_count * 3;
    int index_data_size = indices_count * sizeof(GLuint);
    GLuint *index_data = malloc(index_data_size);

    i = 0;
    for (int p = 0; p < parallels_count-1; ++p) {
        for (int m = 0; m < meridians_count; ++m) {
            int next_p = p+1;
            int next_m = (m+1) % meridians_count;

            index_data[i++] = p*meridians_count + m;
            index_data[i++] = next_p*meridians_count + m;
            index_data[i++] = p*meridians_count + next_m;

            index_data[i++] = p*meridians_count + next_m;
            index_data[i++] = next_p*meridians_count + m;
            index_data[i++] = next_p*meridians_count + next_m;
        }
    }

    GLuint vao;
    GLuint vbo;
    GLuint shader_program;

    const char *vertex_shader = "#version 410\n"
        "in vec3 pos;"
        "uniform mat4 model;"
        "out vec3 color;"
        "void main () {"
        "    color = (pos + vec3(1.0))/2.0;"
        "    gl_Position = model * vec4(pos, 1.0);"
        "    gl_PointSize = 4.0;"
        "}";
    const char *fragment_shader = "#version 410\n"
        "in vec3 color;"
        "out vec4 frag_color;"
        "void main () {"
        "    frag_color = vec4(color, 1.0);"
        "}";
    GLuint vert_shader, frag_shader;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sphere_size, sphere, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data_size, index_data, GL_STATIC_DRAW);

    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader, NULL);
    glCompileShader(vert_shader);
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader, NULL);
    glCompileShader(frag_shader);
    shader_program = glCreateProgram();
    glAttachShader(shader_program, frag_shader);
    glAttachShader(shader_program, vert_shader);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    GLint model_mat_location = glGetUniformLocation(shader_program, "model");

    bool running = true;
    float sphere_y_rot = 0;

    while (running) {
        prefc_prev = perfc;
        QueryPerformanceCounter(&perfc);
        dt = (float)(perfc.QuadPart - prefc_prev.QuadPart) / (float)perfc_freq.QuadPart;
        if (dt > max_dt) {
            dt = max_dt;
        }

        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            switch (msg.message) {
                case WM_QUIT:
                    running = false;
                    break;

                case WM_KEYDOWN:
                case WM_KEYUP:
                    switch (msg.wParam) {
                        case VK_ESCAPE:
                            running = false;
                            break;
                    }
                    break;

                default:
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    break;
            }
        }

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sphere_y_rot += 0.25f*PI*dt;
        float c = cosf(sphere_y_rot);
        float s = sinf(sphere_y_rot);
        float m[16];
        m[0]  =  c;   m[1]  = 0; m[2]  =  -s;  m[3]  = 0;
        m[4]  = -s*s; m[5]  = c; m[6]  = -c*s; m[7]  = 0;
        m[8]  =  c*s; m[9]  = s; m[10] =  c*c; m[11] = 0;
        m[12] =  0;   m[13] = 0; m[14] =  0;   m[15] = 1;
        glUniformMatrix4fv(model_mat_location, 1, GL_TRUE, m);

        //glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, NULL);
        glDrawElements(GL_LINE_STRIP, indices_count, GL_UNSIGNED_INT, NULL);

        SwapBuffers(hdc);
    }
}
