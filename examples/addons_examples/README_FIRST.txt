===================================
HOW TO COMPILE THE ADDON EXAMPLES:
===================================

To compile these two examples (main.cpp and main2.cpp), no makefile is provided.

Instead two project files are present:
->	addons_examples.pro: A Qt Creator project file that can be used in Ubuntu only and must be edited to set the paths and the example to compile (handy for Linux and Mac users). This file can be tweaked to use the libraries of your choice (see below).
->	addons_examples_mingw.cbp: A CodeBlocks project file that can be used to compile the first demo for Windows (to compile the second demo simply replace main.cpp with main2.cpp). This demo requires the glew library only to compile.
Even if you use another IDE, in case of compilation problems, it can still be useful to open these files with a text editor and see their content.

You can test the two examples (without compiling them) in your web browser by clicking on the two .html files inside the html subfolder (although not all addons are active in this build).
If you want to compile the examples to .html, please read at the bottom of this page.

====================================
WHAT IS IMGUI ADDONS ?
====================================
It's a collection of "extra imgui widgets" together with an automatic way of "binding" ImGui to a specific openGL library (glfw, SDL2, glut and WinAPI), so that a single cpp "main" file can be used for all of them.

It supports openGL only (==> DIRECTX IS NOT SUPPORTED <==).

ImGui Addons does NOT modify the ImGui library itself in any way (i.e. imgui.cpp, imgui_draw.cpp and imgui_demo.cpp are untouched); it just adds:
-> the "addons" subfolder.
-> the two files "imgui_user.h" and "imgui_user.inl" (in the base ImGui folder).

Currently the extra imgui widgets that are available are:
-> imguistyleserializer: 	to load and save ImGuiStyle from/to file.
-> imguifilesystem:			this addon provides: chooseFileDialog, chooseFolderDialog, saveFileDialog
							together with plenty of handy methods to perform file system operations and
							an experimental support for browsing inside zip files (through an additional definition).
-> imguidatechooser:		a combobox-like datechooser.
-> imguilistview:			a list view widget with a lot of optional features (setting its height, row sorting through column header clicking, cell editing).
-> imguitoolbar:			a very flexible imagebutton-bar that can be used inside ImGui Windows (with dynamic layout) and outside (docked at the sides of the screen).
-> imguipanelmanager:		a mini dock panel layout. Basically it uses imguitoolbar and optionally assigns an ImGui Window to some buttons. Please see main2.cpp for an extensive example on how to use it.
-> imguivariouscontrols:	a series of minor widgets, such as:
							-> ProgressBar.
							-> PopupMenuSimple	(a fast, single column, scrollable, popup menu).
							-> PopupMenu (a single column menu with image entries).
							-> ColorChooser (based on the code from: https://github.com/benoitjacquier/imgui)
-> imguinodegrapheditor:	W.I.P. (based on the code posted by Omar, the creator of ImGui)

And in addition:
-> imguistring:				a string class based on ImVector<char>. It does not support iterators, but has many methods that std::string has: thus it can be used to replace std::string in many algorithms.
-> imguihelper:				currently it just has: OpenWithDefaultApplication(...) that should work with urls, folders and files.

Tip: every single imgui "widget" addon listed above can be excluded by defining at the project level something like: NO_IMGUIFILESYSTEM, etc (and the first demo, main.cpp, should always compile).

===========================================
HOW TO USE IMGUI ADDONS IN YOUR PROJECTS:
===========================================
Basically to use and compile projects that use the imgui addon framework you can follow these steps:

1 -> No ADDITIONAL .cpp file must be added to your project (i.e. DO NOT add any .cpp file inside the "addons" folder). Instead define at the project level: IMGUI_INCLUDE_IMGUI_USER_H and IMGUI_INCLUDE_IMGUI_USER_INL. This way the two files "imgui_user.h" and "imgui_user.inl" should include all the addons automatically.

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
And then in our main function you can simply call a method (for example ImImpl_Main(NULL,argc,argv);) to have the automatic binding with the library you've chosen (see the code below or in the examples for further info).

3 -> If you use a binding, OPTIONALLY some of these definitions might work at the project level (depending on the binding you choose):
IMIMPL_SHADER_NONE # no shaders at all, and no vertex buffer object as well (minimal implementation).
IMIMPL_SHADER_GL3  # shader uses openGL 3.3 (glsl #version 330)
IMIMPL_SHADER_GLES # shader uses gles (and if IMIMPL_SHADER_GL3 is defined glsl #version 300 es)
IMGUI_USE_GLEW     # inits the glew library (needs -lGLEW). This definition might be mandatory for IMGUI_USE_WINAPI_BINDING.


----------------------------------------------------------------------------------------------------------------------------------
SPARE SINGLE ADDON USAGE
---------------------------
If you just want to add a single addon (a pair of addonName.h/.cpp files) to an existing project WITHOUT following the steps above, you can probably just include its header file,
add the include folders: $IMGUI_HOME and $IMGUI_HOME/addons/addonName to your project (where $IMGUI_HOME is the path where imgui.h/.cpp are located) and compile the file $IMGUI_HOME/addons/addonName/addonName.cpp, where "addonName" is the name of the addon you want to use. Be warned that some addons might depend on others: e.g. imguipanelmanager depends on imguitoolbar: so you may need to include both addons.

However I'm not sure this approach works for all the addons, since some .cpp files need to be included after imgui.cpp to access its internals: recent imgui versions provide the file:
imgui_internal.h, you may try including this at the top of the addonName.cpp file, but there's no guarantee it will work.


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

		// However I get access to all addons from here now
}

int main(int argc, char** argv)
{
	ImImpl_Main(NULL,argc,argv);
	return 0;
}

On Ubuntu, I can compile it with the following command line (provided that imgui.h is two folders up, and that I want to use glfw):
gcc  -o basicExample mainBasic.cpp -I"../../" ../../imgui.cpp ../../imgui_draw.cpp -D"IMGUI_INCLUDE_IMGUI_USER_H" -D"IMGUI_INCLUDE_IMGUI_USER_INL" -I"/usr/include/GLFW" -D"IMGUI_USE_GLFW_BINDING" -L"/usr/lib/x86_64-linux-gnu" -lglfw -lX11 -lm -lGL -lstdc++ -s



======================================================================================================================================
EXTRA: COMPILING TO HTML USING EMSCRIPTEN:
===========================================
Follow these steps:
1) Using a terminal (=command line), make sure you have a working emcc setup (try: emcc -v).
2) Navigate (cd) to this folder.
3) To compile the first example try:
em++ -O2 -o main.html main.cpp -I"../../" ../../imgui.cpp ../../imgui_draw.cpp  --preload-file myNumbersTexture.png -D"IMGUI_INCLUDE_IMGUI_USER_H" -D"IMGUI_INCLUDE_IMGUI_USER_INL" -D"IMGUI_USE_SDL2_BINDING" -D"NO_IMGUIFILESYSTEM" -D"NO_IMGUISTYLESERIALIZER" -s USE_SDL=2 -s LEGACY_GL_EMULATION=0 -s ALLOW_MEMORY_GROWTH=1 -lm -lGL
4) To compile the second example try:
em++ -O2 -o main2.html main2.cpp -I"../../" ../../imgui.cpp ../../imgui_draw.cpp  --preload-file myNumbersTexture.png  --preload-file Tile8x8.png -D"IMGUI_INCLUDE_IMGUI_USER_H" -D"IMGUI_INCLUDE_IMGUI_USER_INL" -D"IMGUI_USE_SDL2_BINDING" -D"NO_IMGUIFILESYSTEM" -s USE_SDL=2 -s LEGACY_GL_EMULATION=0 -lm -lGL

Some notes:
->	As you can see it now uses the SDL2 binding (-D"IMGUI_USE_SDL2_BINDING"). Further work is required from my side to make the GLFW3 and GLUT bindings compatible with emscripten.

->	Also note that I used: -D"NO_IMGUIFILESYSTEM": this is because the filesystem is not accessible from the browser AFAIK (but I'm a very newbie here...).

->	In addition: -D"NO_IMGUISTYLESERIALIZER" has been added to the first demo, because I don't know if it's possible to save a .style file. 
	However I bet we can simply load an existing style in InitGL() from a .style file (named myimgui.style and placed in this folder) this way: 
	em++ -O2 -o main.html main.cpp -I"../../" ../../imgui.cpp ../../imgui_draw.cpp  --preload-file myNumbersTexture.png --preload-file myimgui.style -D"IMGUI_INCLUDE_IMGUI_USER_H" -D"IMGUI_INCLUDE_IMGUI_USER_INL" -D"IMGUI_USE_SDL2_BINDING" -D"NO_IMGUIFILESYSTEM" -D"NO_IMGUISTYLESERIALIZER_SAVE_STYLE" -s USE_SDL=2 -s LEGACY_GL_EMULATION=0 -s ALLOW_MEMORY_GROWTH=1 -lm -lGL


