// requires:
// defining IMGUI_INCLUDE_IMGUI_USER_H and IMGUI_INCLUDE_IMGUI_USER_INL
// at the project level

#pragma once
#ifndef IMGUI_USER_ADDONS_INL_
#define IMGUI_USER_ADDONS_INL_

#ifdef IMGUI_USE_MINIZIP	// requires linking to library -lZlib
//extern "C" {
#include "./imguifilesystem/minizip/ioapi.c"
#include "./imguifilesystem/minizip/unzip.c"
#include "./imguifilesystem/minizip/zip.c"
//}
#endif //IMGUI_USE_MINIZIP

#include "./imguihelper/imguihelper.cpp"
#include "./imguilistview/imguilistview.cpp"
#include "./imguifilesystem/imguifilesystem.cpp"
#include "./imguitoolbar/imguitoolbar.cpp"
#include "./imguipanelmanager/imguipanelmanager.cpp"
#include "./imguivariouscontrols/imguivariouscontrols.cpp"
#include "./imguistyleserializer/imguistyleserializer.cpp"
#include "./imguidatechooser/imguidatechooser.cpp"

#ifdef IMGUI_USE_AUTO_BINDING	// defined in imgui_user.h
#	ifdef IMGUI_USE_GLUT_BINDING
#		include "./imguibindings/ImImpl_Binding_Glut.h"
#	elif IMGUI_USE_SDL2_BINDING
#		include "./imguibindings/ImImpl_Binding_SDL2.h"
#	elif IMGUI_USE_GLFW_BINDING
#		include "./imguibindings/ImImpl_Binding_Glfw3.h"
#	elif (defined(_WIN32) || defined(IMGUI_USE_WINAPI_BINDING))
#		include "./imguibindings/ImImpl_Binding_WinAPI.h"
#	else // IMGUI_USE_SOME_BINDING
#		include "./imguibindings/ImImpl_Binding_Glfw3.h"
#	endif // IMGUI_USE_SOME_BINDING
#	include "./imguibindings/imguibindings.cpp"
#endif //IMGUI_USE_AUTO_BINDING

#endif //IMGUI_USER_ADDONS_INL_

