#include "imguivariouscontrols.h"

#include <imgui_internal.h> // intellisense

#ifndef NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE
#ifndef IMGUI_USE_AUTO_BINDING
#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../imguibindings/stb_image.h"
#endif //STBI_INCLUDE_STB_IMAGE_H
#endif //IMGUI_USE_AUTO_BINDING
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
struct gif_result : stbi__gif {
    unsigned char *data;
    struct gif_result *next;
};
#ifdef __cplusplus
}
#endif //__cplusplus
//#define DEBUG_OUT_TEXTURE
#ifdef DEBUG_OUT_TEXTURE
#ifndef STBI_INCLUDE_STB_IMAGE_WRITE_H
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "./addons/imguibindings/stb_image_write.h"
#endif //DEBUG_OUT_TEXTURE
#endif //STBI_INCLUDE_STB_IMAGE_WRITE_H
#endif //NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE


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
    // This one has just been added to ImGui:
    //char txt[48]="";sprintf(txt,"%3d%% (ImGui default progress bar)",(int)(progress*100));
    //ImGui::ProgressBar(progress,ImVec2(0,0),txt);
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
    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    RenderFrame(frame_bb.Min,ImVec2(frame_bb.Min.x+color_quad_size,frame_bb.Max.y), ImColor(style.Colors[ImGuiCol_Text]), true, style.FrameRounding);
    RenderFrame(ImVec2(frame_bb.Min.x+1,frame_bb.Min.y+1), ImVec2(frame_bb.Min.x+color_quad_size-1,frame_bb.Max.y-1), ImColor(color), true, style.FrameRounding);

    RenderFrame(ImVec2(frame_bb.Max.x-arrow_size, frame_bb.Min.y), frame_bb.Max, GetColorU32(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), true, style.FrameRounding); // FIXME-ROUNDING
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

		    //strncpy(&buf[selectionStart],&buf[selectionEnd],buf_size-selectionEnd);				// Valgrind complains here, but I KNOW that source and destination overlap: I just want to shift chars to the left!
		    for (int i=0,isz=buf_size-selectionEnd;i<isz;i++) buf[i+selectionStart]=buf[i+selectionEnd];// I do it manually, so Valgrind is happy

		    for (int i=selectionStart+buf_size-selectionEnd;i<buf_size;i++) buf[i]='\0';		// This is mandatory at the end
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


bool ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
    return false;

    ImVec2 size = imageSize;
    if (size.x<=0 && size.y<=0) {size.x=size.y=ImGui::GetTextLineHeightWithSpacing();}
    else {
        if (size.x<=0)          size.x=size.y;
        else if (size.y<=0)     size.y=size.x;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;
    }

    ImGuiState& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(label);
    const ImVec2 textSize = ImGui::CalcTextSize(label,NULL,true);
    const bool hasText = textSize.x>0;

    const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
    const ImVec2 totalSizeWithoutPadding(size.x+innerSpacing+textSize.x,size.y>textSize.y ? size.y : textSize.y);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding*2);
    ImVec2 start(0,0);
    start = window->DC.CursorPos + padding;if (size.y<textSize.y) start.y+=(textSize.y-size.y)*.5f;
    const ImRect image_bb(start, start + size);
    start = window->DC.CursorPos + padding;start.x+=size.x+innerSpacing;if (size.y>textSize.y) start.y+=(size.y-textSize.y)*.5f;
    ItemSize(bb);
    if (!ItemAdd(bb, &id))
    return false;

    bool hovered=false, held=false;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if (bg_col.w > 0.0f)
    window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

    window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

    if (textSize.x>0) ImGui::RenderText(start,label);
    return pressed;
}

#ifndef NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE
struct AnimatedImageInternal {
    protected:

    int w,h,frames;
    ImVector<unsigned char> buffer;
    ImVector<float> delays;
    ImTextureID persistentTexId;              // This will be used when all frames can fit into a single texture (very good for performance and memory)
    int numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture;
    bool hoverModeIfSupported;
    bool persistentTexIdIsNotOwned;
    mutable bool isAtLeastOneWidgetInHoverMode;  // internal

    mutable  int lastFrameNum;
    mutable float delay;
    mutable float timer;
    mutable ImTextureID texId;
    mutable ImVec2 uvFrame0,uvFrame1;   // used by persistentTexId
    mutable int lastImGuiFrameUpdate;

    inline void updateTexture() const   {
        // fix updateTexture() to use persistentTexID when necessary
        IM_ASSERT(AnimatedImage::GenerateOrUpdateTextureCb!=NULL);	// Please use ImGui::AnimatedGif::SetGenerateOrUpdateTextureCallback(...) before calling this method
        if (frames<=0) return;
        else if (frames==1) {
            if (!texId) AnimatedImage::GenerateOrUpdateTextureCb(texId,w,h,4,&buffer[0],false,false,false);
            return;
        }

        // These two lines sync animation in multiple items:
        if (texId && lastImGuiFrameUpdate==ImGui::GetFrameCount()) return;
        lastImGuiFrameUpdate=ImGui::GetFrameCount();
        if (hoverModeIfSupported && !isAtLeastOneWidgetInHoverMode) {
            // reset animation here:
            timer=-1.f;lastFrameNum=-1;delay=0;
            //calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
        }
        isAtLeastOneWidgetInHoverMode = false;

        float lastDelay = delay;
        if (timer>0) {
            delay = ImGui::GetTime()*100.f-timer;
            if (delay<0) timer = -1.f;
        }
        if (timer<0) {timer = ImGui::GetTime()*100.f;delay=0.f;}

        const int imageSz = 4 * w * h;
        IM_ASSERT(sizeof(unsigned short)==2*sizeof(unsigned char));
        bool changed = false;
        float frameTime=0.f;
        bool forceUpdate = false;
        if (lastFrameNum<0) {forceUpdate=true;lastFrameNum=0;}
        for (int i=lastFrameNum;i<frames;i++)   {
            frameTime = delays[i];
            //fprintf(stderr,"%d/%d) %1.2f\n",i,frames,frameTime);
            if (delay <= lastDelay+frameTime) {
                changed = (i!=lastFrameNum || !texId);
                lastFrameNum = i;
                if (changed || forceUpdate)    {
                    if (!persistentTexId) AnimatedImage::GenerateOrUpdateTextureCb(texId,w,h,4,&buffer[imageSz*i],false,false,false);
                    else {
                        texId = persistentTexId;
                        // calculate uvFrame0 and uvFrame1 here based on 'i' and numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture
                        calculateTexCoordsForFrame(i,uvFrame0,uvFrame1);
                    }
                }
                //fprintf(stderr,"%d/%d) %1.2f %1.2f %1.2f\n",i,frames,frameTime,delay,lastDelay);
                delay = lastDelay;
                return;
            }
            lastDelay+=frameTime;
            if (i==frames-1) i=-1;
        }

    }

    public:
    AnimatedImageInternal()  {persistentTexIdIsNotOwned=false;texId=persistentTexId=NULL;clear();}
    ~AnimatedImageInternal()  {texId=persistentTexId=NULL;clear();persistentTexIdIsNotOwned=false;}
    AnimatedImageInternal(char const *filename,bool useHoverModeIfSupported=false)  {persistentTexIdIsNotOwned = false;texId=persistentTexId=NULL;load(filename,useHoverModeIfSupported);}
    AnimatedImageInternal(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayDetweenFramesInCs,bool useHoverMode=false) {
        persistentTexIdIsNotOwned = false;texId=persistentTexId=NULL;
        create(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayDetweenFramesInCs,useHoverMode);
    }
    void clear() {
        w=h=frames=lastFrameNum=0;delay=0.f;timer=-1.f;buffer.clear();delays.clear();
        numFramesPerRowInPersistentTexture = numFramesPerColInPersistentTexture = 0;
        uvFrame0.x=uvFrame0.y=0;uvFrame1.x=uvFrame1.y=1;
        lastImGuiFrameUpdate = -1;hoverModeIfSupported=isAtLeastOneWidgetInHoverMode = false;
        if (texId || persistentTexId) IM_ASSERT(AnimatedImage::FreeTextureCb!=NULL);   // Please use ImGui::AnimatedGif::SetFreeTextureCallback(...)
        if (texId) {if (texId!=persistentTexId) AnimatedImage::FreeTextureCb(texId);texId=NULL;}
        if (persistentTexId)  {if (!persistentTexIdIsNotOwned) AnimatedImage::FreeTextureCb(persistentTexId);persistentTexId=NULL;}
    }

    bool load(char const *filename,bool useHoverModeIfSupported=false)  {
        ImGui::AnimatedImageInternal& ag = *this;

        // Code based on:
        // https://gist.github.com/urraka/685d9a6340b26b830d49

        FILE *f;
        stbi__context s;
        ag.clear();
        ag.persistentTexIdIsNotOwned = false;
        bool ok = false;

        if (!(f = stbi__fopen(filename, "rb"))) {
            stbi__errpuc("can't fopen", "Unable to open file");
            return false;
        }

        stbi__start_file(&s, f);

        if (stbi__gif_test(&s))
        {
            ok =true;
            int c;
            stbi__gif g;
            gif_result head;
            gif_result *prev = 0, *gr = &head;

            memset(&g, 0, sizeof(g));
            memset(&head, 0, sizeof(head));

            ag.frames = 0;

            while ((gr->data = stbi__gif_load_next(&s, &g, &c, 4)))
            {
		if (gr->data == (unsigned char*)&s)
                {
                    gr->data = 0;
                    break;
                }

                if (prev) prev->next = gr;
                gr->delay = g.delay;
                prev = gr;
                gr = (gif_result*) stbi__malloc(sizeof(gif_result));
                memset(gr, 0, sizeof(gif_result));
                ++ag.frames;
            }

	    STBI_FREE(g.out);
            if (gr != &head)    {
                STBI_FREE(gr);
            }

            if (ag.frames > 0)
            {
                ag.w = g.w;
                ag.h = g.h;
            }

            if (ag.frames==1) {
                ag.buffer.resize(ag.w*ag.h*4);
                memcpy(&ag.buffer[0],head.data,ag.buffer.size());
                STBI_FREE(head.data);
            }


            if (ag.frames > 1)
            {
                unsigned int size = 4 * g.w * g.h;
                unsigned char *p = 0;
                float *pd = 0;

                ag.buffer.resize(ag.frames * size);//(size + 2));
                ag.delays.resize(ag.frames);
                gr = &head;
                p = &ag.buffer[0];
                pd = &ag.delays[0];

                IM_ASSERT(sizeof(unsigned short)==2*sizeof(unsigned char));	// Not sure that this is necessary
                unsigned short tmp = 0;
                unsigned char* pTmp = (unsigned char*) &tmp;
                while (gr)
                {
                    prev = gr;
                    memcpy(p, gr->data, size);
                    p += size;
                    tmp = 0;
                    // We should invert these two lines for big-endian machines:
                    pTmp[0] = gr->delay & 0xFF;
                    pTmp[1] = (gr->delay & 0xFF00) >> 8;
                    *pd++ = (float) tmp;
                    gr = gr->next;

                    STBI_FREE(prev->data);
                    if (prev != &head) STBI_FREE(prev);
                }

                if (AnimatedImage::MaxPersistentTextureSize.x>0 && AnimatedImage::MaxPersistentTextureSize.y>0)	{
                    // code path that checks 'MaxPersistentTextureSize' and puts all into a single texture (rearranging the buffer)
                    ImVec2 textureSize = AnimatedImage::MaxPersistentTextureSize;
                    int maxNumFramesPerRow = (int)textureSize.x/(int)ag.w;
                    int maxNumFramesPerCol = (int)textureSize.y/(int)ag.h;
                    int maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
                    int cnt = 0;
                    ImVec2 lastValidTextureSize(0,0);
                    while (maxNumFramesInATexture>=ag.frames)	{
                        // Here we just halve the 'textureSize', so that, if it fits, we save further texture space
                        lastValidTextureSize = textureSize;
                        if (cnt%2==0) textureSize.y = textureSize.y/2;
                        else textureSize.x = textureSize.x/2;
                        maxNumFramesPerRow = (int)textureSize.x/(int)ag.w;
                        maxNumFramesPerCol = (int)textureSize.y/(int)ag.h;
                        maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
                        ++cnt;
                    }
                    if (cnt>0)  {
                        textureSize=lastValidTextureSize;
                        maxNumFramesPerRow = (int)textureSize.x/(int)ag.w;
                        maxNumFramesPerCol = (int)textureSize.y/(int)ag.h;
                        maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
                    }
                    if (maxNumFramesInATexture>=ag.frames)	{
                        numFramesPerRowInPersistentTexture = maxNumFramesPerRow;
                        numFramesPerColInPersistentTexture = maxNumFramesPerCol;

                        rearrangeBufferForPersistentTexture();

                        // generate persistentTexture,delete buffer
                        IM_ASSERT(AnimatedImage::GenerateOrUpdateTextureCb!=NULL);	// Please use ImGui::AnimatedGif::SetGenerateOrUpdateTextureCallback(...) before calling this method
                        AnimatedImage::GenerateOrUpdateTextureCb(persistentTexId,ag.w*maxNumFramesPerRow,ag.h*maxNumFramesPerCol,4,&buffer[0],false,false,false);
                        buffer.clear();

                        hoverModeIfSupported = useHoverModeIfSupported;
                        //fprintf(stderr,"%d x %d (%d x %d)\n",numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture,(int)textureSize.x,(int)textureSize.y);

                    }
                }
            }
        }
        else
        {
            ok = false;
            // TODO: Here we could load other image formats...
        }

        fclose(f);
        return ok;
    }
    bool create(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayDetweenFramesInCs,bool useHoverMode=false)   {
        clear();
        persistentTexIdIsNotOwned = false;
        IM_ASSERT(myTexId);
        IM_ASSERT(animationImageWidth>0 && animationImageHeight>0);
        IM_ASSERT(numFrames>0);
        IM_ASSERT(delayDetweenFramesInCs>0);
        IM_ASSERT(numFramesPerRowInTexture*numFramesPerColumnInTexture>=numFrames);
        if (!myTexId || animationImageWidth<=0 || animationImageHeight<=0
                || numFrames<=0 || delayDetweenFramesInCs<=0 || (numFramesPerRowInTexture*numFramesPerColumnInTexture<numFrames))
            return false;
        persistentTexId = myTexId;
        persistentTexIdIsNotOwned = true;
        w = animationImageWidth;
        h = animationImageHeight;
        frames = numFrames;
        numFramesPerRowInPersistentTexture = numFramesPerRowInTexture;
        numFramesPerColInPersistentTexture = numFramesPerColumnInTexture;
        delays.resize(frames);
        for (int i=0;i<frames;i++) delays[i] = delayDetweenFramesInCs;
        hoverModeIfSupported = useHoverMode;
        return true;
    }

    inline bool areAllFramesInASingleTexture() const {return persistentTexId!=NULL;}
    void render(ImVec2 size=ImVec2(0,0), const ImVec2& uv0=ImVec2(0,0), const ImVec2& uv1=ImVec2(1,1), const ImVec4& tint_col=ImVec4(1,1,1,1), const ImVec4& border_col=ImVec4(0,0,0,0)) const  {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;
        if (size.x==0) size.x=w;
        else if (size.x<0) size.x=-size.x*w;
        if (size.y==0) size.y=h;
        else if (size.y<0) size.y=-size.y*h;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        if (border_col.w > 0.0f)
            bb.Max += ImVec2(2,2);
        ItemSize(bb);
        if (!ItemAdd(bb, NULL))
            return;

        updateTexture();

        ImVec2 uv_0 = uv0;
        ImVec2 uv_1 = uv1;
        if (persistentTexId) {
            bool hovered = true;	// to fall back when useHoverModeIfSupported == false;
            if (hoverModeIfSupported) {
                hovered = ImGui::IsItemHovered();
                if (hovered) isAtLeastOneWidgetInHoverMode = true;
            }
            if (hovered)	{
                const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
                uv_0 = uvFrame0 + uv0*uvFrameDelta;
                uv_1 = uvFrame0 + uv1*uvFrameDelta;
            }
            else {
                // We must use frame zero here:
                ImVec2 uvFrame0,uvFrame1;
                calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
                const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
                uv_0 = uvFrame0 + uv0*uvFrameDelta;
                uv_1 = uvFrame0 + uv1*uvFrameDelta;
            }
        }
        if (border_col.w > 0.0f)
        {
            window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
            window->DrawList->AddImage(texId, bb.Min+ImVec2(1,1), bb.Max-ImVec2(1,1), uv_0, uv_1, GetColorU32(tint_col));
        }
        else
        {
            window->DrawList->AddImage(texId, bb.Min, bb.Max, uv_0, uv_1, GetColorU32(tint_col));
        }
    }
    bool renderAsButton(const char* label,ImVec2 size=ImVec2(0,0), const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1)) const {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        if (size.x==0) size.x=w;
        else if (size.x<0) size.x=-size.x*w;
        if (size.y==0) size.y=h;
        else if (size.y<0) size.y=-size.y*h;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;

        ImGuiState& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        // Default to using texture ID as ID. User can still push string/integer prefixes.
        // We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
        ImGui::PushID((void *)this);
        const ImGuiID id = window->GetID(label);
        ImGui::PopID();

        const ImVec2 textSize = ImGui::CalcTextSize(label,NULL,true);
        const bool hasText = textSize.x>0;

        const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
        const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
        const ImVec2 totalSizeWithoutPadding(size.x+innerSpacing+textSize.x,size.y>textSize.y ? size.y : textSize.y);
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding*2);
        ImVec2 start(0,0);
        start = window->DC.CursorPos + padding;if (size.y<textSize.y) start.y+=(textSize.y-size.y)*.5f;
        const ImRect image_bb(start, start + size);
        start = window->DC.CursorPos + padding;start.x+=size.x+innerSpacing;if (size.y>textSize.y) start.y+=(size.y-textSize.y)*.5f;
        ItemSize(bb);
        if (!ItemAdd(bb, &id))
            return false;

        bool hovered=false, held=false;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);

        updateTexture();

        ImVec2 uv_0 = uv0;
        ImVec2 uv_1 = uv1;
        if (hovered && hoverModeIfSupported) isAtLeastOneWidgetInHoverMode = true;
        if ((persistentTexId && hoverModeIfSupported && hovered) || !persistentTexId || !hoverModeIfSupported) {
            const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
            uv_0 = uvFrame0 + uv0*uvFrameDelta;
            uv_1 = uvFrame0 + uv1*uvFrameDelta;
        }
        else {
            // We must use frame zero here:
            ImVec2 uvFrame0,uvFrame1;
            calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
            const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
            uv_0 = uvFrame0 + uv0*uvFrameDelta;
            uv_1 = uvFrame0 + uv1*uvFrameDelta;
        }


        // Render
        const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
        if (bg_col.w > 0.0f)
            window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

        window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv_0, uv_1, GetColorU32(tint_col));

        if (hasText) ImGui::RenderText(start,label);
        return pressed;
    }


    inline int getWidth() const {return w;}
    inline int getHeight() const {return h;}
    inline int getNumFrames() const {return frames;}
    inline ImTextureID getTexture() const {return texId;}

    private:
    AnimatedImageInternal(const AnimatedImageInternal& ) {}
    void operator=(const AnimatedImageInternal& ) {}
    void rearrangeBufferForPersistentTexture()  {
        const int newBufferSize = w*numFramesPerRowInPersistentTexture*h*numFramesPerColInPersistentTexture*4;

        // BUFFER: frames images one below each other: size: 4*w x (h*frames)
        // TMP:    frames images numFramesPerRowInPersistentTexture * (4*w) x (h*numFramesPerColInPersistentTexture)

        const int strideSz = w*4;
        const int frameSz = strideSz*h;
        ImVector<unsigned char> tmp;tmp.resize(newBufferSize);

        unsigned char* pw=&tmp[0];
        const unsigned char* pr=&buffer[0];

        int frm=0,colSz=0;
        while (frm<frames)	{
            for (int y = 0; y<h;y++)    {
                pr=&buffer[frm*frameSz + y*strideSz];
                colSz = numFramesPerRowInPersistentTexture>(frames-frm)?(frames-frm):numFramesPerRowInPersistentTexture;
                for (int col = 0; col<colSz;col++)    {
                    memcpy(pw,pr,strideSz);
                    pr+=frameSz;
                    pw+=strideSz;
                }
                if (colSz<numFramesPerRowInPersistentTexture) {
                    for (int col = colSz;col<numFramesPerRowInPersistentTexture;col++)    {
                        memset(pw,0,strideSz);
                        pw+=strideSz;
                    }
                }
            }
            frm+=colSz;
        }

        //-----------------------------------------------------------------------
        buffer.swap(tmp);

#       ifdef DEBUG_OUT_TEXTURE
        stbi_write_png("testOutputPng.png", w*numFramesPerRowInPersistentTexture,h*numFramesPerColInPersistentTexture, 4, &buffer[0], w*numFramesPerRowInPersistentTexture*4);
#       undef DEBUG_OUT_TEXTURE
#       endif //DEBUG_OUT_TEXTURE
    }
    void calculateTexCoordsForFrame(int frm,ImVec2& uv0Out,ImVec2& uv1Out) const    {
        uv0Out=ImVec2((float)(frm%numFramesPerRowInPersistentTexture)/(float)numFramesPerRowInPersistentTexture,(float)(frm/numFramesPerRowInPersistentTexture)/(float)numFramesPerColInPersistentTexture);
        uv1Out=ImVec2(uv0Out.x+1.f/(float)numFramesPerRowInPersistentTexture,uv0Out.y+1.f/(float)numFramesPerColInPersistentTexture);
    }

};

AnimatedImage::FreeTextureDelegate AnimatedImage::FreeTextureCb =
#ifdef IMGUI_USE_AUTO_BINDING
    &ImImpl_FreeTexture;
#else //IMGUI_USE_AUTO_BINDING
    NULL;
#endif //IMGUI_USE_AUTO_BINDING
AnimatedImage::GenerateOrUpdateTextureDelegate AnimatedImage::GenerateOrUpdateTextureCb =
#ifdef IMGUI_USE_AUTO_BINDING
    &ImImpl_GenerateOrUpdateTexture;
#else //IMGUI_USE_AUTO_BINDING
    NULL;
#endif //IMGUI_USE_AUTO_BINDING

ImVec2 AnimatedImage::MaxPersistentTextureSize(2048,2048);

AnimatedImage::AnimatedImage(const char *filename, bool useHoverModeIfSupported)    {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal(filename,useHoverModeIfSupported);
}
AnimatedImage::AnimatedImage(ImTextureID myTexId, int animationImageWidth, int animationImageHeight, int numFrames, int numFramesPerRowInTexture, int numFramesPerColumnInTexture, float delayBetweenFramesInCs, bool useHoverMode) {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayBetweenFramesInCs,useHoverMode);
}
AnimatedImage::AnimatedImage()  {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal();
}
AnimatedImage::~AnimatedImage() {
    clear();
    ptr->~AnimatedImageInternal();
    ImGui::MemFree(ptr);ptr=NULL;
}
void AnimatedImage::clear() {ptr->clear();}
void AnimatedImage::render(ImVec2 size, const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col, const ImVec4 &border_col) const   {ptr->render(size,uv0,uv1,tint_col,border_col);}
bool AnimatedImage::renderAsButton(const char *label, ImVec2 size, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)   {return ptr->renderAsButton(label,size,uv0,uv1,frame_padding,bg_col,tint_col);}
bool AnimatedImage::load(const char *filename, bool useHoverModeIfSupported)    {return ptr->load(filename,useHoverModeIfSupported);}
bool AnimatedImage::create(ImTextureID myTexId, int animationImageWidth, int animationImageHeight, int numFrames, int numFramesPerRowInTexture, int numFramesPerColumnInTexture, float delayBetweenFramesInCs, bool useHoverMode)   {return ptr->create(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayBetweenFramesInCs,useHoverMode);}
int AnimatedImage::getWidth() const {return ptr->getWidth();}
int AnimatedImage::getHeight() const    {return ptr->getHeight();}
int AnimatedImage::getNumFrames() const {return ptr->getNumFrames();}
bool AnimatedImage::areAllFramesInASingleTexture() const    {return ptr->areAllFramesInASingleTexture();}
#endif //NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE


/*
    inline ImVec2 mouseToPdfRelativeCoords(const ImVec2 &mp) const {
       return ImVec2((mp.x+cursorPosAtStart.x-startPos.x)*(uv1.x-uv0.x)/zoomedImageSize.x+uv0.x,
               (mp.y+cursorPosAtStart.y-startPos.y)*(uv1.y-uv0.y)/zoomedImageSize.y+uv0.y);
    }
    inline ImVec2 pdfRelativeToMouseCoords(const ImVec2 &mp) const {
        return ImVec2((mp.x-uv0.x)*(zoomedImageSize.x)/(uv1.x-uv0.x)+startPos.x-cursorPosAtStart.x,(mp.y-uv0.y)*(zoomedImageSize.y)/(uv1.y-uv0.y)+startPos.y-cursorPosAtStart.y);
    }
*/
bool ImageZoomAndPan(ImTextureID user_texture_id, const ImVec2& size,float aspectRatio,float& zoom,ImVec2& zoomCenter,int panMouseButtonDrag,int resetZoomAndPanMouseButton,const ImVec2& zoomMaxAndZoomStep)
{
    bool rv = false;
    ImGuiWindow* window = GetCurrentWindow();
    if (!window || window->SkipItems) return rv;
    ImVec2 curPos = ImGui::GetCursorPos();
    const ImVec2 wndSz(size.x>0 ? size.x : ImGui::GetWindowSize().x-curPos.x,size.y>0 ? size.y : ImGui::GetWindowSize().y-curPos.y);

    IM_ASSERT(wndSz.x!=0 && wndSz.y!=0 && zoom!=0);

    // Here we use the whole size (although it can be partially empty)
    ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + wndSz.x,window->DC.CursorPos.y + wndSz.y));
    ItemSize(bb);
    if (!ItemAdd(bb, NULL)) return rv;

    ImVec2 imageSz = wndSz;
    ImVec2 remainingWndSize(0,0);
    if (aspectRatio!=0) {
        const float wndAspectRatio = wndSz.x/wndSz.y;
        if (aspectRatio >= wndAspectRatio) {imageSz.y = imageSz.x/aspectRatio;remainingWndSize.y = wndSz.y - imageSz.y;}
        else {imageSz.x = imageSz.y*aspectRatio;remainingWndSize.x = wndSz.x - imageSz.x;}
    }

    if (ImGui::IsItemHovered()) {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.MouseWheel!=0) {
            if (io.KeyCtrl) {
                const float zoomStep = zoomMaxAndZoomStep.y;
                const float zoomMin = 1.f;
                const float zoomMax = zoomMaxAndZoomStep.x;
                if (io.MouseWheel < 0) {zoom/=zoomStep;if (zoom<zoomMin) zoom=zoomMin;}
                else {zoom*=zoomStep;if (zoom>zoomMax) zoom=zoomMax;}
                rv = true;
                /*if (io.FontAllowUserScaling) {
                    // invert effect:
                    // Zoom / Scale window
                    ImGuiState& g = *GImGui;
                    ImGuiWindow* window = g.HoveredWindow;
                    float new_font_scale = ImClamp(window->FontWindowScale - g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
                    float scale = new_font_scale / window->FontWindowScale;
                    window->FontWindowScale = new_font_scale;

                    const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
                    window->Pos += offset;
                    window->PosFloat += offset;
                    window->Size *= scale;
                    window->SizeFull *= scale;
                }*/
            }
            else  {
                const bool scrollDown = io.MouseWheel <= 0;
                const float zoomFactor = .5/zoom;
                if ((!scrollDown && zoomCenter.y > zoomFactor) || (scrollDown && zoomCenter.y <  1.f - zoomFactor))  {
                    const float slideFactor = zoomMaxAndZoomStep.y*0.1f*zoomFactor;
                    if (scrollDown) {
                        zoomCenter.y+=slideFactor;///(imageSz.y*zoom);
                        if (zoomCenter.y >  1.f - zoomFactor) zoomCenter.y =  1.f - zoomFactor;
                    }
                    else {
                        zoomCenter.y-=slideFactor;///(imageSz.y*zoom);
                        if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
                    }
                    rv = true;
                }
            }
        }
        if (io.MouseClicked[resetZoomAndPanMouseButton]) {zoom=1.f;zoomCenter.x=zoomCenter.y=.5f;rv = true;}
        if (ImGui::IsMouseDragging(panMouseButtonDrag,1.f))   {
            zoomCenter.x-=io.MouseDelta.x/(imageSz.x*zoom);
            zoomCenter.y-=io.MouseDelta.y/(imageSz.y*zoom);
            rv = true;
            ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
        }
    }

    const float zoomFactor = .5/zoom;
    if (rv) {
        if (zoomCenter.x < zoomFactor) zoomCenter.x = zoomFactor;
        else if (zoomCenter.x > 1.f - zoomFactor) zoomCenter.x = 1.f - zoomFactor;
        if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
        else if (zoomCenter.y > 1.f - zoomFactor) zoomCenter.y = 1.f - zoomFactor;
    }

    ImVec2 uvExtension(2.f*zoomFactor,2.f*zoomFactor);
    if (remainingWndSize.x > 0) {
        const float remainingSizeInUVSpace = 2.f*zoomFactor*(remainingWndSize.x/imageSz.x);
        const float deltaUV = uvExtension.x;
        const float remainingUV = 1.f-deltaUV;
        if (deltaUV<1) {
            float adder = (remainingUV < remainingSizeInUVSpace ? remainingUV : remainingSizeInUVSpace);
            uvExtension.x+=adder;
            remainingWndSize.x-= adder * zoom * imageSz.x;
            imageSz.x+=adder * zoom * imageSz.x;

            if (zoomCenter.x < uvExtension.x*.5f) zoomCenter.x = uvExtension.x*.5f;
            else if (zoomCenter.x > 1.f - uvExtension.x*.5f) zoomCenter.x = 1.f - uvExtension.x*.5f;
        }
    }
    if (remainingWndSize.y > 0) {
        const float remainingSizeInUVSpace = 2.f*zoomFactor*(remainingWndSize.y/imageSz.y);
        const float deltaUV = uvExtension.y;
        const float remainingUV = 1.f-deltaUV;
        if (deltaUV<1) {
            float adder = (remainingUV < remainingSizeInUVSpace ? remainingUV : remainingSizeInUVSpace);
            uvExtension.y+=adder;
            remainingWndSize.y-= adder * zoom * imageSz.y;
            imageSz.y+=adder * zoom * imageSz.y;

            if (zoomCenter.y < uvExtension.y*.5f) zoomCenter.y = uvExtension.y*.5f;
            else if (zoomCenter.y > 1.f - uvExtension.y*.5f) zoomCenter.y = 1.f - uvExtension.y*.5f;
        }
    }

    ImVec2 uv0((zoomCenter.x-uvExtension.x*.5f),(zoomCenter.y-uvExtension.y*.5f));
    ImVec2 uv1((zoomCenter.x+uvExtension.x*.5f),(zoomCenter.y+uvExtension.y*.5f));


    /* // Here we use just the window size, but then ImGui::IsItemHovered() should be moved below this block. How to do it?
    ImVec2 startPos=window->DC.CursorPos;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    ImVec2 endPos(startPos.x+imageSz.x,startPos.y+imageSz.y);
    ImRect bb(startPos, endPos);
    ItemSize(bb);
    if (!ItemAdd(bb, NULL)) return rv;*/

    ImVec2 startPos=bb.Min,endPos=bb.Max;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    endPos.x = startPos.x + imageSz.x;
    endPos.y = startPos.y + imageSz.y;

    window->DrawList->AddImage(user_texture_id, startPos, endPos, uv0, uv1);

    return rv;
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
