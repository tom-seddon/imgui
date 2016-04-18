#include <imgui.h>
#ifndef NO_IMGUIDATECHOOSER
#include <time.h>   // very common plain c header file used only by DateChooser
#endif //NO_IMGUIDATECHOOSER


// Helper stuff we'll use later ----------------------------------------------------
ImTextureID myImageTextureId2 = 0;
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
        lv.headers.push_back(ImGui::ListViewHeader("Path",NULL,ImGui::ListView::HT_STRING,-1,80,"","",true,ImGui::ListViewHeaderEditing(true,1024)));
        lv.headers.push_back(ImGui::ListViewHeader("Offset",NULL,ImGui::ListView::HT_INT,-1,40,"","",true));
        lv.headers.push_back(ImGui::ListViewHeader("Bytes","The number of bytes",ImGui::ListView::HT_UNSIGNED,-1,40));
        lv.headers.push_back(ImGui::ListViewHeader("Valid","A boolean flag",ImGui::ListView::HT_BOOL,-1,70,"Flag: ","!",true,ImGui::ListViewHeaderEditing(true)));
        lv.headers.push_back(ImGui::ListViewHeader("Length","A float[3] array",ImGui::ListViewHeaderType(ImGui::ListView::HT_FLOAT,3),2,100,""," mt",ImGui::ListViewHeaderSorting(true,1),ImGui::ListViewHeaderEditing(true,3,-180.0,180.0))); // Note that here we use 2 decimals (precision), but 3 when editing; we use an explicit call to "ListViewHeaderType",specifying that the HT_FLOAT is composed by three elements; we have used an explicit call to "ListViewHeaderSorting" specifying that the items must be sorted based on the second float.
        lv.headers.push_back(ImGui::ListViewHeader("Color",NULL,ImGui::ListView::HT_COLOR,-1,70,"","",true,ImGui::ListViewHeaderEditing(true))); // precision = -1 -> Hex notation; precision > 1 -> float notation; other = undefined behaviour. To display alpha we must use "ListViewHeaderType" explicitely like in the line above, specifying 4.

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

            ti.user_texture_id = reinterpret_cast<void*> (myImageTextureId2);
            float y = (float)(i/3)/3.f,x=(float)(i%3)/3.f;
            ti.uv0.x=x;ti.uv0.y=y;
            ti.uv1.x=x+1.f/3.f;ti.uv1.y=y+1.f/3.f;
            //ti.tint_col.z=ti.tint_col.x=0;          // tint color = image color is multiplied by this color
            ti.bg_col.x=ti.bg_col.y=ti.bg_col.z=1;ti.bg_col.w=1;  // bg color (used in transparent pixels of the image)

            item = (MyListViewItem*) ImGui::MemAlloc(sizeof(MyListViewItem));                       // MANDATORY (ImGuiListView::~ImGuiListView() will delete these with ImGui::MemFree(...))
            IMIMPL_PLACEMENT_NEW(item) MyListViewItem(
                        ti,
                        i,
                        "My '  ' Dummy Path",
                        i*3,
                        (unsigned)i*4,(i%3==0)?true:false,
                        ImVec4((float)(i*30)/2.7345672,(float)(i%30)/2.7345672,(float)(i*5)/1.34,1.f),  // ImVec3 does not exist... so we use an ImVec4 to initialize a float[3]
                        ImVec4((float)i/(float)(isz-1),0.8f,1.0f-(float)i/(float)(isz-1),1.0f),         // HT_COLOR
                        i%3
            );    // MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
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

// These are only needed if you need to modify them at runtime (almost never).
// Otherwise you can set them directly in "main" (see at the botton of this file).
extern bool gImGuiDynamicFPSInsideImGui;
extern float gImGuiInverseFPSClampInsideImGui;
extern float gImGuiInverseFPSClampOutsideImGui;
//------------------------------------------------------------------------------------

static const char* styleFileName = "./myimgui.style";
static const char* styleFileNamePersistent = "/persistent_folder/myimgui.style";   // Needed by Emscripten only


void InitGL()	// Mandatory
{
    if (!myImageTextureId2) myImageTextureId2 = ImImpl_LoadTexture("./myNumbersTexture.png");


//  Optional: loads a style
#   if (!defined(NO_IMGUISTYLESERIALIZER) && !defined(NO_IMGUISTYLESERIALIZER_LOAD_STYLE))
    const char* pStyleFileName = styleFileName;
//#define TEST_TO_DELETE
#ifdef TEST_TO_DELETE
    ImGuiFs::PathStringVector dirs;
    ImGuiFs::DirectoryGetDirectories("/",dirs);
    for (int i=0;i<dirs.size();i++) printf("%s\n",dirs[i]);
    fflush(stdout);
#undef TEST_TO_DELETE
#endif //TEST_TO_DELETE
#   if (!defined(NO_IMGUIEMSCRIPTEN) && !defined(NO_IMGUIHELPER) && !defined(NO_IMGUIHELPER_SERIALIZATION) && !defined(NO_IMGUIHELPER_SERIALIZATION_LOAD))
    //ImGui::EmscriptenFileSystemHelper::Init();
    //while (!ImGui::EmscriptenFileSystemHelper::IsInSync()) {WaitFor(1500);}   // No way this ends...
    //if (ImGuiHelper::FileExists(styleFileNamePersistent)) pStyleFileName = styleFileNamePersistent; // Never...
    //else {printf("\"%s\" does not exist yet\n",styleFileNamePersistent);fflush(stdout);}
#   endif //NO_IMGUIEMSCRIPTEN & C
    if (!ImGui::LoadStyle(pStyleFileName,ImGui::GetStyle()))   {
        printf("Warning: \"%s\" not present.\n",pStyleFileName);fflush(stdout);
    }
#   endif //NO_IMGUISTYLESERIALIZER


//  Optional CTRL + MW to zoom
    //ImGui::GetIO().FontAllowUserScaling = true;

//#define TEST_SERIALIZER // development only: but I definitely need to perform some tests...
#ifdef TEST_SERIALIZER
struct Parser {
static bool ParseCallback(ImGui::FieldType ft,int numArrayElements,void* pValue,const char* name,void* userPtr)    {
    switch (ft) {
    case ImGui::FT_STRING:  {
        const char* txt = (const char*) pValue;     // we can just use strlen(txt) and copy it with strcpy/strncpy...
        fprintf(stderr,"[FT_STRING-%d:%s]: \"%s\" (%d)\n",numArrayElements,name,txt,(int)strlen((const char*) pValue));
    }
    break;
    case ImGui::FT_TEXTLINE:    {   // Called once per line
        const char* txt = (const char*) pValue;     // we can just use strlen(txt) and copy it with strcpy/strncpy...
        fprintf(stderr,"[FT_TEXTLINE-%d:%s]: \"%s\" (%d)\n",numArrayElements,name,txt,(int)strlen((const char*) pValue));
    }
    break;
    }
    return false;
}
};

const char* fileName = "myTest.conf";
{
    ImGuiHelper::Serializer s(fileName);
    s.save("This is a string that can be\nshort or long.","MyString");
    s.saveTextLines("This is a string that can be\nshort or long.","MyStringTextLines");
}
{
    ImGuiHelper::Deserializer d(fileName);
    const char* offset;
    offset = d.parse(&Parser::ParseCallback,NULL,offset);
}
#endif //TEST_SERIALIZER
}
void ResizeGL(int /*w*/,int /*h*/)	// Mandatory
{

}
void DestroyGL()    // Mandatory
{
    if (myImageTextureId2) {ImImpl_FreeTexture(myImageTextureId2);}

}
void DrawGL()	// Mandatory
{
    const ImVec4 defaultClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    static ImVec4 clearColor = defaultClearColor;
    ImImpl_ClearColorBuffer(clearColor);    // Warning: it does not clear depth buffer

    // Pause/Resume ImGui and process input as usual
    if (!ImGui::GetIO().WantCaptureKeyboard)    {
        if (ImGui::IsKeyPressed('h',false)       // case sensitive
                || gImGuiFunctionKeyReleased[0])     // 0 = F1, 11 = F12
            gImGuiPaused = !gImGuiPaused;       // TODO: fix minor visual bug (if possible) happening at the first "restoring ImGui" frame (comment out the line "if (gImGuiPaused) return;" to see it)
    }

    // More detailed example on how to overuse ImGui to process input. This works even when ImGui is paused:
    if (gImGuiWereOutsideImGui) // true when "gImGuiPaused" or simply when we're outside ImGui Windows with both mouse and cursor.
    {
        ImGuiIO& io = ImGui::GetIO();
        static unsigned myStrangeCounter=0;

        if (!io.WantCaptureKeyboard && !io.WantCaptureMouse && !io.WantTextInput )   // always "true" if we "leave" "if (gImGuiWereOutsideImGui)" AFAIR
        {

            // (Key and Mouse) Input events fall into two main categories:
            // 1) "stateful" events (= pressed or released events) are easier to process. e.g.:

            if (ImGui::IsKeyPressed('H',false))        // case sensitive
            {printf("'H' key pressed outside Imgui (%u)\n",myStrangeCounter++);fflush(stdout);}
            // Tips for processing other "stateful" events (e.g. key/mouse pressed/released):
            // a) for ImGui "known special chars", we can use something like:
            //      if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Escape],false)  {...}
            // b) for mouse Pressed/Released events:
            //      if (io.KeysPressed[...]) {...}
            // c) for F1-F12 Pressed/Released events use gImGuiFunctionKeyXXX[0-11]:
            //      if (gImGuiFunctionKeyReleased[0]) {...}

            // 2) "immediate" (or "continuous") events (= down events)
            // When taking actions based on continuous events (e.g. "down events"), IMHO it's better to sync
            // them to ensure the same behavior at differernt FPS.
            // "inputProcessingInterval" should be a fixed amount >= our inverse frame rate.
            // However, since we will later allow the user to modify the frame rate, we can't leave it constant here.
            const float inputProcessingInterval = gImGuiInverseFPSClampOutsideImGui<=0 ? 0.03 : gImGuiInverseFPSClampOutsideImGui*2.f;  // Input processing frequency (better leave it constant)
            static float timer = ImGui::GetTime();
            float delta = ImGui::GetTime() - timer;
            if (delta<inputProcessingInterval) {
                if (delta<0) timer = ImGui::GetTime();  // protects from overflow ? (probably just an intention)
            }
            else {
                timer+=delta;
                //-------------------------------------------------------------------
                if (io.KeysDown[io.KeyMap[ImGuiKey_RightArrow]]) {printf("Right arrow pressed outside Imgui (%u)\n",myStrangeCounter++);fflush(stdout);}
                //if (io.MouseDown[2]) {printf("Middle Mouse Button pressed outside Imgui (%u)\n",myStrangeCounter++);fflush(stdout);}
                //-------------------------------------------------------------------
            }
        }

    }

    if (gImGuiPaused) return; // exit early (even if ocassional ImGui calls should be allowed, as "immediate mode GUI" is made to mix GUI calls with normal code)


    ShowExampleAppMainMenuBar();    // This is plain ImGui

    static bool show_test_window = true;
    static bool show_another_window = false;
    static bool show_node_graph_editor_window = false;
    static bool show_splitter_test_window = false;
    static bool show_dock_window = false;
    static bool show_tab_windows = false;


    // 1. Show a simple window
    {
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        // Me: However I've discovered that when I clamp the FPS to a low value (e.g.10), I have to catch double clicks manually in my binding to make them work.
        // They work, but for some strange reasons only with windows properly set up through ImGui::Begin(...) and ImGui::End(...) (and whose name is NOT 'Debug').
        // [Please remember that double clicking the titlebar of a window minimizes it]
        // No problem with full frame rates.
        static bool open = true;static bool no_border = false;static float bg_alpha = -1.f;
        ImGui::Begin("Debug ", &open, ImVec2(450,300),bg_alpha,no_border ? 0 : ImGuiWindowFlags_ShowBorders);  // Try using 10 FPS and replacing the title with "Debug"...

        ImGui::Text("\n");ImGui::Separator();ImGui::Text("Pause/Resume ImGui and process input as usual");ImGui::Separator();
        ImGui::Text("Press F1 (or lowercase 'h') to turn ImGui on and off.");
        ImVec4 halfTextColor = ImGui::GetStyle().Colors[ImGuiCol_Text];halfTextColor.w*=0.5f;
        ImGui::TextColored(halfTextColor,"(Please read the code for further tips about input processing).");


        ImGui::Text("\n");ImGui::Separator();ImGui::Text("Test Windows");ImGui::Separator();
#       if (!defined(NO_IMGUISTYLESERIALIZER) && !defined(NO_IMGUISTYLESERIALIZER_SAVE_STYLE))
        show_test_window ^= ImGui::Button("Test Window");
#       endif //NO_IMGUISTYLESERIALIZER
#       ifndef NO_IMGUITOOLBAR
        show_another_window ^= ImGui::Button("Another Window With Toolbar Test");
#       endif //NO_IMGUITOOLBAR
#       ifndef NO_IMGUINODEGRAPHEDITOR
        show_node_graph_editor_window ^= ImGui::Button("Another Window With NodeGraphEditor");
#       endif //NO_IMGUINODEGRAPHEDITOR
        show_splitter_test_window ^= ImGui::Button("Show splitter test window");
#       ifdef YES_IMGUIDOCK
        show_dock_window ^= ImGui::Button("Another Window With ImGuiDock");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","LumixEngine Dock system test.\nHowever I guess it was intended to be used globally.\nSo the result looks ugly, the main menu hides the tabs\nand once started the only way to stop it is to click here again.\nI really suggest you test it globally in a brand new demo.");
#       endif //YES_IMGUIDOCK
#       ifndef NO_IMGUITABWINDOW
        show_tab_windows ^= ImGui::Button("Show TabWindow Test");
#       endif //NO_IMGUITABWINDOW


        // Calculate and show framerate
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("Frame rate options");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","It might be necessary to move the mouse \"outside\" and \"inside\" ImGui for these options to update properly.");
        ImGui::Separator();
        static float ms_per_frame[120] = { 0 };
        static int ms_per_frame_idx = 0;
        static float ms_per_frame_accum = 0.0f;
        ms_per_frame_accum -= ms_per_frame[ms_per_frame_idx];
        ms_per_frame[ms_per_frame_idx] = ImGui::GetIO().DeltaTime * 1000.0f;
        ms_per_frame_accum += ms_per_frame[ms_per_frame_idx];
        ms_per_frame_idx = (ms_per_frame_idx + 1) % 120;
        const float ms_per_frame_avg = ms_per_frame_accum / 120;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);
        bool clampFPSOutsideImGui = gImGuiInverseFPSClampOutsideImGui > 0;
        ImGui::Checkbox("Clamp FPS when \"outside\" ImGui.",&clampFPSOutsideImGui);
        if (clampFPSOutsideImGui)    {
            if (gImGuiInverseFPSClampOutsideImGui<=0) gImGuiInverseFPSClampOutsideImGui = 1.f/60.f;
            float FPS = 1.f/gImGuiInverseFPSClampOutsideImGui;
            if (ImGui::SliderFloat("FPS when \"outside\" ImGui",&FPS,5.f,60.f,"%.2f")) gImGuiInverseFPSClampOutsideImGui = 1.f/FPS;
        }
        else gImGuiInverseFPSClampOutsideImGui = -1.f;
        bool clampFPSInsideImGui = gImGuiInverseFPSClampInsideImGui > 0;
        ImGui::Checkbox("Clamp FPS when \"inside\" ImGui.",&clampFPSInsideImGui);
        if (clampFPSInsideImGui)    {
            if (gImGuiInverseFPSClampInsideImGui<=0) gImGuiInverseFPSClampInsideImGui = 1.f/60.f;
            float FPS = 1.f/gImGuiInverseFPSClampInsideImGui;
            if (ImGui::SliderFloat("FPS when \"inside\" ImGui",&FPS,5.f,60.f,"%.2f")) gImGuiInverseFPSClampInsideImGui = 1.f/FPS;
        }
        else gImGuiInverseFPSClampInsideImGui = -1.f;
        ImGui::Checkbox("Use dynamic FPS when \"inside\" ImGui.",&gImGuiDynamicFPSInsideImGui);

        ImGui::Text("\n");ImGui::Separator();ImGui::Text("Font options");ImGui::Separator();
        //ImGui::Checkbox("Font Allow User Scaling", &ImGui::GetIO().FontAllowUserScaling);
        //if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","If true, CTRL + mouse wheel scales the window\n(or just its font size if child window).");
        ImGui::PushItemWidth(275);
        ImGui::DragFloat("Global Font Scale", &ImGui::GetIO().FontGlobalScale, 0.005f, 0.3f, 2.0f, "%.2f"); // scale everything
        ImGui::PopItemWidth();
        if (ImGui::GetIO().FontGlobalScale!=1.f)    {
            ImGui::SameLine(0,10);
            if (ImGui::SmallButton("Reset##glFontGlobalScale")) ImGui::GetIO().FontGlobalScale = 1.f;
        }

        // Some options ported from imgui_demo.cpp
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("Window options");ImGui::Separator();
        ImGui::Checkbox("No border", &no_border);
        ImGui::SameLine(0,25);
        ImGui::PushItemWidth(100);
        ImGui::DragFloat("Window Fill Alpha", &bg_alpha, 0.005f, -0.01f, 1.0f, bg_alpha < 0.0f ? "(default)" : "%.3f"); // Not exposing zero here so user doesn't "close" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
        ImGui::PopItemWidth();
        ImGui::PushItemWidth(275);
        ImGui::ColorEdit3("glClearColor",&clearColor.x);
        ImGui::PopItemWidth();
        if (clearColor.x!=defaultClearColor.x || clearColor.y!=defaultClearColor.y || clearColor.z!=defaultClearColor.z)    {
            ImGui::SameLine(0,10);
            if (ImGui::SmallButton("Reset##glClearColorReset")) clearColor = defaultClearColor;
        }

        // imguistyleserializer test
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguistyleserializer");ImGui::Separator();
#       if (!defined(NO_IMGUISTYLESERIALIZER) && !defined(NO_IMGUISTYLESERIALIZER_SAVE_STYLE))
        ImGui::Text("Please modify the current style in:");
        ImGui::Text("ImGui Demo->Window Options->Style Editor");
        static bool loadCurrentStyle = false;
        static bool saveCurrentStyle = false;
        static bool resetCurrentStyle = false;
        loadCurrentStyle = ImGui::Button("Load Saved Style");
        saveCurrentStyle = ImGui::Button("Save Current Style");
        resetCurrentStyle = ImGui::Button("Reset Current Style To: ");
        ImGui::SameLine();
        static int styleEnumNum = 1;
        ImGui::PushItemWidth(135);
        ImGui::Combo("###StyleEnumCombo",&styleEnumNum,ImGui::GetDefaultStyleNames(),(int) ImGuiStyle_Count,(int) ImGuiStyle_Count);
        ImGui::PopItemWidth();
        if (ImGui::IsItemHovered()) {
            if   (styleEnumNum==0)      ImGui::SetTooltip("%s","\"Default\"\nThis is the default\nImGui theme");
            else if (styleEnumNum==1)   ImGui::SetTooltip("%s","\"Gray\"\nThis is the default\ntheme of this demo");
            else if (styleEnumNum==2)   ImGui::SetTooltip("%s","\"OSX\"\nPosted by @itamago here:\nhttps://github.com/ocornut/imgui/pull/511\n(hope I can use it)");
        }

        ImGui::SameLine();
        static float hueShift = 0;
        ImGui::PushItemWidth(50);
        ImGui::DragFloat("HueShift##styleShiftHue",&hueShift,.005f,0,1,"%.2f");
        ImGui::PopItemWidth();
        if (hueShift!=0)   {
            ImGui::SameLine();
            if (ImGui::SmallButton("reset##styleReset")) {hueShift=0.f;}
        }
        const bool mustInvertColors = ImGui::Button("Invert Colors:##styleInvertColors");
        ImGui::SameLine();
        ImGui::PushItemWidth(50);
        static float invertColorThreshold = .1f;
        ImGui::DragFloat("Saturation Threshold##styleLumThres",&invertColorThreshold,.005f,0.f,0.5f,"%.2f");
        ImGui::PopItemWidth();
        if (mustInvertColors)  ImGui::ChangeStyleColors(ImGui::GetStyle(),invertColorThreshold);

        const char* pStyleFileName =  styleFileName;    // defined globally
        if (loadCurrentStyle)   {
#               if (!defined(NO_IMGUIEMSCRIPTEN) && !defined(NO_IMGUIHELPER) && !defined(NO_IMGUIHELPER_SERIALIZATION) && !defined(NO_IMGUIHELPER_SERIALIZATION_LOAD))
            if (ImGuiHelper::FileExists(styleFileNamePersistent)) pStyleFileName = styleFileNamePersistent;
#               endif //NO_IMGUIEMSCRIPTEN & C
            if (!ImGui::LoadStyle(pStyleFileName,ImGui::GetStyle()))   {
                fprintf(stderr,"Warning: \"%s\" not present.\n",pStyleFileName);
            }
        }
        if (saveCurrentStyle)   {
#               ifndef NO_IMGUIEMSCRIPTEN
            pStyleFileName = styleFileNamePersistent;
#               endif //NO_IMGUIEMSCRIPTEN
            if (!ImGui::SaveStyle(pStyleFileName,ImGui::GetStyle()))   {
                fprintf(stderr,"Warning: \"%s\" cannot be saved.\n",pStyleFileName);
            }
            else {
#                   ifndef NO_IMGUIEMSCRIPTEN
                ImGui::EmscriptenFileSystemHelper::Sync();
#                   endif //NO_IMGUIEMSCRIPTEN
            }
        }
        if (resetCurrentStyle)  {
            ImGui::ResetStyle(styleEnumNum,ImGui::GetStyle());
            if (hueShift!=0) ImGui::ChangeStyleColors(ImGui::GetStyle(),0.f,hueShift);
        }
#       else //NO_IMGUISTYLESERIALIZER
        ImGui::Text("%s","Excluded from this build.\n");
#       endif //NO_IMGUISTYLESERIALIZER


        // imguifilesystem tests:
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguifilesystem");ImGui::Separator();
#       ifndef NO_IMGUIFILESYSTEM
        const char* startingFolder = "./";
        const char* optionalFileExtensionFilterString = "";//".jpg;.jpeg;.png;.tiff;.bmp;.gif;.txt";

        //------------------------------------------------------------------------------------------
        // 1 - ChooseFileDialogButton setup:
        //------------------------------------------------------------------------------------------
        ImGui::Text("Please choose a file: ");ImGui::SameLine();
        const bool browseButtonPressed = ImGui::Button("...");
        static ImGuiFs::Dialog fsInstance;
        const char* chosenPath = fsInstance.chooseFileDialog(browseButtonPressed,startingFolder,optionalFileExtensionFilterString);
        if (strlen(chosenPath)>0) {
            // A path (chosenPath) has been chosen right now. However we can retrieve it later using: fsInstance.getChosenPath()
        }
        if (strlen(fsInstance.getChosenPath())>0) ImGui::Text("Chosen path: \"%s\"",fsInstance.getChosenPath());

        //------------------------------------------------------------------------------------------
        // 2 - ChooseFolderDialogButton setup:
        //------------------------------------------------------------------------------------------
        ImGui::Text("Please choose a folder: ");ImGui::SameLine();
        const bool browseButtonPressed2 = ImGui::Button("...##2");
        static ImGuiFs::Dialog fsInstance2;
        const char* chosenFolder = fsInstance2.chooseFolderDialog(browseButtonPressed2,fsInstance2.getLastDirectory());
        if (strlen(chosenFolder)>0) {
            // A path (chosenFolder) has been chosen right now. However we can retrieve it later using: fsInstance2.getChosenPath()
        }
        if (strlen(fsInstance2.getChosenPath())>0) ImGui::Text("Chosen folder: \"%s\"",fsInstance2.getChosenPath());

        //------------------------------------------------------------------------------------------
        // 3 - SaveFileDialogButton setup:
        //------------------------------------------------------------------------------------------
        ImGui::Text("Please pretend to save the dummy file 'myFilename.png' to: ");ImGui::SameLine();
        const bool browseButtonPressed3 = ImGui::Button("...##3");
        static ImGuiFs::Dialog fsInstance3;
        const char* savePath = fsInstance3.saveFileDialog(browseButtonPressed3,"/usr/include","myFilename.png",".jpg;.jpeg;.png;.tiff;.bmp;.gif;.txt;.zip");//optionalFileExtensionFilterString);
        if (strlen(savePath)>0) {
            // A path (savePath) has been chosen right now. However we can retrieve it later using: fsInstance3.getChosenPath()
        }
        if (strlen(fsInstance3.getChosenPath())>0) ImGui::Text("Chosen save path: \"%s\"",fsInstance3.getChosenPath());


#       else //NO_IMGUIFILESYSTEM
        ImGui::Text("%s","Excluded from this build.\n");
#       endif //NO_IMGUIFILESYSTEM

        // DateChooser Test:
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguidatechooser");ImGui::Separator();
#       ifndef NO_IMGUIDATECHOOSER
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
        static tm myDate={0};       // IMPORTANT: must be static! (plenty of compiler warnings here...)
        if (ImGui::DateChooser("Date Chooser##MyDateChooser",myDate,"%d/%m/%Y",true)) {
            // A new date has been chosen
            //fprintf(stderr,"A new date has been chosen exacty now: \"%.2d-%.2d-%.4d\"\n",myDate.tm_mday,myDate.tm_mon+1,myDate.tm_year+1900);
        }
        ImGui::Text("Chosen date: \"%.2d-%.2d-%.4d\"",myDate.tm_mday,myDate.tm_mon+1,myDate.tm_year+1900);
#       else       //NO_IMGUIDATECHOOSER
        ImGui::Text("%s","Excluded from this build.\n");
#       endif      //NO_IMGUIDATECHOOSER

        // imguivariouscontrols
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguivariouscontrols");ImGui::Separator();
#       ifndef NO_IMGUIVARIOUSCONTROLS
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
        // ColorComboTest:
        static ImVec4 chosenColor2(1,1,1,1);
        if (ImGui::ColorCombo("MyColorCombo",&chosenColor2))
        {
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


        // Based on the code from: https://github.com/Roflraging
        ImGui::Spacing();
        ImGui::Text("InputTextMultiline with horizontal scrolling:");
        static char buffer[1024] = "Code posted by Roflraging to the ImGui Issue Section (https://github.com/ocornut/imgui/issues/383).";
        const float height = 60;
        ImGui::PushID(buffer);
        ImGui::InputTextMultilineWithHorizontalScrolling("ITMWHS", buffer, 1024, height);   // Note that now the label is not displayed ATM
        ImGui::PopID();

        ImGui::Spacing();
        ImGui::Text("Same as above with a context-menu that should work (more or less):");
        static char buffer2[1024] = "Code posted by Roflraging to the ImGui Issue Section (https://github.com/ocornut/imgui/issues/383).";
        ImGui::PushID(buffer2);
        static bool popup_open = false;static int threeStaticInts[3]={0,0,0};
        ImGui::InputTextMultilineWithHorizontalScrollingAndCopyCutPasteMenu("ITMWHS2", buffer2, 1024, height,popup_open,threeStaticInts);
        ImGui::PopID();

        ImGui::Spacing();
        ImGui::ImageButtonWithText(reinterpret_cast<ImTextureID>(myImageTextureId2),"MyImageButtonWithText",ImVec2(16,16),ImVec2(0,0),ImVec2(0.33334f,0.33334f));

#       ifndef NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE
        // One instance per image, but it can feed multiple widgets
        static ImGui::AnimatedImage gif(myImageTextureId2,64,64,9,3,3,30,true);
        ImGui::SameLine();
        gif.render();
        ImGui::SameLine();
        gif.renderAsButton("myButton123",ImVec2(-.5f,-.5f));    // Negative size multiplies the 'native' gif size
#       endif //NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE

        ImGui::Spacing();
        ImGui::Text("Image with zoom (CTRL+MW) and pan (RMB drag):");
        //ImGui::TextColored(halfTextColor,"(Problem: zooming interfence with GetIO().FontAllowUserScaling).");
        static float zoom = 1.f;static ImVec2 pan(.5f,.5f);
        // This requires     ImGui::GetIO().FontAllowUserScaling = false;
        ImGui::ImageZoomAndPan(reinterpret_cast<ImTextureID>(myImageTextureId2),ImVec2(0,150),1.f,zoom,pan);    // aspect ratio can be aero for stretch mode
#       else //NO_IMGUIVARIOUSCONTROLS
        ImGui::Text("%s","Excluded from this build.\n");
#       endif //NO_IMGUIVARIOUSCONTROLS

        // TabLabels Test:
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguitabwindow");ImGui::Separator();
#       ifndef NO_IMGUITABWINDOW
        // Based on the code by krys-spectralpixel (https://github.com/krys-spectralpixel), posted here: https://github.com/ocornut/imgui/issues/261
        ImGui::Spacing();
        ImGui::Text("TabLabels (based on the code by krys-spectralpixel):");
        static const char* tabNames[] = {"Render","Layers","Scene","World","Object","Constraints","Modifiers","Data","Material","Texture","Particle","Physics"};
        static const int numTabs = sizeof(tabNames)/sizeof(tabNames[0]);
        static const char* tabTooltips[numTabs] = {"Render Tab Tooltip","This tab cannot be closed","Scene Tab Tooltip","","Object Tab Tooltip","","","","","Tired to add tooltips...",""};
        static int tabItemOrdering[numTabs] = {0,1,2,3,4,5,6,7,8,9,10,11};
        static int selectedTab = 0;
        static int optionalHoveredTab = 0;
        static bool allowTabLabelDragAndDrop=true;static bool tabLabelWrapMode = true;static bool allowClosingTabs = true;
        int justClosedTabIndex=-1,justClosedTabIndexInsideTabItemOrdering = -1,oldSelectedTab = selectedTab;
        /*const bool tabSelectedChanged =*/ ImGui::TabLabels(numTabs,tabNames,selectedTab,tabTooltips,tabLabelWrapMode,&optionalHoveredTab,&tabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,&justClosedTabIndex,&justClosedTabIndexInsideTabItemOrdering);
        // Optional stuff
        if (justClosedTabIndex==1) {
            tabItemOrdering[justClosedTabIndexInsideTabItemOrdering] = justClosedTabIndex;   // Prevent the user from closing Tab "Layers"
            selectedTab = oldSelectedTab;   // This is safer, in case we had closed the selected tab
        }
        // Draw tab page
        ImGui::Spacing();ImGui::Text("Tab Page For Tab: \"%s\" here.",tabNames[selectedTab]);
        ImGui::Checkbox("Wrap Mode##TabLabelWrapMode",&tabLabelWrapMode);
        ImGui::SameLine();ImGui::Checkbox("Drag And Drop##TabLabelDragAndDrop",&allowTabLabelDragAndDrop);
        ImGui::SameLine();ImGui::Checkbox("Closable##TabLabelClosing",&allowClosingTabs);
        ImGui::SameLine();if (ImGui::SmallButton("Reset Tabs")) {for (int i=0;i<numTabs;i++) tabItemOrdering[i] = i;}
        //if (optionalHoveredTab>=0) ImGui::Text("Mouse is hovering Tab Label: \"%s\".\n\n",tabNames[optionalHoveredTab]);
#       else //NO_IMGUITABWINDOW
        ImGui::Text("%s","Excluded from this build.\n");
#       endif //NO_IMGUITABWINDOW

#       ifdef DOES_NOT_WORK
        // BadCodeEditor Test:
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguicodeeditor");ImGui::Separator();
#       ifndef NO_IMGUITABWINDOW
        ImGui::Spacing();
        ImGui::Text("ImGui::InputTextWithSyntaxHighlighting(...):");
        ImGui::TextColored(halfTextColor,"(Cursor doesn't work. I think I'll give up and remove it).");
        static const char* myCode="# include <sadd.h>\n\nusing namespace std;\n\n//This is a comment\nclass MyClass\n{\npublic:\nMyClass() {}\nvoid Init(int num)\n{  // for loop\nfor (int t=0;t<20;t++)\n	{\n     mNum=t; /* setting var */\n     const float myFloat = 1.25f;\n      break;\n	}\n}\n\nprivate:\nint mNum;\n};\n\nstatic const char* SomeStrings[] = {\"One\"/*Comment One*//*Comment*/,\"Two /*Fake Comment*/\",\"Three\\\"Four\"};\n\nwhile (i<25 && i>=0)   {\n\ti--;\nbreak;} /*comment*/{/*This should not fold*/}/*comment2*/for (int i=0;i<20;i++)    {\n\t\t\tcontinue;//OK\n} // end second folding\n\nfor (int j=0;j<200;j++)  {\ncontinue;}\n\n//region Custom Region Here\n{\n//something inside here\n}\n//endregion\n\n/*\nMultiline\nComment\nHere\n*/\n\n/*\nSome Unicode Characters here:\n€€€€\n*/\n\n";
        static char bceBuffer[1024]="";
        if (bceBuffer[0]=='\0') strcpy(bceBuffer,myCode);   //Bad init (use initGL() to fill the buffer
        ImGui::InputTextWithSyntaxHighlighting("ITWSH_JustForID",bceBuffer,sizeof(bceBuffer),ImGuiCe::LANG_CPP,ImVec2(0,300));
#       else //NO_IMGUICODEEDITOR
        ImGui::Text("%s","Excluded from this build.\n");
#       endif //NO_IMGUICODEEDITOR
#       endif //DOES_NOT_WORK

#       ifdef YES_IMGUISDF
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguisdf");ImGui::Separator();
        // Well, we should move the init stuff to InitGL(), clean up textures, etc. (all skipped to avoid multiple preprocessor branches around this file)
        static ImTextureID sdfTexture = 0;
        static ImGui::SdfTextChunk* sdfTextChunk = NULL;
        static char sdfTextBuffer[8192]="\t";
        static ImGui::SdfAnimation* sdfManualAnimation = NULL;  // Entirely optional (might confuse users)
        if (!sdfTextChunk) {
            static bool mustInit=true;
            if (mustInit)   {
                mustInit = false;
                //---------------------
                const char* sdfFontPath = "fonts/Sdf/DejaVuSerifCondensed-Bold.fnt";
                const char* sdfImagePath = "fonts/Sdf/DejaVuSerifCondensed-Bold.png";
                sdfTexture = ImImpl_LoadTexture(sdfImagePath,0,false,false,false);
                ImGui::SdfCharset* charset = ImGui::SdfAddCharsetFromFile(sdfFontPath,sdfTexture,ImGui::SdfCharsetProperties());
                IM_ASSERT(sdfTexture && charset);
                sdfTextChunk = ImGui::SdfAddTextChunk(charset,ImGui::SDF_BT_OUTLINE,ImGui::SdfTextChunkProperties(20,ImGui::SDF_CENTER,ImGui::SDF_MIDDLE,ImVec2(.5f,.5f),ImVec2(.5f,.5f)));
                IM_ASSERT(sdfTextChunk);

                //static const char* sdfSampleText = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
                static const char* sdfSampleText = "Lorem ipsum <COLOR=00FF00>dolor</COLOR> sit <I>amet</I>, consectetur <B>adipiscing</B> elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n<SCALE=2.0><HALIGN=LEFT>Left<HALIGN=CENTER>Center<HALIGN=RIGHT>Right</SCALE>";
                strcpy(sdfTextBuffer,sdfSampleText);
                ImGui::SdfAddTextWithTags(sdfTextChunk,sdfTextBuffer); // Actually we can append multiple of these calls together

                const bool enableOptionalStuff = true; // TO FIX
                if (enableOptionalStuff) {
                    sdfManualAnimation = ImGui::SdfAddAnimation();
                    ImGui::SdfTextChunkSetManualAnimation(sdfTextChunk,sdfManualAnimation); // This does not activate it by default
                }
            }
        }
        if (sdfTextChunk)   {
            static bool enableSdfTextChunk = false;
            ImGui::Checkbox("Enable SDF Test",&enableSdfTextChunk);
            if (enableSdfTextChunk) {
                if (ImGui::CollapsingHeader("SDF On Screen Text")) ImGui::SdfTextChunkEdit(sdfTextChunk,sdfTextBuffer,sizeof(sdfTextBuffer)/sizeof(sdfTextBuffer[0]));

                // To render all sdf text chunks:
                ImGui::SdfRender(); // This should be moved at the top of DrawGL(), to work when ImGui is not active too. [or after ImGui::Render(), but that spot is not supported at the moment here...]
            }
         }
#       endif //YES_IMGUISDF

#       ifdef YES_IMGUISOLOUD
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguisoloud");ImGui::Separator();
        // Well, we should move the init stuff to InitGL(), clean up textures, etc. (all skipped to avoid multiple preprocessor branches spread around this file)

        // Code dirty-copied from the SoLoud Welcome example (I'm a newbie...)
        static SoLoud::Soloud soloud; // Engine core

        static bool enableSoLoud = false;
        if (ImGui::Checkbox("Start SoLoud Test",&enableSoLoud))    {
            if (enableSoLoud)   {
                // Initialize SoLoud (automatic back-end selection)
                // also, enable visualization for FFT calc
                soloud.init();
                soloud.setVisualizationEnable(1);

                static SoLoud::WavStream ogg; // (SoLoud::Wav still works with ogg too)
                ogg.load("tetsno.ogg");       // Load an ogg file in streaming mode (however if I set it to loop, then it plays multiple times at the same time if I click the check box twice...)
                soloud.play(ogg);

                static SoLoud::Wav wav;                 // One sample source
                wav.load("AKWF_c604_0024.wav");       // Load a wave file
                //wav.setLooping(1);                          // Tell SoLoud to loop the sound
                /*int handle1 = */soloud.play(wav);             // Play it
                //soloud.setVolume(handle1, 0.5f);            // Set volume; 1.0f is "normal"
                //soloud.setPan(handle1, -0.2f);              // Set pan; -1 is left, 1 is right
                //soloud.setRelativePlaySpeed(handle1, 0.9f); // Play a bit slower; 1.0f is normal

                // extra sources need additional definitions (at the project level not here), unless you define YES_IMGUISOLOUD_ALL (a.t.p.l. not here).
                // For example:
#               ifdef YES_IMGUISOLOUD_SFXR
                static SoLoud::Sfxr sfxr;
                sfxr.loadPreset(SoLoud::Sfxr::COIN,0);
                soloud.play(sfxr);
#               endif //YES_IMGUISOLOUD_SFXR
#               ifdef YES_IMGUISOLOUD_SPEECH
                // Configure sound source
                static SoLoud::Speech speech; // A sound source (speech, in this case)
                speech.setText("Hello, world.");
                speech.setVolume(7);
                soloud.play(speech);    // Play the sound source (we could do this several times if we wanted)
#               endif //YES_IMGUISOLOUD_SPEECH


/*             // These should work too (AFAIK), but audio files are missing...
#               ifdef YES_IMGUISOLOUD_MODPLUG
                static SoLoud::Modplug mod;
                mod.load("audio/BRUCE.S3M");
                soloud.play(mod);
#               endif //YES_IMGUISOLOUD_MODPLUG
#               ifdef YES_IMGUISOLOUD_MONOTONE
                static SoLoud::Monotone mon;
                mon.load("audio/Jakim - Aboriginal Derivatives.mon");
                soloud.play(mon);
#               endif //YES_IMGUISOLOUD_MONOTONE
#               ifdef YES_IMGUISOLOUD_TEDSID
                static SoLoud::TedSid ted;
                ted.load("audio/ted_storm.prg.dump");
                soloud.play(ted);
                static SoLoud::TedSid sid;
                sid.load("audio/Modulation.sid.dump");
                soloud.play(sid);
#               endif //YES_IMGUISOLOUD_TEDSID
*/

            }
            else {
                soloud.stopAll();
                soloud.deinit();     // Does this clean all the sources too ?
            }
        }
        if (enableSoLoud && soloud.getVoiceCount()==0)  {
            soloud.deinit();    // Does this clean all the sources too ?
            enableSoLoud = false;
        }
#       endif //YES_IMGUISOLOUD

        // ListView Test:
        ImGui::Text("\n");ImGui::Separator();ImGui::Text("imguilistview");ImGui::Separator();
#       ifndef NO_IMGUILISTVIEW
        MyTestListView();
#       else //NO_IMGUILISTVIEW
        ImGui::Text("%s","Excluded from this build.\n");
#       endif //NO_IMGUILISTVIEW


        ImGui::Separator();

        ImGui::End();
    }

    // 2. Show another simple window, this time using an explicit Begin/End pair
#   ifndef NO_IMGUITOOLBAR
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window, ImVec2(500,100));
        {
            // imguitoolbar test (note that it can be used both inside and outside windows (see below)
            ImGui::Separator();ImGui::Text("imguitoolbar");ImGui::Separator();
            static ImGui::Toolbar toolbar;
            void* myImageTextureIdVoitPtr = reinterpret_cast<void*>(myImageTextureId2);
            if (toolbar.getNumButtons()==0)  {
                char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                for (int i=0;i<9;i++) {
                    strcpy(tmp,"toolbutton ");
                    sprintf(&tmp[strlen(tmp)],"%d",i+1);
                    uv0 = ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                    uv1 = ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);

                    toolbar.addButton(ImGui::Toolbutton(tmp,myImageTextureIdVoitPtr,uv0,uv1,ImVec2(16,16)));
                }
                toolbar.addSeparator(16);
                toolbar.addButton(ImGui::Toolbutton("toolbutton 11",myImageTextureIdVoitPtr,uv0,uv1,ImVec2(16,16),true,true,ImVec4(0.8,0.8,1.0,1)));  // Note that separator "eats" one toolbutton index as if it was a real button
                toolbar.addButton(ImGui::Toolbutton("toolbutton 12",myImageTextureIdVoitPtr,uv0,uv1,ImVec2(16,16),true,false,ImVec4(1.0,0.8,0.8,1)));  // Note that separator "eats" one toolbutton index as if it was a real button

                toolbar.setProperties(true,false,false,ImVec2(0.0f,0.f),ImVec2(0.25,1));
            }
            const int pressed = toolbar.render();
            if (pressed>=0) fprintf(stderr,"Toolbar1: pressed:%d\n",pressed);
        }
        // Here we can open a child window if we want to toolbar not to scroll
        ImGui::Spacing();ImGui::Text("imguitoolbar can be used inside windows too.\nThe first series of buttons can be used as a tab control.\nPlease resize the window and see the dynamic layout.\n");
        ImGui::End();
    }
#   endif //NO_IMGUITOOLBAR

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
#   if (!defined(NO_IMGUISTYLESERIALIZER) && !defined(NO_IMGUISTYLESERIALIZER_SAVE_STYLE))
    if (show_test_window)
    {
        //ImGui::SetNewWindowDefaultPos(ImVec2(650, 20));        // Normally user code doesn't need/want to call this, because positions are saved in .ini file. Here we just want to make the demo initial state a bit more friendly!
        ImGui::ShowTestWindow(&show_test_window);
    }
#   endif // NO_IMGUISTYLESERIALIZER
#   ifndef NO_IMGUINODEGRAPHEDITOR
    if (show_node_graph_editor_window) {
        ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiSetCond_FirstUseEver);
        if (ImGui::Begin("Example: Custom Node Graph", &show_node_graph_editor_window)){
#           ifndef IMGUINODEGRAPHEDITOR_NOTESTDEMO
            ImGui::TestNodeGraphEditor();   // see its code for further info
#           endif //IMGUINODEGRAPHEDITOR_NOTESTDEMO
            ImGui::End();
        }
    }
#   endif //NO_IMGUINODEGRAPHEDITOR
    if (show_splitter_test_window)  {
        // based on a snippet by Omar
        static ImVec2 lastWindowSize(500,500);      // initial window size
        static const float splitterWidth = 6.f;
        static float w = 200.0f;                    // initial size of the top/left window
        static float h = 300.0f;

        ImGui::Begin("Splitter test",&show_splitter_test_window,lastWindowSize);//,-1.f,ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const bool sizeChanged = lastWindowSize.x!=windowSize.x || lastWindowSize.y!=windowSize.y;
        if (sizeChanged) lastWindowSize = windowSize;
        bool splitterActive = false;
        const ImVec2 os(ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().WindowPadding.x,
                        ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().WindowPadding.y);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));

        // window top left
        ImGui::BeginChild("child1", ImVec2(w, h), true);
        ImGui::EndChild();
        // horizontal splitter
        ImGui::SameLine();
        ImGui::InvisibleButton("hsplitter", ImVec2(splitterWidth,h));
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        splitterActive = ImGui::IsItemActive();
        if (splitterActive)  w += ImGui::GetIO().MouseDelta.x;
        if (splitterActive || sizeChanged)  {
            const float minw = ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().FramePadding.x;
            const float maxw = minw + windowSize.x - splitterWidth - ImGui::GetStyle().WindowMinSize.x;
            if (w>maxw)         w = maxw;
            else if (w<minw)    w = minw;
        }
        ImGui::SameLine();
        // window top right
        ImGui::BeginChild("child2", ImVec2(0, h), true);
        ImGui::EndChild();
        // vertical splitter
        ImGui::InvisibleButton("vsplitter", ImVec2(-1,splitterWidth));
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        splitterActive = ImGui::IsItemActive();
        if (splitterActive)  h += ImGui::GetIO().MouseDelta.y;
        if (splitterActive || sizeChanged)  {
            const float minh = ImGui::GetStyle().WindowPadding.y + ImGui::GetStyle().FramePadding.y;
            const float maxh = windowSize.y - splitterWidth - ImGui::GetStyle().WindowMinSize.y;
            if (h>maxh)         h = maxh;
            else if (h<minh)    h = minh;
        }
        // window bottom
        ImGui::BeginChild("child3", ImVec2(0,0), true);
        ImGui::EndChild();

        ImGui::PopStyleVar();

        ImGui::End();
    }
#   ifdef YES_IMGUIDOCK
    if (show_dock_window)   {
        // I don't know how to use it, I'm a little dumb here... I probably have to create tabs somehow...
        // or maybe it's supposed to be used on its own, without other ImGui "normal" windows...

        ImGui::BeginDock("MyFirstDockedWindow",NULL,0);
        ImGui::Text("LumixEngine Dock system test window 1.");
        ImGui::EndDock();

        ImGui::BeginDock("MySecondDockedWindow",NULL,0);
        ImGui::Text("LumixEngine Dock system test window 2.");
        ImGui::EndDock();

    }
#   endif //YES_IMGUIDOCK
#   ifndef NO_IMGUITABWINDOW
    if (show_tab_windows)   {
        static ImGui::TabWindow tabWindows[2];  // Note: there's more than this: there are methods to save/load all TabWindows together, but we don't use them here. Also we don't use "custom callbacks", "TabLabel modified states" and TabLabel context-menus here to keep things simple.
        static bool showTabWindow[2] = {true,true};

        if (showTabWindow[0] && ImGui::Begin("TabWindow1", &showTabWindow[0], ImVec2(400,600),.85f,ImGuiWindowFlags_NoScrollbar))  {
            ImGui::TabWindow&  tabWindow = tabWindows[0];
            if (!tabWindow.isInited()) {
                static const char* tabNames[] = {"Test","Render","Layers","Scene","World","Object","Constraints","Modifiers","Data","Material","Texture","Particle","Physics"};
                static const int numTabs = sizeof(tabNames)/sizeof(tabNames[0]);
                static const char* tabTooltips[numTabs] = {"Test Tab Tooltip","Render Tab Tooltip","Layers Tab Tooltip","Scene Tab Tooltip","This tab cannot be dragged around","Object Tab Tooltip","","","","This tab cannot be dragged around","Tired to add tooltips...",""};
                for (int i=0;i<numTabs;i++) {
                    tabWindow.addTabLabel(tabNames[i],tabTooltips[i],i%3!=0,i%5!=4);
                }
            }
            tabWindow.render(); // Must be inside a window
            ImGui::End();
        }

        if (showTabWindow[1] && ImGui::Begin("TabWindow2", &showTabWindow[1], ImVec2(400,600),.85f,ImGuiWindowFlags_NoScrollbar))  {
            ImGui::TabWindow&  tabWindow2 = tabWindows[1];
            tabWindow2.render();
            ImGui::End();
        }

        if (!showTabWindow[0] && !showTabWindow[1]) {
            // reset flags
            showTabWindow[0] = showTabWindow[1] = true;
            show_tab_windows = false;
        }
    }
#   endif //NO_IMGUITABWINDOW

    // imguitoolbar test 2: two global toolbars one at the top and one at the left
#   ifndef NO_IMGUITOOLBAR
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

#   ifndef NO_IMGUIFILESYSTEM
    // Really tiny visual optimization here:
    ImGuiFs::Dialog::WindowLTRBOffsets.x = 24;   // save some pixels from left-right screen borders
    ImGuiFs::Dialog::WindowLTRBOffsets.y = gMainMenuBarSize.y + 32; // save some pixels from top-bottom screen borders
#   endif //NO_IMGUIFILESYSTEM

#   endif //NO_IMGUITOOLBAR
}





//#   define USE_ADVANCED_SETUP   // in-file definition (see below). For now it just adds custom fonts and different FPS settings (please read below).

// Application code
#ifndef IMGUI_USE_AUTO_BINDING_WINDOWS  // IMGUI_USE_AUTO_ definitions get defined automatically (e.g. do NOT touch them!)
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
                                  //-40.f; // If < 0, it's the number of lines that fit the whole screen (but without any kind of vertical spacing)


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
    &ranges[0],
    NULL,                                                               // optional ImFontConfig* (useful for merging glyph to the default font, according to ImGui)
    false                                                               // true = addDefaultImGuiFontAsFontZero
    ); // If you need to add more than one TTF file, there's another ctr (TO TEST).
    // Here are some optional tweaking of the desired FPS settings (they can be changed at runtime if necessary, but through some global values defined in imguibindinds.h)
    gImGuiInitParams.gFpsClampInsideImGui = 30.0f;  // Optional Max allowed FPS (!=0, default -1 => unclamped). Useful for editors and to save GPU and CPU power.
    gImGuiInitParams.gFpsDynamicInsideImGui = true; // If true when inside ImGui, the FPS is not constant (at gFpsClampInsideImGui), but goes from a very low minimum value to gFpsClampInsideImGui dynamically. Useful for editors and to save GPU and CPU power.
    gImGuiInitParams.gFpsClampOutsideImGui = 10.f;  // Optional Max allowed FPS (!=0, default -1 => unclamped). Useful for setting a different FPS for your main rendering.

    ImImpl_Main(&gImGuiInitParams,argc,argv);
#   endif //USE_ADVANCED_SETUP

	return 0;
}
#else //IMGUI_USE_AUTO_BINDING_WINDOWS
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
    gImGuiInitParams.gFpsClampInsideImGui = 30.0f;  // Optional Max allowed FPS (default -1 => unclamped). Useful for editors and to save GPU and CPU power.
    gImGuiInitParams.gFpsDynamicInsideImGui = true; // If true when inside ImGui, the FPS is not constant (at gFpsClampInsideImGui), but goes from a very low minimum value to gFpsClampInsideImGui dynamically. Useful for editors and to save GPU and CPU power.
    gImGuiInitParams.gFpsClampOutsideImGui = 10.f;  // Optional Max allowed FPS (default -1 => unclamped). Useful for setting a different FPS for your main rendering.

    ImImpl_WinMain(&gImGuiInitParams,hInstance,hPrevInstance,lpCmdLine,iCmdShow);
#   endif //#   USE_ADVANCED_SETUP

    return 0;
}
#endif //IMGUI_USE_AUTO_BINDING_WINDOWS



