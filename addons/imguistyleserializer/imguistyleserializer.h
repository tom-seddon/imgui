#ifndef IMGUISTYLESERIALIZER_H_
#define IMGUISTYLESERIALIZER_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

enum ImGuiStyleEnum {
    ImGuiStyle_Default=0,
    ImGuiStyle_Gray,        // This is the default theme of my main.cpp demo.
    ImGuiStyle_OSX,         // Posted by @itamago here: https://github.com/ocornut/imgui/pull/511 (hope I can use it)
    ImGuiStyle_OSXOpaque,
    ImGuiStyle_DarkOpaque,

    ImGuiStyle_DefaultInverse,
    ImGuiStyle_OSXInverse,
    ImGuiStyle_OSXOpaqueInverse,
    ImGuiStyle_DarkOpaqueInverse,

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

// satThresholdForInvertingLuminance: in [0,1] if == 0.f luminance is not inverted at all
// shiftHue: in [0,1] if == 0.f hue is not changed at all
void ChangeStyleColors(ImGuiStyle& style,float satThresholdForInvertingLuminance=.1f,float shiftHue=0.f);
} // namespace ImGui

#endif //IMGUISTYLESERIALIZER_H_

