#include "imguitabwindow.h"
#include <imgui_internal.h>
#include <imgui.h>  // intellisense

// TODO: Clean this code, it's a mess!

namespace ImGui {

void TabWindow::TabLabel::DestroyTabLabel(TabWindow::TabLabel*& tab)  {
    if (TabWindow::TabLabelDeletingCb) TabWindow::TabLabelDeletingCb(tab);
    tab->~TabLabel();
    ImGui::MemFree(tab);
    tab=NULL;
}

struct TabNode  {
    friend class TabLabel;
    friend class TabWindow;
    friend struct TabWindowDragData;

    ImVector<TabWindow::TabLabel* > tabs;   // only in leaf nodes
    TabWindow::TabLabel* selectedTab;
    TabNode *parent;   // (reference)
    TabNode *child[2];  // (owned)
    char* name;         // (owned)
    float splitterPerc; // in [0,1]
    bool horizontal;
    mutable int contentWindowFlags;
    TabNode() {tabs.clear();selectedTab=NULL;parent=NULL;contentWindowFlags=0;for (int i=0;i<2;i++) child[i]=NULL;name=NULL;
               horizontal=false;splitterPerc=0.5f;}
    ~TabNode() {
        clear();
        if (name) {ImGui::MemFree(name);name=NULL;}
    }
    inline bool isLeafNode() const {return (!child[0] && !child[1]);}
    void clear()  {
        for (int i=0;i<2;i++) {
            TabNode*& ch = child[i];
            if (ch) {
                ch->clear();  // delete child nodes too
                ch->~TabNode();
                ImGui::MemFree(ch);
                ch=NULL;
            }
        }
        for (int i=0,isz=tabs.size();i<isz;i++) {
            TabWindow::TabLabel*& tab = tabs[i];
            TabWindow::TabLabel::DestroyTabLabel(tab);
        }
        tabs.clear();
    }
    TabNode *addTabLabel(TabWindow::TabLabel *tab, int childPosLTRB=-1, int pos=-1)     {
        IM_ASSERT(tab);
        IM_ASSERT(this->isLeafNode());
        IM_ASSERT(!parent || (!parent->isLeafNode() && parent->child[0] && parent->child[1] && parent->tabs.size()==0));
        if (childPosLTRB==-1)   {
            if (pos<0 || pos>tabs.size()) pos=tabs.size();
            tabs.push_back(tab);
            for (int i=tabs.size()-2;i>=pos;--i) tabs[i+1] = tabs[i];
            tabs[pos] = tab;
            return this;
        }
        IM_ASSERT(childPosLTRB>=0 && childPosLTRB<4);
        horizontal = (childPosLTRB==1 || childPosLTRB==3);
        splitterPerc = 0.5f;
        const bool spFirst = (childPosLTRB==0 || childPosLTRB==1);
        // create the two child nodes
        for (int i=0;i<2;i++)   {
            TabNode* ch = child[i];
            ch = (TabNode*) ImGui::MemAlloc(sizeof(TabNode));
            new (ch) TabNode();
            child[i] = ch;
            ch->parent = this;
            /*if (name) {
            int sz = strlen(name)+8;
            ch->name = (char*) ImGui::MemAlloc(sz);
            strcpy(ch->name,name);
            strcat(ch->name,".child");
            sprintf(&ch->name[sz-2],"%d",i);
            ch->name[sz-1]='\0';
            //fprintf(stderr,"Added TabNode: \"%s\"\n",ch->name); // "##main.child3"
        }*/
        }
        assignChildNames(false);

        // We must move tabs to child[]:
        TabNode* ch = spFirst ? child[1] : child[0];
        ch->tabs.resize(tabs.size());
        for (int i=0,isz=tabs.size();i<isz;i++) {
            TabWindow::TabLabel* tab = tabs[i];
            ch->tabs[i] = tab;
        }
        tabs.clear();
        ch->selectedTab = selectedTab;
        selectedTab = NULL;
        // We must insert tab
        ch = spFirst ? child[0] : child[1];
        ch->selectedTab = tab;
        return ch->addTabLabel(tab,-1,pos);
    }
    TabNode* findTabLabel(TabWindow::TabLabel* tab,bool recursive=false)  {
        if (!tab) return NULL;
        if (recursive) {
            TabNode * n = NULL;
            for (int i=0;i<2;i++)
                if ((n=child[i]->findTabLabel(tab,true))) return n;
        }
        for (int i=0,isz=tabs.size();i<isz;i++)
            if (tabs[i]==tab) return this;
        return NULL;
    }
    bool removeTabLabel(TabWindow::TabLabel* tab,bool recursive=false,TabNode** pOptionalActiveTabNodeToChange=NULL,bool dontDeleteTabLabel=false)  {
        if (!tab) return false;
        if (recursive) {
            for (int i=0;i<2;i++)
                if (child[i] && child[i]->removeTabLabel(tab,true,pOptionalActiveTabNodeToChange,dontDeleteTabLabel)) return true;
        }
        IM_ASSERT(tab);
        IM_ASSERT(this->isLeafNode());
        for (int i=0,isz=tabs.size();i<isz;i++) {
            if (tabs[i]==tab) {
                if (selectedTab == tab) selectedTab = NULL;
                if (!dontDeleteTabLabel) TabWindow::TabLabel::DestroyTabLabel(tabs[i]);
                for (int j=i;j<isz-1;j++) tabs[j] = tabs[j+1];
                tabs.pop_back();
                if (tabs.size()==0 && parent) {
                    // We must merge this with parent
                    TabNode* parent = this->parent;
                    IM_ASSERT(parent->child[0] && parent->child[1]);
                    IM_ASSERT(parent->child[0]==this || parent->child[1]==this);
                    IM_ASSERT(parent->child[0]!=parent->child[1]);

                    int id = parent->child[0]==this ? 0 : 1;
                    // delete parent->child[id]: it's empty (Hey! that's me! Am I allowed delete myself?)
                    {
                        TabNode* ch = parent->child[id];
                        IM_ASSERT(ch==this);
                        IM_ASSERT(ch->isLeafNode());
                        parent->child[id] = NULL;
                        if (pOptionalActiveTabNodeToChange && *pOptionalActiveTabNodeToChange==ch) *pOptionalActiveTabNodeToChange=parent;
                        IM_ASSERT(ch->tabs.size()==0);
                        // We defer deleting it at the bottom of this method for extended safety
                    }
                    // merge the other child with parent
                    id = (id == 1) ? 0 : 1;// other parent child
                    {
                        TabNode* ch = parent->child[id];
                        if (ch->isLeafNode())   {
                            if (pOptionalActiveTabNodeToChange && *pOptionalActiveTabNodeToChange==ch) *pOptionalActiveTabNodeToChange=parent;
                            IM_ASSERT(parent->tabs.size()==0);
                            parent->tabs.resize(ch->tabs.size());
                            for (int i=0,isz=ch->tabs.size();i<isz;i++) {
                                parent->tabs[i] = ch->tabs[i];
                            }
                            ch->tabs.clear();
                            parent->selectedTab = ch->selectedTab;
                            parent->splitterPerc = 0.5f;

                            parent->child[id] = NULL;
                        }
                        else {
                            IM_ASSERT(ch->tabs.size()==0);
                            IM_ASSERT(parent->tabs.size()==0);

                            // We must replace "parent" with "ch" and then delete "parent"
                            // Nope: it's better to "deep clone "ch" to "parent" and delete "ch"

                            if (pOptionalActiveTabNodeToChange && *pOptionalActiveTabNodeToChange==ch) *pOptionalActiveTabNodeToChange=parent;

                            if (ch->name) {ImGui::MemFree(ch->name);ch->name=NULL;}
                            parent->child[0] = ch->child[0];ch->child[0]=NULL;
                            parent->child[1] = ch->child[1];ch->child[1]=NULL;ch->parent=NULL;
                            parent->child[0]->parent = parent->child[1]->parent = parent;
                            parent->contentWindowFlags = ch->contentWindowFlags;
                            parent->horizontal = ch->horizontal;
                            parent->selectedTab = ch->selectedTab;
                            parent->splitterPerc = ch->splitterPerc;
                            parent->assignChildNames(true);

                        }

                        // delete the other child
                        ch->~TabNode();
                        ImGui::MemFree(ch);
                        ch = NULL;
                        // delete me
                        ch = this;
                        ch->~TabNode();
                        ImGui::MemFree(ch);
                    }



                }
                return true;
            }
        }
        return false;
    }
    bool isEmpty(bool recursive=false) {
        if (tabs.size()!=0) return false;
        if (recursive) {
            for (int i=0;i<2;i++)
                if (child[i] && !child[i]->isEmpty(true)) return false;
        }
        return true;
    }
    TabNode* getFirstLeaftNode() {return isLeafNode() ? this : child[0]->getFirstLeaftNode();}
    void setName(const char* lbl)  {
        if (name) {ImGui::MemFree(name);name=NULL;}
        const char e = '\0';if (!lbl) lbl=&e;
        const int sz = strlen(lbl)+1;
        name = (char*) ImGui::MemAlloc(sz+1);strcpy(name,lbl);
    }
    void assignChildNames(bool recursive=false)  {
        const int sz = strlen(name)+8;
        for (int i=0;i<2;i++) {
            TabNode* ch = child[i];
            if (!ch) continue;
            if (ch->name) {ImGui::MemFree(ch->name);ch->name=NULL;}
            ch->name = (char*) ImGui::MemAlloc(sz);
            strcpy(ch->name,name);
            strcat(ch->name,".child");
            sprintf(&ch->name[sz-2],"%d",i);
            ch->name[sz-1]='\0';
            if (recursive) ch->assignChildNames(true);
        }
    }

    void render(const ImVec2& windowSize,void* ptr);
};


struct TabWindowDragData {
    TabWindow::TabLabel* draggingTabSrc;
    TabNode* draggingTabNodeSrc;
    ImGuiWindow* draggingTabImGuiWindowSrc;
    TabWindow* draggingTabWindowSrc;
    ImVec2 draggingTabSize;
    ImVec2 draggingTabOffset;

    TabWindow::TabLabel* draggingTabDst;
    TabNode* draggingTabNodeDst;
    ImGuiWindow* draggingTabImGuiWindowDst;
    TabWindow* draggingTabWindowDst;

    TabWindowDragData() {reset();}
    void resetDraggingSrc() {
        draggingTabSrc = NULL;
        draggingTabNodeSrc = NULL;
        draggingTabImGuiWindowSrc = NULL;
        draggingTabWindowSrc = NULL;
        draggingTabSize = draggingTabOffset = ImVec2(0,0);
    }
    void resetDraggingDst() {
        draggingTabDst = NULL;
        draggingTabNodeDst = NULL;
        draggingTabImGuiWindowDst = NULL;
        draggingTabWindowDst = NULL;
    }
    inline void reset() {resetDraggingSrc();resetDraggingDst();}
    inline bool isDraggingSrcValid() const {
        return (draggingTabSrc && draggingTabNodeSrc && draggingTabImGuiWindowSrc);
    }
    inline bool isDraggingDstValid() const {
        return (draggingTabDst && draggingTabNodeDst && draggingTabImGuiWindowDst);
    }
    inline int findDraggingSrcIndex(const TabWindow::TabLabel* tab=NULL) const {
        if (!tab) tab = draggingTabSrc;
        for (int i=0,isz=draggingTabNodeSrc->tabs.size();i<isz;i++) {
            if (draggingTabNodeSrc->tabs[i] == tab) return i;
        }
        return -1;
    }
    inline int findDraggingDstIndex(const TabWindow::TabLabel* tab=NULL) const {
        if (!tab) tab = draggingTabDst;
        for (int i=0,isz=draggingTabNodeDst->tabs.size();i<isz;i++) {
            if (draggingTabNodeDst->tabs[i] == tab) return i;
        }
        return -1;
    }
    inline static TabNode* FindTabNodeByName(TabNode* firstNode,const char* name,int numCharsToMatch=-1) {
        if ((numCharsToMatch==-1 && strcmp(firstNode->name,name)==0)
            || (strncmp(firstNode->name,name,numCharsToMatch)==0)) return firstNode;
        TabNode* rv = NULL;
        for (int i=0;i<2;i++)   {
            TabNode* ch = firstNode->child[i];
            if (ch && (rv=FindTabNodeByName(ch,name,numCharsToMatch))) return rv;
        }
        return NULL;
    }

    inline void drawDragButton(ImDrawList* drawList,const ImVec2& wp,const ImVec2& mp,const float draggedBtnAlpha=0.75f)   {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 start(wp.x+mp.x-draggingTabOffset.x-draggingTabSize.x*0.5f,wp.y+mp.y-draggingTabOffset.y-draggingTabSize.y*0.5f);
        const ImVec2 end(start.x+draggingTabSize.x,start.y+draggingTabSize.y);
        const ImVec4& btnColor = style.Colors[ImGuiCol_Button];
        drawList->AddRectFilled(start,end,ImColor(btnColor.x,btnColor.y,btnColor.z,btnColor.w*draggedBtnAlpha),style.FrameRounding);
        start.x+=style.FramePadding.x;start.y+=style.FramePadding.y;
        const ImVec4& txtColor = style.Colors[ImGuiCol_Text];
        drawList->AddText(start,ImColor(txtColor.x,txtColor.y,txtColor.z,txtColor.w*draggedBtnAlpha),draggingTabSrc->label);
    }

};
static TabWindowDragData gDragData;
struct MyTabWindowHelperStruct {
    bool isRMBclicked;
    bool isMMBreleased;
    static bool isMouseDragging;
    bool isASplitterActive;
    static TabWindow::TabLabel* tabLabelPopup;
    static bool tabLabelPopupChanged;

    TabWindow* tabWindow;

    static ImVector<TabWindow::TabLabel*> TabsToClose;
    static ImVector<TabNode*> TabsToCloseNodes;
    static ImVector<TabWindow*> TabsToCloseParents;

    ImVec2 itemSpacing;
    ImVec4 color;
    ImVec4 colorActive;
    ImVec4 colorHover;
    ImVec4 colorText;
    ImVec4 colorSelectedTab;
    ImVec4 colorSelectedTabHovered;
    ImVec4 colorSelectedTabText;

    ImVec4 splitterColor;
    ImVec4 splitterColorHovered;
    ImVec4 splitterColorActive;

    MyTabWindowHelperStruct(TabWindow* _tabWindow) {
        isMMBreleased = ImGui::IsMouseReleased(2);
        isMouseDragging = ImGui::IsMouseDragging(0,2.f);
        isRMBclicked = ImGui::IsMouseClicked(1);
        isASplitterActive = false;
        tabWindow = _tabWindow;

        ImGuiStyle& style = ImGui::GetStyle();
        color =        style.Colors[ImGuiCol_Button];
        colorActive =  style.Colors[ImGuiCol_ButtonActive];
        colorHover =   style.Colors[ImGuiCol_ButtonHovered];
        colorText =    style.Colors[ImGuiCol_Text];
        itemSpacing =   style.ItemSpacing;

        colorSelectedTab=ImVec4(color.x,color.y,color.z,color.w*0.5f);
        colorSelectedTabHovered=ImVec4(colorHover.x,colorHover.y,colorHover.z,colorHover.w*0.5f);
        colorSelectedTabText=ImVec4(colorText.x*0.8f,colorText.y*0.8f,colorText.z*0.6f,colorText.w*0.8f);

        splitterColor = ImVec4(TabWindow::SplitterColor.x,TabWindow::SplitterColor.y,TabWindow::SplitterColor.z,0.2f);
        splitterColorHovered = ImVec4(TabWindow::SplitterColor.x,TabWindow::SplitterColor.y,TabWindow::SplitterColor.z,0.35f);
        splitterColorActive = ImVec4(TabWindow::SplitterColor.x,TabWindow::SplitterColor.y,TabWindow::SplitterColor.z,0.5f);

        storeStyleVars();
    }
    ~MyTabWindowHelperStruct() {restoreStyleVars();restoreStyleColors();}
    inline void restoreStyleColors() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Button] =         color;
        style.Colors[ImGuiCol_ButtonActive] =   colorActive;
        style.Colors[ImGuiCol_ButtonHovered] =  colorHover;
        style.Colors[ImGuiCol_Text] =           colorText;
    }
    inline void storeStyleVars() {ImGui::GetStyle().ItemSpacing = ImVec2(1,1);}
    inline void restoreStyleVars() {ImGui::GetStyle().ItemSpacing = itemSpacing;}

    inline static void ResetTabsToClose() {
        TabsToClose.clear();TabsToCloseNodes.clear();TabsToCloseParents.clear();
    }
};
TabWindow::TabLabel* MyTabWindowHelperStruct::tabLabelPopup = NULL;
bool  MyTabWindowHelperStruct::tabLabelPopupChanged = false;
bool MyTabWindowHelperStruct::isMouseDragging = false;
ImVector<TabWindow::TabLabel*> MyTabWindowHelperStruct::TabsToClose;
ImVector<TabNode*> MyTabWindowHelperStruct::TabsToCloseNodes;
ImVector<TabWindow*> MyTabWindowHelperStruct::TabsToCloseParents;
ImVec4 TabWindow::SplitterColor(1,1,1,1);
float TabWindow::SplitterSize(8.f);
TabWindow::TabLabelCallback TabWindow::WindowContentDrawerCb=NULL;
void* TabWindow::WindowContentDrawerUserPtr=NULL;
TabWindow::TabLabelCallback TabWindow::TabLabelPopupMenuDrawerCb=NULL;
void* TabWindow::TabLabelPopupMenuDrawerUserPtr=NULL;
TabWindow::TabLabelClosingCallback TabWindow::TabLabelClosingCb=NULL;
void* TabWindow::TabLabelClosingUserPtr=NULL;
TabWindow::TabLabelDeletingCallback TabWindow::TabLabelDeletingCb=NULL;

void TabNode::render(const ImVec2 &windowSize, void *ptr)
{   
    const float splitterSize = TabWindow::SplitterSize;
    bool splitterActive = false;
    MyTabWindowHelperStruct& mhs = *((MyTabWindowHelperStruct*)ptr);


    IM_ASSERT(name);
    if (child[0])   {
        IM_ASSERT(child[1]);
        IM_ASSERT(tabs.size()==0);
        ImGui::BeginChild(name,windowSize,false,ImGuiWindowFlags_NoScrollbar);

        ImVec2 ws = windowSize;
        float splitterPercToPixels = 0.f,splitterDelta = 0.f;
        if (horizontal) {
            splitterPercToPixels = ws.y*splitterPerc;
            child[0]->render(ImVec2(ws.x,splitterPercToPixels),ptr);
            // Horizontal Splitter ------------------------------------------
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
            ImGui::PushStyleColor(ImGuiCol_Button,mhs.splitterColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mhs.splitterColorHovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,mhs.splitterColorActive);
            ImGui::PushID(this);
            ImGui::Button("##splitter0", ImVec2(ws.x,splitterSize));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            splitterActive = !mhs.isASplitterActive && ImGui::IsItemActive();
            mhs.isASplitterActive |= splitterActive;
            if (splitterActive)  splitterDelta = ImGui::GetIO().MouseDelta.y;
            else splitterDelta = 0.f;
            if (splitterActive)  {
                float& h = splitterPercToPixels;
                const float minh = splitterSize;
                const float maxh = ws.y-splitterSize-20.f;          // Warning: 20.f is hard-coded!
                if (h+splitterDelta>maxh)           splitterDelta = (h!=maxh) ? (maxh-h) : 0.f;
                else if (h+splitterDelta<minh)      splitterDelta = (h!=minh) ? (minh-h) : 0.f;
                h+=splitterDelta;
                splitterPerc = splitterPercToPixels/ws.y;
            }
            ImGui::PopID();
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
            //------------------------------------------------------
            child[1]->render(ImVec2(ws.x,ws.y-splitterPercToPixels),ptr);
        }
        else {
            splitterPercToPixels = ws.x*splitterPerc;
            child[0]->render(ImVec2(splitterPercToPixels,ws.y),ptr);
            // Vertical Splitter ------------------------------------------
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
            ImGui::PushStyleColor(ImGuiCol_Button,mhs.splitterColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mhs.splitterColorHovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,mhs.splitterColorActive);
            ImGui::PushID(this);
            ImGui::SameLine(0,0);
            ImGui::Button("##splitter1", ImVec2(splitterSize,ws.y));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            splitterActive = !mhs.isASplitterActive && ImGui::IsItemActive();
            mhs.isASplitterActive |= splitterActive;
            if (splitterActive)  splitterDelta = ImGui::GetIO().MouseDelta.x;
            else splitterDelta = 0.f;
            if (splitterActive)  {
                float& w = splitterPercToPixels;
                const float minw = splitterSize;
                const float maxw = ws.x-splitterSize;
                if (w + splitterDelta>maxw)         splitterDelta = (w!=maxw) ? (maxw-w) : 0.f;
                else if (w + splitterDelta<minw)    splitterDelta = (w!=minw) ? (minw-w) : 0.f;
                w+=splitterDelta;
                splitterPerc = splitterPercToPixels/ws.x;
            }
            ImGui::SameLine(0,0);
            ImGui::PopID();
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
            //------------------------------------------------------
            child[1]->render(ImVec2(ws.x-splitterPercToPixels,ws.y),ptr);
        }

        ImGui::EndChild();  // name
        return;
    }

    // Leaf Node
    IM_ASSERT(!child[1]);

    ImGui::BeginChild(name,windowSize,false,ImGuiWindowFlags_NoScrollbar);

    //fprintf(stderr,"%s\n",name);

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiState& g = *GImGui;
    TabWindowDragData& dd = gDragData;

    //TabWindow::TabLabel* hoveredTab = NULL;
    //----------------------------------------------------------------
    {
        //ImGui::BeginGroup();
        const int numTabs = tabs.size();
        if (numTabs>0 && !selectedTab) selectedTab = tabs[0];

        float windowWidth = 0.f,sumX=0.f;
        windowWidth = ImGui::GetWindowWidth() - style.WindowPadding.x - (ImGui::GetScrollMaxY()>0 ? style.ScrollbarSize : 0.f);
        TabWindow::TabLabel* newSelectedTab = selectedTab;

        bool selection_changed = false;
        for (int i = 0; i < numTabs; i++)
        {
            TabWindow::TabLabel& tab = *tabs[i];
            if (sumX > 0.f) {
                sumX+=style.ItemSpacing.x;   // Maybe we can skip it if we use SameLine(0,0) below
                sumX+=ImGui::CalcTextSize(tab.label).x+2.f*style.FramePadding.x;
                if (sumX>windowWidth) sumX = 0.f;
                else ImGui::SameLine();
            }

            if (selectedTab == &tab) {
                // Push the style
                style.Colors[ImGuiCol_Button] =         mhs.colorSelectedTab;
                style.Colors[ImGuiCol_ButtonActive] =   mhs.colorSelectedTab;
                style.Colors[ImGuiCol_ButtonHovered] =  mhs.colorSelectedTabHovered;
                style.Colors[ImGuiCol_Text] =           mhs.colorSelectedTabText;
            }
            // Draw the button
            ImGui::PushID(&tab);   // otherwise two tabs with the same name would clash.
            if (ImGui::Button(tab.label) || tab.mustSelectNextFrame)   {selection_changed = (selectedTab != &tab);newSelectedTab = &tab;tab.mustSelectNextFrame = false;}
            ImGui::PopID();
            if (selectedTab == &tab) {
                // Reset the style
                style.Colors[ImGuiCol_Button] =         mhs.color;
                style.Colors[ImGuiCol_ButtonActive] =   mhs.colorActive;
                style.Colors[ImGuiCol_ButtonHovered] =  mhs.colorHover;
                style.Colors[ImGuiCol_Text] =           mhs.colorText;
            }
            if (sumX==0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line

            if (tab.mustCloseNextFrame) {
                tab.mustCloseNextFrame = false;
                mhs.TabsToClose.push_back(&tab);
                mhs.TabsToCloseNodes.push_back(this);
                mhs.TabsToCloseParents.push_back(mhs.tabWindow);
            }
            else if (ImGui::IsItemHoveredRect()) {
                //hoveredTab = &tab;
                if (tab.tooltip && strlen(tab.tooltip)>0 && &tab!=mhs.tabLabelPopup)  ImGui::SetTooltip("%s",tab.tooltip);                

                if (mhs.isMouseDragging && !mhs.isASplitterActive) {
                    if (!dd.draggingTabSrc && tab.draggable) {

                        dd.draggingTabSrc = &tab;
                        dd.draggingTabNodeSrc = this;
                        dd.draggingTabImGuiWindowSrc = g.HoveredWindow;
                        dd.draggingTabWindowSrc = mhs.tabWindow;

                        dd.draggingTabSize = ImGui::GetItemRectSize();
                        const ImVec2& mp = ImGui::GetIO().MousePos;
                        const ImVec2 draggingTabCursorPos = ImGui::GetCursorPos();
                        dd.draggingTabOffset=ImVec2(
                                    mp.x+dd.draggingTabSize.x*0.5f-sumX+ImGui::GetScrollX(),
                                    mp.y+dd.draggingTabSize.y*0.5f-draggingTabCursorPos.y+ImGui::GetScrollY()
                                    );

                        //fprintf(stderr,"Hovered Start Window:%s\n",g.HoveredWindow ? g.HoveredWindow->Name : "NULL");
                    }
                }
                else if (dd.draggingTabSrc && dd.draggingTabSrc!=&tab){
                    dd.draggingTabDst = &tab;
                    dd.draggingTabNodeDst = this;
                    dd.draggingTabImGuiWindowDst = g.HoveredWindow;
                    dd.draggingTabWindowDst = mhs.tabWindow;
                }

                if (mhs.isRMBclicked && TabWindow::TabLabelPopupMenuDrawerCb) {
                    //ImGuiState& g = *GImGui; while (g.OpenedPopupStack.size() > 0) g.OpenedPopupStack.pop_back();   // Close all existing context-menus
                    //ImGui::OpenPopup(TabWindow::GetTabLabelPopupMenuName());
                    mhs.tabLabelPopup = (TabWindow::TabLabel*)&tab;
                    mhs.tabLabelPopupChanged = true;
                    // fprintf(stderr,"open popup\n");  // This gets actually called...
                }
                else if (tab.closable)  {
                    if (mhs.isMMBreleased) {
                        mhs.TabsToClose.push_back(&tab);
                        mhs.TabsToCloseNodes.push_back(this);
                        mhs.TabsToCloseParents.push_back(mhs.tabWindow);
                    }
                }
            }
        }

        selectedTab = newSelectedTab;
        if (selection_changed) mhs.tabWindow->activeNode = this;

        //ImGui::EndGroup();//allTabsSize = ImGui::GetItemRectSize();
        //----------------------------------------------------------------
        mhs.restoreStyleColors();   // safe to be called without matching
        mhs.restoreStyleVars();     // needs matching
        ImGui::BeginChild("user",ImVec2(0,0),false,contentWindowFlags);
        if (/*selectedTab &&*/ TabWindow::WindowContentDrawerCb) {
            TabWindow::WindowContentDrawerCb(selectedTab,*mhs.tabWindow,TabWindow::WindowContentDrawerUserPtr);
        }
        ImGui::EndChild();  // user
        mhs.storeStyleVars();
    }
    //----------------------------------------------------------------

    ImGui::EndChild();  // name

}


void TabWindow::render()
{
    IM_ASSERT(ImGui::GetCurrentWindow());   // Call me inside a window

    if (!init) {init=true;}
    if (!activeNode) activeNode = mainNode->getFirstLeaftNode();

    ImVec2 windowSize = ImGui::GetWindowSize();   // TabWindow::render() must be called inside a Window ATM
    windowSize.x-=2.f*ImGui::GetStyle().WindowPadding.x;
    windowSize.y-=2.f*ImGui::GetStyle().WindowPadding.y;

    TabWindowDragData& dd = gDragData;

    static int frameCnt = -1;
    ImGuiState& g = *GImGui;
    if (frameCnt!=g.FrameCount) {
        frameCnt=g.FrameCount;
        //--------------------------------------------------------------
        // Some "static" actions here:
        //--------------------------------------------------------------
        // 1) Close Tabs
        //--------------------------------------------------------------
        if (MyTabWindowHelperStruct::TabsToClose.size()>0)   {
            const int sz = MyTabWindowHelperStruct::TabsToClose.size();
            ImVector<bool> dontCloseTabLabels;
            dontCloseTabLabels.resize(sz);
            for (int i=0;i<sz;i++) dontCloseTabLabels[i]=false;
            if (TabWindow::TabLabelClosingCb)   {
                TabWindow::TabLabelClosingCb(MyTabWindowHelperStruct::TabsToClose,MyTabWindowHelperStruct::TabsToCloseParents,dontCloseTabLabels,TabWindow::TabLabelClosingUserPtr);
            }
            for (int i=0;i<sz;i++) {
                if (!dontCloseTabLabels[i]) {
                    TabNode* node = MyTabWindowHelperStruct::TabsToCloseNodes[i];
                    TabLabel* tabLabel = MyTabWindowHelperStruct::TabsToClose[i];
                    TabWindow* tabWindow = MyTabWindowHelperStruct::TabsToCloseParents[i];

                    if (MyTabWindowHelperStruct::tabLabelPopup == tabLabel) MyTabWindowHelperStruct::tabLabelPopup = NULL;
                    if (dd.draggingTabSrc == tabLabel) dd.resetDraggingSrc();
                    if (dd.draggingTabDst == tabLabel) dd.resetDraggingDst();

                    if (!node->removeTabLabel(tabLabel,false,&tabWindow->activeNode))   {
                        fprintf(stderr,"Error: Can't delete TabLabel: \"%s\"\n",tabLabel->getLabel());
                    }
                }
            }
        }
        MyTabWindowHelperStruct::ResetTabsToClose();
        // 2) Display Tab Menu ------------------------------------------
        if (TabLabelPopupMenuDrawerCb && MyTabWindowHelperStruct::tabLabelPopup) {
            if (MyTabWindowHelperStruct::tabLabelPopupChanged) {
                MyTabWindowHelperStruct::tabLabelPopupChanged = false;
                ImGuiState& g = *GImGui; while (g.OpenedPopupStack.size() > 0) g.OpenedPopupStack.pop_back();   // Close all existing context-menus
                ImGui::OpenPopup(TabWindow::GetTabLabelPopupMenuName());
            }
            TabLabelPopupMenuDrawerCb(MyTabWindowHelperStruct::tabLabelPopup,*this,TabLabelPopupMenuDrawerUserPtr);
        }
        // 3) Display dragging button only if no hover window is present (otherwise we need to draw something under it before, see below)
        if (!g.HoveredWindow && dd.draggingTabSrc)  {
            const ImVec2& mp = ImGui::GetIO().MousePos;
            const ImVec2 wp = dd.draggingTabImGuiWindowSrc->Pos;
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRectFullScreen(); // New
            dd.drawDragButton(drawList,wp,mp);
        }
        //----------------------------------------------------------------
        gDragData.resetDraggingDst();
        //----------------------------------------------------------------
    }

    MyTabWindowHelperStruct mhs(this);
    mainNode->render(windowSize,(void*)&mhs);

    // Draw dragging stuff and Apply drag logic -------------------------------------------
    if (g.HoveredRootWindow==ImGui::GetCurrentWindow())
    {
        ImGuiStyle& style = ImGui::GetStyle();
        int hoversInt = 0;  // 1 = center, 3 = center-top, 4 = center-right, 5 = center-bottom, 2 = center-left,

        // Draw tab label while mouse drags it
        if (dd.draggingTabSrc) {
            IM_ASSERT(dd.draggingTabImGuiWindowSrc);
            const ImVec2& mp = ImGui::GetIO().MousePos;
            const ImVec2 wp = dd.draggingTabImGuiWindowSrc->Pos;
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            const ImGuiWindow* hoveredWindow = g.HoveredWindow;
            //const ImGuiWindow* hoveredRootWindow = g.HoveredRootWindow;
            int hoveredWindowNameSz = 0;
            // Window -----------------
            if (hoveredWindow && hoveredWindow!=dd.draggingTabImGuiWindowSrc
                    && (hoveredWindowNameSz=strlen(hoveredWindow->Name))>4 && strcmp(&hoveredWindow->Name[hoveredWindowNameSz-4],"user")==0
                    //&& strncmp(g.ActiveIdWindow->Name,hoveredWindow->Name,hoveredWindowNameSz-5)!=0 // works for g.ActiveIdWindow or g.FocusedWindow
                    )
            {

                // Background
                const ImVec2 wp = hoveredWindow->Pos;
                const ImVec2 ws = hoveredWindow->Size;
                ImVec2 start(wp.x,wp.y);
                ImVec2 end(start.x+ws.x,start.y+ws.y);
                const float draggedBtnAlpha = 0.35f;
                const ImVec4& bgColor = style.Colors[ImGuiCol_TitleBg];
                drawList->AddRectFilled(start,end,ImColor(bgColor.x,bgColor.y,bgColor.z,bgColor.w*draggedBtnAlpha),style.FrameRounding);

                // central quad
                const float defaultQuadAlpha = 0.75f;
                const ImTextureID tid = DockPanelIconTextureID;
                ImU32 quadCol = ImColor(1.f,1.f,1.f,defaultQuadAlpha);
                ImU32 quadColHovered = ImColor(0.5f,0.5f,1.f,1.f);
                const float minDim = ws.x < ws.y ? ws.x : ws.y;
                const float MIN_SIZE = 75.f;
                const float centralQuadDim =(minDim*0.25f)>=MIN_SIZE?(minDim*0.25f):
                                                                     (minDim<MIN_SIZE)?minDim:
                                                                                       (minDim*0.5f)>=MIN_SIZE?(minDim*0.5f):
                                                                                                               MIN_SIZE;
                const float singleQuadDim = centralQuadDim*0.3333333334f;
                ImVec2 uv0,uv1;bool hovers;
                // central quad top
                uv0=ImVec2(0.22916f,0.f);uv1=ImVec2(0.45834f,0.22916f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f-singleQuadDim;
                end.x = start.x+singleQuadDim;
                end.y = start.y+singleQuadDim;
                hovers = ImGui::IsMouseHoveringRect(start,end,false);
                if (hovers) hoversInt = 3;
                drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                // central quad right
                uv0=ImVec2(0.45834f,0.22916f);uv1=ImVec2(0.6875f,0.45834f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f + singleQuadDim;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f;
                end.x = start.x+singleQuadDim;
                end.y = start.y+singleQuadDim;
                hovers = ImGui::IsMouseHoveringRect(start,end,false);
                if (hovers) hoversInt = 4;
                drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                // central quad bottom
                uv0=ImVec2(0.22916f,0.45834f);uv1=ImVec2(0.45834f,0.6875f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f+singleQuadDim;
                end.x = start.x+singleQuadDim;
                end.y = start.y+singleQuadDim;
                hovers = ImGui::IsMouseHoveringRect(start,end,false);
                if (hovers) hoversInt = 5;
                drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                // central quad left
                uv0=ImVec2(0.0f,0.22916f);uv1=ImVec2(0.22916f,0.45834f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f - singleQuadDim;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f;
                end.x = start.x+singleQuadDim;
                end.y = start.y+singleQuadDim;
                hovers = ImGui::IsMouseHoveringRect(start,end,false);
                if (hovers) hoversInt = 2;
                drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                // central quad center
                uv0=ImVec2(0.22916f,0.22916f);uv1=ImVec2(0.45834f,0.45834f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f;
                end.x = start.x+singleQuadDim;
                end.y = start.y+singleQuadDim;
                hovers = //hoversInt==0;
                        ImGui::IsMouseHoveringRect(start,end,false);
                if (hovers) hoversInt = 1;
                drawList->AddImage(tid,start,end,uv0,uv1,hovers ? quadColHovered : quadCol);
                // Refinement: draw remaining 4 inert quads
                uv0=ImVec2(0.f,0.f);uv1=ImVec2(0.22916f,0.22916f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f - singleQuadDim;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f - singleQuadDim;
                end.x = start.x+singleQuadDim;end.y = start.y+singleQuadDim;
                drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
                uv0=ImVec2(0.45834f,0.f);uv1=ImVec2(0.6875f,0.22916f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f + singleQuadDim;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f - singleQuadDim;
                end.x = start.x+singleQuadDim;end.y = start.y+singleQuadDim;
                drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
                uv0=ImVec2(0.f,0.45834f);uv1=ImVec2(0.22916f,0.6875f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f - singleQuadDim;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f + singleQuadDim;
                end.x = start.x+singleQuadDim;end.y = start.y+singleQuadDim;
                drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
                uv0=ImVec2(0.45834f,0.45834f);uv1=ImVec2(0.6875f,0.6875f);
                start.x = wp.x + (ws.x-singleQuadDim)*0.5f + singleQuadDim;
                start.y = wp.y + (ws.y-singleQuadDim)*0.5f + singleQuadDim;
                end.x = start.x+singleQuadDim;end.y = start.y+singleQuadDim;
                drawList->AddImage(tid,start,end,uv0,uv1,quadCol);
            }
            // Button -----------------
            dd.drawDragButton(drawList,wp,mp);
            // -------------------------------------------------------------------
            ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
        }

        // Drop tab label onto another
        if (dd.draggingTabDst) {
            // swap draggingTabSrc and draggingTabDst
            IM_ASSERT(dd.isDraggingSrcValid());
            IM_ASSERT(dd.isDraggingDstValid());
            IM_ASSERT(dd.draggingTabSrc!=dd.draggingTabDst);

            if (dd.draggingTabNodeSrc!=dd.draggingTabNodeDst) {
                bool srcWasSelected = dd.draggingTabNodeSrc->selectedTab == dd.draggingTabSrc;
                bool dstWasSelected = dd.draggingTabNodeDst->selectedTab == dd.draggingTabDst;
                if (srcWasSelected) dd.draggingTabNodeSrc->selectedTab = dd.draggingTabDst;
                if (dstWasSelected) dd.draggingTabNodeDst->selectedTab = dd.draggingTabSrc;
            }

            const int iSrc = dd.findDraggingSrcIndex();
            IM_ASSERT(iSrc>=0);
            const int iDst = dd.findDraggingDstIndex();
            IM_ASSERT(iDst>=0);
            dd.draggingTabNodeDst->tabs[iDst] = dd.draggingTabSrc;
            dd.draggingTabNodeSrc->tabs[iSrc] = dd.draggingTabDst;

            dd.reset();
            //fprintf(stderr,"Drop tab label onto another\n");
        }

        // Reset draggingTabIndex if necessary
        if (!MyTabWindowHelperStruct::isMouseDragging) {
            if (hoversInt && dd.draggingTabSrc && dd.draggingTabImGuiWindowSrc && dd.draggingTabImGuiWindowSrc!=g.HoveredWindow)
            {
                // Drop tab label onto a window portion
                int nameSz = strlen(g.HoveredWindow->Name);
                static const char trailString[] = ".user";
                static const int trailStringSz = (int) strlen(trailString);
                IM_ASSERT(nameSz>=trailStringSz);
                IM_ASSERT(strcmp(&g.HoveredWindow->Name[nameSz-trailStringSz],trailString)==0);
                const char* startMatchCh = strstr(g.HoveredWindow->Name,".##main"),*startMatchCh2 = NULL;
                if (startMatchCh)   {
                    while ((startMatchCh2 = strstr(&g.HoveredWindow->Name[(int)(startMatchCh-g.HoveredWindow->Name)+7],".##main"))) {
                        startMatchCh = startMatchCh2;
                    }
                }
                const int startMatchIndex = startMatchCh ? ((int)(startMatchCh-g.HoveredWindow->Name)+1) : 0;
                IM_ASSERT(nameSz>=trailStringSz-startMatchIndex);

                ImVector<char> tmp;tmp.resize(nameSz);
                strncpy(&tmp[0],&g.HoveredWindow->Name[startMatchIndex],nameSz-trailStringSz-startMatchIndex);
                tmp[nameSz-trailStringSz-startMatchIndex]='\0';
                //fprintf(stderr,"\"%s\"\n",&tmp[0]);
                dd.draggingTabNodeDst = TabWindowDragData::FindTabNodeByName(mainNode,&tmp[0]);

                //fprintf(stderr,"Item: \"%s\" dragged to window:\"%s\" at pos: %d\n",dd.draggingTabSrc->label,g.HoveredWindow ? g.HoveredWindow->Name : "NULL",hoversInt);
                //if (dd.draggingTabNodeDst)  fprintf(stderr,"dd.draggingTabNodeDst->tabs.size()=%d\n",(int)dd.draggingTabNodeDst->tabs.size());
                //else fprintf(stderr,"No dd.draggingTabNodeDst.\n");
                //TODO: move dd.draggingTabSrc and delete the src node if empty------------
                // How can I find dd.draggingTabNodeDst from g.HoveredWindow->Name?
                // I must strip ".HorizontalStrip.content.user" and then seek TabNode::Name
                //-------------------------------------------------------------------------
                if (dd.draggingTabNodeDst) {
                    if (hoversInt!=1 && dd.draggingTabNodeDst->tabs.size()==0) hoversInt=1;
                    if (!(dd.draggingTabNodeDst==dd.draggingTabNodeSrc && (dd.draggingTabNodeDst->tabs.size()==0 || hoversInt==1))) {
                        // We must:

                        // 1) remove dd.draggingTabSrc from dd.draggingTabNodeSrc
                        if (!dd.draggingTabNodeSrc->removeTabLabel(dd.draggingTabSrc,false,&dd.draggingTabNodeDst,true))   {
                            //fprintf(stderr,"Error: !dd.draggingTabNodeSrc->removeTabLabel(dd.draggingTabSrc,false,&activeNode,true): \"%s\"\n",dd.draggingTabSrc->getLabel());
                        }
                        // 2) append if to dd.draggingTabNodeDst
                        activeNode = dd.draggingTabNodeDst->addTabLabel(dd.draggingTabSrc,hoversInt==1 ? -1 : hoversInt-2);
                        //----------------------------------------------------
                    }
                    //else fprintf(stderr,"Do nothing.\n");
                }

                dd.resetDraggingDst();
            }
            if (dd.draggingTabSrc) dd.resetDraggingSrc();
        }
    }

}

void TabWindow::clearNodes() {
    if (mainNode)   {
        mainNode->~TabNode();
        ImGui::MemFree(mainNode);
        mainNode=NULL;
    }
    activeNode = NULL;
}
void TabWindow::clear() {mainNode->clear();activeNode=mainNode;}

TabWindow::TabWindow() {
    mainNode = (TabNode*) ImGui::MemAlloc(sizeof(TabNode));
    new (mainNode) TabNode();
    mainNode->name = (char*) ImGui::MemAlloc(7);strcpy(mainNode->name,"##main");
    activeNode=mainNode;
    init=false;
}

TabWindow::~TabWindow() {clearNodes();}

TabWindow::TabLabel *TabWindow::addTabLabel(const char *label, void *userPtr, const char *tooltip, bool closable, bool draggable) {
    TabLabel* tab = (TabLabel*) ImGui::MemAlloc(sizeof(TabLabel));
    new (tab) TabLabel(label,tooltip,closable,draggable);
    tab->userPtr = userPtr;
    if (!activeNode) activeNode = mainNode->getFirstLeaftNode();
    activeNode = activeNode->addTabLabel(tab);
    return tab;
}

bool TabWindow::removeTabLabel(TabWindow::TabLabel *tab) {
    if (!tab) return false;
    if (!mainNode->removeTabLabel(tab,true,&activeNode)) {
        fprintf(stderr,"Error: cannot remove TabLabel: \"%s\"\n",tab->getLabel());
        return false;
    }
    return true;
}



ImTextureID TabWindow::DockPanelIconTextureID = NULL;
const unsigned char* TabWindow::GetDockPanelIconImagePng(int* bufferSizeOut) {
    // I have drawn all the icons that compose this image myself.
    // I took inspiration from the icons bundled with: https://github.com/dockpanelsuite/dockpanelsuite (that is MIT licensed).
    // So no copyright issues for this AFAIK.
    static const unsigned char png[] =
    {
 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,3,0,0,0,244,224,145,249,0,0,0,192,80,76,84,69,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,69,
 69,71,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,244,246,252,226,239,253,220,237,253,228,228,229,201,227,253,201,227,254,228,227,230,227,227,230,224,224,229,191,222,253,190,
 222,253,180,216,252,216,216,216,217,217,217,213,216,234,215,215,216,174,213,252,183,213,247,213,213,213,213,213,213,160,200,248,200,200,200,199,199,199,198,198,198,197,195,209,189,189,189,181,
 181,181,181,181,181,172,168,161,170,165,200,84,150,235,142,140,202,147,139,190,44,131,234,63,128,224,121,127,173,63,106,202,85,106,173,102,102,104,92,97,153,103,182,173,125,0,0,0,
 19,116,82,78,83,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,224,104,78,28,249,0,0,7,27,73,68,65,84,120,218,213,155,91,119,170,60,16,134,123,219,
 91,191,11,65,91,165,202,194,202,65,192,66,101,175,84,249,255,255,234,75,8,135,64,102,66,2,150,189,246,187,43,118,183,101,230,233,100,114,152,144,190,188,170,116,85,233,85,79,
 35,54,94,212,55,199,113,124,65,164,13,144,198,151,8,145,6,0,122,111,164,13,16,71,1,38,13,128,40,56,185,176,180,1,46,168,9,119,28,224,18,157,74,166,187,40,66,74,
 223,117,180,1,162,192,85,216,24,5,112,203,63,92,223,141,194,48,36,169,187,53,2,128,108,196,254,12,128,107,48,31,32,13,244,0,110,97,165,79,81,55,67,0,208,134,54,192,
 81,150,49,0,100,67,27,224,163,209,129,189,42,25,3,64,54,180,1,222,101,25,3,64,54,180,1,54,178,140,1,32,27,218,0,182,36,115,0,200,134,54,128,37,43,55,5,128,
 108,104,3,108,189,173,237,237,156,189,237,237,247,236,178,115,182,198,17,128,108,232,2,16,47,241,118,137,231,181,23,199,219,23,134,0,160,13,237,8,252,39,203,184,9,32,27,255,
 30,128,149,91,179,1,68,27,166,0,86,81,230,235,153,0,61,27,134,0,235,156,78,233,205,221,19,1,250,54,204,0,214,121,181,142,168,239,158,6,48,176,97,4,176,78,8,151,
 55,29,96,104,67,3,224,218,1,120,181,246,29,192,213,20,96,96,99,28,128,46,202,47,1,218,13,29,205,101,233,244,110,200,138,130,72,1,224,156,174,191,10,64,253,71,116,81,
 238,96,0,186,4,83,1,152,255,128,250,119,202,66,214,157,1,104,18,84,139,82,200,134,26,160,242,127,162,254,183,110,10,212,116,116,89,174,75,192,0,252,56,149,85,47,203,177,
 178,177,2,160,254,105,178,1,53,149,235,108,27,130,177,210,149,1,184,62,96,163,46,76,208,250,179,246,175,22,37,8,34,117,233,202,0,28,68,28,0,174,63,3,29,255,156,32,
 80,150,174,153,74,21,0,86,127,114,255,82,85,199,234,58,82,186,78,75,48,171,116,165,0,188,254,188,113,145,94,158,50,47,77,85,213,43,171,194,123,149,133,119,225,135,73,81,
 91,160,198,196,210,53,187,170,245,82,151,127,225,241,240,190,177,109,207,18,122,234,141,121,145,202,58,14,80,245,195,66,248,97,111,103,217,155,247,143,99,248,231,79,41,150,174,89,
 132,235,18,167,67,128,164,7,192,188,16,207,102,227,247,198,227,239,244,178,167,151,155,4,144,120,20,224,141,3,136,83,85,22,40,20,197,234,8,84,0,231,207,60,15,233,63,118,
 9,115,246,31,250,70,228,8,108,187,8,244,0,92,92,167,224,194,1,126,106,128,141,189,151,1,242,182,158,61,87,151,51,253,56,203,0,59,11,1,112,92,31,203,21,55,136,56,
 192,93,106,130,245,186,139,192,153,251,100,174,43,255,20,228,92,180,0,235,181,208,4,28,224,222,7,240,99,44,87,0,128,186,9,248,218,145,3,36,109,4,142,236,117,100,47,161,
 9,156,164,54,188,199,0,130,20,203,149,22,224,103,16,1,171,32,219,22,32,4,106,251,176,139,128,71,248,18,203,243,244,0,196,92,233,0,104,223,18,34,96,221,238,2,192,230,
 120,60,28,142,135,158,18,17,224,94,133,171,141,192,247,183,18,64,204,21,4,192,34,165,8,144,28,14,31,67,245,1,202,130,222,181,179,213,0,80,174,0,0,180,9,214,69,217,
 7,144,220,127,188,159,111,61,128,50,111,12,31,80,0,40,87,186,36,252,254,254,60,126,52,17,216,21,20,96,215,13,68,242,230,198,155,144,132,12,128,181,65,99,248,19,1,128,
 114,165,15,208,207,129,14,32,161,14,223,171,215,91,171,65,14,48,203,167,81,0,57,87,48,0,218,11,197,8,188,201,242,110,114,47,112,120,19,40,0,164,92,65,154,128,17,36,
 194,56,240,38,239,239,228,29,192,206,227,217,197,13,171,1,134,185,2,0,0,147,81,2,108,48,37,242,100,228,121,227,0,195,92,193,35,32,2,212,46,217,84,120,222,212,19,99,
 14,0,236,71,1,164,92,105,1,72,24,226,0,30,253,178,157,159,146,60,161,31,213,37,201,61,121,50,114,26,0,190,88,192,123,129,144,43,0,0,184,30,96,218,145,132,58,206,
 147,146,94,168,8,222,4,131,173,236,6,0,202,21,205,8,216,246,198,78,118,57,255,26,17,190,213,159,142,109,107,83,25,230,116,248,92,224,1,0,37,33,213,28,77,239,205,5,
 17,158,132,118,158,123,185,117,3,0,196,159,46,110,121,61,209,19,245,100,180,7,0,252,20,94,48,178,53,225,189,161,225,111,37,127,227,139,82,23,191,15,5,16,115,165,5,112,
 93,120,201,198,86,197,244,254,198,108,156,118,215,128,45,203,29,252,190,241,38,160,185,210,2,40,11,144,214,11,231,172,175,99,53,11,62,29,11,185,194,1,16,233,148,69,149,167,
 145,218,144,1,96,185,82,1,96,53,173,171,71,48,90,1,177,53,97,10,42,246,85,0,175,87,173,210,112,188,2,99,171,98,56,83,124,87,9,208,18,220,111,178,72,157,3,26,
 21,96,230,224,82,3,52,4,5,184,189,226,106,250,31,175,13,95,199,8,10,108,143,72,123,167,80,93,29,191,142,17,160,0,79,241,63,182,83,74,9,42,128,181,181,171,180,183,
 141,119,74,199,54,40,212,0,193,169,6,104,182,89,19,243,173,90,85,10,142,2,68,129,203,155,96,93,84,155,35,133,53,1,0,217,162,210,2,184,52,0,85,185,80,214,254,13,
 1,144,77,58,61,128,168,1,96,5,83,227,223,16,32,72,225,177,196,16,128,150,172,187,255,38,2,128,61,201,24,96,234,67,171,127,30,128,61,118,93,207,121,116,219,0,12,199,
 18,93,0,246,212,115,159,208,139,83,213,6,9,251,32,147,0,6,99,137,118,4,228,39,223,246,180,38,24,140,37,218,0,180,46,178,217,107,54,192,96,44,49,1,144,138,211,137,
 73,216,27,75,180,1,230,30,225,16,123,129,56,150,104,3,28,228,77,162,219,162,221,112,238,49,158,217,0,225,80,134,7,153,40,0,56,150,232,2,64,199,176,76,1,192,177,100,
 73,0,112,44,209,6,160,250,185,223,127,168,186,39,54,166,0,208,88,162,11,224,167,35,245,175,38,128,52,150,232,2,184,238,72,253,171,3,0,141,37,186,0,206,228,170,76,0,
 128,198,18,45,128,103,28,235,101,0,208,88,162,3,240,12,85,0,208,88,178,36,0,216,149,151,3,240,227,191,12,224,250,224,88,178,24,128,234,169,225,34,0,170,231,166,139,0,
 168,158,28,47,2,48,231,120,255,2,122,249,171,1,184,94,179,223,7,80,28,33,161,90,6,0,223,41,92,8,0,223,43,93,8,0,59,163,146,6,6,0,89,150,17,242,168,180,
 162,50,3,192,206,168,152,0,156,226,47,138,240,104,52,17,32,153,14,144,93,68,128,223,139,64,230,103,205,103,167,172,255,45,1,32,48,203,1,236,140,10,8,16,199,245,103,151,
 56,147,210,128,24,198,191,6,192,206,168,128,0,95,95,95,252,247,253,250,250,5,128,254,25,21,8,128,8,27,185,228,89,0,216,25,149,229,0,144,51,42,2,64,235,244,65,58,
 61,126,7,160,59,163,210,2,60,154,161,49,142,31,162,208,94,240,152,10,208,63,163,82,3,172,196,176,163,0,89,60,125,28,192,206,168,52,17,88,137,97,199,0,252,25,35,33,
 118,70,165,203,129,213,56,192,156,185,0,59,163,2,245,2,69,14,84,237,64,41,204,103,67,236,140,10,4,176,18,252,175,158,15,48,62,25,173,154,95,252,119,0,60,125,128,0,
 4,72,39,1,96,39,52,96,128,149,244,217,19,1,102,172,7,126,35,2,230,107,194,137,0,216,25,21,243,85,241,20,0,197,95,253,82,45,0,160,248,187,103,42,211,101,57,5,
 88,25,2,40,254,242,155,202,20,224,145,25,6,224,233,181,161,105,0,158,94,29,63,219,255,235,255,165,109,33,83,179,145,173,73,0,0,0,0,73,69,78,68,174,66,96,130
};

    if (bufferSizeOut) *bufferSizeOut = (int) (sizeof(png)/sizeof(png[0]));
    return png;
}


} // namespace ImGui

