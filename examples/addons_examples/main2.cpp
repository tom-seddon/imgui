#include <imgui.h>

GLuint myImageTextureId = 0;
GLuint myImageTextureId2 = 0;

void InitGL()	// Mandatory
{

/*
if (!ImGui::LoadStyle("./myimgui.style",ImGui::GetStyle()))   {
    fprintf(stderr,"Warning: \"./myimgui.style\" not present.\n");
}
*/

if (!myImageTextureId) myImageTextureId = ImImpl_LoadTexture("./myCubeTexture.png");
if (!myImageTextureId2) myImageTextureId2 = ImImpl_LoadTexture("./myNumbersTexture.png");

//ImGui::GetIO().MouseDrawCursor = true;
}

static ImGui::PanelManager mgr;
static ImVec2 gMainMenuBarSize(0,0);
static bool gShowMainMenuBar = true;
static void ShowExampleMenuFile()
{
    ImGui::MenuItem("(dummy menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}
    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Colors"))
    {
        for (int i = 0; i < ImGuiCol_COUNT; i++)
            ImGui::MenuItem(ImGui::GetStyleColName((ImGuiCol)i));
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}
static void ShowExampleMenuBar(bool isMainMenu=false)
{
    //static bool ids[2];
    //ImGui::PushID((const void*) isMainMenu ? &ids[0] : &ids[1]);
    const bool open = isMainMenu ? ImGui::BeginMainMenuBar() : ImGui::BeginMenuBar();
    if (open)
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::Separator();
            static bool booleanProp=true;
            ImGui::MenuItem("Boolean Test", NULL, &booleanProp);
            ImGui::EndMenu();
        }
        if (isMainMenu) {
            gMainMenuBarSize = ImGui::GetWindowSize();
            ImGui::EndMainMenuBar();
        }
        else ImGui::EndMenuBar();
    }
    //ImGui::PopID();
}

void ResizeGL(int w,int h)	// Mandatory
{
    static ImVec2 initialSize(w,h);
    //fprintf(stderr,"ResizeGL(%d,%d); ImGui::DisplaySize(%d,%d);\n",w,h,(int)ImGui::GetIO().DisplaySize.x,(int)ImGui::GetIO().DisplaySize.y);

    mgr.setToolbarsScaling((float)w/initialSize.x,(float)h/initialSize.y);
    // This line is only necessary if we have a global menu bar:
    if (gShowMainMenuBar) mgr.setDisplayPortion(ImVec4(0,gMainMenuBarSize.y,w,h-gMainMenuBarSize.y));

}


static const char* leftWindowNames[] = {"Debug","Another Window","ImGui Test","Test popup menu"};
static void DrawLeftWindows(ImGui::PanelManagerWindowData& wd)    {
if (!wd.isToggleWindow || ImGui::Begin(wd.name,&wd.closed,wd.pos,-1.f,ImGuiWindowFlags_NoSavedSettings))    {
    if (strcmp(wd.name,leftWindowNames[0])==0)  {        
        static float f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        //show_test_window ^= ImGui::Button("Test Window");
        //show_another_window ^= ImGui::Button("Another Window");

        // Calculate and show framerate
        static float ms_per_frame[120] = { 0 };
        static int ms_per_frame_idx = 0;
        static float ms_per_frame_accum = 0.0f;
        ms_per_frame_accum -= ms_per_frame[ms_per_frame_idx];
        ms_per_frame[ms_per_frame_idx] = ImGui::GetIO().DeltaTime * 1000.0f;
        ms_per_frame_accum += ms_per_frame[ms_per_frame_idx];
        ms_per_frame_idx = (ms_per_frame_idx + 1) % 120;
        const float ms_per_frame_avg = ms_per_frame_accum / 120;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);

#           ifdef DEMO_CAN_LOAD_SAVE_STYLES
        ImGui::Text("\n");
        ImGui::Text("Please modify the current style in:");
        ImGui::Text("ImGui Test->Window Options->Style Editor");
        static bool loadCurrentStyle = false;
        static bool saveCurrentStyle = false;
        static bool resetCurrentStyle = false;
        loadCurrentStyle = ImGui::Button("Load Saved Style");
        saveCurrentStyle = ImGui::Button("Save Current Style");
        resetCurrentStyle = ImGui::Button("Reset Current Style");
        if (loadCurrentStyle)   {
            if (!ImGui::LoadStyle("./myimgui.style",ImGui::GetStyle()))   {
                fprintf(stderr,"Warning: \"./myimgui.style\" not present.\n");
            }
        }
        if (saveCurrentStyle)   {
            if (!ImGui::SaveStyle("./myimgui.style",ImGui::GetStyle()))   {
                fprintf(stderr,"Warning: \"./myimgui.style\" cannot be saved.\n");
            }
        }
        if (resetCurrentStyle)  ImGui::GetStyle() = ImGuiStyle();

        if (myImageTextureId) {
            ImVec2 canvas_size = ImVec2(ImGui::GetWindowContentRegionMax().x*0.25f,ImGui::GetWindowContentRegionMax().x*0.25f);
            ImGui::Image((void*) myImageTextureId,canvas_size);//,ImVec2(0,0),ImVec2(1,1),ImVec4(1,1,0,1),ImVec4(1,0,0,1));//ImVec2(200,200));
            //ImGui::ImageButton((void*) myImageTextureId,canvas_size,ImVec2(0,0),ImVec2(1,1),-1,ImVec4(1,1,0,1),ImVec4(1,0,0,1));//ImVec2(200,200));
        }
        //if (ImGui::InvisibleButton("InvisibleButton",ImVec2(10,200))) ImGui::Text("Invisible Button Pressed");

#           endif //DEMO_CAN_LOAD_SAVE_STYLES
    }
    else if (strcmp(wd.name,leftWindowNames[1])==0)    {        
        if (!wd.isToggleWindow) {
            ImGui::PushItemWidth(150);
            ImGui::SliderFloat("Window Size",&wd.length,16,wd.dockPos<ImGui::PanelManager::TOP ? ImGui::GetIO().DisplaySize.y-wd.pos.y : ImGui::GetIO().DisplaySize.x-wd.pos.x);
            ImGui::PopItemWidth();
            ImGui::Separator();
        }
        ImGui::Text("Hello world from window \"%s\"",wd.name);                
    }
    else if (strcmp(wd.name,leftWindowNames[2])==0)    {
        ImGui::ShowTestWindow();
        /*
        if (!wd.isToggleWindow) {
            ImGui::PushItemWidth(150);
            ImGui::SliderFloat("Window Size",&wd.length,16,wd.dockPos<ImGui::PanelManager::TOP ? ImGui::GetIO().DisplaySize.y-wd.pos.y : ImGui::GetIO().DisplaySize.x-wd.pos.x);
            ImGui::PopItemWidth();
            ImGui::Separator();
        }
        ImGui::Text("Hello world from window \"%s\"",wd.name);
        */
    }
    else if (strcmp(wd.name,leftWindowNames[3])==0) {
        // POPUP TEST:
        if (ImGui::TreeNode("Popup"))
        {
            static bool popup_open = false;
            static int selected_fish = -1;
            const char* fishes[] = { "Bream", "Haddock", "Mackerel", "Pollock", "Tilefish" };
            if (ImGui::Button("Select.."))  {
                popup_open = true;
                //wd.persistFocus=true;   // No way
                //ImGui::SetNextWindowFocus();  // No way
            }
            ImGui::SameLine();
            ImGui::Text(selected_fish == -1 ? "<None>" : fishes[selected_fish]);
            if (popup_open)
            {

                //ImGui::BeginPopup(&popup_open);
                ImGui::OpenPopup("MyAquariumPopup");
                if (ImGui::BeginPopup("MyAquariumPopup"))   {
                    ImGui::Text("Aquarium");
                    ImGui::Separator();
                    for (size_t i = 0; i < sizeof(fishes)/sizeof(fishes[0]); i++)
                    {
                        if (ImGui::Selectable(fishes[i], false))
                        {
                            selected_fish = i;
                            popup_open = false;
                        }
                    }
                    if (popup_open)   // NEW: close menu when mouse goes away
                    {
                        ImVec2 pos = ImGui::GetWindowPos();pos.x-=5;pos.y-=5;
                        ImVec2 size = ImGui::GetWindowSize();size.x+=10;size.y+=10;
                        const ImVec2& mousePos = ImGui::GetIO().MousePos;
                        if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) popup_open = false;
                    }
                    ImGui::EndPopup();
                }


            }
            ImGui::TreePop();
        }
        if (ImGui::Button("Launch Browser"))    {
            const char* url = "/home/flix/Downloads2/Column separator_border sizing issue · Issue #170 · ocornut_imgui.html.maff";
            //const char* url = "/media/flix/HDMaxi/Video";   // opens a folder
            //const char* url = "/home/flix/Pictures/Selection_003.png";  //opens an image
            ImGui::OpenWithDefaultApplication(url);
        }
        for (int i=0;i<20;i++) ImGui::TextWrapped("This window is being created by the ShowTestWindow() function. Please refer to the code for programming reference.\n\nUser Guide:");

    }
}
if (wd.isToggleWindow) ImGui::End();

}

static const char* rightWindowNames[] = {"Debug2","Another Window2","ImGui Test2"};
static const char* bottomWindowNames[] = {"Debug3","Another Window3","ImGui Test3","Test Window 31","Test Window 32","Test Window 33","Test Window 34","Test Window 35","Test Window 36","Test Window 37","Test Window 38","Test Window 39","Test Window 40","Test Window 41"};
static void DrawGenericWindows(ImGui::PanelManagerWindowData& wd)    {
if (!wd.isToggleWindow || ImGui::Begin(wd.name,&wd.closed,wd.pos,-1.f,ImGuiWindowFlags_NoSavedSettings))    {
    if (!wd.isToggleWindow) {
        ImGui::PushItemWidth(150);
        ImGui::SliderFloat("Window Size",&wd.length,16,wd.dockPos<ImGui::PanelManager::TOP ? ImGui::GetIO().DisplaySize.y-wd.pos.y : ImGui::GetIO().DisplaySize.x-wd.pos.x);
        ImGui::PopItemWidth();
        ImGui::Separator();
    }
    ImGui::Text("Hello world from window \"%s\"",wd.name);
}
if (wd.isToggleWindow) ImGui::End();
}


void DrawGL()	// Mandatory
{

        glClearColor(0.8f, 0.6f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (gShowMainMenuBar) ShowExampleMenuBar(true);

        if (mgr.isEmpty()) {
            // Hp) All the associated windows MUST have an unique name WITHOUT using the '##' chars that ImGui supports

            // LEFT PANE
            {
                ImGui::PanelManager::Pane* pane = mgr.addPane(ImGui::PanelManager::LEFT,"myFirstToolbarLeft##foo");
                if (pane)   {
                    char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                    for (int i=8;i>=0;i--) {
                        strcpy(tmp,"toolbutton ");sprintf(&tmp[strlen(tmp)],"%d",8-i+1);    // used only for the Toolbutton's tooltip (no ImGui::ID needed)
                        uv0=ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                        uv1=ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);

                        pane->addButtonAndWindow(ImGui::Toolbutton(tmp,(void*)myImageTextureId2,uv0,uv1,ImVec2(24,48)),
                                                 ImGui::PanelManagerPaneAssociatedWindow(i<(int)(sizeof(leftWindowNames)/sizeof(leftWindowNames[0])) ? leftWindowNames[i] : NULL,-1,&DrawLeftWindows)); // "NULL" just adds a toolbutton
                    }
                    pane->addSeparator(16); // Note that a separator "eats" one toolbutton index as if it was a real button

                    pane->addButtonOnly(ImGui::Toolbutton("toolbutton 11",(void*)myImageTextureId2,uv0,uv1,ImVec2(24,32),true,false));  // Here we add a toggle button
                    pane->addButtonOnly(ImGui::Toolbutton("toolbutton 12",(void*)myImageTextureId2,uv0,uv1,ImVec2(24,32),true,false));                    
                    //pane->setToolbarProperties(true,true,ImVec2(0.0f,0.0f),ImVec2(0.25f,0.9f),ImVec4(0.85,0.85,1,1));
                }
            }
            // RIGHT PANE
            {
                ImGui::PanelManager::Pane* pane = mgr.addPane(ImGui::PanelManager::RIGHT,"myFirstToolbarRight##foo");
                if (pane)   {
                    char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                    for (int i=0;i<9;i++) {
                        strcpy(tmp,"toolbutton ");sprintf(&tmp[strlen(tmp)],"%d",8-i+1);    // used only for the Toolbutton's tooltip (no ImGui::ID needed)
                        uv0=ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                        uv1=ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);
                        const float tmp2 = uv0.x;uv0.x=uv1.x;uv1.x=tmp2;  // reverse x

                        pane->addButtonAndWindow(ImGui::Toolbutton(tmp,(void*)myImageTextureId2,uv0,uv1,ImVec2(24,48)),
                                                 ImGui::PanelManagerPaneAssociatedWindow(i<(int)(sizeof(rightWindowNames)/sizeof(rightWindowNames[0])) ? rightWindowNames[i] : NULL,-1,&DrawGenericWindows)); // "NULL" just adds a toolbutton

                    }
                    pane->addSeparator(16);   // Note that a separator "eats" one toolbutton index as if it was a real button

                    pane->addButtonOnly(ImGui::Toolbutton("toolbutton 11",(void*)myImageTextureId2,uv0,uv1,ImVec2(24,32),true,false));
                    pane->addButtonOnly(ImGui::Toolbutton("toolbutton 12",(void*)myImageTextureId2,uv0,uv1,ImVec2(24,32),true,false));

                    //pane->setToolbarProperties(true,true,ImVec2(1.0f,0.0f),ImVec2(0.25f,0.9f),ImVec4(0.85,0.85,1,1));
                }
            }
            // BOTTOM PANE (better init syntax)
            {
                ImGui::PanelManager::Pane* pane = mgr.addPane(ImGui::PanelManager::BOTTOM,"myFirstToolbarBottom##foo");
                if (pane)   {
                    char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                    for (int i=0;i<9;i++) {
                        strcpy(tmp,"toolbutton ");sprintf(&tmp[strlen(tmp)],"%d",8-i+1);    // used only for the Toolbutton's tooltip (no ImGui::ID needed)
                        uv0=ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                        uv1=ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);
                        //const float tmp = uv0.x;uv0.x=uv1.x;uv1.x=tmp;  // reverse x

                        pane->addButtonAndWindow(ImGui::Toolbutton(tmp,(void*)myImageTextureId2,uv0,uv1,ImVec2(32,32)),
                                                ImGui::PanelManagerPaneAssociatedWindow(i<(int)(sizeof(bottomWindowNames)/sizeof(bottomWindowNames[0])) ? bottomWindowNames[i] : NULL,-1,&DrawGenericWindows)); // "NULL" just adds a toolbutton
                    }
                    size_t cnt = pane->addSeparator(16); // Note that separator "eats" one toolbutton index as if it was a real button
                    cnt = pane->addButtonAndWindow(ImGui::Toolbutton("toolbutton 11",(void*)myImageTextureId2,uv0,uv1,ImVec2(32,24),true,false),
                                                  ImGui::PanelManager::Pane::AssociatedWindow(cnt<(sizeof(bottomWindowNames)/sizeof(bottomWindowNames[0])) ? bottomWindowNames[cnt] : NULL,-1,&DrawGenericWindows));
                    cnt = pane->addButtonAndWindow(ImGui::Toolbutton("toolbutton 12",(void*)myImageTextureId2,uv0,uv1,ImVec2(42,24),true,false),
                                                  ImGui::PanelManager::Pane::AssociatedWindow(cnt<(int)(sizeof(bottomWindowNames)/sizeof(bottomWindowNames[0])) ? bottomWindowNames[cnt] : NULL,-1,&DrawGenericWindows));

                    //pane->setToolbarProperties(true,false,ImVec2(0.5f,1.0f),ImVec2(0.25f,0.9f),ImVec4(0.85,0.85,1,1));
                }
            }
            // TOP PANE
            {
                // Here we create a top pane by cloning the bottom one so that the same windows can be shown in two opposite sides.
                // To clone single buttons (and not the whole pane) please use: pane->addClonedButtonAndWindow(...);
                // IMPORTANT: Toggle Toolbuttons (and associated windows) can't be cloned and are just skipped if present
                const ImGui::PanelManager::Pane* bottomPane = mgr.getPane(ImGui::PanelManager::BOTTOM);
                if (bottomPane) {
                    ImGui::PanelManager::Pane* pane = mgr.addPane(ImGui::PanelManager::TOP,"myFirstToolbarTop##foo");
                    if (pane)   {
                        pane->addClonedPane(*bottomPane,false);

                        pane->addSeparator(32);
                        pane->addButtonOnly(ImGui::Toolbutton("Normal Manual Button 1",(void*)myImageTextureId2,ImVec2(0,0),ImVec2(1.f/3.f,1.f/3.f),ImVec2(32,32)));//,false,false,ImVec4(0,1,0,1)));  // Here we add a free button
                        pane->addButtonOnly(ImGui::Toolbutton("Normal Manual Button 2",(void*)myImageTextureId2,ImVec2(0,0),ImVec2(1.f/3.f,1.f/3.f),ImVec2(32,32)));  // Here we add a free button
                        pane->addButtonOnly(ImGui::Toolbutton("Normal Manual Button 3",(void*)myImageTextureId2,ImVec2(0,0),ImVec2(1.f/3.f,1.f/3.f),ImVec2(32,32)));  // Here we add a free button

                        pane->addSeparator(32);
                        pane->addButtonOnly(ImGui::Toolbutton("Show/Hide Main Menu Bar",(void*)myImageTextureId2,ImVec2(0.34,0),ImVec2(1-0.34,1),ImVec2(24,24),true,gShowMainMenuBar));  // Here we add a toggle button
                        pane->addButtonOnly(ImGui::Toolbutton("Show/Hide central window",(void*)myImageTextureId2,ImVec2(0,0),ImVec2(1,1),ImVec2(24,24),true,false));  // Here we add a toggle button

                        //pMustShowCentralWidget = &pane->bar.getButton(pane->bar.getNumButtons()-1)->isDown;  // confortable, but dangerous if we delete the last button at runtime!

                        //pane->setToolbarProperties(false,true,ImVec2(0.5f,0.0f),ImVec2(0.25f,0.9f),ImVec4(0.85,0.85,1,1));
                    }
                }
            }

            // These two lines are only necessary to accomodate space for the global menu bar we're using:
            if (gShowMainMenuBar)   {
                const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
                const ImVec4 displayPortion = ImVec4(0,gMainMenuBarSize.y,displaySize.x,displaySize.y-gMainMenuBarSize.y);
                mgr.setDisplayPortion(displayPortion);
            }
        }

        ImGui::PanelManagerPane* pressedPane=NULL;  // Not used here
        int pressedPaneButtonIndex = -1;            // Not used here
        if (mgr.render(&pressedPane,&pressedPaneButtonIndex))   {
            //const ImVec2& iqp = mgr.getCentralQuadPosition();
            //const ImVec2& iqs = mgr.getCentralQuadSize();
            //fprintf(stderr,"Inner Quad Size changed to {%1.f,%1.f,%1.f,%1.f}\n",iqp.x,iqp.y,iqs.x,iqs.y);
        }

        // (Optional) Some manual feedback to the user:
        if (pressedPane && pressedPaneButtonIndex!=-1)
        {
            static const char* paneNames[]={"LEFT","RIGHT","TOP","BOTTOM"};
            if (!pressedPane->getWindowName(pressedPaneButtonIndex)) {
                ImGui::Toolbutton* pButton = NULL;
                pressedPane->getButtonAndWindow(pressedPaneButtonIndex,&pButton);
                if (pButton->isToggleButton) {
                    fprintf(stderr,"Pressed manual toggle button (number: %d on pane: %s)\n",pressedPaneButtonIndex,paneNames[pressedPane->pos]);
                    if (pressedPane->pos==ImGui::PanelManager::TOP && pressedPaneButtonIndex==(int)pressedPane->getSize()-2) {
                        gShowMainMenuBar=!gShowMainMenuBar;
                        const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
                        const ImVec4 displayPortion = ImVec4(0,gMainMenuBarSize.y,displaySize.x,displaySize.y-gMainMenuBarSize.y);
                        mgr.setDisplayPortion(gShowMainMenuBar ? displayPortion : ImVec4(0,0,-1,-1));
                    }
                }
                else fprintf(stderr,"Pressed manual button (number: %d on pane: %s)\n",pressedPaneButtonIndex,paneNames[pressedPane->pos]);
            }
            else {
                ImGui::Toolbutton* pButton = NULL;
                pressedPane->getButtonAndWindow(pressedPaneButtonIndex,&pButton);
                if (pButton->isToggleButton) fprintf(stderr,"Pressed toggle button (number: %d on pane: %s)\n",pressedPaneButtonIndex,paneNames[pressedPane->pos]);
                else fprintf(stderr,"Pressed dock button (number: %d on pane: %s)\n",pressedPaneButtonIndex,paneNames[pressedPane->pos]);
            }

        }

        //if (pMustShowCentralWidget && *pMustShowCentralWidget)
        if (mgr.getPaneTop()->isButtonPressed(mgr.getPaneTop()->getSize()-1))
        {
            const ImVec2& iqs = mgr.getCentralQuadSize();
            if (iqs.x>ImGui::GetStyle().WindowMinSize.x && iqs.y>ImGui::GetStyle().WindowMinSize.y) {
                ImGui::SetNextWindowPos(mgr.getCentralQuadPosition());
                ImGui::SetNextWindowSize(mgr.getCentralQuadSize());
                if (ImGui::Begin("Central Window"))    {
                    ImGui::Text("Example central window");
                }
                ImGui::End();
            }
        }



}
void DestroyGL()    // Mandatory
{
    if (myImageTextureId) {glDeleteTextures(1,&myImageTextureId);myImageTextureId=0;}
    if (myImageTextureId2) {glDeleteTextures(1,&myImageTextureId2);myImageTextureId2=0;}

}



//#   define USE_ADVANCED_SETUP


// Application code
#ifndef IMIMPL_USE_WINAPI
int main(int argc, char** argv)
{
#   ifndef USE_ADVANCED_SETUP
    // Basic
    ImImpl_Main(NULL,argc,argv);

#   else //USE_ADVANCED_SETUP
    // Advanced
        static const ImWchar ranges[] =
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x20AC, 0x20AC,	// €
            0x2122, 0x2122,	// ™
            0x2196, 0x2196, // ↖
            0x21D6, 0x21D6, // ⇖
            0x2B01, 0x2B01, // ⬁
            0x2B09, 0x2B09, // ⬉
            0x2921, 0x2922, // ⤡ ⤢
            0x263A, 0x263A, // ☺
            0x266A, 0x266A, // ♪
            0, // € ™ ↖ ⇖ ⬁ ⬉ ⤡ ⤢ ☺ ♪
        };
    const float fontSizeInPixels = 18.f;


    // These lines load an embedded font. [However these files are way too big... inside <imgui.cpp> they used a better format storing bytes at groups of 4, so the files are more concise (1/4?) than mine]
    const unsigned char ttfMemory[] =
//#   include "./fonts/DejaVuSerifCondensed-Bold.ttf.stbz.inl"  // stbz.inl files needs a special definition to be defined in "ImImpl_RenderDrawLists.h"
//#   include "./fonts/DroidSerif-Bold.ttf.stbz.inl"
#   include "./fonts/DejaVuSerifCondensed-Bold.ttf.inl"
//#   include "./fonts/DroidSerif-Bold.ttf.inl"

;

    ImImpl_InitParams gImGuiInitParams(
    -1,-1,NULL,                                                         // optional window width, height, title

    NULL,
    //"./fonts/DejaVuSerifCondensed-Bold.ttf",                          // optional custom font from file (main custom font)
    //"./fonts/DroidSerif-Bold.ttf",                                       // optional custom font from file (main custom font)                                                              // optional white spot in font texture (returned by the console if not set)

    //NULL,0,
    &ttfMemory[0],sizeof(ttfMemory)/sizeof(ttfMemory[0]),               // optional custom font from memory (secondary custom font) WARNING (licensing problem): e.g. embedding a GPL font in your code can make your code GPL as well.

    fontSizeInPixels,
    &ranges[0]
    );
    gImGuiInitParams.gFpsClamp = 20.0f;                                 // Optional Max allowed FPS (default -1 => unclamped). Useful for editors and to save GPU and CPU power.

    ImImpl_Main(&gImGuiInitParams,argc,argv);
#   endif //USE_ADVANCED_SETUP

	return 0;
}
#else //IMIMPL_USE_WINAPI
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int iCmdShow)   // This branch has made my code less concise (I will consider stripping it)
{
#   ifndef USE_ADVANCED_SETUP
    // Basic
    ImImpl_WinMain(NULL,hInstance,hPrevInstance,lpCmdLine,iCmdShow);
#   else //USE_ADVANCED_SETUP
    // Advanced
    // These lines load an embedded font. [However these files are way too big... inside <imgui.cpp> they used a better format storing bytes at groups of 4, so the files are more concise (1/4?) than mine]
    const unsigned char fntMemory[] =
#   include "./fonts/DejaVuSansCondensedBoldOutlineRGBAbinary18.fnt.inl"
    const unsigned char imgMemory[] =
#   include "./fonts/DejaVuSansCondensedBoldOutlineRGBAbinary18_0.png.inl"

    ImImpl_InitParams gImGuiInitParams(
    -1,-1,NULL,                                                         // optional window width, height, title
    NULL,//"./fonts/DejaVuSansCondensedBoldOutlineRGBAbinary18.fnt",    // optional custom font from file (main custom font)
    NULL,//"./fonts/DejaVuSansCondensedBoldOutlineRGBAbinary18_0.png",  // optional custom font from file (main custom font)
    -1,-1,                                                              // optional white spot in font texture (returned by the console if not set)
    &fntMemory[0],sizeof(fntMemory)/sizeof(fntMemory[0]),               // optional custom font from memory (secondary custom font) WARNING (licensing problem): e.g. embedding a GPL font in your code can make your code GPL as well.
    &imgMemory[0],sizeof(imgMemory)/sizeof(imgMemory[0])                // optional custom font from memory (secondary custom font) WARNING (licensing problem): e.g. embedding a GPL font in your code can make your code GPL as well.
    );
    gImGuiInitParams.gFpsClamp = 10.0f;                                 // Optional Max allowed FPS (default -1 => unclamped). Useful for editors and to save GPU and CPU power.
    ImImpl_WinMain(&gImGuiInitParams,hInstance,hPrevInstance,lpCmdLine,iCmdShow);
#   endif //#   USE_ADVANCED_SETUP

    return 0;
}
#endif //IMIMPL_USE_WINAPI



