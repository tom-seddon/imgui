#include "imguihelper.h"

#ifdef _WIN32
#include <shellapi.h>	// ShellExecuteA(...) - Shell32.lib
#include <objbase.h>    // CoInitializeEx(...)  - ole32.lib
#else //_WIN32
#include <unistd.h>
#endif //_WIN32

namespace ImGui {

/*
bool GetButtonBehavior(const ImRect& bb, const ImGuiID& id, bool* out_hovered, bool* out_held, bool allow_key_modifiers, bool repeat, bool pressed_on_click)	{
    return ButtonBehavior(bb,id,out_hovered,out_held,allow_key_modifiers,repeat,pressed_on_click);
}
ImRect GetWindowRect()	{
 	ImGuiState& g = *GImGui;
    if (g.CurrentWindow) return  g.CurrentWindow->Rect();
    return ImRect(0,0,0,0);
}
bool IsWindowDoubleClicked(const ImGuiAabb& bb, const ImGuiID& id,int button,bool* out_hovered, bool* out_held, bool allow_key_modifiers, bool repeat, bool pressed_on_click)	{
	ImGuiState& g = *GImGui;
    if (g.CurrentWindow) {
        bool held=false;
        ButtonBehavior(bb, id, out_hovered, &held, allow_key_modifiers, repeat, pressed_on_click);
        if (out_held) *out_held=held;
        if (g.HoveredWindow == g.CurrentWindow && held && g.IO.MouseDoubleClicked[button])
		return true;
	}
    return false;
}
*/

bool OpenWithDefaultApplication(const char* url,bool exploreModeForWindowsOS)	{
#       ifdef _WIN32
            //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);  // Needed ??? Well, let's suppose the user initializes it himself for now"
            return ((size_t)ShellExecuteA( NULL, exploreModeForWindowsOS ? "explore" : "open", url, "", ".", SW_SHOWNORMAL ))>32;
#       else //_WIN32
            if (exploreModeForWindowsOS) exploreModeForWindowsOS = false;   // No warnings
            char tmp[2024];
            const char* openPrograms[]={"xdg-open","gnome-open"};	// Not sure what can I append here for MacOS

            static int openProgramIndex=-2;
            if (openProgramIndex==-2)   {
                openProgramIndex=-1;
                for (size_t i=0,sz=sizeof(openPrograms)/sizeof(openPrograms[0]);i<sz;i++) {
                    strcpy(tmp,"/usr/bin/");	// Well, we should check all the folders inside $PATH... and we ASSUME that /usr/bin IS inside $PATH (see below)
                    strcat(tmp,openPrograms[i]);
                    FILE* fd = fopen(tmp,"r");
                    if (fd) {
                        fclose(fd);
                        openProgramIndex = (int)i;
                        //printf(stderr,"found %s\n",tmp);
                        break;
                    }
                }
            }

            // Note that here we strip the prefix "/usr/bin" and just use openPrograms[openProgramsIndex].
            // Also note that if nothing has been found we use "xdg-open" (that might still work if it exists in $PATH, but not in /usr/bin).
            strcpy(tmp,openPrograms[openProgramIndex<0?0:openProgramIndex]);

            strcat(tmp," \"");
            strcat(tmp,url);
            strcat(tmp,"\"");
            return system(tmp)==0;
#       endif //_WIN32
}

} // namespace Imgui

