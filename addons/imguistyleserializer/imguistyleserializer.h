#ifndef IMGUISTYLESERIALIZER_H_
#define IMGUISTYLESERIALIZER_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API


namespace ImGui	{

bool SaveStyle(const char* filename,const ImGuiStyle& style);
bool LoadStyle(const char* filename,ImGuiStyle& style);

} // namespace ImGui

#endif //IMGUISTYLESERIALIZER_H_

