#ifndef IMGUITABWINDOW_H_
#define IMGUITABWINDOW_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

// USAGE:
/*
1) In the main "ImGui loop":
----------------------------
    static bool open = true;
    if (ImGui::Begin("Main", &open, ImVec2(400,600),-1.f,ImGuiWindowFlags_NoScrollbar))  {

        static ImGui::TabWindow tabWindow;
        if (!tabWindow.isInited()) {
            static const char* tabNames[] = {"Render","Layers","Scene","World","Object","Constraints","Modifiers","Data","Material","Texture","Particle","Physics"};
            static const int numTabs = sizeof(tabNames)/sizeof(tabNames[0]);
            static const char* tabTooltips[numTabs] = {"Render Tab Tooltip","Layers Tab Tooltip","Scene Tab Tooltip","","Object Tab Tooltip","","","","","Tired to add tooltips...",""};
            for (int i=0;i<numTabs;i++) {
                tabWindow.addTabLabel(tabNames[i],NULL,tabTooltips[i]); // see additional args to prevent a tab from (MMB) closing
            }
        }

        tabWindow.render(); // Must be called inside "its" window (and sets isInited() to false)
        ImGui::End();
    }
    // Optional add other ImGui::Window-TabWindow pairs here

2) At init time:
----------------
    // Callbacks:
    ImGui::TabWindow::SetWindowContentDrawerCallback(&TabContentProvider,NULL); // Mandatory
    ImGui::TabWindow::SetTabLabelPopupMenuDrawerCallback(&TabLabelPopupMenuProvider,NULL);  // Optional (if tou need context-menu)
    //ImGui::TabWindow::SetTabLabelClosingCallback(&OnTabLabelClosing,NULL);  // Optional
    //ImGui::TabWindow::SetTabLabelDeletingCallback(&OnTabLabelDeleting); // Optional (to delete your userPts)

    // Texture Loading
    int pngIconSize = 0;
    const unsigned char* pngIcon = ImGui::TabWindow::GetDockPanelIconImagePng(&pngIconSize);
    ImGui::TabWindow::DockPanelIconTextureID = reinterpret_cast<ImTextureID>(MyTextureFromMemoryPngMethod(pngIcon,pngIconSize));  // User must load it (using GetDockPanelIconImagePng and in some cases flipping Y to match tex coords unique convention).


    // Optional Style
    //ImGui::TabWindow::SplitterColor = ImVec4(...);
    //ImGui::TabWindow::SplitterSize = ...;
    // TODO: buttons must become tab label with a close button and thier style should be exposed

3) At deinit time:
-------------------
    Free: ImGui::TabWindow::DockPanelIconTextureID if not NULL.

4) Finally, in the global scope (at the top of you code), define the callbacks:
-------------------------------------------------------------------------------
void TabContentProvider(ImGui::TabWindow::TabLabel* tab,ImGui::TabWindow& parent,void* userPtr) {
    // Users will use tab->userPtr here most of the time
    ImGui::Spacing();ImGui::Separator();
    if (tab) ImGui::Text("Here is the content of tab label: \"%s\"\n",tab->getLabel());
    else {ImGui::Text("EMPTY TAB LABEL DOCKING SPACE.");ImGui::Text("PLEASE DRAG AND DROP TAB LABELS HERE!");}
    ImGui::Separator();ImGui::Spacing();
}
void TabLabelPopupMenuProvider(ImGui::TabWindow::TabLabel* tab,ImGui::TabWindow& parent,void* userPtr) {
    if (ImGui::BeginPopup(ImGui::TabWindow::GetTabLabelPopupMenuName()))   {
        ImGui::PushID(tab->getLabel());
        ImGui::Text("\"%s\"",tab->getLabel());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::MenuItem("Entry 1");
        ImGui::MenuItem("Entry 2");
        ImGui::MenuItem("Entry 3");
        ImGui::MenuItem("Entry 4");
        ImGui::MenuItem("Entry 5");
        ImGui::PopID();
        ImGui::EndPopup();
    }

}
void OnTabLabelClosing(const ImVector<ImGui::TabWindow::TabLabel*>& tabLabels,const ImVector<ImGui::TabWindow*>& parents,ImVector<bool>& forbidClosingOut,void* userPtr) {
    // Set "forbidClosingOut[..] = true" to prevent a tab label from closing. Please do not resize "forbidClosingOut".
    // (TODO: actually this was intented to fire a Modal Dialog, but I guess this can't be done without mods)
}
void OnTabLabelDeleting(ImGui::TabWindow::TabLabel* tab) {
    // Use this callback to delete tab->userPtr if you used it
}
*/

// BETTER ALTERNATIVES:
/*
There are better alternatives to Imgui::TabWindow:
-> https://github.com/thennequin/ImWindow   [for Window OS]
-> https://github.com/nem0/LumixEngine/blob/master/src/studio_lib/imgui/imgui_user.inl [lumixengine's Dock]
Please see: https://github.com/ocornut/imgui/issues for further info
*/

namespace ImGui {


class TabWindow {
public:
struct TabLabel {
    friend class TabNode;
    friend class TabWindow;
    friend struct TabWindowDragData;
protected:
    char* label;      // owned
    char* tooltip;    // ownded
    bool closable;
    bool draggable;
    TabLabel(const char* _label=NULL,const char* _tooltip=NULL,bool _closable=true,bool _draggable=true)    {
        label = tooltip = NULL;
        if (_label) {
            const int sz = strlen(_label);
            label = (char*) ImGui::MemAlloc(sz+1);
            strcpy(label,_label);
        }
        userPtr = NULL;
        if (_tooltip) {
            const int sz = strlen(_tooltip);
            tooltip = (char*) ImGui::MemAlloc(sz+1);
            strcpy(tooltip,_tooltip);
        }
        closable = _closable;
        draggable = _draggable;
        mustCloseNextFrame = false;
        mustSelectNextFrame = false;
    }
    ~TabLabel() {
        if (label) {ImGui::MemFree(label);label=NULL;}
        if (tooltip) {ImGui::MemFree(tooltip);tooltip=NULL;}
    }
    static void DestroyTabLabel(TabLabel*& tab);
public:
    const char* getLabel() const {return label;}
    void setLabel(const char* lbl)  {
        if (label) {ImGui::MemFree(label);label=NULL;}
        const char e = '\0';if (!lbl) lbl=&e;
        const int sz = strlen(lbl)+1;
        label = (char*) ImGui::MemAlloc(sz+1);strcpy(label,lbl);
    }
    void setTooltip(const char* tt)  {
        if (tooltip) {ImGui::MemFree(tooltip);tooltip=NULL;}
        const char e = '\0';if (!tt) tt=&e;
        const int sz = strlen(tt)+1;
        tooltip = (char*) ImGui::MemAlloc(sz+1);strcpy(tooltip,tt);
    }
    const char* getTooltip() const {return tooltip;}
    mutable void* userPtr;
    mutable bool mustCloseNextFrame;
    mutable bool mustSelectNextFrame;
};
protected:
public:
typedef void (*TabLabelCallback)(TabLabel* tabLabel,TabWindow& parent,void* userPtr);
typedef void (*TabLabelClosingCallback)(const ImVector<TabLabel*>& tabLabels,const ImVector<TabWindow*>& parents,ImVector<bool>& forbidClosingOut,void* userPtr);
typedef void (*TabLabelDeletingCallback)(TabLabel* tabLabel);


protected:
struct TabNode* mainNode;  // owned
struct TabNode* activeNode;
bool init;
void clearNodes();

static TabLabelCallback WindowContentDrawerCb;
static void* WindowContentDrawerUserPtr;
static TabLabelCallback TabLabelPopupMenuDrawerCb;
static void* TabLabelPopupMenuDrawerUserPtr;
static TabLabelClosingCallback TabLabelClosingCb;
static void* TabLabelClosingUserPtr;
static TabLabelDeletingCallback TabLabelDeletingCb;


public:
TabWindow();
~TabWindow();

bool isInited() const {return init;}
TabLabel* addTabLabel(const char* label,void* userPtr=NULL,const char* tooltip=NULL,bool closable=true,bool draggable=true);
bool removeTabLabel(TabLabel* tab);
void clear();

static void SetWindowContentDrawerCallback(TabLabelCallback _windowContentDrawer,void* userPtr=NULL) {
    WindowContentDrawerCb=_windowContentDrawer;
    WindowContentDrawerUserPtr=userPtr;
}
static void SetTabLabelPopupMenuDrawerCallback(TabLabelCallback _tabLabelPopupMenuDrawer,void* userPtr=NULL) {
    TabLabelPopupMenuDrawerCb=_tabLabelPopupMenuDrawer;
    TabLabelPopupMenuDrawerUserPtr=userPtr;
}
static void SetTabLabelClosingCallback(TabLabelClosingCallback _tabLabelClosing,void* userPtr=NULL) {
    TabLabelClosingCb=_tabLabelClosing;
    TabLabelClosingUserPtr=userPtr;
}
static void SetTabLabelDeletingCallback(TabLabelDeletingCallback _tabLabelDeleting) {TabLabelDeletingCb=_tabLabelDeleting;}

inline static const char* GetTabLabelPopupMenuName() {return "TabWindowTabLabelPopupMenu";}

void render();


static const unsigned char* GetDockPanelIconImagePng(int* bufferSizeOut=NULL); // Manually redrawn based on the ones in https://github.com/dockpanelsuite/dockpanelsuite (that is MIT licensed). So no copyright issues for this AFAIK, but I'm not a lawyer and I cannot guarantee it.
static ImTextureID DockPanelIconTextureID;  // User must load it (using GetDockPanelIconImagePng) and free it.
static ImVec4 SplitterColor;
static float SplitterSize;

protected:

friend struct TabLabel;
friend struct TabNode;
friend struct TabWindowDragData;
};


} // namespace ImGui


#endif //IMGUITABWINDOW_H_
