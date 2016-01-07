#ifndef IMGUITABWINDOW_H_
#define IMGUITABWINDOW_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

// This addon is available here:                        https://gist.github.com/Flix01/2cdf1db8d936100628c0
// and is bundled in the "ImGui Addons Branch" here:    https://github.com/Flix01/imgui/tree/2015-10-Addons
// Wiki about the "ImGui Addons Branch" is here:        https://github.com/Flix01/imgui/wiki/ImGui-Addons-Branch-Home

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
                tabWindow.addTabLabel(tabNames[i],tabTooltips[i]); // see additional args to prevent a tab from closing and from dragging
            }
        }

        tabWindow.render(); // Must be called inside "its" window (and sets isInited() to false)
        ImGui::End();
    }
    // Optional add other ImGui::Window-TabWindow pairs here

2) At init time:
----------------
    // Callbacks:
    ImGui::TabWindow::SetWindowContentDrawerCallback(&TabContentProvider,NULL); // If not set TabWindowLabel::render() will be called instead (if you prefer extending TabWindowLabel)
    ImGui::TabWindow::SetTabLabelPopupMenuDrawerCallback(&TabLabelPopupMenuProvider,NULL);  // Optional (if you need context-menu)
    //ImGui::TabWindow::SetTabLabelClosingCallback(&OnTabLabelClosing,NULL);  // Optional (to fix/modify)
    //ImGui::TabWindow::SetTabLabelDeletingCallback(&OnTabLabelDeleting); // Optional (to delete your userPts)

    // Texture Loading (*)
    if (!ImGui::TabWindow::DockPanelIconTextureID)  {
        int pngIconSize = 0;
        const unsigned char* pngIcon = ImGui::TabWindow::GetDockPanelIconImagePng(&pngIconSize);
        ImGui::TabWindow::DockPanelIconTextureID = reinterpret_cast<ImTextureID>(MyTextureFromMemoryPngMethod(pngIcon,pngIconSize));  // User must load it (using GetDockPanelIconImagePng and in some cases flipping Y to match tex coords unique convention).
    }

    // Optional Style
    //ImGui::TabLabelStyle& tabStyle = ImGui::TabLabelStyle::Get();
    // ... modify tabStyle ...
    //ImGui::TabWindow::SplitterSize = ...;ImGui::TabWindow::SplitterColor = ...;   // modify splitter style

3) At deinit time:
-------------------
    Free: ImGui::TabWindow::DockPanelIconTextureID if not NULL. (*)

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

TIPS ABOUT TEXTURE LOADING;
(*): -> Texture loading/freeing is mandatory only if you're not using an IMGUI_USE_XXX_BINDING, or if you don't know
        what IMGUI_USE_XXX_BINDING is.
     -> If you prefer to load the texture from an external image, I'll provide it here: https://gist.github.com/Flix01/2cdf1db8d936100628c0
     -> Since internally we use texcoords, we had to choose a single convention for it. That means that it might be necessary for
        some people to load the image upside down (stb_image has a build-in method to do it).

*/


// KNOWN BUGS:
/*
-> If you scale the tab labels (e.g. with CTRL+mouse wheel), the dragged tab is not scaled.
*/


// ROADMAP:
/*
-> Add filtering between TabWindows:
   1) TabWindow::isIsolated that prevents any tab label from flying outside
   2) TabWindow::excludeTabWindow(TabWindow& tabWindow) and TabWindow::includeTabWindow(TabWindow& tabWindow) for preventing (or not) tabWindows from exchanging tab labels with each other.
-> In TabLabelStyle:
   1) Remove all ImFont* fields and add enum values (regular, bold ,italic, bolditalic)
      for font, fontSelected, fontModified and fontSelectedModified.
      Then user can assign static TabWindow::ImFont* values for each font enum.
*/


// BETTER ALTERNATIVES:
/*
There are better alternatives to Imgui::TabWindow:
-> https://github.com/thennequin/ImWindow   [for Window OS]
-> https://github.com/nem0/LumixEngine/blob/master/src/studio_lib/imgui/imgui_dock.inl [lumixengine's Dock]
Please see: https://github.com/ocornut/imgui/issues for further info
*/

namespace ImGui {

struct TabLabelStyle {
enum Colors {
    Col_TabLabel = 0,
    Col_TabLabelHovered,
    Col_TabLabelActive,
    Col_TabLabelBorder,
    Col_TabLabelText,

    Col_TabLabelSelected,
    Col_TabLabelSelectedHovered,
    Col_TabLabelSelectedActive,
    Col_TabLabelSelectedBorder,
    Col_TabLabelSelectedText,

    Col_TabLabelCloseButtonHovered,
    Col_TabLabelCloseButtonActive,
    Col_TabLabelCloseButtonBorder,
    Col_TabLabelCloseButtonTextHovered,

    Col_TabLabel_Count
};
ImU32 colors[Col_TabLabel_Count];

float fillColorGradientDeltaIn0_05; // vertical gradient if > 0 (looks nice but it's very slow)
float rounding;
float borderWidth;

ImFont* font;                       // TODO: remove ImFont* and add enum values (regular, bold ,italic, bolditalic)
ImFont* fontSelected;               // for font, fontSelected, fontModified and fontSelectedModified

float closeButtonRounding;
float closeButtonBorderWidth;
float closeButtonTextWidth;

bool antialiasing;

TabLabelStyle();

void reset() {Reset(*this);}
static bool Edit(TabLabelStyle& style);
static void Reset(TabLabelStyle& style) {style = TabLabelStyle();}

// Gets the default style instance (same as TabLabelStyle::style)
inline static TabLabelStyle& Get() {return style;}
// Gets a new instance = default style instance blended with ImGui::GetStyle().Alpha
static const TabLabelStyle& GetMergedWithWindowAlpha();
static TabLabelStyle style;
static const char* ColorNames[Col_TabLabel_Count];

};

class TabWindow {
public:
struct TabLabel {
    friend class TabNode;
    friend class TabWindow;
    friend struct TabWindowDragData;
private:
    // [Nothing is used as ImGui ID: the ImGui ID is the address of the TabLabel]
    char* label;      // [owned] text displayed by the TabLabel (one more char is allocated to optimize appending an asterisk to it)
    char* tooltip;    // [owned] tooltip displayed by the TabLabel
    char* userText;   // [owned] user text
    bool modified;
protected:
    bool closable;
    bool draggable;
    TabLabel(const char* _label=NULL,const char* _tooltip=NULL,bool _closable=true,bool _draggable=true)    {
        label = tooltip = NULL;
        userPtr = NULL;userText=NULL;
        setLabel(_label);
        setTooltip(_tooltip);
        closable = _closable;
        draggable = _draggable;
        mustCloseNextFrame = false;
        mustSelectNextFrame = false;
        wndFlags = 0;
        modified = false;
    }
    virtual ~TabLabel() {
        if (label) {ImGui::MemFree(label);label=NULL;}
        if (tooltip) {ImGui::MemFree(tooltip);tooltip=NULL;}
        if (userText) {ImGui::MemFree(userText);userText=NULL;}
    }
    static void DestroyTabLabel(TabLabel*& tab);
public:
    inline const char* getLabel() const {return label;}
    void setLabel(const char* lbl,bool appendAnAsteriskAndMarkAsModified=false)  {
        if (label) {ImGui::MemFree(label);label=NULL;}
        const char e = '\0';if (!lbl) lbl=&e;
        const int sz = strlen(lbl)+1;       // we allocate one char more (optimization for appending an asterisk)
        label = (char*) ImGui::MemAlloc(sz+1);strcpy(label,lbl);
        if (appendAnAsteriskAndMarkAsModified)  {
            modified = true;strcat(label,"*");
        }
        else modified = false;
    }
    inline bool getModified() const {return modified;}
    void setModified(bool flag) {
        if (modified == flag) return;
        modified = flag;int sz = strlen(label);
        if (modified)   {if (sz==0 || label[sz-1]!='*') strcat(label,"*");}
        else            {if (sz>0 && label[sz-1]=='*') label[sz-1]='\0';}
    }
    inline const char* getTooltip() const {return tooltip;}
    void setTooltip(const char* tt)  {
        if (tooltip) {ImGui::MemFree(tooltip);tooltip=NULL;}
        const char e = '\0';if (!tt) tt=&e;
        const int sz = strlen(tt);
        tooltip = (char*) ImGui::MemAlloc(sz+1);strcpy(tooltip,tt);
    }
    void setUserText(const char* _userText)  {
        if (userText) {ImGui::MemFree(userText);userText=NULL;}
        if (_userText)  {
            const int sz = strlen(_userText);
            userText = (char*) ImGui::MemAlloc(sz+1);strcpy(userText,_userText);
        }
    }
    inline const char* getUserText() const {return userText;}
    mutable void* userPtr;
    mutable bool mustCloseNextFrame;
    mutable bool mustSelectNextFrame;
    mutable int wndFlags;               // used for the imgui child window that host the tab content

    // This method will be used ONLY if you don't set TabWindow::SetWindowContentDrawerCallback(...),
    // and you prefer extending from this class
    virtual void render() {
        ImGui::TextWrapped("Here is the content of tab label: \"%s\". Please consider using ImGui::TabWindow::SetWindowContentDrawerCallback(...) to set a callback for it, or extending TabWindowLabel and implement its render() method. ",getLabel());
    }


};
protected:
public:
typedef void (*TabLabelCallback)(TabLabel* tabLabel,TabWindow& parent,void* userPtr);
typedef void (*TabLabelClosingCallback)(const ImVector<TabLabel*>& tabLabels,const ImVector<TabWindow*>& parents,ImVector<bool>& forbidClosingOut,void* userPtr);
typedef void (*TabLabelDeletingCallback)(TabLabel* tabLabel);

protected:
struct TabNode* mainNode;   // owned
struct TabNode* activeNode; // reference
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

// Handy for initialization before calling render() the firsat time
bool isInited() const {return init;}

// Here "label" is NOT used as ImGui ID (you shouldn't worry about it): it's just the text you want to display
TabLabel* addTabLabel(const char* label,const char* tooltip=NULL,bool closable=true,bool draggable=true,void* userPtr=NULL,const char* userText=NULL,int ImGuiWindowFlagsForContent=0);
TabLabel* addTabLabel(TabLabel* tabLabel,bool checkIfAlreadyPresent=true);  // use it only if you extend TabLabel
bool removeTabLabel(TabLabel* tab);
void clear();

// Find methods (untested)
TabLabel* findTabLabelFromTooltip(const char* tooltip) const;
TabLabel* findTabLabelFromUserPtr(void* userPtr) const;
TabLabel* findTabLabelFromUserText(const char* userText) const;
static TabLabel* FindTabLabelFromTooltip(const char* tooltip,const TabWindow* pTabWindows,int numTabWindows,int* pOptionalTabWindowIndexOut);
static TabLabel* FindTabLabelFromUserPtr(void* userPtr,const TabWindow* pTabWindows,int numTabWindows,int* pOptionalTabWindowIndexOut);
static TabLabel* FindTabLabelFromUserText(const char* userText,const TabWindow* pTabWindows,int numTabWindows,int* pOptionalTabWindowIndexOut);

// Callbacks
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

// Main method
void render();

// Texture And Memory Png Data
static const unsigned char* GetDockPanelIconImagePng(int* bufferSizeOut=NULL); // Manually redrawn based on the ones in https://github.com/dockpanelsuite/dockpanelsuite (that is MIT licensed). So no copyright issues for this AFAIK, but I'm not a lawyer and I cannot guarantee it.
static ImTextureID DockPanelIconTextureID;  // User must load it (using GetDockPanelIconImagePng) and free it when no IMGUI_USE_XXX_BINDING is used.

// Style
static ImVec4 SplitterColor;
static float SplitterSize;

protected:

friend struct TabLabel;
friend struct TabNode;
friend struct TabWindowDragData;
};
typedef TabWindow::TabLabel TabWindowLabel;


// This has nothing to do with TabWindow; it's just a port of this gist: https://gist.github.com/Flix01/3bc3d7b3d996582e034e sharing "TabLabelStyle".
// Please see: https://github.com/ocornut/imgui/issues/261 for further info
// Based on the code by krys-spectralpixel (https://github.com/krys-spectralpixel), posted here: https://github.com/ocornut/imgui/issues/261
/* pOptionalHoveredIndex: a ptr to an optional int that is set to -1 if no tab label is hovered by the mouse.
 * pOptionalItemOrdering: an optional static array of unique integers from 0 to numTabs-1 that maps the tab label order. If one of the numbers is replaced by -1 the tab label is not visible (closed). It can be read/modified at runtime.
 * allowTabReorder (requires pOptionalItemOrdering): allows tab reordering through drag and drop (it modifies pOptionalItemOrdering).
 *                  However it seems to work only when dragging tabs from the left (top) to the right (bottom) and not vice-versa (this is bad, but can't lock the tab order in any way).
 * allowTabClosing (requires pOptionalItemOrdering): adds a close button to the tab. When the close button is clicked, the tab value in pOptionalItemOrdering is set to -1.
 * pOptionalClosedTabIndex (requires allowTabClosing): out variable (int pointer) that returns the index of the closed tab in last call or -1.
 * pOptionalClosedTabIndexInsideItemOrdering (requires allowTabClosing): same as above, but index inside the pOptionalItemOrdering array. Users can use this value to prevent single tabs from closing when their close button is clicked (since we can't mix closable and non-closable tabs here).
*/
bool TabLabels(int numTabs, const char** tabLabels, int& selectedIndex, const char** tabLabelTooltips=NULL , bool wrapMode=true, int* pOptionalHoveredIndex=NULL, int* pOptionalItemOrdering=NULL, bool allowTabReorder=true, bool allowTabClosing=false, int* pOptionalClosedTabIndex=NULL,int * pOptionalClosedTabIndexInsideItemOrdering=NULL);


} // namespace ImGui


#endif //IMGUITABWINDOW_H_
