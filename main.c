/*
  TODO:
  - Single vertex on the pole
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

LRESULT CALLBACK wndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(wnd, msg, wparam, lparam);
  }
  return 0;
}

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prevInst, LPSTR cmdLine, int cmdShow) {
  UNREFERENCED_PARAMETER(prevInst);
  UNREFERENCED_PARAMETER(cmdLine);

  WNDCLASS wndClass = {0};
  wndClass.style = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc = wndProc;
  wndClass.hInstance = inst;
  wndClass.hCursor = LoadCursor(0, IDC_ARROW);
  wndClass.lpszClassName = "Sphere GL";
  RegisterClass(&wndClass);

  int wndWidth = 1920/2;
  int wndHeight = wndWidth;

  RECT crect = {0};
  crect.right = wndWidth;
  crect.bottom = wndHeight;

  DWORD wndStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  AdjustWindowRect(&crect, wndStyle, 0);

  HWND wnd = CreateWindowEx(0, wndClass.lpszClassName, "Sphere GL", wndStyle, 300, 0,
                            crect.right - crect.left, crect.bottom - crect.top,
                            0, 0, inst, 0);
  ShowWindow(wnd, cmdShow);
  UpdateWindow(wnd);

  HDC hdc = GetDC(wnd);

  float dt = 0.0f;
  float targetFps = 60.0f;
  float maxDt = 1.0f / targetFps;
  LARGE_INTEGER perfcFreq = {0};
  LARGE_INTEGER perfc = {0};
  LARGE_INTEGER prefcPrev = {0};

  QueryPerformanceFrequency(&perfcFreq);
  QueryPerformanceCounter(&perfc);

  {
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC ctx = wglCreateContext(hdc);
    wglMakeCurrent(hdc, ctx);

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

  int parallelsCount = 40;
  int meridiansCount = parallelsCount;
  float radius = 1.0f;
  int pointsCount = parallelsCount * meridiansCount;
  int sphereSize = pointsCount * 3 * sizeof(float);
  float *sphere = malloc(sphereSize);

  int i = 0;
  for (int p = 0; p < parallelsCount; ++p) {
    for (int m = 0; m < meridiansCount; ++m) {
      float theta = PI * p / (parallelsCount-1);
      float phi = 2 * PI * m / meridiansCount;
      float x = radius * sinf(theta) * cosf(phi);
      float y = radius * sinf(theta) * sinf(phi);
      float z = radius * cosf(theta);
      sphere[i++] = x;
      sphere[i++] = y;
      sphere[i++] = z;
    }
  }

  int rectCount = parallelsCount * meridiansCount;
  int triangleCount = rectCount * 2;
  int indicesCount = triangleCount * 3;
  int indexDataSize = indicesCount * sizeof(GLuint);
  GLuint *indexData = malloc(indexDataSize);

  i = 0;
  for (int p = 0; p < parallelsCount-1; ++p) {
    for (int m = 0; m < meridiansCount; ++m) {
      int next_p = p+1;
      int next_m = (m+1) % meridiansCount;

      indexData[i++] = p*meridiansCount + m;
      indexData[i++] = next_p*meridiansCount + m;
      indexData[i++] = p*meridiansCount + next_m;

      indexData[i++] = p*meridiansCount + next_m;
      indexData[i++] = next_p*meridiansCount + m;
      indexData[i++] = next_p*meridiansCount + next_m;
    }
  }

  GLuint vao;
  GLuint vbo;
  GLuint shaderProgram;

  const char *vertexShader = "#version 410\n"
    "in vec3 pos;"
    "uniform mat4 model;"
    "out vec3 color;"
    "void main () {"
    "    color = (pos + vec3(1.0))/2.0;"
    "    gl_Position = model * vec4(pos, 1.0);"
    "    gl_PointSize = 4.0;"
    "}";
  const char *fragmentShader = "#version 410\n"
    "in vec3 color;"
    "out vec4 frag_color;"
    "void main () {"
    "    frag_color = vec4(color, 1.0);"
    "}";
  GLuint vertShader, fragShader;

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sphereSize, sphere, GL_STATIC_DRAW);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_STATIC_DRAW);

  vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertShader, 1, &vertexShader, NULL);
  glCompileShader(vertShader);
  fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShader, 1, &fragmentShader, NULL);
  glCompileShader(fragShader);
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, fragShader);
  glAttachShader(shaderProgram, vertShader);
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  GLint modelMatLocation = glGetUniformLocation(shaderProgram, "model");

  bool running = true;
  float sphereYRot = 0;

  while (running) {
    prefcPrev = perfc;
    QueryPerformanceCounter(&perfc);
    dt = (float)(perfc.QuadPart - prefcPrev.QuadPart) / (float)perfcFreq.QuadPart;
    if (dt > maxDt) {
      dt = maxDt;
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

    sphereYRot += 0.25f*PI*dt;
    float c = cosf(sphereYRot);
    float s = sinf(sphereYRot);
    float m[16];
    m[0]  =  c;   m[1]  = 0; m[2]  =  -s;  m[3]  = 0;
    m[4]  = -s*s; m[5]  = c; m[6]  = -c*s; m[7]  = 0;
    m[8]  =  c*s; m[9]  = s; m[10] =  c*c; m[11] = 0;
    m[12] =  0;   m[13] = 0; m[14] =  0;   m[15] = 1;
    glUniformMatrix4fv(modelMatLocation, 1, GL_TRUE, m);

    //glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, NULL);
    glDrawElements(GL_LINE_STRIP, indicesCount, GL_UNSIGNED_INT, NULL);

    SwapBuffers(hdc);
  }
}
