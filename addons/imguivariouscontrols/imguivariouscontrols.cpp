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
inline static bool ColorChooserInternal(ImVec4 *pColorOut,bool supportsAlpha,bool showSliders,ImGuiWindowFlags extra_flags=0,bool* pisAnyItemActive=NULL,float windowWidth = 180)    {
    bool colorSelected = false;
    if (pisAnyItemActive) *pisAnyItemActive=false;
    const bool isCombo = (extra_flags&ImGuiWindowFlags_ComboBox);

    ImVec4 color = pColorOut ? *pColorOut : ImVec4(0,0,0,1);
    if (!supportsAlpha) color.w=1.f;

    ImGuiState& g = *GImGui;
    const float smallWidth = windowWidth/9.f;

    static const ImU32 black = ColorConvertFloat4ToU32(ImVec4(0,0,0,1));
    static const ImU32 white = ColorConvertFloat4ToU32(ImVec4(1,1,1,1));
    static float hue, sat, val;

    ImGui::ColorConvertRGBtoHSV( color.x, color.y, color.z, hue, sat, val );


    ImGuiWindow* colorWindow = GetCurrentWindow();

    const float quadSize = windowWidth - smallWidth - colorWindow->WindowPadding.x*2;
    if (isCombo) ImGui::SetCursorPosX(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x);
    // Hue Saturation Value
    if (ImGui::BeginChild("ValueSaturationQuad##ValueSaturationQuadColorChooser", ImVec2(quadSize, quadSize), false,extra_flags ))
    //ImGui::BeginGroup();
    {
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
        /*bool pressed = */ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_NoKeyModifiers);///*false,*/ false);
        if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
        if (held)   {
            ImVec2 pos = g.IO.MousePos - window->Pos;
            sat = ImSaturate(pos.x / (float)quadSize);
            val = 1 - ImSaturate(pos.y / (float)quadSize);
            ImGui::ColorConvertHSVtoRGB(hue, sat, val, color.x, color.y, color.z);
            colorSelected = true;
        }

    }
    ImGui::EndChild();	// ValueSaturationQuad
    //ImGui::EndGroup();

    ImGui::SameLine();

    if (isCombo) ImGui::SetCursorPosX(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x+quadSize);

    //Vertical tint
    if (ImGui::BeginChild("Tint##TintColorChooser", ImVec2(smallWidth, quadSize), false,extra_flags))
    //ImGui::BeginGroup();
    {
        const int step = 8;
        const int width = (int)smallWidth;
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

        window->DrawList->AddCircle(window->Pos + ImVec2(smallWidth*0.5f, hue*quadSize), 4, black, 4);
        //window->DrawList->AddLine(window->Pos + ImVec2(0, hue*quadSize), window->Pos + ImVec2(width, hue*quadSize), ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)));
        bool hovered, held;
        const ImGuiID id = window->GetID("Tint");
        ImRect bb(window->Pos, window->Pos + window->Size);
        /*bool pressed = */ButtonBehavior(bb, id, &hovered, &held,ImGuiButtonFlags_NoKeyModifiers);// /*false,*/ false);
        if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
        if (held)
        {

            ImVec2 pos = g.IO.MousePos - window->Pos;
            hue = ImClamp( pos.y / (float)quadSize, 0.0f, 1.0f );
            ImGui::ColorConvertHSVtoRGB( hue, sat, val, color.x, color.y, color.z );
            colorSelected = true;
        }

    }
    ImGui::EndChild(); // "Tint"
    //ImGui::EndGroup();

    if (showSliders)
    {
        //Sliders
        //ImGui::PushItemHeight();
        if (isCombo) ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x,ImGui::GetCursorPos().y+colorWindow->WindowPadding.y+quadSize));
        ImGui::AlignFirstTextHeightToWidgets();
        ImGui::Text("Sliders");
        static bool useHsvSliders = false;
        static const char* btnNames[2] = {"to HSV","to RGB"};
        const int index = useHsvSliders?1:0;
        ImGui::SameLine();
        if (ImGui::SmallButton(btnNames[index])) useHsvSliders=!useHsvSliders;

        ImGui::Separator();
        const ImVec2 sliderSize = isCombo ? ImVec2(-1,quadSize) : ImVec2(-1,-1);
        if (ImGui::BeginChild("Sliders##SliderColorChooser", sliderSize, false,extra_flags))
        {


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
                if (pisAnyItemActive) *pisAnyItemActive|=sliderMoved;
            }


        }
        ImGui::EndChild();
    }

    if (colorSelected && pColorOut) *pColorOut = color;

    return colorSelected;
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

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,2));

    if (open) ImGui::SetNextWindowFocus();
    //if (ImGui::BeginPopupModal("Color Chooser##myColorChoserPrivate",open,WindowFlags))
    if (ImGui::Begin("Color Chooser##myColorChoserPrivate",open,windowSize,-1.f,WindowFlags))
    {
        colorSelected = ColorChooserInternal(pColorOut,supportsAlpha,true);

        //ImGui::EndPopup();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);

    return colorSelected;

}

// Based on the code from: https://github.com/benoitjacquier/imgui
bool ColorCombo(const char* label,ImVec4 *pColorOut,bool supportsAlpha,float width,bool closeWhenMouseLeavesIt)    {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiState& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float itemWidth = width>=0 ? width : ImGui::CalcItemWidth();
    const ImVec2 label_size = ImGui::CalcTextSize(label);
    const float color_quad_size = (g.FontSize + style.FramePadding.x);
    const float arrow_size = (g.FontSize + style.FramePadding.x * 2.0f);
    ImVec2 totalSize = ImVec2(label_size.x+color_quad_size+arrow_size, label_size.y) + style.FramePadding*2.0f;
    if (totalSize.x < itemWidth) totalSize.x = itemWidth;
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + totalSize);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max);// + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, &id)) return false;
    const float windowWidth = frame_bb.Max.x - frame_bb.Min.x - style.FramePadding.x;


    ImVec4 color = pColorOut ? *pColorOut : ImVec4(0,0,0,1);
    if (!supportsAlpha) color.w=1.f;

    const bool hovered = IsHovered(frame_bb, id);

    const ImRect value_bb(frame_bb.Min, frame_bb.Max - ImVec2(arrow_size, 0.0f));
    RenderFrame(frame_bb.Min, frame_bb.Max, window->Color(ImGuiCol_FrameBg), true, style.FrameRounding);
    RenderFrame(frame_bb.Min,ImVec2(frame_bb.Min.x+color_quad_size,frame_bb.Max.y), ImColor(style.Colors[ImGuiCol_Text]), true, style.FrameRounding);
    RenderFrame(ImVec2(frame_bb.Min.x+1,frame_bb.Min.y+1), ImVec2(frame_bb.Min.x+color_quad_size-1,frame_bb.Max.y-1), ImColor(color), true, style.FrameRounding);

    RenderFrame(ImVec2(frame_bb.Max.x-arrow_size, frame_bb.Min.y), frame_bb.Max, window->Color(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), true, style.FrameRounding); // FIXME-ROUNDING
    RenderCollapseTriangle(ImVec2(frame_bb.Max.x-arrow_size, frame_bb.Min.y) + style.FramePadding, true);

    RenderTextClipped(ImVec2(frame_bb.Min.x+color_quad_size,frame_bb.Min.y) + style.FramePadding, value_bb.Max, label, NULL, NULL);

    if (hovered)
    {
        SetHoveredID(id);
        if (g.IO.MouseClicked[0])
        {
            SetActiveID(0);
            if (IsPopupOpen(id))
            {
                ClosePopup(id);
            }
            else
            {
                FocusWindow(window);
                ImGui::OpenPopup(label);
            }
        }
        static ImVec4 copiedColor(1,1,1,1);
        static const ImVec4* pCopiedColor = NULL;
        if (g.IO.MouseClicked[1]) { // right-click (copy color)
            copiedColor = color;
            pCopiedColor = &copiedColor;
            //fprintf(stderr,"Copied\n");
        }
        else if (g.IO.MouseClicked[2] && pCopiedColor && pColorOut) { // middle-click (paste color)
            pColorOut->x = pCopiedColor->x;
            pColorOut->y = pCopiedColor->y;
            pColorOut->z = pCopiedColor->z;
            if (supportsAlpha) pColorOut->w = pCopiedColor->w;
            color = *pColorOut;
            //fprintf(stderr,"Pasted\n");
        }
    }

    bool value_changed = false;
    if (IsPopupOpen(id))
    {
        ImRect popup_rect(ImVec2(frame_bb.Min.x, frame_bb.Max.y), ImVec2(frame_bb.Max.x, frame_bb.Max.y));
        //popup_rect.Max.y = ImMin(popup_rect.Max.y, g.IO.DisplaySize.y - style.DisplaySafeAreaPadding.y); // Adhoc height limit for Combo. Ideally should be handled in Begin() along with other popups size, we want to have the possibility of moving the popup above as well.
        ImGui::SetNextWindowPos(popup_rect.Min);
        ImGui::SetNextWindowSize(ImVec2(windowWidth,-1));//popup_rect.GetSize());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,2));

        bool mustCloseCombo = false;
        const ImGuiWindowFlags flags =  ImGuiWindowFlags_ComboBox;
        if (BeginPopupEx(label, flags))
        {
            bool comboItemActive = false;
            value_changed = ColorChooserInternal(pColorOut,supportsAlpha,false,flags,&comboItemActive,windowWidth);
            if (closeWhenMouseLeavesIt && !comboItemActive)
            {
                const float distance = g.FontSize*1.75f;//1.3334f;//24;
                //fprintf(stderr,"%1.f",distance);
                ImVec2 pos = ImGui::GetWindowPos();pos.x-=distance;pos.y-=distance;
                ImVec2 size = ImGui::GetWindowSize();
                size.x+=2.f*distance;
                size.y+=2.f*distance+windowWidth*8.f/9.f;   // problem: is seems that ImGuiWindowFlags_ComboBox does not return the full window height
                const ImVec2& mousePos = ImGui::GetIO().MousePos;
                if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) {
                    mustCloseCombo = true;
                    //fprintf(stderr,"Leaving ColorCombo: pos(%1f,%1f) size(%1f,%1f)\n",pos.x,pos.y,size.x,size.y);
                }
            }
            ImGui::EndPopup();
        }
        if (mustCloseCombo && IsPopupOpen(id)) ClosePopup(id);
        ImGui::PopStyleVar(3);
    }
    return value_changed;
}


// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
struct MultilineScrollState {
    // Input.
    float scrollRegionX;
    float scrollX;
    ImGuiStorage *storage;
    const char* textToPasteInto;
    int actionToPerformCopyCutSelectAllFrom1To3;

    // Output.
    bool newScrollPositionAvailable;
    float newScrollX;
    int CursorPos;
    int SelectionStart; //                                      // Read (== to SelectionEnd when no selection)
    int SelectionEnd;   //                                      // Read
};
// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
static int MultilineScrollCallback(ImGuiTextEditCallbackData *data) {
    //static int cnt=0;fprintf(stderr,"MultilineScrollCallback (%d)\n",++cnt);
    MultilineScrollState *scrollState = (MultilineScrollState *)data->UserData;

    ImGuiID cursorId = ImGui::GetID("cursor");
    int oldCursorIndex = scrollState->storage->GetInt(cursorId, 0);

    if (oldCursorIndex != data->CursorPos)  {
        int begin = data->CursorPos;

        while ((begin > 0) && (data->Buf[begin - 1] != '\n'))   {
            --begin;
        }

        float cursorOffset = ImGui::CalcTextSize(data->Buf + begin, data->Buf + data->CursorPos).x;
        float SCROLL_INCREMENT = scrollState->scrollRegionX * 0.25f;

        if (cursorOffset < scrollState->scrollX)    {
            scrollState->newScrollPositionAvailable = true;
            scrollState->newScrollX = cursorOffset - SCROLL_INCREMENT; if (scrollState->newScrollX<0) scrollState->newScrollX=0;
        }
        else if ((cursorOffset - scrollState->scrollRegionX) >= scrollState->scrollX)   {
            scrollState->newScrollPositionAvailable = true;
            scrollState->newScrollX = cursorOffset - scrollState->scrollRegionX + SCROLL_INCREMENT;
        }
    }

    scrollState->storage->SetInt(cursorId, data->CursorPos);

    scrollState->CursorPos = data->CursorPos;
    if (data->SelectionStart<=data->SelectionEnd) {scrollState->SelectionStart = data->SelectionStart;scrollState->SelectionEnd = data->SelectionEnd;}
    else {scrollState->SelectionStart = data->SelectionEnd;scrollState->SelectionEnd = data->SelectionStart;}

    return 0;
}
// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
bool InputTextMultilineWithHorizontalScrolling(const char* label, char* buf, size_t buf_size, float height, ImGuiInputTextFlags flags, bool* pOptionalIsHoveredOut, int *pOptionalCursorPosOut, int *pOptionalSelectionStartOut, int *pOptionalSelectionEndOut,float SCROLL_WIDTH)  {
    float scrollbarSize = ImGui::GetStyle().ScrollbarSize;
    //float labelWidth = ImGui::CalcTextSize(label).x + scrollbarSize;
    MultilineScrollState scrollState = {};

    // Set up child region for horizontal scrolling of the text box.
    ImGui::BeginChild(label, ImVec2(0/*-labelWidth*/, height), false, ImGuiWindowFlags_HorizontalScrollbar);
    scrollState.scrollRegionX = ImGui::GetWindowWidth() - scrollbarSize; if (scrollState.scrollRegionX<0) scrollState.scrollRegionX = 0;
    scrollState.scrollX = ImGui::GetScrollX();
    scrollState.storage = ImGui::GetStateStorage();
    bool changed = ImGui::InputTextMultiline(label, buf, buf_size, ImVec2(SCROLL_WIDTH-scrollbarSize, (height - scrollbarSize)>0?(height - scrollbarSize):0),
                                             flags | ImGuiInputTextFlags_CallbackAlways, MultilineScrollCallback, &scrollState);
    if (pOptionalIsHoveredOut) *pOptionalIsHoveredOut = ImGui::IsItemHovered();

    if (scrollState.newScrollPositionAvailable) {
        ImGui::SetScrollX(scrollState.newScrollX);
    }

    ImGui::EndChild();
    //ImGui::SameLine();
    //ImGui::Text("%s",label);

    if (pOptionalCursorPosOut) *pOptionalCursorPosOut = scrollState.CursorPos;
    if (pOptionalSelectionStartOut) *pOptionalSelectionStartOut = scrollState.SelectionStart;
    if (pOptionalSelectionEndOut)   *pOptionalSelectionEndOut = scrollState.SelectionEnd;

    return changed;
}

// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
bool InputTextMultilineWithHorizontalScrollingAndCopyCutPasteMenu(const char *label, char *buf, int buf_size, float height,bool& staticBoolVar,int *staticArrayOfThreeIntegersHere, ImGuiInputTextFlags flags, bool *pOptionalHoveredOut,float SCROLL_WIDTH, const char *copyName, const char *cutName, const char* pasteName)   {
    bool isHovered=false;
    int& cursorPos=staticArrayOfThreeIntegersHere[0];
    int& selectionStart=staticArrayOfThreeIntegersHere[1];
    int& selectionEnd=staticArrayOfThreeIntegersHere[2];
    bool& popup_open = staticBoolVar;
    const bool changed = InputTextMultilineWithHorizontalScrolling(label,buf,(size_t)buf_size,height,flags,&isHovered,popup_open ? NULL : &cursorPos,popup_open ? NULL : &selectionStart,popup_open ? NULL : &selectionEnd,SCROLL_WIDTH);
    if (pOptionalHoveredOut) *pOptionalHoveredOut=isHovered;
    // Popup Menu ------------------------------------------

    const bool readOnly = flags&ImGuiInputTextFlags_ReadOnly;       // "Cut","","Paste" not available
    const bool hasSelectedText = selectionStart != selectionEnd;	// "Copy","Cut" available

    if (hasSelectedText || !readOnly)	{
        const bool onlyPaste = !readOnly && !hasSelectedText;
        const char* clipboardText = ImGui::GetIO().GetClipboardTextFn();
        const bool canPaste = clipboardText && strlen(clipboardText)>0;
        if (onlyPaste && !canPaste) popup_open = false;
        else {
            static const char* entries[] = {"Copy","Cut","","Paste"};   // "" is separator
            const char* pEntries[4]={copyName?copyName:entries[0],cutName?cutName:entries[1],entries[2],pasteName?pasteName:entries[3]};
            popup_open|= ImGui::GetIO().MouseClicked[1] && isHovered; // RIGHT CLICK
            int sel = ImGui::PopupMenuSimple(popup_open,onlyPaste ? &pEntries[3] : pEntries,(readOnly||onlyPaste)?1:canPaste? 4:2);
            if (sel==3) sel = 2; // Normally separator takes out one space
            const bool mustCopy = sel==0 && !onlyPaste;
            const bool mustCut = !mustCopy && sel==1;
            const bool mustPaste = !mustCopy && !mustCut && (sel==2 || (sel==0 && onlyPaste));
            if (mustCopy || mustCut || (mustPaste && (selectionStart<selectionEnd))) {
                // Copy to clipboard
                if (!mustPaste)	{
                    const char tmp = buf[selectionEnd];buf[selectionEnd]='\0';
                    ImGui::GetIO().SetClipboardTextFn(&buf[selectionStart]);
                    buf[selectionEnd]=tmp;
                }
                // Delete chars
                if (!mustCopy) {
                    //if (mustPaste) {fprintf(stderr,"Deleting before pasting: %d  %d.\n",selectionStart,selectionEnd);}
                    strncpy(&buf[selectionStart],&buf[selectionEnd],buf_size-selectionEnd);
                    for (int i=selectionStart+buf_size-selectionEnd;i<buf_size;i++) buf[i]='\0';
                }
                popup_open = false;
            }
            if (mustPaste)  {
                // This is VERY HARD to make it work as expected...
                const int cursorPosition = (selectionStart<selectionEnd) ? selectionStart : cursorPos;
                const int clipboardTextSize = strlen(clipboardText);
                int buf_len = strlen(buf);if (buf_len>buf_size) buf_len=buf_size;

                // Step 1- Shift [cursorPosition] to [cursorPosition+clipboardTextSize]
                const int numCharsToShiftRight = buf_len - cursorPosition;
                //fprintf(stderr,"Pasting: \"%s\"(%d) at %d. buf_len=%d buf_size=%d numCharsToShiftRight=%d\n",clipboardText,clipboardTextSize,cursorPosition,buf_len,buf_size,numCharsToShiftRight);

                for (int i=cursorPosition+numCharsToShiftRight>buf_size?buf_size-1:cursorPosition+numCharsToShiftRight-1;i>=cursorPosition;i--) {
                    if (i+clipboardTextSize<buf_size) {
                        //fprintf(stderr,"moving to the right char (%d): '%c' (%d)\n",i,buf[i],(int)buf[i]);
                        buf[i+clipboardTextSize] = buf[i];
                    }
                }
                // Step 2- Overwrite [cursorPosition] o [cursorPosition+clipboardTextSize]
                for (int i=cursorPosition,isz=cursorPosition+clipboardTextSize>=buf_size?buf_size:cursorPosition+clipboardTextSize;i<isz;i++) buf[i]=clipboardText[i-cursorPosition];

                popup_open = false;
            }
        }
    }
    else popup_open = false;
    //------------------------------------------------------------------
    return changed;
}


// Based on the code by krys-spectralpixel (https://github.com/krys-spectralpixel), posted here: https://github.com/ocornut/imgui/issues/261
bool TabLabels(int numTabs, const char** tabLabels, int& selectedIndex, const char** tabLabelTooltips, bool wrapMode, int *pOptionalHoveredIndex, int* pOptionalItemOrdering, bool allowTabReorder, bool allowTabClosingThroughMMB, int *pOptionalClosedTabIndex, int *pOptionalClosedTabIndexInsideItemOrdering) {
    ImGuiStyle& style = ImGui::GetStyle();

    const ImVec2 itemSpacing =  style.ItemSpacing;
    const ImVec4 color =        style.Colors[ImGuiCol_Button];
    const ImVec4 colorActive =  style.Colors[ImGuiCol_ButtonActive];
    const ImVec4 colorHover =   style.Colors[ImGuiCol_ButtonHovered];
    style.ItemSpacing.x =       1;
    style.ItemSpacing.y =       1;


    if (numTabs>0 && (selectedIndex<0 || selectedIndex>=numTabs)) {
        if (!pOptionalItemOrdering)  selectedIndex = 0;
        else selectedIndex = -1;
    }
    if (pOptionalHoveredIndex) *pOptionalHoveredIndex = -1;
    if (pOptionalClosedTabIndex) *pOptionalClosedTabIndex = -1;
    if (pOptionalClosedTabIndexInsideItemOrdering) *pOptionalClosedTabIndexInsideItemOrdering = -1;

    // Parameters to adjust to make autolayout work as expected:----------
    // The correct values are probably the ones in the comments, but I took some margin so that they work well
    // with a (medium size) vertical scrollbar too [Ok I should detect its presence and use the appropriate values...].
    const float btnOffset =         2.f*style.FramePadding.x;   // [2.f*style.FramePadding.x] It should be: ImGui::Button(text).size.x = ImGui::CalcTextSize(text).x + btnOffset;
    const float sameLineOffset =    2.f*style.ItemSpacing.x;    // [style.ItemSpacing.x]      It should be: sameLineOffset = ImGui::SameLine().size.x;
    const float uniqueLineOffset =  2.f*style.WindowPadding.x;  // [style.WindowPadding.x]    Width to be sutracted by windowWidth to make it work.
    //--------------------------------------------------------------------

    float windowWidth = 0.f,sumX=0.f;
    if (wrapMode) windowWidth = ImGui::GetWindowWidth() - uniqueLineOffset;

    static int draggingTabIndex = -1;int draggingTabTargetIndex = -1;   // These are indices inside pOptionalItemOrdering
    const bool isMouseDragging = ImGui::IsMouseDragging(0,-1.f);
    const bool isMMBreleased = ImGui::IsMouseReleased(2);
    int justClosedTabIndex = -1;

    bool selection_changed = false;
    for (int j = 0,i; j < numTabs; j++)
    {
        i = pOptionalItemOrdering ? pOptionalItemOrdering[j] : j;
        if (i==-1) continue;

        // push the style
        if (i == selectedIndex)
        {
            style.Colors[ImGuiCol_Button] =         colorActive;
            style.Colors[ImGuiCol_ButtonActive] =   colorActive;
            style.Colors[ImGuiCol_ButtonHovered] =  colorActive;
        }
        else
        {
            style.Colors[ImGuiCol_Button] =         color;
            style.Colors[ImGuiCol_ButtonActive] =   colorActive;
            style.Colors[ImGuiCol_ButtonHovered] =  colorHover;
        }

        ImGui::PushID(i);   // otherwise two tabs with the same name would clash.

        if (!wrapMode) {if (i>0) ImGui::SameLine();}
        else if (sumX > 0.f) {
            sumX+=sameLineOffset;   // Maybe we can skip it if we use SameLine(0,0) below
            sumX+=ImGui::CalcTextSize(tabLabels[i]).x+btnOffset;
            if (sumX>windowWidth) sumX = 0.f;
            else ImGui::SameLine();
        }

        // Draw the button
        if (ImGui::Button(tabLabels[i]))   {selection_changed = (selectedIndex!=i);selectedIndex = i;}
        if (wrapMode && sumX==0.f) {
            // First element of a line
            sumX = ImGui::GetItemRectSize().x;
        }

        if (ImGui::IsItemHovered()) {
            if (pOptionalHoveredIndex) *pOptionalHoveredIndex = i;
            if (tabLabelTooltips && tabLabelTooltips[i] && strlen(tabLabelTooltips[i])>0)  ImGui::SetTooltip("%s",tabLabelTooltips[i]);

            if (pOptionalItemOrdering)  {
                if (allowTabReorder)  {
                    if (isMouseDragging) {
                        if (draggingTabIndex==-1) {
                            draggingTabIndex = j;
                        }
                    }
                    else if (draggingTabIndex>=0 && draggingTabIndex<numTabs && draggingTabIndex!=j){
                        draggingTabTargetIndex = j; // For some odd reasons this seems to get called only when draggingTabIndex < i !
                    }
                }
                if (allowTabClosingThroughMMB)  {
                    if (isMMBreleased) {
                        justClosedTabIndex = i;
                        if (pOptionalClosedTabIndex) *pOptionalClosedTabIndex = i;
                        if (pOptionalClosedTabIndexInsideItemOrdering) *pOptionalClosedTabIndexInsideItemOrdering = j;
                        pOptionalItemOrdering[j] = -1;
                    }
                }
            }
        }
        ImGui::PopID();
    }

    if (draggingTabTargetIndex!=-1) {
        // swap draggingTabIndex ang draggingTabTargetIndex in pOptionalItemOrdering
        const int tmp = pOptionalItemOrdering[draggingTabTargetIndex];
        pOptionalItemOrdering[draggingTabTargetIndex] = pOptionalItemOrdering[draggingTabIndex];
        pOptionalItemOrdering[draggingTabIndex] = tmp;
        //fprintf(stderr,"%d %d\n",draggingTabIndex,draggingTabTargetIndex);
        draggingTabTargetIndex = draggingTabIndex = -1;
    }
    if (!isMouseDragging) draggingTabIndex = -1;
    if (selectedIndex == justClosedTabIndex && selectedIndex>=0)    {
        selectedIndex = -1;
        for (int j = 0,i; j < numTabs; j++) {
            i = pOptionalItemOrdering ? pOptionalItemOrdering[j] : j;
            if (i==-1) continue;
            selectedIndex = i;
            break;
        }
    }

    // Restore the style
    style.Colors[ImGuiCol_Button] =         color;
    style.Colors[ImGuiCol_ButtonActive] =   colorActive;
    style.Colors[ImGuiCol_ButtonHovered] =  colorHover;
    style.ItemSpacing =                     itemSpacing;

    return selection_changed;
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
