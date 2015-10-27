#include "imguivariouscontrols.h"

namespace ImGui {
static float GetWindowFontScale() {
    //ImGuiState& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    return window->FontWindowScale;
}


float ProgressBar(const char *optionalPrefixText, float value, const float minValue, const float maxValue, const char *format, const ImVec2 &sizeOfBarWithoutTextInPixels, const ImVec4 &colorLeft, const ImVec4 &colorRight, const ImVec4 &colorBorder)    {
    if (value<minValue) value=minValue;
    else if (value>maxValue) value = maxValue;
    const float valueFraction = (maxValue==minValue) ? 1.0f : ((value-minValue)/(maxValue-minValue));
    const bool needsPercConversion = strstr(format,"%%")!=NULL;

    ImVec2 size = sizeOfBarWithoutTextInPixels;
    if (size.x<=0) size.x = ImGui::GetWindowWidth()*0.25f;
    if (size.y<=0) size.y = ImGui::GetTextLineHeightWithSpacing(); // or without

    const ImFontAtlas* fontAtlas = ImGui::GetIO().Fonts;

    if (optionalPrefixText && strlen(optionalPrefixText)>0) {
        ImGui::AlignFirstTextHeightToWidgets();
        ImGui::Text("%s",optionalPrefixText);
        ImGui::SameLine();
    }

    if (valueFraction>0)   {
        ImGui::Image(fontAtlas->TexID,ImVec2(size.x*valueFraction,size.y), fontAtlas->TexUvWhitePixel,fontAtlas->TexUvWhitePixel,colorLeft,colorBorder);
    }
    if (valueFraction<1)   {
        if (valueFraction>0) ImGui::SameLine(0,0);
        ImGui::Image(fontAtlas->TexID,ImVec2(size.x*(1.f-valueFraction),size.y), fontAtlas->TexUvWhitePixel,fontAtlas->TexUvWhitePixel,colorRight,colorBorder);
    }
    ImGui::SameLine();

    ImGui::Text(format,needsPercConversion ? (valueFraction*100.f+0.0001f) : value);
    return valueFraction;
}

void TestProgressBar()  {    
    const float time = ((float)(((unsigned int) (ImGui::GetTime()*1000.f))%50000)-25000.f)/25000.f;
    float progress=(time>0?time:-time);
    // No IDs needed for ProgressBars:
    ImGui::ProgressBar("ProgressBar",progress);
    ImGui::ProgressBar("ProgressBar",1.f-progress);
    ImGui::ProgressBar("",500+progress*1000,500,1500,"%4.0f (absolute value in [500,1500] and fixed bar size)",ImVec2(150,-1));
    ImGui::ProgressBar("",500+progress*1000,500,1500,"%3.0f%% (same as above, but with percentage and new colors)",ImVec2(150,-1),ImVec4(0.7,0.7,1,1),ImVec4(0.05,0.15,0.5,0.8),ImVec4(0.8,0.8,0,1));
}


int PopupMenuSimple(bool &open, const char **pEntries, int numEntries, const char *optionalTitle, int *pOptionalHoveredEntryOut, int startIndex, int endIndex, bool reverseItems, const char *scrollUpEntryText, const char *scrollDownEntryText)   {
    int selectedEntry = -1;
    if (pOptionalHoveredEntryOut) *pOptionalHoveredEntryOut=-1;
    if (!open) return selectedEntry;
    if (numEntries==0 || !pEntries) {
        open = false;
        return selectedEntry;
    }

    float fs = 1.f;
#   ifdef IMGUI_INCLUDE_IMGUI_USER_INL
    fs = ImGui::GetWindowFontScale();   // Internal to <imgui.cpp>
#   endif //   IMGUI_INCLUDE_IMGUI_USER_INL

    if (!open) return selectedEntry;
    ImGui::PushID(&open);   // or pEntries ??
    //ImGui::BeginPopup(&open);
    ImGui::OpenPopup("MyOwnPopupSimpleMenu");
    if (ImGui::BeginPopup("MyOwnPopupSimpleMenu"))  {
        if (optionalTitle) {ImGui::Text("%s",optionalTitle);ImGui::Separator();}
        if (startIndex<0) startIndex=0;
        if (endIndex<0) endIndex = numEntries-1;
        if (endIndex>=numEntries) endIndex = numEntries-1;
        const bool needsScrolling = (endIndex-startIndex+1)<numEntries;
        if (scrollUpEntryText && needsScrolling) {
            ImGui::SetWindowFontScale(fs*0.75f);
            if (reverseItems ? (endIndex+1<numEntries) : (startIndex>0))    {
                const int entryIndex = reverseItems ? -3 : -2;
                if (ImGui::Selectable(scrollUpEntryText, false))  {
                    selectedEntry = entryIndex;//open = false;    // Hide menu
                }
                else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = entryIndex;
            }
            else ImGui::Text(" ");
            ImGui::SetWindowFontScale(fs);
        }
        if (!reverseItems)  {
            for (int i = startIndex; i <= endIndex; i++)    {
                const char* entry = pEntries[i];
                if (!entry || strlen(entry)==0) ImGui::Separator();
                else {
                    if (ImGui::Selectable(entry, false))  {
                        selectedEntry = i;open = false;    // Hide menu
                    }
                    else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = i;
                }
            }
        }
        else {
            for (int i = endIndex; i >= startIndex; i--)    {
                const char* entry = pEntries[i];
                if (!entry || strlen(entry)==0) ImGui::Separator();
                else {
                    if (ImGui::Selectable(entry, false))  {
                        selectedEntry = i;open = false;    // Hide menu
                    }
                    else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = i;
                }

            }
        }
        if (scrollDownEntryText && needsScrolling) {
            const float fs = ImGui::GetWindowFontScale();      // Internal to <imgui.cpp>
            ImGui::SetWindowFontScale(fs*0.75f);
            if (reverseItems ? (startIndex>0) : (endIndex+1<numEntries))    {
                const int entryIndex = reverseItems ? -2 : -3;
                if (ImGui::Selectable(scrollDownEntryText, false))  {
                    selectedEntry = entryIndex;//open = false;    // Hide menu
                }
                else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = entryIndex;
            }
            else ImGui::Text(" ");
            ImGui::SetWindowFontScale(fs);
        }
        if (open)   // close menu when mouse goes away
        {
            const float d = 10;
            ImVec2 pos = ImGui::GetWindowPos();pos.x-=d;pos.y-=d;
            ImVec2 size = ImGui::GetWindowSize();size.x+=2.f*d;size.y+=2.f*d;
            const ImVec2& mousePos = ImGui::GetIO().MousePos;
            if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) open = false;
        }
    }
    ImGui::EndPopup();
    ImGui::PopID();

    return selectedEntry;    
}

int PopupMenuSimpleCopyCutPasteOnLastItem(bool readOnly) {
    static bool open = false;
    static const char* entries[] = {"Copy","Cut","","Paste"};   // "" is separator
    //open|=ImGui::Button("Show Popup Menu Simple");                    // BUTTON
    open|= ImGui::GetIO().MouseClicked[1] && ImGui::IsItemHovered(); // RIGHT CLICK
    int selectedEntry = PopupMenuSimple(open,entries,readOnly?1:4);
    if (selectedEntry>2) selectedEntry = 2; // Normally separator takes out one space
    return selectedEntry;
    // About "open": when user exits popup-menu, "open" becomes "false". Please set it to "true" to display it again (we do it using open|=[...])
}


int PopupMenuSimple(PopupMenuSimpleParams &params, const char **pTotalEntries, int numTotalEntries, int numAllowedEntries, bool reverseItems, const char *optionalTitle, const char *scrollUpEntryText, const char *scrollDownEntryText)    {
    if (numAllowedEntries<1 || numTotalEntries==0) {params.open=false;return -1;}
    if (params.endIndex==-1) params.endIndex=reverseItems ? numTotalEntries-1 : numAllowedEntries-1;
    if (params.startIndex==-1) params.startIndex=params.endIndex-numAllowedEntries+1;

    const int oldHoveredEntry = params.hoveredEntry;
    params.selectedEntry = PopupMenuSimple(params.open,pTotalEntries,numTotalEntries,optionalTitle,&params.hoveredEntry,params.startIndex,params.endIndex,reverseItems,scrollUpEntryText,scrollDownEntryText);

    if (params.hoveredEntry<=-2 || params.selectedEntry<=-2)   {
        if (oldHoveredEntry!=params.hoveredEntry) params.scrollTimer = ImGui::GetTime();
        const float newTime = ImGui::GetTime();
        if (params.selectedEntry<=-2 || (newTime - params.scrollTimer > 0.4f))    {
            params.scrollTimer = newTime;
            if (params.hoveredEntry==-2 || params.selectedEntry==-2)   {if (params.startIndex>0) {--params.startIndex;--params.endIndex;}}
            else if (params.hoveredEntry==-3 || params.selectedEntry==-3) {if (params.endIndex<numTotalEntries-1) {++params.startIndex;++params.endIndex;}}

        }
    }
    if (!params.open && params.resetScrollingWhenRestart) {
        params.endIndex=reverseItems ? numTotalEntries-1 : numAllowedEntries-1;
        params.startIndex=params.endIndex-numAllowedEntries+1;
    }
    return params.selectedEntry;
}

void TestPopupMenuSimple(const char *scrollUpEntryText, const char *scrollDownEntryText) {
    // Recent Files-like menu
    static const char* recentFileList[] = {"filename01","filename02","filename03","filename04","filename05","filename06","filename07","filename08","filename09","filename10"};

    static PopupMenuSimpleParams pmsParams;
    pmsParams.open|= ImGui::GetIO().MouseClicked[1];// RIGHT CLICK
    const int selectedEntry = PopupMenuSimple(pmsParams,recentFileList,(int) sizeof(recentFileList)/sizeof(recentFileList[0]),5,true,"RECENT FILES",scrollUpEntryText,scrollDownEntryText);
    if (selectedEntry>=0) {
        // Do something: clicked entries[selectedEntry]
    }
}

inline static void ClampColor(ImVec4& color)    {
    float* pf;
    pf = &color.x;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.y;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.z;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.w;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
}

// Based on the code from: https://github.com/benoitjacquier/imgui
bool ColorChooser(bool* open,ImVec4 *pColorOut,bool supportsAlpha)   {
    static bool lastOpen = false;
    static const ImVec2 windowSize(175,285);

    if (open && !*open) {lastOpen=false;return false;}
    if (open && *open && *open!=lastOpen) {
        ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
        ImGui::SetNextWindowSize(windowSize);
        lastOpen=*open;
    }

    //ImGui::OpenPopup("Color Chooser##myColorChoserPrivate");

    bool colorSelected = false;

    ImGuiWindowFlags WindowFlags = 0;
    //WindowFlags |= ImGuiWindowFlags_NoTitleBar;
    WindowFlags |= ImGuiWindowFlags_NoResize;
    //WindowFlags |= ImGuiWindowFlags_NoMove;
    WindowFlags |= ImGuiWindowFlags_NoScrollbar;
    WindowFlags |= ImGuiWindowFlags_NoCollapse;
    WindowFlags |= ImGuiWindowFlags_NoScrollWithMouse;

    //if (ImGui::BeginPopupModal("Color Chooser##myColorChoserPrivate",open,WindowFlags))
    if (ImGui::Begin("Color Chooser##myColorChoserPrivate",open,windowSize,-1.f,WindowFlags))
    {
        ImVec4 color = pColorOut ? *pColorOut : ImVec4(0,0,0,1);
        if (!supportsAlpha) color.w=1.f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);


            ImGuiState& g = *GImGui;
            const int windowWidth = 180;
            const int smallWidth = 20;

            static const ImU32 black = ColorConvertFloat4ToU32(ImVec4(0,0,0,1));
            static const ImU32 white = ColorConvertFloat4ToU32(ImVec4(1,1,1,1));
            static float hue, sat, val;

            ImGui::ColorConvertRGBtoHSV( color.x, color.y, color.z, hue, sat, val );


            ImGuiWindow* colorWindow = GetCurrentWindow();

        const int quadSize = windowWidth - smallWidth - colorWindow->WindowPadding.x * 2 - g.Style.ItemSpacing.x;
        // Hue Saturation Value
        if (ImGui::BeginChild("ValueSaturationQuad", ImVec2(quadSize, quadSize), false ))   {
            const int step = 5;
            ImVec2 pos = ImVec2(0, 0);
            ImGuiWindow* window = GetCurrentWindow();

            ImVec4 c00(1, 1, 1, 1);
            ImVec4 c10(1, 1, 1, 1);
            ImVec4 c01(1, 1, 1, 1);
            ImVec4 c11(1, 1, 1, 1);
            for (int y = 0; y < step; y++) {
                for (int x = 0; x < step; x++) {
                    float s0 = (float)x / (float)step;
                    float s1 = (float)(x + 1) / (float)step;
                    float v0 = 1.0 - (float)(y) / (float)step;
                    float v1 = 1.0 - (float)(y + 1) / (float)step;


                    ImGui::ColorConvertHSVtoRGB(hue, s0, v0, c00.x, c00.y, c00.z);
                    ImGui::ColorConvertHSVtoRGB(hue, s1, v0, c10.x, c10.y, c10.z);
                    ImGui::ColorConvertHSVtoRGB(hue, s0, v1, c01.x, c01.y, c01.z);
                    ImGui::ColorConvertHSVtoRGB(hue, s1, v1, c11.x, c11.y, c11.z);

                    window->DrawList->AddRectFilledMultiColor(window->Pos + pos, window->Pos + pos + ImVec2(quadSize / step, quadSize / step),
                        ImGui::ColorConvertFloat4ToU32(c00),
                        ImGui::ColorConvertFloat4ToU32(c10),
                        ImGui::ColorConvertFloat4ToU32(c11),
                        ImGui::ColorConvertFloat4ToU32(c01));

                    pos.x += quadSize / step;
                }
                pos.x = 0;
                pos.y += quadSize / step;
            }

            window->DrawList->AddCircle(window->Pos + ImVec2(sat, 1-val)*quadSize, 4, val<0.5f?white:black, 4);

            const ImGuiID id = window->GetID("ValueSaturationQuad");
            ImRect bb(window->Pos, window->Pos + window->Size);
            bool hovered, held;
            /*bool pressed = */ImGui::ButtonBehavior(bb, id, &hovered, &held, false, false);
            if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
            if (held)   {
                ImVec2 pos = g.IO.MousePos - window->Pos;
                sat = ImSaturate(pos.x / (float)quadSize);
                val = 1 - ImSaturate(pos.y / (float)quadSize);
                ImGui::ColorConvertHSVtoRGB(hue, sat, val, color.x, color.y, color.z);
                colorSelected = true;
            }
        ImGui::EndChild();	// ValueSaturationQuad
        }

        ImGui::SameLine();

        //Vertical tint
        if (ImGui::BeginChild("Tint", ImVec2(20, quadSize), false))  {
            const int step = 8;
            const int width = 20;
            ImGuiWindow* window = GetCurrentWindow();
            ImVec2 pos(0, 0);
            ImVec4 c0(1, 1, 1, 1);
            ImVec4 c1(1, 1, 1, 1);
            for (int y = 0; y < step; y++) {
                float tint0 = (float)(y) / (float)step;
                float tint1 = (float)(y + 1) / (float)step;
                ImGui::ColorConvertHSVtoRGB(tint0, 1.0, 1.0, c0.x, c0.y, c0.z);
                ImGui::ColorConvertHSVtoRGB(tint1, 1.0, 1.0, c1.x, c1.y, c1.z);

                window->DrawList->AddRectFilledMultiColor(window->Pos + pos, window->Pos + pos + ImVec2(width, quadSize / step),
                    ColorConvertFloat4ToU32(c0),
                    ColorConvertFloat4ToU32(c0),
                    ColorConvertFloat4ToU32(c1),
                    ColorConvertFloat4ToU32(c1));

                pos.y += quadSize / step;
            }

            window->DrawList->AddCircle(window->Pos + ImVec2(10, hue*quadSize), 4, black, 4);
            //window->DrawList->AddLine(window->Pos + ImVec2(0, hue*quadSize), window->Pos + ImVec2(width, hue*quadSize), ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)));
            bool hovered, held;
            const ImGuiID id = window->GetID("Tint");
            ImRect bb(window->Pos, window->Pos + window->Size);
            /*bool pressed = */ButtonBehavior(bb, id, &hovered, &held, false, false);
            if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
            if (held)
            {

                ImVec2 pos = g.IO.MousePos - window->Pos;
                hue = ImClamp( pos.y / (float)quadSize, 0.0f, 1.0f );                
                ImGui::ColorConvertHSVtoRGB( hue, sat, val, color.x, color.y, color.z );
                colorSelected = true;
            }
            ImGui::EndChild(); // "Tint"
        }


        //Sliders
        ImGui::AlignFirstTextHeightToWidgets();
        ImGui::Text("Sliders");
        static bool useHsvSliders = false;
        static const char* btnNames[2] = {"to HSV","to RGB"};
        const int index = useHsvSliders?1:0;
        ImGui::SameLine();
        if (ImGui::SmallButton(btnNames[index])) useHsvSliders=!useHsvSliders;

        ImGui::Separator();

        {
            int r = ImSaturate( useHsvSliders ? hue : color.x )*255.f;
            int g = ImSaturate( useHsvSliders ? sat : color.y )*255.f;
            int b = ImSaturate( useHsvSliders ? val : color.z )*255.f;
            int a = ImSaturate( color.w )*255.f;

            static const char* names[2][3]={{"R","G","B"},{"H","S","V"}};
            bool sliderMoved = false;
            sliderMoved|= ImGui::SliderInt(names[index][0], &r, 0, 255);
            sliderMoved|= ImGui::SliderInt(names[index][1], &g, 0, 255);
            sliderMoved|= ImGui::SliderInt(names[index][2], &b, 0, 255);
            sliderMoved|= (supportsAlpha && ImGui::SliderInt("A", &a, 0, 255));
            if (sliderMoved)
            {
                colorSelected = true;
                color.x = (float)r/255.f;
                color.y = (float)g/255.f;
                color.z = (float)b/255.f;
                if (useHsvSliders)  ImGui::ColorConvertHSVtoRGB(color.x,color.y,color.z,color.x,color.y,color.z);
                if (supportsAlpha) color.w = (float)a/255.f;
            }
            //ColorConvertRGBtoHSV(s_color.x, s_color.y, s_color.z, tint, sat, val);*/
        }


        ImGui::PopStyleVar();

        //ImGui::EndPopup();
        ImGui::End();

        if (colorSelected && pColorOut) *pColorOut = color;

    }

    return colorSelected;

}


/* // Snippet by Omar. To evalutate. But in main.cpp thare's another example that supports correct window resizing.
 * // And here is not straightforward to determine the sizes we want to use...
//#include <imgui_internal.h>
static void DrawSplitter(float& size0, float& size1,const char* label="##Splitter",int split_vertically=true,float min_size0=20.f, float min_size1=20.f,float thickness=8.f)
{
    ImVec2 backup_pos = ImGui::GetCursorPos();
    if (split_vertically) ImGui::SetCursorPosY(backup_pos.y + size0);
    else ImGui::SetCursorPosX(backup_pos.x + size0);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0));          // We don't draw while active/pressed because as we move the panes the splitter button will be 1 frame late
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f,0.6f,0.6f,0.10f));
    ImGui::Button(label, ImVec2(!split_vertically ? thickness : -1.0f, split_vertically ? thickness : -1.0f));
    ImGui::PopStyleColor(3);


    // This is to allow having other buttons OVER our splitter. All of this should be moved to ImGui
    // GImGui can be accessed with: #include <imgui_internal.h>
    if (GImGui->HoveredId == GImGui->CurrentWindow->DC.LastItemID)
        GImGui->HoveredIdAllowHoveringOthers = true;
    if (GImGui->ActiveId == GImGui->CurrentWindow->DC.LastItemID)
        GImGui->ActiveIdAllowHoveringOthers = true;

    if (ImGui::IsItemActive())
    {
        ImGui::SetMouseCursor(split_vertically ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
        float mouse_delta = split_vertically ? ImGui::GetIO().MouseDelta.y : ImGui::GetIO().MouseDelta.x;

        // Minimum pane size
        if (mouse_delta < min_size0 - size0)   mouse_delta = min_size0 - size0;
        if (mouse_delta > size1 - min_size1)   mouse_delta = size1 - min_size1;

        // Apply resize
        size0 += mouse_delta;
        size1 -= mouse_delta;
    }
    ImGui::SetCursorPos(backup_pos);
}
*/


} // namespace ImGui
