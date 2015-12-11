// requires:
// defining IMGUI_INCLUDE_IMGUI_USER_H and IMGUI_INCLUDE_IMGUI_USER_INL
// at the project level

#pragma once
#ifndef IMGUI_USER_ADDONS_H_
#define IMGUI_USER_ADDONS_H_

#ifdef IMGUI_USE_MINIZIP	// requires linking to library -lZlib
#   ifndef IMGUI_USE_ZLIB
#   define IMGUI_USE_ZLIB	// requires linking to library -lZlib
#   endif //IMGUI_USE_ZLIB
#endif //IMGUI_USE_MINIZIP

#ifndef NO_IMGUISTRING
#include "./imguistring/imguistring.h"
#endif //NO_IMGUISTRING

#undef IMGUI_USE_AUTO_BINDING
#if (defined(IMGUI_USE_GLUT_BINDING) || defined(IMGUI_USE_SDL2_BINDING) || defined(IMGUI_USE_GLFW_BINDING) || defined(IMGUI_USE_WINAPI_BINDING))
#	define IMGUI_USE_AUTO_BINDING
# 	include "./imguibindings/imguibindings.h"
#endif //IMGUI_USE_AUTO_BINDING

#ifndef NO_IMGUIHELPER
#include "./imguihelper/imguihelper.h"
#endif //NO_IMGUIHELPER
#ifndef NO_IMGUILISTVIEW
#include "./imguilistview/imguilistview.h"
#endif //NO_IMGUILISTVIEW
#ifndef NO_IMGUIFILESYSTEM
#include "./imguifilesystem/imguifilesystem.h"
#endif //NO_IMGUIFILESYSTEM
#ifndef NO_IMGUITOOLBAR
#include "./imguitoolbar/imguitoolbar.h"
#endif //NO_IMGUITOOLBAR
#ifndef NO_IMGUIPANELMANAGER
#include "./imguipanelmanager/imguipanelmanager.h"
#endif //NO_IMGUIPANELMANAGER
#ifndef NO_IMGUIVARIOUSCONTROLS
#include "./imguivariouscontrols/imguivariouscontrols.h"
#endif //NO_IMGUIVARIOUSCONTROLS
#ifndef NO_IMGUISTYLESERIALIZER
#include "./imguistyleserializer/imguistyleserializer.h"
#endif //NO_IMGUISTYLESERIALIZER
#ifndef NO_IMGUIDATECHOOSER
#include "./imguidatechooser/imguidatechooser.h"
#endif //NO_IMGUIDATECHOOSER
#ifndef NO_IMGUICODEEDITOR
#include "./imguicodeeditor/imguicodeeditor.h"
#endif //NO_IMGUICODEEDITOR
#ifdef IMGUISCINTILLA_ACTIVATED
#include "./imguiscintilla/imguiscintilla.h"
#endif //IMGUISCINTILLA_ACTIVATED
#ifndef NO_IMGUIGRAPHEDITOR
#include "./imguinodegrapheditor/imguinodegrapheditor.h"
#endif //NO_IMGUIGRAPHEDITOR

#endif //IMGUI_USER_ADDONS_H_

