#ifndef IMGUIBINDINGS_H_
#define IMGUIBINDINGS_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

// include openGL headers here:
#ifdef _WIN32
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#include <windows.h>
#include <imm.h>
#endif //_WIN32

//# define GLEW_STATIC		// Optional, depending on which glew lib you want to use
#ifdef IMGUI_USE_GLEW
#   ifdef __APPLE__   // or __MACOSX__ ?
#       include <OpenGL/glew.h>     // guessing...
#   else //__APPLE
#       include <GL/glew.h>
#   endif //__APPLE
#else //IMGUI_USE_GLEW
#   define GL_GLEXT_PROTOTYPES
#endif //IMGUI_USE_GLEW


#if (!defined(IMGUI_USE_WINAPI_BINDING) && !defined(IMGUI_USE_GLFW_BINDING) && !defined(IMGUI_USE_SDL2_BINDING) && !defined(IMGUI_USE_GLUT_BINDING))
#   ifdef _WIN32
#       define IMGUI_USE_WINAPI_BINDING
#   else //_WIN32
#       define IMGUI_USE_GLFW_BINDING
#   endif //_WIN32
#endif // !defined(...)

#ifdef IMGUI_USE_GLUT_BINDING
//-------------------------------------------------------------------------------
#   ifdef __APPLE__   // or __MACOSX__ ?
#       include <OpenGL/glut.h>             // guessing...
#   else //__APPLE
#       include <GL/glut.h>
#   endif //__APPLE
#   ifdef __FREEGLUT_STD_H__
#       ifdef __APPLE__   // or __MACOSX__ ?
#           include <OpenGL/freeglut_ext.h>     // guessing...
#       else //__APPLE
#           include <GL/freeglut_ext.h>
#       endif //__APPLE
#   endif //__FREEGLUT_STD_H__
//-------------------------------------------------------------------------------
#elif IMGUI_USE_SDL2_BINDING
//-------------------------------------------------------------------------------
#   ifndef IMGUI_USE_GLEW
#       if (!defined(IMIMPL_SHADER_GLES) || defined (IMIMPL_SHADER_NONE))
#           include <SDL2/SDL_opengl.h>
#       else //IMIMPL_SHADER_GLES
#           include <SDL2/SDL_opengles2.h>
#       endif //IMIMPL_SHADER_GLES
#   endif //IMGUI_USE_GLEW
#   include <SDL2/SDL.h>
//-------------------------------------------------------------------------------
#elif IMGUI_USE_GLFW_BINDING
//-------------------------------------------------------------------------------
//# define GLFW_STATIC   //// Optional, depending on which glfw lib you want to use
#   include <GLFW/glfw3.h>
#   ifdef _WIN32
#       define GLFW_EXPOSE_NATIVE_WIN32
#       define GLFW_EXPOSE_NATIVE_WGL
#       include <GLFW/glfw3native.h>
#   endif //_WIN32
//-------------------------------------------------------------------------------
#elif (defined(_WIN32) || defined(IMGUI_USE_WINAPI_BINDING))
//-------------------------------------------------------------------------------
#   ifndef IMGUI_USE_GLEW
//      I've never managed to make this branch work => when using Windows, ALWAYS use glew (on Linux it's much easier)
#       define GL_GLEXT_PROTOTYPES
#       ifdef __APPLE__   // or __MACOSX__ ?
#           include <OpenGL/glext.h>     // guessing...
#       else //__APPLE
#           include <GL/glext.h>
#       endif //__APPLE
#   endif // IMGUI_USE_GLEW
#   ifdef __APPLE__   // or __MACOSX__ ?
#       include <OpenGL/gl.h>       // guessing...
#   else //__APPLE
#       include <GL/gl.h>
#   endif //__APPLE
//--------------------------------------------------------------------------------
#else // IMGUI_USE_SOME_BINDING
#error: No IMGUI_USE_XXX_BINDING defined
#include "./addons/imguibindings/ImImpl_Binding_Glfw3.h"
#endif // IMGUI_USE_SOME_BINDING

#include <string.h>
#include <stdio.h>

extern void InitGL();
extern void ResizeGL(int w,int h);
extern void DrawGL();
extern void DestroyGL();

// These variables can be declared extern and set at runtime-----------------------------------------------------
extern bool gImGuiPaused;// = false;
extern float gImGuiInverseFPSClamp;// = -1.0f;    // CAN'T BE 0. < 0 = No clamping.
extern bool gImGuiCapturesInput;             // When false the input events can be directed somewhere else
extern bool gImGuiBindingMouseDblClicked[5];
// --------------------------------------------------------------------------------------------------------------

struct ImImpl_InitParams	{
	ImVec2 gWindowSize;
	char gWindowTitle[1024];
    char gOptionalTTFFilePath[2048];
    float gOptionalTTFFileFontSizeInPixels;
    const ImWchar* gOptionalTTFFileGlyphRanges;
    float gFpsClamp;	// <0 -> no clamp
    const unsigned char* pOptionalReferenceToTTFFileInMemory;
    size_t      pOptionalSizeOfTTFFileInMemory;
    ImImpl_InitParams(
            int windowWidth=1270,
            int windowHeight=720,
            const char* windowTitle=NULL,
            const char* OptionalTTFFilePath=NULL,
            const unsigned char*    _pOptionalReferenceToTTFFileInMemory=NULL,
            size_t                  _pOptionalSizeOfTTFFileInMemory=0,
            const float OptionalTTFFontSizeInPixels=15.0f,
            const ImWchar* OptionalTTFGlyphRanges=NULL
    ) :
    gOptionalTTFFileFontSizeInPixels(OptionalTTFFontSizeInPixels),
    gOptionalTTFFileGlyphRanges(OptionalTTFGlyphRanges),
    gFpsClamp(-1.0f),
    pOptionalReferenceToTTFFileInMemory(_pOptionalReferenceToTTFFileInMemory),
    pOptionalSizeOfTTFFileInMemory(_pOptionalSizeOfTTFFileInMemory)
	{
        gWindowSize.x = windowWidth<=0?1270:windowWidth;gWindowSize.y = windowHeight<=0?720:windowHeight;

		gWindowTitle[0]='\0';
		if (windowTitle)	{
			const size_t len = strlen(windowTitle);
			if (len<1023) strcat(gWindowTitle,windowTitle);
			else		  {
				memcpy(gWindowTitle,windowTitle,1023);
				gWindowTitle[1023]='\0';
			}
		}
		else strcat(gWindowTitle,"ImGui OpenGL Example");

        gOptionalTTFFilePath[0]='\0';
        if (OptionalTTFFilePath)	{
            const size_t len = strlen(OptionalTTFFilePath);
            if (len<2047) strcat(gOptionalTTFFilePath,OptionalTTFFilePath);
        }

        if (OptionalTTFFilePath || (_pOptionalReferenceToTTFFileInMemory && _pOptionalSizeOfTTFFileInMemory>0) )    {
            if (!gOptionalTTFFileGlyphRanges)   {
                gOptionalTTFFileGlyphRanges = GetGlyphRangesDefault();
            }
        }
	}
    private:
    // Retrieve list of range (2 int per range, values are inclusive)
    static const ImWchar*   GetGlyphRangesDefault()
    {
        static const ImWchar ranges[] =
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x20AC, 0x20AC,	// €
            0x2122, 0x2122,	// ™
            0x263A, 0x263A, // ☺
            0x266A, 0x266A, // ♪
            0,
        };
        return &ranges[0];
    }
};

#ifdef IMGUI_USE_WINAPI_BINDING
extern int ImImpl_WinMain(const ImImpl_InitParams* pOptionalInitParams,HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int iCmdShow);
#else //IMGUI_USE_WINAPI_BINDING
extern int ImImpl_Main(const ImImpl_InitParams* pOptionalInitParams=NULL,int argc=0, char** argv=NULL);
#endif //IMGUI_USE_WINAPI_BINDING


extern void InitImGuiFontTexture(const ImImpl_InitParams* pOptionalInitParams=NULL);
extern void DestroyImGuiFontTexture();

extern void InitImGuiProgram();
extern void DestroyImGuiProgram();

extern void InitImGuiBuffer();
extern void DestroyImGuiBuffer();
extern void ImImpl_RenderDrawLists(ImDrawData* draw_data);


extern void WaitFor(unsigned int ms);
extern GLuint ImImpl_LoadTexture(const char* filename,int req_comp=0,GLenum magFilter=GL_LINEAR,GLenum minFilter=GL_LINEAR_MIPMAP_LINEAR,GLenum wrapS=GL_REPEAT,GLenum wrapT=GL_REPEAT);
extern GLuint ImImpl_LoadTextureFromMemory(const unsigned char* filenameInMemory,int filenameInMemorySize,int req_comp=0,GLenum magFilter=GL_LINEAR,GLenum minFilter=GL_LINEAR_MIPMAP_LINEAR,GLenum wrapS=GL_REPEAT,GLenum wrapT=GL_REPEAT);

#ifdef IMIMPL_FORCE_DEBUG_CONTEXT
extern "C" void GLDebugMessageCallback(GLenum source, GLenum type,
    GLuint id, GLenum severity,GLsizei length, const GLchar *msg,const void *userParam);
#endif //IMIMPL_FORCE_DEBUG_CONTEXT


#endif //IMGUIBINDINGS_H_

