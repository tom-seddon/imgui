// requires:
// defining IMGUI_INCLUDE_IMGUI_USER_H and IMGUI_INCLUDE_IMGUI_USER_INL
// at the project level

#pragma once
#ifndef IMGUI_USER_ADDONS_H_
#define IMGUI_USER_ADDONS_H_


#include "./imguihelper/imguihelper.h"
#include "./imguilistview/imguilistview.h"
#include "./imguifilesystem/imguifilesystem.h"
#include "./imguitoolbar/imguitoolbar.h"
#include "./imguipanelmanager/imguipanelmanager.h"
#include "./imguivariouscontrols/imguivariouscontrols.h"
#include "./imguistyleserializer/imguistyleserializer.h"
#include "./imguidatechooser/imguidatechooser.h"

#undef IMGUI_USE_AUTO_BINDING
#if (defined(IMGUI_USE_GLUT_BINDING) || defined(IMGUI_USE_SDL2_BINDING) || defined(IMGUI_USE_GLFW_BINDING) || defined(IMGUI_USE_WINAPI_BINDING))
#	define IMGUI_USE_AUTO_BINDING
# 	include "./imguibindings/imguibindings.h"
#endif //IMGUI_USE_AUTO_BINDING

#endif //IMGUI_USER_ADDONS_H_

