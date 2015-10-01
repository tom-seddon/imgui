To compile these examples, no make file is provided.

A single Qt Creator .pro file is provided. It can be used in Ubuntu only and must be edited to set the paths and the example to compile.


But basically you can follow these steps:

1 -> No additional .cpp file must be added to your project (i.e. DO NOT add any .cpp file inside the addons folder). Instead define at the project level: IMGUI_INCLUDE_IMGUI_USER_H and IMGUI_INCLUDE_IMGUI_USER_INL. This way the two files imgui_user.h and imgui_user.inl should include all the addons automatically.

2 -> OPTIONALLY define ONE (and only one) of the following at the project level (typically when you're using ImGui in a new demo project):
IMGUI_USE_GLUT_BINDING		# needs -lglut (or maybe -lGLUT)
IMGUI_USE_SDL2_BINDING		# needs SDL2 libraries
IMGUI_USE_GLFW_BINDING		# needs -lglfw (version3)
IMGUI_USE_WINAPI_BINDING	# needs the windows API
When one of these is defined, then in your main.cpp file (or somewhere else) you MUST define these 4 methods globally:
void InitGL() {}
void ResizeGL(int w,int h) {}
void DrawGL() {}
void DestroyGL() {}
And then in our main function you can simply call a method (for example ImImpl_Main(NULL,argc,argv);) to have the automatic binding with the library you've chosen (see the code in the examples for further info).

3 -> If you use a binding, OPTIONALLY some of these definitions might work at the project level (depending on the binding you choose):
IMIMPL_SHADER_NONE # no shaders at all, and no vertex buffer object as well (minimal implementation).
IMIMPL_SHADER_GL3  # shader uses openGL 3.3 (glsl #version 330)
IMIMPL_SHADER_GLES # shader uses gles (and if IMIMPL_SHADER_GL3 is defined glsl #version 300 es)
IMGUI_USE_GLEW     # inits the glew library (needs -lGLEW). This definition might be mandatory for IMGUI_USE_WINAPI_BINDING.


-----------------------------------------------------------------------------------------------------------------------------------
A MINIMAL EXAMPLE: mainBasic.cpp
---------------------------------

#include <imgui.h>	

// Mandatory methods
void InitGL() {}
void ResizeGL(int w,int h) {}
void DestroyGL() {}
void DrawGL()	
{
        glClearColor(0.8f, 0.6f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        static bool open = true;
        ImGui::Begin("Debug", &open, ImVec2(300,300)); 
        ImGui::Text("Hello, world!");
		ImGui::End();	

		// However I got access to all addons from here now
}

int main(int argc, char** argv)
{
	ImImpl_Main(NULL,argc,argv);
	return 0;
}

On Ubuntu, I can compile it with the following command line (provided that imgui.h is two folders up, and that I want to use glfw):
gcc  -o basicExample mainBasic.cpp -I"../../" ../../imgui.cpp ../../imgui_draw.cpp -D"IMGUI_INCLUDE_IMGUI_USER_H" -D"IMGUI_INCLUDE_IMGUI_USER_INL" -I"/usr/include/GLFW" -D"IMGUI_USE_GLFW_BINDING" -L"/usr/lib/x86_64-linux-gnu" -lglfw -lX11 -lm -lGL -lstdc++ -s



