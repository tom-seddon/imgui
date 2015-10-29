// Creating a node graph editor for ImGui
// Quick demo, not production code! This is more of a demo of how to use ImGui to create custom stuff.
// Better version by @daniel_collin here https://gist.github.com/emoon/b8ff4b4ce4f1b43e79f2
// See https://github.com/ocornut/imgui/issues/306
// v0.02
// Animated gif: https://cloud.githubusercontent.com/assets/8225057/9472357/c0263c04-4b4c-11e5-9fdf-2cd4f33f6582.gif

#include "imguinodegrapheditor.h"

// NB: You can use math functions/operators on ImVec2 if you #define IMGUI_DEFINE_MATH_OPERATORS and #include "imgui_internal.h"
// Here we only declare simple +/- operators so others don't leak into the demo code.
//static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x+rhs.x, lhs.y+rhs.y); }
//static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x-rhs.x, lhs.y-rhs.y); }


namespace ImGui	{

NodeGraphEditor::Style NodeGraphEditor::style;  // static variable initialization
inline static bool EditColorImU32(const char* label,ImU32& color) {
    static ImVec4 tmp;
    tmp = ImColor(color);
    const bool changed = ImGui::ColorEdit4(label,&tmp.x);
    if (changed) color = ImColor(tmp);
    return changed;
}
bool NodeGraphEditor::Style::Edit(NodeGraphEditor::Style& s) {
    bool changed = false;ImVec4 tmp;
    const float dragSpeed = 0.5f;
    const char prec[] = "%1.1f";
    ImGui::PushID(&s);
    changed|=ImGui::ColorEdit4( "color_background",&s.color_background.x);
    changed|=EditColorImU32(    "color_grid",s.color_grid);
    changed|=ImGui::DragFloat(  "grid_line_width",&s.grid_line_width,dragSpeed,1.f,32.f,prec);
    changed|=ImGui::DragFloat(  "grid_size",&s.grid_size,dragSpeed,8.f,512.f,prec);

    changed|=EditColorImU32(    "color_node",s.color_node);
    changed|=EditColorImU32(    "color_node_frame",s.color_node_frame);
    changed|=EditColorImU32(    "color_node_selected",s.color_node_selected);
    changed|=EditColorImU32(    "color_node_hovered",s.color_node_hovered);
    changed|=ImGui::DragFloat(  "node_rounding",&s.node_rounding,dragSpeed,0.f,16.f,prec);
    changed|=ImGui::DragFloat2(  "node_window_padding",&s.node_window_padding.x,dragSpeed,0.f,8.f,prec);

    changed|=EditColorImU32(    "color_node_input_slots",s.color_node_input_slots);
    changed|=EditColorImU32(    "color_node_output_slots",s.color_node_output_slots);
    changed|=ImGui::DragFloat(  "node_slots_radius",&s.node_slots_radius,dragSpeed,1.f,10.f,prec);

    changed|=EditColorImU32(    "color_link",s.color_link);
    changed|=ImGui::DragFloat(  "link_line_width",&s.link_line_width,dragSpeed,1.f,6.f,prec);
    changed|=ImGui::DragFloat(  "link_control_point_distance",&s.link_control_point_distance,dragSpeed,10.f,200.f,prec);
    changed|=ImGui::DragInt(  "link_num_segments",&s.link_num_segments,dragSpeed,0,16.f);

    changed|=ImGui::ColorEdit4( "color_node_title",&s.color_node_title.x);
    changed|=ImGui::ColorEdit4( "color_node_input_slots_names",&s.color_node_input_slots_names.x);
    changed|=ImGui::ColorEdit4( "color_node_output_slots_names",&s.color_node_output_slots_names.x);

    ImGui::PopID();
    return changed;
}

#if (!defined(NO_IMGUIHELPER) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
#include "../imguihelper/imguihelper.h"
bool NodeGraphEditor::Style::Save(const NodeGraphEditor::Style &style, const char *filename)    {
    ImGuiHelper::Serializer s(filename);
    if (!s.isValid()) return false;

    ImVec4 tmpColor = ImColor(style.color_background);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_background",4);
    tmpColor = ImColor(style.color_grid);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_grid",4);
    s.save(ImGuiHelper::FT_FLOAT,&style.grid_line_width,"grid_line_width");
    s.save(ImGuiHelper::FT_FLOAT,&style.grid_size,"grid_size");

    tmpColor = ImColor(style.color_node);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_node",4);
    tmpColor = ImColor(style.color_node_frame);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_node_frame",4);
    tmpColor = ImColor(style.color_node_selected);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_node_selected",4);
    tmpColor = ImColor(style.color_node_hovered);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_node_hovered",4);
    s.save(ImGuiHelper::FT_FLOAT,&style.node_rounding,"node_rounding");
    s.save(ImGuiHelper::FT_FLOAT,&style.node_window_padding.x,"node_window_padding",2);

    tmpColor = ImColor(style.color_node_input_slots);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_node_input_slots",4);
    tmpColor = ImColor(style.color_node_output_slots);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_node_output_slots",4);
    s.save(ImGuiHelper::FT_FLOAT,&style.node_slots_radius,"node_slots_radius");

    tmpColor = ImColor(style.color_link);s.save(ImGuiHelper::FT_COLOR,&tmpColor.x,"color_link",4);
    s.save(ImGuiHelper::FT_FLOAT,&style.link_line_width,"link_line_width");
    s.save(ImGuiHelper::FT_FLOAT,&style.link_control_point_distance,"link_control_point_distance");
    s.save(ImGuiHelper::FT_INT,&style.link_num_segments,"link_num_segments");

    s.save(ImGuiHelper::FT_COLOR,&style.color_node_title.x,"color_node_title",4);
    s.save(ImGuiHelper::FT_COLOR,&style.color_node_input_slots_names.x,"color_node_input_slots_names",4);
    s.save(ImGuiHelper::FT_COLOR,&style.color_node_output_slots_names.x,"color_node_output_slots_names",4);

    return true;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
#include "../imguihelper/imguihelper.h"
static bool StyleParser(ImGuiHelper::FieldType ft,int /*numArrayElements*/,void* pValue,const char* name,void* userPtr)    {
    NodeGraphEditor::Style& s = *((NodeGraphEditor::Style*) userPtr);
    ImVec4& tmp = *((ImVec4*) pValue);  // we cast it soon to float for now...
    switch (ft) {
    case FT_FLOAT:
        if (strcmp(name,"grid_line_width")==0)                              s.grid_line_width = tmp.x;
        else if (strcmp(name,"grid_size")==0)                               s.grid_size = tmp.x;
        else if (strcmp(name,"node_rounding")==0)                           s.node_rounding = tmp.x;
        else if (strcmp(name,"node_window_padding")==0)                     s.node_window_padding = ImVec2(tmp.x,tmp.y);
        else if (strcmp(name,"node_slots_radius")==0)                       s.node_slots_radius = tmp.x;
        else if (strcmp(name,"link_line_width")==0)                         s.link_line_width = tmp.x;
        else if (strcmp(name,"link_control_point_distance")==0)             s.link_control_point_distance = tmp.x;
    break;
    case FT_INT:
        if (strcmp(name,"link_num_segments")==0)                            s.link_num_segments = *((int*)pValue);
    break;
    case FT_COLOR:
        if (strcmp(name,"color_background")==0)                             s.color_background = ImColor(tmp);
        else if (strcmp(name,"color_grid")==0)                              s.color_grid = ImColor(tmp);
        else if (strcmp(name,"color_node")==0)                              s.color_node = ImColor(tmp);
        else if (strcmp(name,"color_node_frame")==0)                        s.color_node_frame = ImColor(tmp);
        else if (strcmp(name,"color_node_selected")==0)                     s.color_node_selected = ImColor(tmp);
        else if (strcmp(name,"color_node_hovered")==0)                      s.color_node_hovered = ImColor(tmp);
        else if (strcmp(name,"color_node_input_slots")==0)                  s.color_node_input_slots = ImColor(tmp);
        else if (strcmp(name,"color_node_output_slots")==0)                 s.color_node_output_slots = ImColor(tmp);
        else if (strcmp(name,"color_link")==0)                              s.color_link = ImColor(tmp);
        else if (strcmp(name,"color_node_title")==0)                        s.color_node_title = ImColor(tmp);
        else if (strcmp(name,"color_node_input_slots_names")==0)            s.color_node_input_slots_names = ImColor(tmp);
        else if (strcmp(name,"color_node_output_slots_names")==0)           s.color_node_output_slots_names = ImColor(tmp);
    break;
    default:
    // TODO: check
    break;
    }
    return false;
}
bool NodeGraphEditor::Style::Load(NodeGraphEditor::Style &style, const char *filename)  {
    ImGuiHelper::Deserializer d(filename);
    if (!d.isValid()) return false;
    d.parse(StyleParser,(void*)&style);
    return true;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#endif //NO_IMGUIHELPER_SERIALIZATION

void NodeGraphEditor::render()
{
    if (!inited) init();
    static const ImVec4 transparent = ImVec4(1,1,1,0);

    const ImGuiIO io = ImGui::GetIO();

    // Draw a list of nodes on the left side
    bool open_context_menu = false,open_delete_only_context_menu = false;
    Node* node_hovered_in_list = NULL;
    Node* node_hovered_in_scene = NULL;

    if (show_left_pane) {
        // Helper stuff for setting up the left splitter
        static ImVec2 lastWindowSize=ImGui::GetWindowSize();      // initial window size
        ImVec2 windowSize = ImGui::GetWindowSize();
        const bool windowSizeChanged = lastWindowSize.x!=windowSize.x || lastWindowSize.y!=windowSize.y;
        if (windowSizeChanged) lastWindowSize = windowSize;
        static float w = lastWindowSize.x*0.2f;                    // initial width of the left window

        //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));

        ImGui::BeginChild("node_list", ImVec2(w,0));

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Node List##node_list_1",NULL,false))   {
            ImGui::Separator();
            for (int node_idx = 0; node_idx < nodes.Size; node_idx++)   {
                Node* node = nodes[node_idx];
                ImGui::PushID((const void*) node);
                if (ImGui::Selectable(node->Name, node == selectedNode)) selectedNode = node;
                if (ImGui::IsItemHovered()) {
                    node_hovered_in_list = node;
                    open_context_menu |= ImGui::IsMouseClicked(1);
                }
                ImGui::PopID();
            }
        }
        ImGui::Separator();
        if (selectedNode)   {
            const char* nodeInfo = selectedNode->getInfo();
            if (nodeInfo && nodeInfo[0]!='\0')  {
            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Selected Node##selectedNode",NULL,false))   {
                ImGui::Separator();
                ImGui::TextWrapped("%s",nodeInfo);
            }
            ImGui::Separator();
            }
        }
        if (show_style_editor)   {
            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Style Editor##styleEditor",NULL,false))   {
                ImGui::Separator();
                ImGui::ColorEditMode(colorEditMode);
                Style::Edit(this->style);
                ImGui::Separator();
#if             (!defined(NO_IMGUIHELPER) && !defined(NO_IMGUIHELPER_SERIALIZATION))
                const char* saveName = "nodeGraphEditor.nge.style";
#               ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
                if (ImGui::SmallButton("Save##saveGNEStyle")) {
                    Style::Save(this->style,saveName);
                }
                ImGui::SameLine();
#               endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#               ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
                if (ImGui::SmallButton("Load##loadGNEStyle")) {
                    Style::Load(this->style,saveName);
                }
                ImGui::SameLine();
#               endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#               endif //NO_IMGUIHELPER_SERIALIZATION

                if (ImGui::SmallButton("Reset##resetGNEStyle")) {
                    Style::Reset(this->style);
                }
            }
            ImGui::Separator();
        }
        ImGui::EndChild();


        // horizontal splitter
        ImGui::SameLine(0);
        static const float splitterWidth = 6.f;

        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(1,1,1,0.2f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(1,1,1,0.35f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(1,1,1,0.5f));
        ImGui::Button("##hsplitter1", ImVec2(splitterWidth,-1));
        ImGui::PopStyleColor(3);
        const bool splitterActive = ImGui::IsItemActive();
        if (ImGui::IsItemHovered() || splitterActive) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        if (splitterActive)  w += ImGui::GetIO().MouseDelta.x;
        if (splitterActive || windowSizeChanged)  {
            const float minw = ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().FramePadding.x;
            const float maxw = minw + windowSize.x - splitterWidth - ImGui::GetStyle().WindowMinSize.x;
            if (w>maxw)         w = maxw;
            else if (w<minw)    w = minw;
        }
        ImGui::SameLine(0);

        //ImGui::PopStyleVar();

    }

    const bool isMouseDraggingForScrolling = ImGui::IsMouseDragging(2, 0.0f);

    ImGui::BeginChild("GraphNodeChildWindow", ImVec2(0,0), true);

    const float& NODE_SLOT_RADIUS = style.node_slots_radius;
    const float NODE_SLOT_RADIUS_SQUARED = (NODE_SLOT_RADIUS*NODE_SLOT_RADIUS);
    const ImVec2& NODE_WINDOW_PADDING = style.node_window_padding;
    const float MOUSE_DELTA_SQUARED = io.MouseDelta.x*io.MouseDelta.x+io.MouseDelta.y*io.MouseDelta.y;
    const float MOUSE_DELTA_SQUARED_THRESHOLD = NODE_SLOT_RADIUS_SQUARED * 0.05f;    // We don't detect "mouse release" events while dragging links onto slots. Instead we check that our mouse delta is small enough. Otherwise we couldn't hover other slots while dragging links.

    // Create our child canvas
    if (show_top_pane)   {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0,0));

        ImGui::Checkbox("Show connection names", &show_connection_names);
        ImGui::SameLine(ImGui::GetWindowWidth()-120);
        ImGui::Checkbox("Show grid", &show_grid);
        ImGui::TextWrapped("%s","Hold MMB to scroll. Double-click LMB on slots to remove their links.\n");
        ImGui::SameLine(ImGui::GetWindowWidth()-120);
        // Color Mode
        static const char* btnlbls[2]={"HSV##myColorBtnType","RGB##myColorBtnType"};
        if (colorEditMode!=ImGuiColorEditMode_RGB)  {
            if (ImGui::SmallButton(btnlbls[0])) {
                colorEditMode = ImGuiColorEditMode_RGB;
                ImGui::ColorEditMode(colorEditMode);
            }
        }
        else if (colorEditMode!=ImGuiColorEditMode_HSV)  {
            if (ImGui::SmallButton(btnlbls[1])) {
                colorEditMode = ImGuiColorEditMode_HSV;
                ImGui::ColorEditMode(colorEditMode);
            }
        }
        ImGui::SameLine(0);ImGui::Text("Color Mode");
        // ------------------
        ImGui::PopStyleVar(2);
    }


    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1,1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, style.color_background);
    ImGui::BeginChild("scrolling_region", ImVec2(0,0), true, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(120.0f);

    ImVec2 offset = ImGui::GetCursorScreenPos() - scrolling;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSplit(2);

    // Display grid
    if (show_grid)
    {
        ImVec2 offset2 = ImGui::GetCursorPos() - scrolling;
        const ImU32& GRID_COLOR = style.color_grid;
        const float& GRID_SZ = style.grid_size;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        for (float x = fmodf(offset2.x,GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
            draw_list->AddLine(ImVec2(x,0.0f)+win_pos, ImVec2(x,canvas_sz.y)+win_pos, GRID_COLOR,style.grid_line_width);
        for (float y = fmodf(offset2.y,GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
            draw_list->AddLine(ImVec2(0.0f,y)+win_pos, ImVec2(canvas_sz.x,y)+win_pos, GRID_COLOR,style.grid_line_width);
    }


    // Display links
    draw_list->ChannelsSetCurrent(0); // Background
    const ImVec2 link_cp(style.link_control_point_distance,0);
    for (int link_idx = 0; link_idx < links.Size; link_idx++)
    {
        NodeLink& link = links[link_idx];
        Node* node_inp = link.InputNode;
        Node* node_out = link.OutputNode;
        ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link.InputSlot);
        ImVec2 p2 = offset + node_out->GetInputSlotPos(link.OutputSlot);
        draw_list->AddBezierCurve(p1, p1+link_cp, p2-link_cp, p2,style.color_link, style.link_line_width, style.link_num_segments);
    }
    // Display dragging link
    const bool cantDragAnything = isMouseDraggingForScrolling;
    bool isLMBDraggingForMakingLinks = !cantDragAnything && ImGui::IsMouseDragging(0, 0.0f);
    bool isDragNodeValid = dragNode.isValid();
    if (isLMBDraggingForMakingLinks && isDragNodeValid)   {
        if (dragNode.inputSlotIdx!=-1)  {   // Dragging from the output slot of dragNode
            ImVec2 p1 = offset + dragNode.node->GetOutputSlotPos(dragNode.inputSlotIdx);
            const ImVec2& p2 = io.MousePos;//offset + node_out->GetInputSlotPos(link.OutputSlot);
            draw_list->AddBezierCurve(p1, p1+link_cp, p2-link_cp, p2, style.color_link, style.link_line_width, style.link_num_segments);
        }
        else if (dragNode.outputSlotIdx!=-1)  {  // Dragging from the input slot of dragNode
            const ImVec2& p1 = io.MousePos;//
            ImVec2 p2 = offset + dragNode.node->GetInputSlotPos(dragNode.outputSlotIdx);
            draw_list->AddBezierCurve(p1, p1+link_cp, p2-link_cp, p2, style.color_link, style.link_line_width, style.link_num_segments);
        }
    }

    // Display nodes
    ImGui::PushStyleColor(ImGuiCol_Header,transparent);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,transparent);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,transparent);
    bool isSomeNodeMoving = false;Node *node_to_fire_edit_callback = NULL,* node_to_paste_from_copy_source = NULL;bool mustDeleteANodeSoon = false;
    ImGui::ColorEditMode(colorEditMode);

    //static const char* btnNames[3]={"v","^","x"};
    //const float textSizeButtonPaste = ImGui::CalcTextSize(btnNames[0]).x;
    //const float textSizeButtonCopy = ImGui::CalcTextSize(btnNames[1]).x;
    const float textSizeButtonX = ImGui::CalcTextSize("x").x;
    for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
    {
        Node* node = nodes[node_idx];
        ImGui::PushID((const void*) node);
        ImVec2 node_rect_min = offset + node->Pos;

        // Display node contents first
        draw_list->ChannelsSetCurrent(1); // Foreground
        bool old_any_active = ImGui::IsAnyItemActive();
        ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);

        bool nodeInEditMode = false;
        ImGui::BeginGroup(); // Lock horizontal position
        ImGui::SetNextTreeNodeOpened(node->isOpen,ImGuiSetCond_Always);
        if (ImGui::TreeNode("##justArrowPlease")) {ImGui::TreePop();node->isOpen = true;}
        else node->isOpen = false;
        ImGui::SameLine(0,0);

        ImGui::PushStyleColor(ImGuiCol_Text,style.color_node_title);
        ImGui::Text("%s",node->Name);
        if (ImGui::IsItemHovered()) {
            const char* tooltip = node->getTooltip();
            if (tooltip && tooltip[0]!='\0') ImGui::SetTooltip("%s",tooltip);
        }
        //ImGui::PopStyleColor();
        // BUTTONS ========================================================
        //const bool canPaste = sourceCopyNode && sourceCopyNode->typeID==node->typeID;
        if (!node->isOpen) ImGui::SameLine();
        else ImGui::SameLine(-scrolling.x+node->Pos.x+node->Size.x-textSizeButtonX-10
                             //-textSizeButtonCopy
                             //-(canPaste?(textSizeButtonPaste):0)
                             );
        static const ImVec4 transparentColor(1,1,1,0);
        ImGui::PushStyleColor(ImGuiCol_Button,transparentColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.75,0.75,0.75,0.5));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(0.75,0.75,0.75,0.77));
        //ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.8,0.7,0.7,1));
        ImGui::PushID("NodeButtons");
        /*
        if (canPaste) {
            if (ImGui::SmallButton(btnNames[0])) {
                node_to_paste_from_copy_source = node_hovered_in_scene = selectedNode = node;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Paste");
            ImGui::SameLine(0);
        }
	if (ImGui::SmallButton(btnNames[1])) {
	    node_hovered_in_scene = selectedNode = node;
	    copyNode(node);
        }
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Copy");
        ImGui::SameLine(0);
	*/
        if (ImGui::SmallButton("x")) {
            node_hovered_in_scene = selectedNode = node;
            if (!hasLinks(node))  mustDeleteANodeSoon=true;
            else open_delete_only_context_menu = true;  // will ask to delete node later
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Delete");
        ImGui::PopID();
        ImGui::PopStyleColor(4);
        //=================================================================

        if (node->isOpen)
        {
            // this code goes into a virtual method==============================
            nodeInEditMode|=node->render();
            //===================================================================
            isLMBDraggingForMakingLinks&=!nodeInEditMode;   // Don't create links while dragging the mouse to edit node values
        }
        ImGui::EndGroup();
        if (nodeInEditMode) node->startEditingTime = -1.f;
        else if (node->startEditingTime!=0.f) {
            if (nodeCallback)   {
                if (node->startEditingTime<0) node->startEditingTime = ImGui::GetTime();
                else if (ImGui::GetTime()-node->startEditingTime>nodeEditedTimeThreshold) {
                    node->startEditingTime = 0.f;
                    node_to_fire_edit_callback = node;
                }
            }
            else node->startEditingTime = 0.f;
        }

        // Save the size of what we have emitted and whether any of the widgets are being used
        bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
        node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
        ImVec2 node_rect_max = node_rect_min + node->Size;

        // Display node box
        draw_list->ChannelsSetCurrent(0); // Background
        ImGui::SetCursorScreenPos(node_rect_min);
        ImGui::InvisibleButton("node##nodeinvbtn", node->Size);
        if (ImGui::IsItemHovered()) {
            node_hovered_in_scene = node;
	    open_context_menu |= ImGui::IsMouseClicked(1);
        }
        bool node_moving_active = !isMouseDraggingForScrolling && !nodeInEditMode && ImGui::IsItemActive();
        if (node_widgets_active || node_moving_active)  selectedNode = node;
        if (node_moving_active && !isDragNodeValid && ImGui::IsMouseDragging(0, 8.0f)) {
            node->Pos = node->Pos + io.MouseDelta;isSomeNodeMoving=true;
        }

        const ImU32& node_bg_color = (node_hovered_in_list == node || node_hovered_in_scene == node) ? style.color_node_hovered :
                                     (selectedNode == node ? style.color_node_selected : style.color_node);
        draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, style.node_rounding);
        draw_list->AddRect(node_rect_min, node_rect_max, style.color_node_frame, style.node_rounding);
        // Display connectors
        const ImVec2 oldCursorScreenPos = ImGui::GetCursorScreenPos();
        const ImVec2 mouseScreenPos = io.MousePos;;
        ImVec2 connectorScreenPos,deltaPos;const bool canDeleteLinks = true;
        const bool mustDeleteLinkIfSlotIsHovered = canDeleteLinks && io.MouseDoubleClicked[0];
        const bool mustDetectIfSlotIsHoveredForDragNDrop = !cantDragAnything && !isSomeNodeMoving && (!isDragNodeValid || isLMBDraggingForMakingLinks);
        ImGui::PushStyleColor(ImGuiCol_Text,style.color_node_input_slots_names);
        for (int slot_idx = 0; slot_idx < node->InputsCount; slot_idx++)    {
            connectorScreenPos = offset + node->GetInputSlotPos(slot_idx);
            draw_list->AddCircleFilled(connectorScreenPos, NODE_SLOT_RADIUS, style.color_node_input_slots);
            if (show_connection_names && node->InputNames[slot_idx][0]!='\0')   {
                const char* name = node->InputNames[slot_idx];
                if (name)   {
                    ImGui::SetCursorScreenPos(offset + node->GetInputSlotPos(slot_idx)-ImVec2(NODE_SLOT_RADIUS,0)-ImGui::CalcTextSize(name));
                    ImGui::Text("%s",name);
                }
            }
            if (mustDetectIfSlotIsHoveredForDragNDrop || mustDeleteLinkIfSlotIsHovered)    {
                deltaPos.x = mouseScreenPos.x-connectorScreenPos.x;
                deltaPos.y = mouseScreenPos.y-connectorScreenPos.y;
                if ((deltaPos.x*deltaPos.x)+(deltaPos.y*deltaPos.y)<NODE_SLOT_RADIUS_SQUARED)   {
                    if (mustDeleteLinkIfSlotIsHovered)  {
                        // remove the link
                        //printf("To be removed: input slot %d.\n",slot_idx);fflush(stdout);
                        for (int link_idx=0;link_idx<links.size();link_idx++)   {
                            NodeLink& link = links[link_idx];
                            if (link.OutputNode == node && slot_idx == link.OutputSlot)   {
                                if (linkCallback) linkCallback(link,LS_DELETED,*this);
                                // remove link
                                if (link_idx+1 < links.size()) link = links[links.size()-1];    // swap with the last link
                                links.resize(links.size()-1);
                                --link_idx;
                            }
                        }
                    }
                    else if (isLMBDraggingForMakingLinks && !isDragNodeValid) {
                        dragNode.node = node;
                        dragNode.outputSlotIdx = slot_idx;
                        dragNode.inputSlotIdx = -1;
                        dragNode.pos = mouseScreenPos;
                        //printf("Start dragging.\n");fflush(stdout);
                    }
                    else if (isDragNodeValid && dragNode.node!=node
                        && MOUSE_DELTA_SQUARED<MOUSE_DELTA_SQUARED_THRESHOLD   // optional... what I wanted is not to end a connection just when I hover another node...
                    ) {
                        // verify compatibility
                        if (dragNode.inputSlotIdx!=-1)  {
                            // drag goes from the output (dragNode.inputSlotIdx) slot of dragNode.node to the input slot of 'node':
                            if (!avoidCircularLinkLoopsInOut || !isNodeReachableFrom(dragNode.node,true,node))  {
                                if (allowOnlyOneLinkPerInputSlot)   {
                                    // Remove all existing node links to node-slot_idx before adding new link:
                                    for (int link_idx=0;link_idx<links.size();link_idx++)   {
                                        NodeLink& link = links[link_idx];
                                        if (link.OutputNode == node && slot_idx == link.OutputSlot)   {
                                            if (linkCallback) linkCallback(link,LS_DELETED,*this);
                                            // remove link
                                            if (link_idx+1 < links.size()) link = links[links.size()-1];    // swap with the last link
                                            links.resize(links.size()-1);
                                            --link_idx;
                                        }
                                    }
                                }
                                // create link
                                addLink(dragNode.node,dragNode.inputSlotIdx,node,slot_idx,true);
                            }
                        // clear dragNode
                        dragNode.node = NULL;
                        dragNode.outputSlotIdx = dragNode.inputSlotIdx = -1;
                        //printf("End dragging.\n");fflush(stdout);
                        }
                    }
                }
            }
        }
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text,style.color_node_output_slots_names);
        for (int slot_idx = 0; slot_idx < node->OutputsCount; slot_idx++)   {
            connectorScreenPos = offset + node->GetOutputSlotPos(slot_idx);
            draw_list->AddCircleFilled(offset + node->GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, style.color_node_output_slots);
            if (show_connection_names && node->OutputNames[slot_idx][0]!='\0')   {
                const char* name = node->OutputNames[slot_idx];
                if (name)   {
                    ImGui::SetCursorScreenPos(offset + node->GetOutputSlotPos(slot_idx)+ImVec2(NODE_SLOT_RADIUS,0)-ImVec2(0,ImGui::CalcTextSize(name).y));
                    ImGui::Text("%s",name);
                }
            }
            if (mustDetectIfSlotIsHoveredForDragNDrop || mustDeleteLinkIfSlotIsHovered)    {
                deltaPos.x = mouseScreenPos.x-connectorScreenPos.x;
                deltaPos.y = mouseScreenPos.y-connectorScreenPos.y;
                if ((deltaPos.x*deltaPos.x)+(deltaPos.y*deltaPos.y)<NODE_SLOT_RADIUS_SQUARED)   {
                    if (mustDeleteLinkIfSlotIsHovered)  {
                        // remove the link
                        //printf("To be removed: output slot %d.\n",slot_idx);fflush(stdout);
                        for (int link_idx=0;link_idx<links.size();link_idx++)   {
                            NodeLink& link = links[link_idx];
                            if (link.InputNode == node && slot_idx == link.InputSlot)   {
                                if (linkCallback) linkCallback(link,LS_DELETED,*this);
                                // remove link
                                if (link_idx+1 < links.size()) link = links[links.size()-1];    // swap with the last link
                                links.resize(links.size()-1);
                                --link_idx;
                            }
                        }
                    }
                    else if (isLMBDraggingForMakingLinks && !isDragNodeValid) {
                        dragNode.node = node;
                        dragNode.inputSlotIdx = slot_idx;
                        dragNode.outputSlotIdx = -1;
                        dragNode.pos = mouseScreenPos;
                        //printf("Start dragging.\n");fflush(stdout);
                    }
                    else if (isDragNodeValid && dragNode.node!=node
                             && MOUSE_DELTA_SQUARED<MOUSE_DELTA_SQUARED_THRESHOLD    // optional... what I wanted is not to end a connection just when I hover another node...
                    ) {
                        // verify compatibility
                        if (dragNode.outputSlotIdx!=-1)  {
                            // drag goes from the output slot_idx of node to the input slot (dragNode.outputSlotIdx) of dragNode.node:
                            if (!avoidCircularLinkLoopsInOut || !isNodeReachableFrom(node,true,dragNode.node))    {
                                if (allowOnlyOneLinkPerInputSlot)   {
                                    // Remove all existing node links to dragNode before adding new link:
                                    for (int link_idx=0;link_idx<links.size();link_idx++)   {
                                        NodeLink& link = links[link_idx];
                                        if (link.OutputNode == dragNode.node && dragNode.outputSlotIdx == link.OutputSlot)   {
                                            if (linkCallback) linkCallback(link,LS_DELETED,*this);
                                            // remove link
                                            if (link_idx+1 < links.size()) link = links[links.size()-1];    // swap with the last link
                                            links.resize(links.size()-1);
                                            --link_idx;
                                        }
                                    }
                                }
                                // create link
                                addLink(node,slot_idx,dragNode.node,dragNode.outputSlotIdx,true);
                            }
                        // clear dragNode
                        dragNode.node = NULL;
                        dragNode.outputSlotIdx = dragNode.inputSlotIdx = -1;
                        //printf("End dragging.\n");fflush(stdout);
                        }
                    }
                }
            }
        }
        ImGui::PopStyleColor();
        if (!isLMBDraggingForMakingLinks) {
            dragNode.node = NULL; // clear dragNode
            //printf("Quit dragging.\n");fflush(stdout);
        }

        ImGui::SetCursorScreenPos(oldCursorScreenPos);
        ImGui::PopID();
    }
    ImGui::PopStyleColor(3);
    draw_list->ChannelsMerge();

    // Open context menu
    if (!open_context_menu && selectedNode && (selectedNode==node_hovered_in_list || selectedNode==node_hovered_in_scene) && (ImGui::IsKeyReleased(io.KeyMap[ImGuiKey_Delete]) || mustDeleteANodeSoon)) {
        // Delete selected node directly:
        if (selectedNode==node_to_fire_edit_callback) node_to_fire_edit_callback = NULL;
        if (selectedNode==node_to_paste_from_copy_source) node_to_paste_from_copy_source = NULL;
        deleteNode(selectedNode);
        selectedNode = node_hovered_in_list = node_hovered_in_scene = NULL;
    }
    else if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && getNumAvailableNodeTypes()>0 && nodeFactoryFunctionPtr)   {
        if (ImGui::IsMouseClicked(1))   {   // Open context menu
	    selectedNode = node_hovered_in_list = node_hovered_in_scene = NULL;
            open_context_menu = true;
        }
    }
    if (open_delete_only_context_menu)  {
        ImGui::OpenPopup("delete_only_context_menu");
        if (node_hovered_in_list) selectedNode = node_hovered_in_list;
        if (node_hovered_in_scene) selectedNode = node_hovered_in_scene;
    }
    if (open_context_menu)  {
	ImGui::OpenPopup("context_menu");
        if (node_hovered_in_list) selectedNode = node_hovered_in_list;
        if (node_hovered_in_scene) selectedNode = node_hovered_in_scene;
    }


    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,8));
    if (ImGui::BeginPopup("delete_only_context_menu"))  {
        Node* node = selectedNode;
        if (node)   {
            ImGui::Text("Node '%s'", node->Name);
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", NULL, false, true)) {
            if (node==node_to_fire_edit_callback) node_to_fire_edit_callback = NULL;
            if (node==node_to_paste_from_copy_source) node_to_paste_from_copy_source = NULL;
                //printf("Current nodes.size()=%d; Deleting node %s.\n",nodes.size(),node->Name);fflush(stdout);
            deleteNode(node);
            }
        }
        ImGui::EndPopup();
    }
    else if (ImGui::BeginPopup("context_menu"))  {
        Node* node = selectedNode;
        ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
        if (node)   {
            ImGui::Text("Node '%s'", node->Name);
            ImGui::Separator();
            //if (ImGui::MenuItem("Rename..", NULL, false, false)) {}
	    if (ImGui::MenuItem("Copy", NULL, false, true)) copyNode(node);
            if (sourceCopyNode && sourceCopyNode->typeID==node->typeID) {
                if (ImGui::MenuItem("Paste", NULL, false, true)) {
                    node_to_paste_from_copy_source = node;
                }
            }
            if (ImGui::MenuItem("Delete", NULL, false, true)) {
                if (node==node_to_fire_edit_callback) node_to_fire_edit_callback = NULL;
                if (node==node_to_paste_from_copy_source) node_to_paste_from_copy_source = NULL;
                //printf("Current nodes.size()=%d; Deleting node %s.\n",nodes.size(),node->Name);fflush(stdout);
                deleteNode(node);
            }
            //if (ImGui::MenuItem("Copy", NULL, false, false)) {}
        }
        else    {
            /*if (ImGui::MenuItem("Add ExampleNode")) {
                addNode(ExampleNode::Create(scene_pos,0.5f, ImColor(100,100,200)));
            }*/
            ImGui::Text("%s","Add Node Menu");
            ImGui::Separator();
            if (nodeFactoryFunctionPtr) {
                if (sourceCopyNode) {
		    if (ImGui::MenuItem("Paste##cloneCopySource")) {
                        Node* clonedNode = addNode(nodeFactoryFunctionPtr(sourceCopyNode->typeID,scene_pos));
                        clonedNode->fields.copyValuesFrom(sourceCopyNode->fields);
                    }
                    ImGui::Separator();
                }
                for (int nt=0,ntSize=getNumAvailableNodeTypes();nt<ntSize;nt++) {
                    ImGui::PushID(nt);
                    if (ImGui::MenuItem(pNodeTypeNames[availableNodeTypes[nt]])) {
                        addNode(nodeFactoryFunctionPtr(availableNodeTypes[nt],scene_pos));
                    }
                    ImGui::PopID();
                }
            }
            //if (ImGui::MenuItem("Paste", NULL, false, false)) {}
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();



    ImGui::PopItemWidth();

    // Scrolling
    //if (!isSomeNodeMoving && !isaNodeInActiveState && !dragNode.node && ImGui::IsWindowHovered() &&  ImGui::IsMouseDragging(0, 6.0f)) scrolling = scrolling - io.MouseDelta;
    if (isMouseDraggingForScrolling /*&& ImGui::IsWindowHovered()*/ && (ImGui::IsWindowHovered() || ImGui::IsWindowFocused() || ImGui::IsRootWindowFocused())) scrolling = scrolling - io.MouseDelta;


    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    ImGui::EndChild();  // GraphNodeChildWindow

    if (node_to_paste_from_copy_source && sourceCopyNode && node_to_paste_from_copy_source->typeID==sourceCopyNode->typeID)
        node_to_paste_from_copy_source->fields.copyValuesFrom(sourceCopyNode->fields);
    if (nodeCallback && node_to_fire_edit_callback) nodeCallback(node_to_fire_edit_callback,NS_EDITED,*this);


}

void NodeGraphEditor::registerNodeTypes(const char *nodeTypeNames[], int numNodeTypeNames, NodeFactoryDelegate _nodeFactoryFunctionPtr, const int *pOptionalNodeTypesToUse, int numNodeTypesToUse)
{
    this->numNodeTypeNames = numNodeTypeNames;
    this->pNodeTypeNames = numNodeTypeNames>0 ? &nodeTypeNames[0] : NULL;
    this->nodeFactoryFunctionPtr = _nodeFactoryFunctionPtr;
    this->availableNodeTypes.clear();
    if (numNodeTypesToUse>numNodeTypeNames) numNodeTypesToUse = numNodeTypeNames;
    if (pOptionalNodeTypesToUse && numNodeTypesToUse>0) {
        this->availableNodeTypes.resize(numNodeTypesToUse);
        for (int i=0;i<numNodeTypesToUse;i++) this->availableNodeTypes[i] = pOptionalNodeTypesToUse[i];
    }
    else if (numNodeTypeNames>0)    {
        this->availableNodeTypes.resize(numNodeTypeNames);
        for (int i=0;i<numNodeTypeNames;i++) this->availableNodeTypes[i] = i;
    }
    // Is it possible to sort "this->availableNodeTypes" based on "this->pNodeTypeNames",
    // so that it display is elements in alphabetical order ?
}

bool NodeGraphEditor::removeLinkAt(int link_idx) {
    if (link_idx<0 || link_idx>=links.size()) return false;
    // remove link
    NodeLink& link = links[link_idx];
    if (linkCallback) linkCallback(link,LS_DELETED,*this);
    if (link_idx+1 < links.size()) link = links[links.size()-1];    // swap with the last link
    links.resize(links.size()-1);
    return true;
}
void NodeGraphEditor::removeAnyLinkFromNode(Node* node, bool removeInputLinks, bool removeOutputLinks)  {
    for (int link_idx=0;link_idx<links.size();link_idx++)    {
        NodeLink& link = links[link_idx];
        if ((removeOutputLinks && link.InputNode==node) || (removeInputLinks && link.OutputNode==node))    {
            // remove link
            if (linkCallback) linkCallback(link,LS_DELETED,*this);
            if (link_idx+1 < links.size()) link = links[links.size()-1];    // swap with the last link
            links.resize(links.size()-1);
            --link_idx;
        }
    }
}
bool NodeGraphEditor::isLinkPresent(Node *inputNode, int input_slot, Node *outputNode, int output_slot,int* pOptionalIndexInLinkArrayOut) const  {
    if (pOptionalIndexInLinkArrayOut) *pOptionalIndexInLinkArrayOut=-1;
    for (int link_idx=0;link_idx<links.size();link_idx++)    {
        const NodeLink& l = links[link_idx];
        if (l.InputNode==inputNode && l.InputSlot==input_slot &&
            l.OutputNode==outputNode && l.OutputSlot==output_slot) {
            if (pOptionalIndexInLinkArrayOut) *pOptionalIndexInLinkArrayOut=link_idx;
            return true;
        }
    }
    return false;
}
bool NodeGraphEditor::hasLinks(Node *node) const    {
    for (int i=0,isz=links.size();i<isz;i++)    {
        const NodeLink& l = links[i];
        if (l.InputNode==node || l.OutputNode==node) return true;
    }
    return false;
}

void NodeGraphEditor::copyNode(Node *n)	{
    const bool mustDeleteSourceCopyNode = sourceCopyNode && (!n || n->typeID!=sourceCopyNode->typeID);
    if (mustDeleteSourceCopyNode)   {
	sourceCopyNode->~Node();              // ImVector does not call it
	ImGui::MemFree(sourceCopyNode);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
	sourceCopyNode = NULL;
    }
    if (!n) return;
    if (!sourceCopyNode)    {
	if (!nodeFactoryFunctionPtr) return;
	sourceCopyNode = nodeFactoryFunctionPtr(n->typeID,ImVec2(0,0));
    }
    sourceCopyNode->fields.copyValuesFrom(n->fields);
}

void NodeGraphEditor::getInputNodesForNodeAndSlot(const Node* node,int input_slot,ImVector<Node *> &returnValueOut, ImVector<int> *pOptionalReturnValueOutputSlotOut) const  {
    returnValueOut.clear();if (pOptionalReturnValueOutputSlotOut) pOptionalReturnValueOutputSlotOut->clear();
    for (int link_idx=0,link_idx_size=links.size();link_idx<link_idx_size;link_idx++)   {
        const NodeLink& link = links[link_idx];
        if (link.OutputNode == node && link.OutputSlot == input_slot)  {
            returnValueOut.push_back(link.OutputNode);
            if (pOptionalReturnValueOutputSlotOut) pOptionalReturnValueOutputSlotOut->push_back(link.OutputSlot);
        }
    }
}
Node* NodeGraphEditor::getInputNodeForNodeAndSlot(const Node* node,int input_slot,int* pOptionalReturnValueOutputSlotOut) const    {
    if (pOptionalReturnValueOutputSlotOut) *pOptionalReturnValueOutputSlotOut=-1;
    for (int link_idx=0,link_idx_size=links.size();link_idx<link_idx_size;link_idx++)   {
        const NodeLink& link = links[link_idx];
        if (link.OutputNode == node && link.OutputSlot == input_slot)  {
            if (pOptionalReturnValueOutputSlotOut) *pOptionalReturnValueOutputSlotOut = link.OutputSlot;
            return link.OutputNode;
        }
    }
    return NULL;
}
void NodeGraphEditor::getOutputNodesForNodeAndSlot(const Node* node,int output_slot,ImVector<Node *> &returnValueOut, ImVector<int> *pOptionalReturnValueInputSlotOut) const {
    returnValueOut.clear();if (pOptionalReturnValueInputSlotOut) pOptionalReturnValueInputSlotOut->clear();
    for (int link_idx=0,link_idx_size=links.size();link_idx<link_idx_size;link_idx++)   {
        const NodeLink& link = links[link_idx];
        if (link.InputNode == node && link.InputSlot == output_slot)  {
            returnValueOut.push_back(link.InputNode);
            if (pOptionalReturnValueInputSlotOut) pOptionalReturnValueInputSlotOut->push_back(link.InputSlot);
        }
    }
}
bool NodeGraphEditor::isNodeReachableFrom(const Node *node1, int slot1, bool goBackward,const Node* nodeToFind,int* pOptionalNodeToFindSlotOut) const    {

    for (int i=0,isz=links.size();i<isz;i++)    {
        const NodeLink& l = links[i];
        if (goBackward)  {
            if (l.OutputNode == node1 && l.OutputSlot == slot1) {
                if (l.InputNode == nodeToFind) {
                    if (pOptionalNodeToFindSlotOut) *pOptionalNodeToFindSlotOut = l.InputSlot;
                    return true;
                }
                if (isNodeReachableFrom(l.InputNode,goBackward,nodeToFind,pOptionalNodeToFindSlotOut)) return true;
            }
        }
        else {
            if (l.InputNode == node1 && l.InputSlot == slot1) {
                if (l.OutputNode == nodeToFind) {
                    if (pOptionalNodeToFindSlotOut) *pOptionalNodeToFindSlotOut = l.OutputSlot;
                    return true;
                }
                if (isNodeReachableFrom(l.OutputNode,goBackward,nodeToFind,pOptionalNodeToFindSlotOut)) return true;
            }
        }
    }
    return false;
}
bool NodeGraphEditor::isNodeReachableFrom(const Node *node1, bool goBackward,const Node* nodeToFind,int* pOptionalNode1SlotOut,int* pOptionalNodeToFindSlotOut) const  {
    if (pOptionalNode1SlotOut) *pOptionalNode1SlotOut=-1;
    for (int i=0,isz=(goBackward ? node1->InputsCount : node1->OutputsCount);i<isz;i++)  {
        if (isNodeReachableFrom(node1,i,goBackward,nodeToFind,pOptionalNodeToFindSlotOut))  {
            if (pOptionalNode1SlotOut) *pOptionalNode1SlotOut=i;
            return true;
        }
    }
    return false;
}

void Node::init(const char *name, const ImVec2 &pos, const char *inputSlotNamesSeparatedBySemicolons, const char *outputSlotNamesSeparatedBySemicolons, int _nodeTypeID) {
    strncpy(Name, name, IMGUINODE_MAX_NAME_LENGTH); Name[IMGUINODE_MAX_NAME_LENGTH-1] = '\0'; Pos = pos;
    InputsCount = 0; OutputsCount = 0;
    const char *input_names = inputSlotNamesSeparatedBySemicolons, *output_names = outputSlotNamesSeparatedBySemicolons;
    const char *tmp = NULL,*tmp2 = NULL;int length;

    tmp = input_names;tmp2 = NULL;
    if (tmp && strlen(tmp)>0)    {
        while ((tmp2=strchr(tmp,(int)';')) && InputsCount<IMGUINODE_MAX_INPUT_SLOTS)    {
            length = (int) (tmp2-tmp);if (length>IMGUINODE_MAX_SLOT_NAME_LENGTH) length=IMGUINODE_MAX_SLOT_NAME_LENGTH;
            strncpy(InputNames[InputsCount],tmp, length);
            InputNames[InputsCount][length] = '\0';
            ++InputsCount;tmp = ++tmp2;
        }
        if (tmp && InputsCount<IMGUINODE_MAX_INPUT_SLOTS)    {
            length = (int) strlen(tmp);if (length>IMGUINODE_MAX_SLOT_NAME_LENGTH) length=IMGUINODE_MAX_SLOT_NAME_LENGTH;
            strncpy(InputNames[InputsCount],tmp, length);
            InputNames[InputsCount][length] = '\0';
            ++InputsCount;
        }
    }
    tmp = output_names;tmp2 = NULL;
    if (tmp && strlen(tmp)>0)    {
        while ((tmp2=strchr(tmp,(int)';')) && OutputsCount<IMGUINODE_MAX_OUTPUT_SLOTS)    {
            length = (int) (tmp2-tmp);if (length>IMGUINODE_MAX_SLOT_NAME_LENGTH) length=IMGUINODE_MAX_SLOT_NAME_LENGTH;
            strncpy(OutputNames[OutputsCount],tmp, length);
            OutputNames[OutputsCount][length] = '\0';
            ++OutputsCount;tmp = ++tmp2;
        }
        if (tmp && OutputsCount<IMGUINODE_MAX_OUTPUT_SLOTS)    {
            length = (int) strlen(tmp);if (length>IMGUINODE_MAX_SLOT_NAME_LENGTH) length=IMGUINODE_MAX_SLOT_NAME_LENGTH;
            strncpy(OutputNames[OutputsCount],tmp, length);
            OutputNames[OutputsCount][length] = '\0';
            ++OutputsCount;
        }
    }
    typeID = _nodeTypeID;
    user_ptr = NULL;userID=-1;
    startEditingTime = 0;
    isOpen = true;
}

bool FieldInfo::copyFrom(const FieldInfo &f) {
    if (!isCompatibleWith(f)) return false;
    void* myOldPdata = pdata;
    *this = f;
    pdata = myOldPdata;
    return copyPDataValueFrom(f);
}
bool FieldInfo::copyPDataValueFrom(const FieldInfo &f) {
    if (!isCompatibleWith(f) || (!pdata || !f.pdata)) return false;
    if (type==FT_CUSTOM)    {
        if (!copyFieldDelegate) return false;
        else return copyFieldDelegate(*this,f);
    }
    switch (type) {
    case FT_INT:
    case FT_ENUM:
    case FT_UNSIGNED:
    case FT_FLOAT:
    case FT_DOUBLE:
    case FT_COLOR:
    {
        const int numElements = numArrayElements<=0 ? 1 : numArrayElements;
        const size_t elemSize = ((type==FT_INT||type==FT_ENUM)?sizeof(int):
                                                               type==FT_UNSIGNED?sizeof(unsigned):
                                                                                 (type==FT_FLOAT || type==FT_COLOR)?sizeof(float):
                                                                                                                    type==FT_DOUBLE?sizeof(double):
                                                                                                                                    0);
        memcpy(pdata,f.pdata,numElements*elemSize);
    }
        break;
    case FT_BOOL: *((bool*)pdata) = *((bool*)f.pdata);
        break;
    case FT_STRING: memcpy(pdata,f.pdata,precision < f.precision ? precision : f.precision);
        break;
    default:
        //IM_ASSERT(true); // copyPDataValueFrom(...) not defined for this type [we can probably just skip this]
        return false;
    }
    return true;
}
FieldInfo &FieldInfoVector::addFieldInt(void *pdata, int numArrayElements, const char *label, const char *tooltip, int precision, int lowerLimit, int upperLimit, void *userData)   {
    IM_ASSERT(pdata && numArrayElements<=4);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_INT,pdata,label,tooltip,precision,numArrayElements,(double)lowerLimit,(double)upperLimit);
    f.userData = userData;
    return f;
}
FieldInfo &FieldInfoVector::addFieldUnsigned(void *pdata, int numArrayElements, const char *label, const char *tooltip, int precision, unsigned lowerLimit, unsigned upperLimit, void *userData)   {
    IM_ASSERT(pdata && numArrayElements<=4);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_UNSIGNED,pdata,label,tooltip,precision,numArrayElements,(double)lowerLimit,(double)upperLimit);
    f.userData = userData;
    return f;
}
FieldInfo &FieldInfoVector::addFieldFloat(void *pdata, int numArrayElements, const char *label, const char *tooltip, int precision, float lowerLimit, float upperLimit, void *userData, bool needsRadiansToDegs)   {
    IM_ASSERT(pdata && numArrayElements<=4);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_FLOAT,pdata,label,tooltip,precision,numArrayElements,(double)lowerLimit,(double)upperLimit,needsRadiansToDegs);
    f.userData = userData;
    return f;
}
FieldInfo &FieldInfoVector::addFieldDouble(void *pdata, int numArrayElements, const char *label, const char *tooltip, int precision, double lowerLimit, double upperLimit, void *userData, bool needsRadiansToDegs)   {
    IM_ASSERT(pdata && numArrayElements<=4);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_DOUBLE,pdata,label,tooltip,precision,numArrayElements,(double)lowerLimit,(double)upperLimit,needsRadiansToDegs);
    f.userData = userData;
    return f;
}
FieldInfo &FieldInfoVector::addFieldText(void *pdata, int textLength, const char *label, const char *tooltip, bool readOnly, bool multiline, void *userData)   {
    IM_ASSERT(pdata);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_STRING,pdata,label,tooltip,textLength,readOnly?1:0,(double) (multiline?1:0),(double)0);
    f.userData = userData;
    return f;
}
FieldInfo &FieldInfoVector::addFieldEnum(void *pdata, int numEnumElements, FieldInfo::TextFromEnumDelegate textFromEnumFunctionPtr, const char *label, const char *tooltip, void *userData)   {
    IM_ASSERT(pdata && numEnumElements>0 && textFromEnumFunctionPtr);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_ENUM,pdata,label,tooltip,0,0,0,1,false,numEnumElements,textFromEnumFunctionPtr,userData);
    return f;
}
FieldInfo &FieldInfoVector::addFieldBool(void *pdata, const char *label, const char *tooltip, void *userData)   {
    IM_ASSERT(pdata);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_BOOL,pdata,label,tooltip);
    f.userData = userData;
    return f;
}
FieldInfo &FieldInfoVector::addFieldColor(void *pdata, bool useAlpha, const char *label, const char *tooltip, int precision, void *userData)   {
    IM_ASSERT(pdata);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_COLOR,pdata,label,tooltip,precision,useAlpha?4:3,0,1);
    f.userData = userData;
    return f;
}
FieldInfo &FieldInfoVector::addFieldCustom(FieldInfo::RenderFieldDelegate renderFieldDelegate,FieldInfo::CopyFieldDelegate copyFieldDelegate, void *userData)   {
    IM_ASSERT(renderFieldDelegate);
    push_back(FieldInfo());
    FieldInfo& f = (*this)[size()-1];
    f.init(FT_CUSTOM);
    f.renderFieldDelegate=renderFieldDelegate;
    f.copyFieldDelegate=copyFieldDelegate;
    f.userData = userData;
    return f;
}
bool FieldInfoVector::render()   {
    static const int precisionStrSize = 16;static char precisionStr[precisionStrSize];int precisionLastCharIndex;
    static const float dragSpeed = 1.f;

    bool nodeEdited = false;
    for (int i=0,isz=size();i<isz;i++)   {
        FieldInfo& f = (*this)[i];
        ImGui::PushID((const void*) &f);

        const char* label = (/*f.label &&*/ f.label[0]!='\0') ? &f.label[0] : "##DummyLabel";
        if (f.precision>0) {
            strcpy(precisionStr,"%.");
            snprintf(&precisionStr[2], precisionStrSize-2,"%ds",f.precision);
            precisionLastCharIndex = strlen(precisionStr)-1;
        }
        else {
            strcpy(precisionStr,"%s");
            precisionLastCharIndex = 1;
        }

        bool changed = false;
        switch (f.type) {
        case FT_DOUBLE: {
            precisionStr[precisionLastCharIndex]='f';
            const float minValue = (float) f.minValue;
            const float maxValue = (float) f.maxValue;
            const double rtd = f.needsRadiansToDegs ? GetRadiansToDegs<double>() : 1.f;
            const double dtr = f.needsRadiansToDegs ? GetDegsToRadians<double>() : 1.f;
            double* pField = (double*)f.pdata;
            float value[4] = {0,0,0,0};
            for (int vl=0;vl<f.numArrayElements;vl++) {
                value[vl] = (float) ((*(pField+vl))*rtd);
            }
            switch (f.numArrayElements)    {
            case 2: changed = ImGui::DragFloat2(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            case 3: changed = ImGui::DragFloat3(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            case 4: changed = ImGui::DragFloat4(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            default: changed = ImGui::DragFloat(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            }
            if (changed)    {
                for (int vl=0;vl<f.numArrayElements;vl++) {
                    *(pField+vl) = (double) value[vl] * dtr;
                }
            }

        }
            break;
        case FT_FLOAT: {
            precisionStr[precisionLastCharIndex]='f';
            const float minValue = (float) f.minValue;
            const float maxValue = (float) f.maxValue;
            const float rtd = f.needsRadiansToDegs ? GetRadiansToDegs<float>() : 1.f;
            const float dtr = f.needsRadiansToDegs ? GetDegsToRadians<float>() : 1.f;
            float* pField = (float*)f.pdata;
            float value[4] = {0,0,0,0};
            for (int vl=0;vl<f.numArrayElements;vl++) {
                value[vl] = (float) ((*(pField+vl))*rtd);
            }
            bool changed = false;
            switch (f.numArrayElements)    {
            case 2: changed = ImGui::DragFloat2(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            case 3: changed = ImGui::DragFloat3(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            case 4: changed = ImGui::DragFloat4(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            default: changed = ImGui::DragFloat(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            }
            if (changed)    {
                for (int vl=0;vl<f.numArrayElements;vl++) {
                    *(pField+vl) = (float) value[vl]*dtr;
                }
            }
        }
            break;
        case FT_UNSIGNED: {
            //precisionStr[precisionLastCharIndex]='d';
            const int minValue = (int) f.minValue;
            const int maxValue = (int) f.maxValue;
            unsigned* pField = (unsigned*) f.pdata;
            int value[4] = {0,0,0,0};
            for (int vl=0;vl<f.numArrayElements;vl++) {
                value[vl] = (int) *(pField+vl);
            }
            bool changed = false;
            switch (f.numArrayElements)    {
            case 2: changed = ImGui::DragInt2(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            case 3: changed = ImGui::DragInt3(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            case 4: changed = ImGui::DragInt4(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            default: changed = ImGui::DragInt(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            }
            if (changed)    {
                for (int vl=0;vl<f.numArrayElements;vl++) {
                    *(pField+vl) = (unsigned) value[vl];
                }
            }
        }
            break;
        case FT_INT: {
            //precisionStr[precisionLastCharIndex]='d';
            const int minValue = (int) f.minValue;
            const int maxValue = (int) f.maxValue;
            int* pField = (int*) f.pdata;
            int value[4] = {0,0,0,0};
            for (int vl=0;vl<f.numArrayElements;vl++) {
                value[vl] = (int) *(pField+vl);
            }
            bool changed = false;
            switch (f.numArrayElements)    {
            case 2: changed = ImGui::DragInt2(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            case 3: changed = ImGui::DragInt3(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            case 4: changed = ImGui::DragInt4(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            default: changed = ImGui::DragInt(label,value,dragSpeed,minValue,maxValue,precisionStr);break;
            }
            if (changed)    {
                for (int vl=0;vl<f.numArrayElements;vl++) {
                    *(pField+vl) = (int) value[vl];
                }
            }
        }
            break;
        case FT_BOOL:   {
            bool * boolPtr = (bool*) f.pdata;
            changed|=ImGui::Checkbox(label,boolPtr);
        }
            break;
        case FT_ENUM: {
            changed|=ImGui::Combo(label,(int*) f.pdata,f.textFromEnumFunctionPointer,f.userData,f.numEnumElements);
        }
            break;
        case FT_STRING: {
            char* txtField = (char*)  f.pdata;
            const bool readOnly = f.numArrayElements==1;
            const bool multiline = f.minValue==1;
            if (!multiline) changed|=ImGui::InputText(label,txtField,f.precision,ImGuiInputTextFlags_EnterReturnsTrue || (readOnly ? ImGuiInputTextFlags_ReadOnly : 1));
            else changed|=ImGui::InputTextMultiline(label,txtField,f.precision,ImVec2(0,0),ImGuiInputTextFlags_EnterReturnsTrue || (readOnly ? ImGuiInputTextFlags_ReadOnly : 1));
        }
            break;
        case FT_COLOR:  {
            float* pColor = (float*) f.pdata;
            if (f.numArrayElements==3) changed|=ImGui::ColorEdit3(label,pColor);
            else changed|=ImGui::ColorEdit4(label,pColor);
        }
        case FT_CUSTOM: {
            if (f.renderFieldDelegate) changed = f.renderFieldDelegate(f);
        }
            break;
        default:
            IM_ASSERT(true);    // should never happen
            break;
        }
        if (f.type!=FT_CUSTOM)  {
            if (/*f.tooltip &&*/ f.tooltip[0]!='\0' && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",f.tooltip);
        }
        nodeEdited|=changed;
        ImGui::PopID();
    }
    return nodeEdited;
}


#ifndef IMGUINODEGRAPHEDITOR_NOTESTDEMO
enum MyNodeTypes {
    MNT_COLOR_NODE = 0,
    MNT_COMBINE_NODE,
    MNT_COMPLEX_NODE,
    MNT_COUNT
};
static const char* MyNodeTypeNames[MNT_COUNT] = {"Color","Combine","Complex"};  // used in the "add Node" menu (and optionally as node title names)
class ColorNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef ColorNode ThisClass;
    ColorNode() : Base() {}
    static const int TYPE = MNT_COLOR_NODE;

    ImVec4 Color;       // field

    // Support static method for enumIndex (the signature is the same used by ImGui::Combo(...))
    static bool GetTextFromEnumIndex(void* ,int value,const char** pTxt) {
        if (!pTxt) return false;
        static const char* values[] = {"APPLE","LEMON","ORANGE"};
        static int numValues = (int)(sizeof(values)/sizeof(values[0]));
        if (value>=0 && value<numValues) *pTxt = values[value];
        else *pTxt = "UNKNOWN";
        return true;
    }

    virtual const char* getTooltip() const {return "ColorNode tooltip.";}
    virtual const char* getInfo() const {return "ColorNode info.\n\nThis is supposed to display some info about this node.";}

    public:

    // create:
    static ThisClass* Create(const ImVec2& pos) {
        // 1) allocation
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs. Requires: #include <new>. Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));new (node) ThisClass();

        // 2) main init
        node->init("ColorNode",pos,"","r;g;b;a",TYPE);

        // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addFieldColor((void*)&node->Color,true,"Color","color with alpha");

        // 4) set (or load) field values
        node->Color = ImColor(255,255,0,255);

        return node;
    }

    // casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
};
class CombineNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef CombineNode ThisClass;
    CombineNode() : Base() {}
    static const int TYPE = MNT_COMBINE_NODE;

    float fraction;

    virtual const char* getTooltip() const {return "CombineNode tooltip.";}
    virtual const char* getInfo() const {return "CombineNode info.\n\nThis is supposed to display some info about this node.";}

    public:

    // create:
    static ThisClass* Create(const ImVec2& pos) {
        // 1) allocation
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs. Requires: #include <new>. Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));new (node) ThisClass();

        // 2) main init
        node->init("CombineNode",pos,"in1;in2","out",TYPE);

        // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addFieldFloat((void*)&node->fraction,1,"Fraction","Fraction of in1 that is mixed with in2",2,0,1);

        // 4) set (or load) field values
        node->fraction = 0.5f;

        return node;
    }

    // casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}


};
class ComplexNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef ComplexNode ThisClass;
    ComplexNode() : Base() {}
    static const int TYPE = MNT_COMPLEX_NODE;

    float Value[3];     // field 1
    ImVec4 Color;       // field 2
    int enumIndex;      // field 3

    // Support static method for enumIndex (the signature is the same used by ImGui::Combo(...))
    static bool GetTextFromEnumIndex(void* ,int value,const char** pTxt) {
        if (!pTxt) return false;
        static const char* values[] = {"APPLE","LEMON","ORANGE"};
        static int numValues = (int)(sizeof(values)/sizeof(values[0]));
        if (value>=0 && value<numValues) *pTxt = values[value];
        else *pTxt = "UNKNOWN";
        return true;
    }

    virtual const char* getTooltip() const {return "ComplexNode tooltip.";}
    virtual const char* getInfo() const {return "ComplexNode info.\n\nThis is supposed to display some info about this node.";}

    public:

    // create:
    static ThisClass* Create(const ImVec2& pos) {
        // 1) allocation
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs. Requires: #include <new>. Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));new (node) ThisClass();

        // 2) main init
        node->init("ComplexNode",pos,"in1;in2;in3","out1;out2",TYPE);

        // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addFieldFloat((void*)&node->Value[0],3,"Angles","Three floats that are stored in radiant units internally",2,0,360,NULL,true);
        node->fields.addFieldColor((void*)&node->Color,true,"Color","color with alpha");
        node->fields.addFieldEnum(&node->enumIndex,3,&GetTextFromEnumIndex,"Fruit","Choose your favourite");

        // 4) set (or load) field values
        node->Value[0] = 0;node->Value[1] = 3.14f; node->Value[2] = 4.68f;
        node->Color = ImColor(126,200,124,230);
        node->enumIndex = 1;

        return node;
    }

    // helper casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
};
static Node* MyNodeFactory(int nt,const ImVec2& pos) {
    switch (nt) {
    case MNT_COLOR_NODE: return ColorNode::Create(pos);
    case MNT_COMBINE_NODE: return CombineNode::Create(pos);
    case MNT_COMPLEX_NODE: return ComplexNode::Create(pos);
    default:
    IM_ASSERT(true);    // Missing node type creation
    return NULL;
    }
    return NULL;
}
void TestNodeGraphEditor()  {
    static ImGui::NodeGraphEditor nge;
    if (nge.mustInit())	{
        nge.init(); // So nge.mustInit() returns false next time [currently it's optional, since render() calls it anyway]
        // Optional: starting nodes and links (TODO: load from file):-----------
        ImGui::Node* colorNode = nge.addNode(ImGui::ColorNode::Create(ImVec2(40,50)));
        ImGui::Node* complexNode = nge.addNode(ImGui::ComplexNode::Create(ImVec2(40,150)));
        ImGui::Node* combineNode = nge.addNode(ImGui::CombineNode::Create(ImVec2(300,80))); // optionally use e.g.: ImGui::CombineNode::Cast(combineNode)->fraction = 0.8f;
        nge.addLink(colorNode, 0, combineNode, 0);
        nge.addLink(complexNode, 1, combineNode, 1);
        //----------------------------------------------------------------------
        // This adds entries to the "add node" context menu
        nge.registerNodeTypes(MyNodeTypeNames,MNT_COUNT,MyNodeFactory,NULL,-1); // last 2 args can be used to add only a subset of nodes (or to sort their order inside the context menu)
        nge.show_style_editor = true;
    }
    nge.render();
}
#endif //IMGUINODEGRAPHEDITOR_NOTESTDEMO


}	//nmespace ImGui

