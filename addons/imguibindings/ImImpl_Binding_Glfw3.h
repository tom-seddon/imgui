#ifndef IMIMPL_BINDING_H
#define IMIMPL_BINDING_H


#include "imguibindings.h"

#ifdef _WIN32
#   define IMGUI_USE_WIN32_CURSORS     // Optional, but needs at window creation: wc.hCursor = LoadCursor( NULL, NULL); // Now the window class is inside glfw3... Not sure how I can access it...
#   ifdef IMGUI_USE_WIN32_CURSORS
    static const LPCTSTR win32CursorIds[ImGuiMouseCursor_Count_+1] = {
        IDC_ARROW,
        IDC_IBEAM,
        IDC_SIZEALL,      //SDL_SYSTEM_CURSOR_HAND,    // or SDL_SYSTEM_CURSOR_SIZEALL  //ImGuiMouseCursor_Move,                  // Unused by ImGui
        IDC_SIZENS,       //ImGuiMouseCursor_ResizeNS,              // Unused by ImGui
        IDC_SIZEWE,       //ImGuiMouseCursor_ResizeEW,              // Unused by ImGui
        IDC_SIZENESW,     //ImGuiMouseCursor_ResizeNESW,
        IDC_SIZENWSE,     //ImGuiMouseCursor_ResizeNWSE,          // Unused by ImGui
        IDC_ARROW         //,ImGuiMouseCursor_Arrow
    };
    static HCURSOR win32Cursors[ImGuiMouseCursor_Count_+1];
#   endif //IMGUI_USE_WIN32_CURSORS
#else //_WIN32
#   define IMGUI_USE_X11_CURSORS      // Optional (feel free to comment it out if you don't have X11)
#   ifdef IMGUI_USE_X11_CURSORS
    // Before the inclusion of @ref glfw3native.h, you must define exactly one
    // window API macro and exactly one context API macro.
#   define GLFW_EXPOSE_NATIVE_X11
#   define GLFW_EXPOSE_NATIVE_GLX
#   include <glfw3native.h>        // GLFWAPI Display* glfwGetX11Display(void); //GLFWAPI Window glfwGetX11Window(GLFWwindow* window);
#   include <X11/cursorfont.h>
// 52 (closed hand)   58 (hand with forefinger) 124 (spray)   86  (pencil)  150 (wait)
    static const unsigned int x11CursorIds[ImGuiMouseCursor_Count_+1] = {
        2,//SDL_SYSTEM_CURSOR_ARROW,
        152,//SDL_SYSTEM_CURSOR_IBEAM,
        30,//SDL_SYSTEM_CURSOR_SIZEALL,      //SDL_SYSTEM_CURSOR_HAND,    // or SDL_SYSTEM_CURSOR_SIZEALL  //ImGuiMouseCursor_Move,                  // Unused by ImGui
        116,//42,//SDL_SYSTEM_CURSOR_SIZENS,       //ImGuiMouseCursor_ResizeNS,              // Unused by ImGui
        108,//SDL_SYSTEM_CURSOR_SIZEWE,       //ImGuiMouseCursor_ResizeEW,              // Unused by ImGui
        13,//SDL_SYSTEM_CURSOR_SIZENESW,     //ImGuiMouseCursor_ResizeNESW,
        15,//SDL_SYSTEM_CURSOR_SIZENWSE,     //ImGuiMouseCursor_ResizeNWSE,          // Unused by ImGui
        2//SDL_SYSTEM_CURSOR_ARROW         //,ImGuiMouseCursor_Arrow
    };
    static Cursor x11Cursors[ImGuiMouseCursor_Count_+1];
#   endif //IMGUI_USE_X11_CURSORS
#endif //_WIN32





//static
GLFWwindow* window;
//static bool mousePressed[2] = { false, false };
static ImVec2 mousePosScale(1.0f, 1.0f);


// NB: ImGui already provide OS clipboard support for Windows so this isn't needed if you are using Windows only.
static const char* ImImpl_GetClipboardTextFn()
{
    return glfwGetClipboardString(window);
}

static void ImImpl_SetClipboardTextFn(const char* text)
{
    glfwSetClipboardString(window, text);
}

#ifdef _WIN32
// Notify OS Input Method Editor of text input position (e.g. when using Japanese/Chinese inputs, otherwise this isn't needed)
static void ImImpl_ImeSetInputScreenPosFn(int x, int y)
{
    HWND hwnd = glfwGetWin32Window(window);
    if (HIMC himc = ImmGetContext(hwnd))
    {
        COMPOSITIONFORM cf;
        cf.ptCurrentPos.x = x;
        cf.ptCurrentPos.y = y;
        cf.dwStyle = CFS_FORCE_POSITION;
        ImmSetCompositionWindow(himc, &cf);
    }
}
#endif

// GLFW callbacks to get events
static void glfw_error_callback(int /*error*/, const char* description)	{
    fputs(description, stderr);
}
static bool gImGuiAppIsIconified = false;
static void glfw_window_iconify_callback(GLFWwindow* /*window*/,int iconified)    {
    gImGuiAppIsIconified = iconified == GL_TRUE;
}
static void glfw_framebuffer_size_callback(GLFWwindow* /*window*/,int fb_w,int fb_h)  {
    int w, h;glfwGetWindowSize(window, &w, &h);
    mousePosScale.x = (float)fb_w / w;                  // Some screens e.g. Retina display have framebuffer size != from window size, and mouse inputs are given in window/screen coordinates.
    mousePosScale.y = (float)fb_h / h;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)fb_w, (float)fb_h);
}
static void glfw_window_size_callback(GLFWwindow* /*window*/,int w,int h)  {
    int fb_w, fb_h;glfwGetFramebufferSize(window, &fb_w, &fb_h);
    mousePosScale.x = (float)fb_w / w;                  // Some screens e.g. Retina display have framebuffer size != from window size, and mouse inputs are given in window/screen coordinates.
    mousePosScale.y = (float)fb_h / h;
    ResizeGL(w,h);
}
static void glfw_mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int mods)	{
    ImGuiIO& io = ImGui::GetIO();
    if (button >= 0 && button < 5) {
        io.MouseDown[button] = (action == GLFW_PRESS);
        // Manual double click handling:
        static double dblClickTimes[6]={-FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX};  // seconds
        if (action == GLFW_PRESS)   {
            double time = glfwGetTime();
            double& oldTime = dblClickTimes[button];
            bool& mouseDoubleClicked = gImGuiBindingMouseDblClicked[button];
            if (time - oldTime < io.MouseDoubleClickTime) {
                mouseDoubleClicked = true;
                oldTime = -FLT_MAX;
                //fprintf(stderr,"Double Clicked button %d\n",button);
            }
            else {
                //fprintf(stderr,"Not Double Clicked button %d (%1.4f < %1.4f)\n",button,(float)time-(float)oldTime,io.MouseDoubleClickTime);
                mouseDoubleClicked = false;
                oldTime = time;
            }
        }
    }
    io.KeyCtrl = (mods & GLFW_MOD_CONTROL);
    io.KeyShift = (mods & GLFW_MOD_SHIFT);
    io.KeyAlt = (mods & GLFW_MOD_ALT);

}
static void glfw_scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)	{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = (yoffset != 0.0f) ? yoffset > 0.0f ? 1 : - 1 : 0;           // Mouse wheel: -1,0,+1
}
static void glfw_key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int mods)	{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)   io.KeysDown[key] = true;
    if (action == GLFW_RELEASE) io.KeysDown[key] = false;
    io.KeyCtrl = (mods & GLFW_MOD_CONTROL);
    io.KeyShift = (mods & GLFW_MOD_SHIFT);
    io.KeyAlt = (mods & GLFW_MOD_ALT);
}
static void glfw_char_callback(GLFWwindow* /*window*/, unsigned int c)	{
    if (c > 0 && c < 0x10000 && !ImGui::GetIO().KeyCtrl) ImGui::GetIO().AddInputCharacter((unsigned short)c);
}
static void glfw_mouse_enter_leave_callback(GLFWwindow* /*window*/, int entered)	{
    if (entered==GL_FALSE) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos.x=io.MousePos.y=-1.f;
    }
}
static void glfw_mouse_move_callback(GLFWwindow* /*window*/, double x,double y)	{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)x * mousePosScale.x, (float)y * mousePosScale.y);      // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
}
static void InitImGui(const ImImpl_InitParams* pOptionalInitParams=NULL)	{
    int w, h;
    int fb_w, fb_h;
    glfwGetWindowSize(window, &w, &h);
    glfwGetFramebufferSize(window, &fb_w, &fb_h);
    mousePosScale.x = (float)fb_w / w;                  // Some screens e.g. Retina display have framebuffer size != from window size, and mouse inputs are given in window/screen coordinates.
    mousePosScale.y = (float)fb_h / h;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)fb_w, (float)fb_h);  // Display size, in pixels. For clamping windows positions.
    io.DeltaTime = 1.0f/60.0f;                          // Time elapsed since last frame, in seconds (in this sample app we'll override this every frame because our timestep is variable)
    //io.PixelCenterOffset = 0.0f;                        // Align OpenGL texels
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;             // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn = ImImpl_RenderDrawLists;
    io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
    io.GetClipboardTextFn = ImImpl_GetClipboardTextFn;
#ifdef _WIN32
    io.ImeSetInputScreenPosFn = ImImpl_ImeSetInputScreenPosFn;
#endif

    // 3 common init steps
    InitImGuiFontTexture(pOptionalInitParams);
    InitImGuiProgram();
    InitImGuiBuffer();
}


static bool InitBinding(const ImImpl_InitParams* pOptionalInitParams=NULL,int argc=0, char** argv=NULL)	{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())    {
        fprintf(stderr, "Could not call glfwInit(...) successfully.\n");
        return false;
    }

//-ENABLE-OPENGLES COMPATIBILITY PROFILES----------------------------------
#ifndef IMIMPL_SHADER_NONE
#ifdef IMIMPL_SHADER_GLES
#   ifndef IMIMPL_SHADER_GL3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);          // 1 => ES1.1   2 => ES2.0  3 => ES3.0
#   else //IMIMPL_SHADER_GL3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
#   endif //MIMPL_SHADER_GL3
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif // IMIMPL_SHADER_GLES
#endif //IMIMPL_SHADER_NONE
//--------------------------------------------------------------------------
    //glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

#ifdef IMIMPL_FORCE_DEBUG_CONTEXT
glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_TRUE);
#endif //IMIMPL_FORCE_DEBUG_CONTEXT

/*
//-ENABLE-OPENGLES COMPATIBILITY PROFILES----------------------------------
#ifndef IMIMPL_SHADER_NONE
#ifdef IMIMPL_SHADER_GLES
#   ifndef IMIMPL_SHADER_GL3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);          // 1 => ES1.1   2 => ES2.0  3 => ES3.0
#   ifdef IMIMPL_SHADER_GLES_MINOR_VERSION
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, IMIMPL_SHADER_GLES_MINOR_VERSION);
#   endif //IMIMPL_SHADER_GLES_MINOR_VERSION
#   else //IMIMPL_SHADER_GL3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
#   ifdef IMIMPL_SHADER_GLES_MINOR_VERSION
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, IMIMPL_SHADER_GLES_MINOR_VERSION);
#   endif //IMIMPL_SHADER_GLES_MINOR_VERSION
#   endif //MIMPL_SHADER_GL3
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif // IMIMPL_SHADER_GLES
#endif //IMIMPL_SHADER_NONE

#ifndef IMIMPL_SHADER_GLES
#ifdef IMIMPL_FORWARD_COMPAT
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
#ifdef IMIMPL_CORE_PROFILE
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#endif
#endif

#ifdef IMIMPL_SRGB_CAPABLE
    glfwWindowHint(GLFW_SRGB_CAPABLE,GL_TRUE);
#endif
if (pOptionalInitParams && pOptionalInitParams->useOpenGLDebugContext) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_TRUE);
//--------------------------------------------------------------------------
*/

    if (pOptionalInitParams && pOptionalInitParams->gWindowTitle[0]!='\0')  window = glfwCreateWindow(pOptionalInitParams ? pOptionalInitParams->gWindowSize.x : 1270, pOptionalInitParams ? pOptionalInitParams->gWindowSize.y : 720,(const char*) &pOptionalInitParams->gWindowTitle[0], NULL, NULL);
    else		window = glfwCreateWindow(pOptionalInitParams ? pOptionalInitParams->gWindowSize.x : 1270, pOptionalInitParams ? pOptionalInitParams->gWindowSize.y : 720, "ImGui Glfw3 OpenGL example", NULL, NULL);
    if (!window)    {
        fprintf(stderr, "Could not call glfwCreateWindow(...) successfully.\n");
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetCharCallback(window, glfw_char_callback);
    glfwSetCursorPosCallback(window, glfw_mouse_move_callback);
    glfwSetCursorEnterCallback(window, glfw_mouse_enter_leave_callback);

    glfwSetWindowSizeCallback(window, glfw_window_size_callback);
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetWindowIconifyCallback(window, glfw_window_iconify_callback);

        //OpenGL info
    {
        printf("GL Vendor: %s\n", glGetString( GL_VENDOR ));
        printf("GL Renderer : %s\n", glGetString( GL_RENDERER ));
        printf("GL Version (string) : %s\n",  glGetString( GL_VERSION ));
        printf("GLSL Version : %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ));
        //printf("GL Extensions:\n%s\n",(char *) glGetString(GL_EXTENSIONS));
    }

#ifdef IMGUI_USE_GLEW
    GLenum err = glewInit();
    if( GLEW_OK != err )
    {
        fprintf(stderr, "Error initializing GLEW: %s\n",
                glewGetErrorString(err) );
        return false;
    }
#endif //IMGUI_USE_GLEW

#ifdef IMIMPL_FORCE_DEBUG_CONTEXT
    glDebugMessageCallback(GLDebugMessageCallback,NULL);    // last is userParam
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);   // use glDisable(GL_DEBUG_OUTPUT); at runtime to disable it

    // Test:
    //glEnable(GL_DEPTH); // instead of glEnable(GL_DEPTH_TEST); => Produces: GL_INVALID_ENUM error generated. <cap> enum is invalid; expected GL_ALPHA_TEST, GL_BLEND, GL_COLOR_MATERIAL, GL_CULL_FACE, GL_DEPTH_TEST, GL_DITHER, GL_FOG, etc. (136 others).
#endif //IMIMPL_FORCE_DEBUG_CONTEXT

	return true;
}

// Application code
int ImImpl_Main(const ImImpl_InitParams* pOptionalInitParams,int argc, char** argv)
{
    if (!InitBinding(pOptionalInitParams,argc,argv)) return -1;
    InitImGui(pOptionalInitParams);
    ImGuiIO& io = ImGui::GetIO();        
    
    // New: create cursors-------------------------------------------
#   ifdef IMGUI_USE_WIN32_CURSORS
    for (int i=0,isz=ImGuiMouseCursor_Count_+1;i<isz;i++) {
        win32Cursors[i] = LoadCursor(NULL,(LPCTSTR) win32CursorIds[i]);
        if (i==0) SetCursor(win32Cursors[i]);
    }
#   elif defined IMGUI_USE_X11_CURSORS
    Display* x11Display = glfwGetX11Display();
    Window x11Window = glfwGetX11Window(window);
    XColor white;white.red=white.green=white.blue=255;
    XColor black;black.red=black.green=black.blue=0;
    for (int i=0,isz=ImGuiMouseCursor_Count_+1;i<isz;i++) {
        x11Cursors[i] = XCreateFontCursor(x11Display,x11CursorIds[i]);
        //XRecolorCursor(x11Display, x11Cursors[i], &white,&black);
        if (i==0) XDefineCursor(x11Display,x11Window,x11Cursors[i]);
    }
#   endif
    //---------------------------------------------------------------

    InitGL();
 	ResizeGL(io.DisplaySize.x,io.DisplaySize.y);
	
    static double time = 0.0f;

    gImGuiInverseFPSClamp = pOptionalInitParams ? ((pOptionalInitParams->gFpsClamp!=0) ? (1.0f/pOptionalInitParams->gFpsClamp) : 1.0f) : -1.0f;
	
    while (!glfwWindowShouldClose(window))
    {
        if (!gImGuiPaused)	{
            for (size_t i = 0; i < 5; i++) gImGuiBindingMouseDblClicked[i] = false;   // We manually set it (otherwise it won't work with low frame rates)

            static ImGuiMouseCursor oldCursor = ImGuiMouseCursor_Arrow;
            static bool oldMustHideCursor = io.MouseDrawCursor;
            if (oldMustHideCursor!=io.MouseDrawCursor) {
                glfwSetInputMode(window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
                oldMustHideCursor = io.MouseDrawCursor;
                oldCursor = ImGuiMouseCursor_Count_;
            }
            if (!io.MouseDrawCursor) {
                if (oldCursor!=ImGui::GetMouseCursor()) {
                    oldCursor=ImGui::GetMouseCursor();
                    // set the 'native' window cursor to "oldCursor" here (Hehe: glut and SDL2 can handle them automatically, glfw no!)
#                   ifdef IMGUI_USE_WIN32_CURSORS
                    SetCursor(win32Cursors[oldCursor]);           // If this does not work, it's bacause the native Window must be created with a NULL cursor (but how to tell glfw about it?)
#                   elif defined IMGUI_USE_X11_CURSORS
                    XDefineCursor(x11Display,x11Window,x11Cursors[oldCursor]);
#                   endif
                }
            }
        }
        if (gImGuiAppIsIconified || gImGuiInverseFPSClamp==0) {
            //fprintf(stderr,"glfwWaitEvents() Start %1.4f\n",glfwGetTime());
            glfwWaitEvents();
            //fprintf(stderr,"glfwWaitEvents() End %1.4f\n",glfwGetTime());
        }
        else glfwPollEvents();
  
    	// Setup timestep    	
    	const double current_time =  glfwGetTime();
        static float deltaTime = (float)(current_time -time);
        deltaTime = (float) (current_time - time);
        time = current_time;

        if (!gImGuiPaused)	{
            io.DeltaTime = (float) deltaTime;
            // Start the frame
            ImGui::NewFrame();
            for (size_t i = 0; i < 5; i++) {
                io.MouseDoubleClicked[i]=gImGuiBindingMouseDblClicked[i];   // We manually set it (otherwise it won't work with low frame rates)
            }
        }

		DrawGL();

        if (!gImGuiPaused)	{
            bool imguiNeedsInputNow = ImGui::IsMouseHoveringAnyWindow() | ImGui::IsAnyItemActive();
            if (gImGuiCapturesInput != imguiNeedsInputNow) {
                gImGuiCapturesInput = imguiNeedsInputNow;
                //fprintf(stderr,"gImGuiCapturesInput=%s\n",gImGuiCapturesInput?"true":"false");
            }

	        // Rendering        
	        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    	    ImGui::Render();    	    
        }

        glfwSwapBuffers(window);

        // If needed we must wait (inverseFPSClamp-deltaTime) seconds (=> honestly I shouldn't add the * 2.0f factor at the end, but ImGui tells me the wrong FPS otherwise... why? <=)
        if (gImGuiInverseFPSClamp>0 && deltaTime < gImGuiInverseFPSClamp)  WaitFor((unsigned int) ((gImGuiInverseFPSClamp-deltaTime)*1000.f * 2.0f) );

    }

    DestroyGL();
    ImGui::Shutdown();
    DestroyImGuiFontTexture();
    DestroyImGuiProgram();
    DestroyImGuiBuffer();

    // New: delete cursors-------------------------------------------
#   ifdef IMGUI_USE_WIN32_CURSORS
    // Nothing to do
#   elif defined IMGUI_USE_X11_CURSORS
    XUndefineCursor(x11Display,x11Window);
    for (int i=0,isz=ImGuiMouseCursor_Count_+1;i<isz;i++) {
        XFreeCursor(x11Display,x11Cursors[i]);
    }
#   endif
    //---------------------------------------------------------------

    glfwTerminate();
    return 0;
}

#endif //#ifndef IMIMPL_BINDING_H

