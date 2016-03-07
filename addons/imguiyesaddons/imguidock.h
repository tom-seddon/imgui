#ifndef IMGUIDOCK_H_
#define IMGUIDOCK_H_

// Lumix Engine Dock. From: https://github.com/nem0/LumixEngine/blob/master/src/editor/imgui/imgui_dock.h
/*
The MIT License (MIT)

Copyright (c) 2013-2016 Mikulas Florek

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

namespace ImGui {

IMGUI_API void ShutdownDock();
IMGUI_API void RootDock(const ImVec2& pos, const ImVec2& size);
IMGUI_API bool BeginDock(const char* label, bool* opened=NULL, ImGuiWindowFlags extra_flags=0);
IMGUI_API void EndDock();
IMGUI_API void SetDockActive();
// TODO: consider porting LoadDock/SaveDock to imguihelper.h
//IMGUI_API void SaveDock(Lumix::FS::OsFile& file);
//IMGUI_API void LoadDock(lua_State* L);

} // namespace ImGui


#endif //IMGUIDOCK_H_

