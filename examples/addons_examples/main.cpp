#include <imgui.h>
#ifndef NO_IMGUIDATECHOOSER
#include <time.h>   // very common plain c header file used only by DateChooser
#endif //NO_IMGUIDATECHOOSER

// Helper stuff we'll use later ----------------------------------------------------
GLuint myImageTextureId2 = 0;
static ImVec2 gMainMenuBarSize(0,0);
static void ShowExampleAppMainMenuBar() {
    if (ImGui::BeginMainMenuBar())  {
        if (ImGui::BeginMenu("Edit"))   {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        gMainMenuBarSize = ImGui::GetWindowSize();
        ImGui::EndMainMenuBar();
    }
}
#ifndef NO_IMGUILISTVIEW
inline void MyTestListView() {
    ImGui::Spacing();
    static ImGui::ListView lv;
    if (lv.headers.size()==0) {
        lv.headers.push_back(ImGui::ListViewHeader("Icon",NULL,ImGui::ListView::HT_ICON,-1,20));
        lv.headers.push_back(ImGui::ListViewHeader("Index",NULL,ImGui::ListView::HT_INT,-1,30));
        lv.headers.push_back(ImGui::ListViewHeader("Path",NULL,ImGui::ListView::HT_STRING,-1,110,"","",true,ImGui::ListViewHeaderEditing(true,1024)));
        lv.headers.push_back(ImGui::ListViewHeader("Offset",NULL,ImGui::ListView::HT_INT,-1,40,"","",true));
        lv.headers.push_back(ImGui::ListViewHeader("Bytes","The number of bytes",ImGui::ListView::HT_UNSIGNED,-1,40));
        lv.headers.push_back(ImGui::ListViewHeader("Valid","A boolean flag",ImGui::ListView::HT_BOOL,-1,95,"Flag: ","!",true,ImGui::ListViewHeaderEditing(true)));
        lv.headers.push_back(ImGui::ListViewHeader("Length","A float[3] array",ImGui::ListViewHeaderType(ImGui::ListView::HT_FLOAT,3),2,100,""," mt",ImGui::ListViewHeaderSorting(true,1),ImGui::ListViewHeaderEditing(true,3,-180.0,180.0))); // Note that here we use 2 decimals (precision), but 3 when editing; we use an explicit call to "ListViewHeaderType",specifying that the HT_FLOAT is composed by three elements; we have used an explicit call to "ListViewHeaderSorting" specifying that the items must be sorted based on the second float.
        lv.headers.push_back(ImGui::ListViewHeader("Color",NULL,ImGui::ListView::HT_COLOR,-1,95,"","",true,ImGui::ListViewHeaderEditing(true))); // precision = -1 -> Hex notation; precision > 1 -> float notation; other = undefined behaviour. To display alpha we must use "ListViewHeaderType" explicitely like in the line above, specifying 4.

        // Warning: old compilers don't like defining classes inside function scopes
        class MyListViewItem : public ImGui::ListView::ItemBase {
        public:
            // Support static method for enum1 (the signature is the same used by ImGui::Combo(...))
            static bool GetTextFromEnum1(void* ,int value,const char** pTxt) {
                if (!pTxt) return false;
                static const char* values[] = {"APPLE","LEMON","ORANGE"};
                static int numValues = (int)(sizeof(values)/sizeof(values[0]));
                if (value>=0 && value<numValues) *pTxt = values[value];
                else *pTxt = "UNKNOWN";
                return true;
            }

            // Fields and their pointers (MANDATORY!)
            ImGui::ListViewIconData icon;
            int index;
            char path[1024];            // Note that if this column is editable, we must specify: ImGui::ListViewHeaderEditing(true,1024); in the ImGui::ListViewHeader::ctr().
            int offset;
            unsigned bytes;
            bool valid;
            float length[3];
            ImVec4 color;
            int enum1;      // Note that it's an enum!
            const void* getDataPtr(size_t column) const {
                switch (column) {
                case 0: return (const void*) &icon;
                case 1: return (const void*) &index;
                case 2: return (const void*) path;
                case 3: return (const void*) &offset;
                case 4: return (const void*) &bytes;
                case 5: return (const void*) &valid;
                case 6: return (const void*) &length[0];
                case 7: return (const void*) &color;
                case 8: return (const void*) &enum1;
                }
                return NULL;
                // Please note that we can easily try to speed up this method by adding a new field like:
                // const void* fieldPointers[number of fields];    // and assigning them in our ctr
                // Then here we can just use:
                // IM_ASSERT(column<number of fields);
                // return fieldPointers[column];
            }

            // (Optional) ctr for setting values faster later
            MyListViewItem(const ImGui::ListViewIconData& _icon,int _index,const char* _path,int _offset,unsigned _bytes,bool _valid,const ImVec4& _length,const ImVec4& _color,int _enum1)
                : icon(_icon),index(_index),offset(_offset),bytes(_bytes),valid(_valid),color(_color),enum1(_enum1) {
                IM_ASSERT(_path && strlen(_path)<1024);
                strcpy(path,_path);
                length[0] = _length.x;length[1] = _length.y;length[2] = _length.z;  // Note that we have used "ImVec4" for _length, just because ImVec3 does not exist...
            }
            virtual ~MyListViewItem() {}

        };

        // for enums we must use the ctr that takes an ImGui::ListViewHeaderType, so we can pass the additional params to bind the enum:
        lv.headers.push_back(ImGui::ListViewHeader("Enum1","An editable enumeration",ImGui::ListViewHeaderType(ImGui::ListView::HT_ENUM,3,&MyListViewItem::GetTextFromEnum1),-1,-1,"","",true,ImGui::ListViewHeaderEditing(true)));

        // Just a test: 10000 items
        lv.items.resize(10000);
        MyListViewItem* item;ImGui::ListViewIconData ti;
        for (int i=0,isz=(int)lv.items.size();i<isz;i++) {

            ti.user_texture_id = (void*) myImageTextureId2;
            float y = (float)(i/3)/3.f,x=(float)(i%3)/3.f;
            ti.uv0.x=x;ti.uv0.y=y;
            ti.uv1.x=x+1.f/3.f;ti.uv1.y=y+1.f/3.f;
            //ti.tint_col.z=ti.tint_col.x=0;          // tint color = image color is multiplied by this color
            ti.bg_col.x=ti.bg_col.y=ti.bg_col.z=1;ti.bg_col.w=1;  // bg color (used in transparent pixels of the image)

            item = (MyListViewItem*) ImGui::MemAlloc(sizeof(MyListViewItem));                       // MANDATORY (ImGuiListView::~ImGuiListView() will delete these with ImGui::MemFree(...))
            new (item) MyListViewItem(
                        ti,
                        i,
                        "My '  ' Dummy Path",
                        i*3,
                        (unsigned)i*4,(i%3==0)?true:false,
                        ImVec4((float)(i*30)/2.7345672,(float)(i%30)/2.7345672,(float)(i*5)/1.34,1.f),  // ImVec3 does not exist... so we use an ImVec4 to initialize a float[3]
                        ImVec4((float)i/(float)(isz-1),0.8f,1.0f-(float)i/(float)(isz-1),1.0f),         // HT_COLOR
                        i%3
            );    // MANDATORY even with blank ctrs. Requires: #include <new>. Reason: ImVector does not call ctrs/dctrs on items.
            item->path[4]=(char) (33+(i%64));   //just to test sorting on strings
            item->path[5]=(char) (33+(i/127));  //just to test sorting on strings
            lv.items[i] = item;
        }

        //lv.setColorEditingMode(ImGuiColorEditMode_HSV);   // Optional, but it's window-specific: it affects everything in this window AFAIK
    }

    // 2 lines just to have some feedback
    if (ImGui::Button("Scroll to selected row")) lv.scrollToSelectedRow();ImGui::SameLine();
    ImGui::Text("selectedRow:%d selectedColumn:%d isInEditingMode:%s",lv.getSelectedRow(),lv.getSelectedColumn(),lv.isInEditingMode() ? "true" : "false");

    /*
    static ImVector<int> optionalColumnReorder;
    if (optionalColumnReorder.size()==0) {
        const int numColumns = lv.headers.size();
        optionalColumnReorder.resize(numColumns);
        for (int i=0;i<numColumns;i++) optionalColumnReorder[i] = numColumns-i-1;
    }
    */

    static int maxListViewHeight=200;                             // optional: by default is -1 = as height as needed
    ImGui::SliderInt("ListView Height (-1=full)",&maxListViewHeight,-1,500);// Just Testing "maxListViewHeight" here:

    lv.render((float)maxListViewHeight);//(float)maxListViewHeight,&optionalColumnReorder,-1);   // This method returns true when the selectedRow is changed by the user (however when selectedRow gets changed because of sorting it still returns false, because the pointed row-item does not change)

}
#endif //NO_IMGUILISTVIEW
//------------------------------------------------------------------------------------

void InitGL()	// Mandatory
{
    if (!myImageTextureId2) myImageTextureId2 = ImImpl_LoadTexture("./myNumbersTexture.png");
#   if (!defined(NO_IMGUISTYLESERIALIZER) && !defined(NO_IMGUISTYLESERIALIZER_LOAD_STYLE))
    if (!ImGui::LoadStyle("./myimgui.style",ImGui::GetStyle()))   {
        fprintf(stderr,"Warning: \"./myimgui.style\" not present.\n");
    }
#   endif //NO_IMGUISTYLESERIALIZER
    //ImGui::GetIO().MouseDrawCursor = true;
    //ImGui::GetIO().FontAllowUserScaling = true;
}
void ResizeGL(int w,int h)	// Mandatory
{

}
void DestroyGL()    // Mandatory
{
    if (myImageTextureId2) {glDeleteTextures(1,&myImageTextureId2);myImageTextureId2=0;}

}
void DrawGL()	// Mandatory
{

        glClearColor(0.8f, 0.6f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ShowExampleAppMainMenuBar();    // This is plain ImGui

        static bool show_test_window = true;
        static bool show_another_window = false;
        static bool show_node_graph_editor_window = false;
        static bool show_splitter_test_window = false;

        // 1. Show a simple window
        {
            // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
            // Me: However I've discovered that when I clamp the FPS to a low value (e.g.10), I have to catch double clicks manually in my binding to make them work.
            // They work, but for some strange reasons only with windows properly set up through ImGui::Begin(...) and ImGui::End(...) (and whose name is NOT 'Debug').
            // [Please remember that double clicking the titlebar of a window minimizes it]
            // No problem with full frame rates.
            static bool open = true;
            ImGui::Begin("Debug ", &open, ImVec2(450,300));  // Try using 10 FPS and replacing the title with "Debug"...

            static float f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
#           if (!defined(NO_IMGUISTYLESERIALIZER) && !defined(NO_IMGUISTYLESERIALIZER_SAVE_STYLE))
            show_test_window ^= ImGui::Button("Test Window");
#           endif //NO_IMGUISTYLESERIALIZER
#           ifndef NO_IMGUITOOLBAR
            show_another_window ^= ImGui::Button("Another Window With Toolbar Test");
#           endif //NO_IMGUITOOLBAR
#           ifndef NO_IMGUINODEGRAPHEDITOR
            show_node_graph_editor_window ^= ImGui::Button("Another Window With NodeGraphEditor");
#           endif //NO_IMGUINODEGRAPHEDITOR
            show_splitter_test_window ^= ImGui::Button("Show splitter test window");

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

            // imguistyleserializer test
            ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguistyleserializer");ImGui::Separator();
#           if (!defined(NO_IMGUISTYLESERIALIZER) && !defined(NO_IMGUISTYLESERIALIZER_SAVE_STYLE))
            ImGui::Text("Please modify the current style in:");
            ImGui::Text("ImGui Demo->Window Options->Style Editor");
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
#           else //NO_IMGUISTYLESERIALIZER
            ImGui::Text("%s","Excluded from this build.\n");
#           endif //NO_IMGUISTYLESERIALIZER




            // imguifilesystem tests:
            ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguifilesystem");ImGui::Separator();
#           ifndef NO_IMGUIFILESYSTEM
            const char* startingFolder = "./";
            const char* optionalFileExtensionFilterString = "";//".jpg;.jpeg;.png;.tiff;.bmp;.gif;.txt";

            //------------------------------------------------------------------------------------------
            // 1 - ChooseFileDialogButton setup:
            //------------------------------------------------------------------------------------------
            ImGui::Text("Please choose a file: ");ImGui::SameLine();
            const bool browseButtonPressed = ImGui::Button("...");
            static ImGuiFs::Dialog fsInstance;
            fsInstance.chooseFileDialog(browseButtonPressed,startingFolder,optionalFileExtensionFilterString);
            if (strlen(fsInstance.getChosenPath())>0) {
                ImGui::Text("Chosen file: \"%s\"",fsInstance.getChosenPath());
            }

            //------------------------------------------------------------------------------------------
            // 2 - ChooseFolderDialogButton setup:
            //------------------------------------------------------------------------------------------
            ImGui::Text("Please choose a folder: ");ImGui::SameLine();
            const bool browseButtonPressed2 = ImGui::Button("...##2");
            static ImGuiFs::Dialog fsInstance2;
            fsInstance2.chooseFolderDialog(browseButtonPressed2,fsInstance2.getLastDirectory());
            if (strlen(fsInstance2.getChosenPath())>0) {
                ImGui::Text("Chosen folder: \"%s\"",fsInstance2.getChosenPath());
            }

            //------------------------------------------------------------------------------------------
            // 3 - SaveFileDialogButton setup:
            //------------------------------------------------------------------------------------------
            ImGui::Text("Please pretend to save the dummy file 'myFilename.png' to: ");ImGui::SameLine();
            const bool browseButtonPressed3 = ImGui::Button("...##3");
            static ImGuiFs::Dialog fsInstance3;
            fsInstance3.saveFileDialog(browseButtonPressed3,"/usr/include","myFilename.png",".jpg;.jpeg;.png;.tiff;.bmp;.gif;.txt;.zip");//optionalFileExtensionFilterString);
            if (strlen(fsInstance3.getChosenPath())>0) {
                ImGui::Text("Chosen save path: \"%s\"",fsInstance3.getChosenPath());
            }
#           else //NO_IMGUIFILESYSTEM
            ImGui::Text("%s","Excluded from this build.\n");
#           endif //NO_IMGUIFILESYSTEM

            // DateChooser Test:
            ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguidatechooser");ImGui::Separator();
 #          ifndef NO_IMGUIDATECHOOSER
            /*struct tm {
  int tm_sec;			 Seconds.	[0-60] (1 leap second)
  int tm_min;			 Minutes.	[0-59]
  int tm_hour;			 Hours.	[0-23]
  int tm_mday;			 Day.		[1-31]
  int tm_mon;			 Month.	[0-11]
  int tm_year;			 Year	- 1900.
  int tm_wday;			 Day of week.	[0-6]
  int tm_yday;			 Days in year.[0-365]
  };*/
            ImGui::AlignFirstTextHeightToWidgets();
            ImGui::Text("Choose a date:");
            ImGui::SameLine();
            static tm myDate={0};       // IMPORTANT: must be static!
            if (ImGui::DateChooser("Date Chooser##MyDateChooser",myDate,"%d/%m/%Y",true)) {
                // A new date has been chosen
                //fprintf(stderr,"A new date has been chosen exacty now: \"%.2d-%.2d-%.4d\"\n",myDate.tm_mday,myDate.tm_mon+1,myDate.tm_year+1900);
            }
            ImGui::Text("Chosen date: \"%.2d-%.2d-%.4d\"",myDate.tm_mday,myDate.tm_mon+1,myDate.tm_year+1900);
#           else       //NO_IMGUIDATECHOOSER
            ImGui::Text("%s","Excluded from this build.\n");
#           endif      //NO_IMGUIDATECHOOSER

            // imguivariouscontrols            
            ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguivariouscontrols");ImGui::Separator();
#           ifndef NO_IMGUIVARIOUSCONTROLS
            // ProgressBar Test:
            ImGui::TestProgressBar();
            // ColorChooser Test:
            static ImVec4 chosenColor(1,1,1,1);
            static bool openColorChooser = false;
            ImGui::AlignFirstTextHeightToWidgets();ImGui::Text("Please choose a color:");ImGui::SameLine();
            openColorChooser|=ImGui::ColorButton(chosenColor);
            //if (openColorChooser) chosenColor.z=0.f;
            if (ImGui::ColorChooser(&openColorChooser,&chosenColor)) {
                // choice OK here
            }
            // PopupMenuSimple Test:
            // Recent Files-like menu
            static const char* recentFileList[] = {"filename01","filename02","filename03","filename04","filename05","filename06","filename07","filename08","filename09","filename10"};
            static ImGui::PopupMenuSimpleParams pmsParams;
            /*const bool popupMenuButtonClicked = */ImGui::Button("Right-click me##PopupMenuSimpleTest");
            pmsParams.open|= ImGui::GetIO().MouseClicked[1] && ImGui::IsItemHovered(); // RIGHT CLICK on the last widget
                             //popupMenuButtonClicked;    // Or we can just click the button
            const int selectedEntry = ImGui::PopupMenuSimple(pmsParams,recentFileList,(int) sizeof(recentFileList)/sizeof(recentFileList[0]),5,true,"RECENT FILES");
            static int lastSelectedEntry = -1;
            if (selectedEntry>=0) {
                // Do something: clicked recentFileList[selectedEntry].
                // Good in most cases, but here we want to persist the last choice because this branch happens only one frame:
                lastSelectedEntry = selectedEntry;
            }
            if (lastSelectedEntry>=0) {ImGui::SameLine();ImGui::Text("Last selected: %s\n",recentFileList[lastSelectedEntry]);}
            // Fast copy/cut/paste menus
            static char buf[512]="Some sample text";
            ImGui::InputTextMultiline("Right click to have\na (non-functional)\ncopy/cut/paste menu\nin one line of code##TestCopyCutPaste",buf,512);
            const int cutCopyOrPasteSelected = ImGui::PopupMenuSimpleCopyCutPasteOnLastItem();
            if (cutCopyOrPasteSelected>=0)  {
                // Here we have 0 = cut, 1 = copy, 2 = paste
                // However ImGui::PopupMenuSimpleCopyCutPasteOnLastItem() can't perform these operations for you
                // and it's not trivial at all... at least I've got no idea on how to do it!
                // Moreover, the selected text seems to lose focus when the menu is selected...
            }

            // Single column popup menu with icon support. It disappears when the mouse goes away. Never tested.
            // User is supposed to create a static instance of it, add entries once, and then call "render()".
            static ImGui::PopupMenu pm;
            if (pm.isEmpty())   {
                pm.addEntryTitle("Single Menu With Images");
                char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                for (int i=0;i<9;i++) {
                    strcpy(tmp,"Image Menu Entry ");
                    sprintf(&tmp[strlen(tmp)],"%d",i+1);
                    uv0 = ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                    uv1 = ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);

                    pm.addEntry(tmp,reinterpret_cast<void*>(myImageTextureId2),uv0,uv1);
                }

            }
            static bool trigger = false;
            trigger|=ImGui::Button("Press me for a menu with images##PopupMenuWithImagesTest");
            /*const int selectedImageMenuEntry =*/ pm.render(trigger);   // -1 = none
#           else //NO_IMGUIVARIOUSCONTROLS
            ImGui::Text("%s","Excluded from this build.\n");
#           endif //NO_IMGUIVARIOUSCONTROLS


            // ListView Test:
            ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguilistview");ImGui::Separator();
#           ifndef NO_IMGUILISTVIEW
            MyTestListView();
#           else //NO_IMGUILISTVIEW
            ImGui::Text("%s","Excluded from this build.\n");
#           endif //NO_IMGUILISTVIEW

            ImGui::Separator();

            ImGui::End();
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
#       ifndef NO_IMGUITOOLBAR
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window, ImVec2(500,100));
            {
                // imguitoolbar test (note that it can be used both inside and outside windows (see below)
                ImGui::Separator();ImGui::Text("imguitoolbar");ImGui::Separator();
                static ImGui::Toolbar toolbar;
                if (toolbar.getNumButtons()==0)  {
                    char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                    for (int i=0;i<9;i++) {
                        strcpy(tmp,"toolbutton ");
                        sprintf(&tmp[strlen(tmp)],"%d",i+1);
                        uv0 = ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                        uv1 = ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);

                        toolbar.addButton(ImGui::Toolbutton(tmp,(void*)myImageTextureId2,uv0,uv1,ImVec2(16,16)));
                    }
                    toolbar.addSeparator(16);
                    toolbar.addButton(ImGui::Toolbutton("toolbutton 11",(void*)myImageTextureId2,uv0,uv1,ImVec2(16,16),true,true,ImVec4(0.8,0.8,1.0,1)));  // Note that separator "eats" one toolbutton index as if it was a real button
                    toolbar.addButton(ImGui::Toolbutton("toolbutton 12",(void*)myImageTextureId2,uv0,uv1,ImVec2(16,16),true,false,ImVec4(1.0,0.8,0.8,1)));  // Note that separator "eats" one toolbutton index as if it was a real button

                    toolbar.setProperties(true,false,false,ImVec2(0.0f,0.f),ImVec2(0.25,1));
                }
                const int pressed = toolbar.render();
                if (pressed>=0) fprintf(stderr,"Toolbar1: pressed:%d\n",pressed);
            }
            // Here we can open a child window if we want to toolbar not to scroll
            ImGui::Spacing();ImGui::Text("imguitoolbar can be used inside windows too.\nThe first series of buttons can be used as a tab control.\nPlease resize the window and see the dynamic layout.\n");
            ImGui::End();
        }
#       endif //NO_IMGUITOOLBAR

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
#       if (!defined(NO_IMGUISTYLESERIALIZER) && !defined(NO_IMGUISTYLESERIALIZER_SAVE_STYLE))
        if (show_test_window)
        {
            //ImGui::SetNewWindowDefaultPos(ImVec2(650, 20));        // Normally user code doesn't need/want to call this, because positions are saved in .ini file. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowTestWindow(&show_test_window);
        }
#       endif // NO_IMGUISTYLESERIALIZER
#       ifndef NO_IMGUINODEGRAPHEDITOR
        if (show_node_graph_editor_window) {
            static ImGui::NodeGraphEditor nge;
            if (nge.isEmpty())	{
                static const char* mainTexOutputNames[2] = {"rgb","alpha"};
                nge.addNode(0, "MainTex",  ImVec2(40,50), 0.5f, ImColor(255,100,100), 0, 2,NULL,mainTexOutputNames);
                static const char* bumpMapInputNames[1] = {"in"};
                static const char* bumpMapOutputNames[1] = {"bumpOut"};
                nge.addNode(1, "BumpMap",  ImVec2(40,150), 0.42f, ImColor(200,100,200), 1, 1,bumpMapInputNames,bumpMapOutputNames);
                static const char* combineInputNames[3] = {"c1","c2","c3"};
                static const char* combineOutputNames[1] = {"out"};
                nge.addNode(2, "Combine", ImVec2(270,80), 1.0f, ImColor(0,200,100), 3, 1,combineInputNames,combineOutputNames);
                nge.addLink(0, 0, 2, 0);
                nge.addLink(0, 1, 2, 1);
                nge.addLink(1, 0, 2, 2);
            }
            nge.render(&show_node_graph_editor_window);
        }
#       endif //NO_IMGUINODEGRAPHEDITOR
        if (show_splitter_test_window)  {
            // snippet by omar
            ImGui::Begin("Splitter test",&show_splitter_test_window,ImVec2(500,500));

            static float w = 200.0f;
            static float h = 300.0f;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));

            // window top left
            ImGui::BeginChild("child1", ImVec2(w, h), true);
            ImGui::EndChild();
            // horizontal splitter
            ImGui::SameLine();
            ImGui::InvisibleButton("hsplitter", ImVec2(8.0f,h));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (ImGui::IsItemActive())  w += ImGui::GetIO().MouseDelta.x;
            ImGui::SameLine();
            // window top right
            ImGui::BeginChild("child2", ImVec2(0, h), true);
            ImGui::EndChild();
            // vertical splitter
            ImGui::InvisibleButton("vsplitter", ImVec2(-1,8.0f));
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            if (ImGui::IsItemActive())  h += ImGui::GetIO().MouseDelta.y;
            // window bottom
            ImGui::BeginChild("child3", ImVec2(0,0), true);
            ImGui::EndChild();

            ImGui::PopStyleVar();

            ImGui::End();
        }

        // imguitoolbar test 2: two global toolbars one at the top and one at the left
#       ifndef NO_IMGUITOOLBAR
        // These two lines are only necessary to accomodate space for the global menu bar we're using:
        const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        const ImVec4 displayPortion = ImVec4(0,gMainMenuBarSize.y,displaySize.x,displaySize.y-gMainMenuBarSize.y);

        {
            static ImGui::Toolbar toolbar("myFirstToolbar##foo");
            if (toolbar.getNumButtons()==0)  {
                char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                for (int i=0;i<9;i++) {
                    strcpy(tmp,"toolbutton ");
                    sprintf(&tmp[strlen(tmp)],"%d",i+1);
                    uv0 = ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                    uv1 = ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);

                    toolbar.addButton(ImGui::Toolbutton(tmp,reinterpret_cast<void*>(myImageTextureId2),uv0,uv1));
                }
                toolbar.addSeparator(16);
                toolbar.addButton(ImGui::Toolbutton("toolbutton 11",reinterpret_cast<void*>(myImageTextureId2),uv0,uv1,ImVec2(32,32),true,false,ImVec4(0.8,0.8,1.0,1)));  // Note that separator "eats" one toolbutton index as if it was a real button
                toolbar.addButton(ImGui::Toolbutton("toolbutton 12",reinterpret_cast<void*>(myImageTextureId2),uv0,uv1,ImVec2(48,24),true,false,ImVec4(1.0,0.8,0.8,1)));  // Note that separator "eats" one toolbutton index as if it was a real button

                toolbar.setProperties(false,false,true,ImVec2(0.5f,0.f),ImVec2(-1,-1),ImVec4(1,1,1,1),displayPortion);


            }
            const int pressed = toolbar.render();
            if (pressed>=0) {printf("Toolbar1: pressed:%d\n",pressed);fflush(stdout);}
        }
        {
            static ImGui::Toolbar toolbar("myFirstToolbar2##foo");
            if (toolbar.getNumButtons()==0)  {
                char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                for (int i=8;i>=0;i--) {
                    strcpy(tmp,"toolbutton ");
                    sprintf(&tmp[strlen(tmp)],"%d",8-i+1);
                    uv0=ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                    uv1=ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);

                    toolbar.addButton(ImGui::Toolbutton(tmp,reinterpret_cast<void*>(myImageTextureId2),uv0,uv1,ImVec2(24,48)));
                }
                toolbar.addSeparator(16);
                toolbar.addButton(ImGui::Toolbutton("toolbutton 11",reinterpret_cast<void*>(myImageTextureId2),uv0,uv1,ImVec2(24,32),true,false,ImVec4(0.8,0.8,1.0,1)));  // Note that separator "eats" one toolbutton index as if it was a real button
                toolbar.addButton(ImGui::Toolbutton("toolbutton 12",reinterpret_cast<void*>(myImageTextureId2),uv0,uv1,ImVec2(24,32),true,false,ImVec4(1.0,0.8,0.8,1)));  // Note that separator "eats" one toolbutton index as if it was a real button

                toolbar.setProperties(true,true,false,ImVec2(0.0f,0.0f),ImVec2(0.25f,0.9f),ImVec4(0.85,0.85,1,1),displayPortion);

                //toolbar.setScaling(2.0f,1.1f);
            }
            const int pressed = toolbar.render();
            if (pressed>=0) {printf("Toolbar2: pressed:%d\n",pressed);fflush(stdout);}
        }
#       endif //NO_IMGUITOOLBAR
}



//#   define USE_ADVANCED_SETUP   // in-file definition (see below). For now it just adds custom fonts and clamps FPS to 10 (= jerky movements when moving windows).

// Application code
#ifndef IMGUI_USE_WINAPI_BINDING
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
    gImGuiInitParams.gFpsClamp = 10.0f;                                 // Optional Max allowed FPS (default -1 => unclamped). Useful for editors and to save GPU and CPU power.

    ImImpl_Main(&gImGuiInitParams,argc,argv);
#   endif //USE_ADVANCED_SETUP

	return 0;
}
#else //IMGUI_USE_WINAPI_BINDING
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int iCmdShow)   // This branch has made my code less concise (I will consider stripping it)
{
#   ifndef USE_ADVANCED_SETUP
    // Basic
    ImImpl_WinMain(NULL,hInstance,hPrevInstance,lpCmdLine,iCmdShow);
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
    gImGuiInitParams.gFpsClamp = 10.0f;                                 // Optional Max allowed FPS (default -1 => unclamped). Useful for editors and to save GPU and CPU power.

    ImImpl_WinMain(&gImGuiInitParams,hInstance,hPrevInstance,lpCmdLine,iCmdShow);
#   endif //#   USE_ADVANCED_SETUP

    return 0;
}
#endif //IMGUI_USE_WINAPI_BINDING



