#ifndef IMGUISTYLESERIALIZER_H_
#define IMGUISTYLESERIALIZER_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

enum ImGuiStyleEnum {
    ImGuiStyle_Default=0,
    ImGuiStyle_Gray,        // This is the default theme of my main.cpp demo.
    ImGuiStyle_OSX,         // Posted by @itamago here: https://github.com/ocornut/imgui/pull/511 (hope I can use it)
    ImGuiStyle_Count
};

namespace ImGui	{
#ifndef NO_IMGUISTYLESERIALIZER_SAVESTYLE
bool SaveStyle(const char* filename,const ImGuiStyle& style);
#endif //NO_IMGUISTYLESERIALIZER_SAVESTYLE
#ifndef NO_IMGUISTYLESERIALIZER_LOADSTYLE
bool LoadStyle(const char* filename,ImGuiStyle& style);
#endif //NO_IMGUISTYLESERIALIZER_LOADSTYLE
bool ResetStyle(int styleEnum, ImGuiStyle& style);
const char** GetDefaultStyleNames();   // ImGuiStyle_Count names re returned
} // namespace ImGui

#endif //IMGUISTYLESERIALIZER_H_

