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
// Really dumb data structure provided for the example.
// Note that we storing links are INDICES (not ID) to make example code shorter, obviously a bad idea for any general purpose code.
void NodeGraphEditor::render(bool* opened)
{

    ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiSetCond_FirstUseEver);
    if (!ImGui::Begin("Example: Custom Node Graph", opened)){
        ImGui::End();
        return;
    }
    const ImGuiIO io = ImGui::GetIO();

    // Draw a list of nodes on the left side
    bool open_context_menu = false;
    Node* node_hovered_in_list = NULL;
    Node* node_hovered_in_scene = NULL;
    ImGui::BeginChild("node_list", ImVec2(100,0));
    ImGui::Text("Nodes");
    ImGui::Separator();
    for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
    {
        Node* node = nodes[node_idx];
        ImGui::PushID((const void*) node);
        if (ImGui::Selectable(node->Name, node == selectedNode)) selectedNode = node;
        if (ImGui::IsItemHovered()) {
            node_hovered_in_list = node;
            open_context_menu |= ImGui::IsMouseClicked(1);
        }
        ImGui::PopID();
    }
    ImGui::Separator();
    ImGui::Text("Tip:\nDouble-click\nLMB on slots\nto remove\ntheir links.\n");
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();

    const float NODE_SLOT_RADIUS = 5.0f;
    const float NODE_SLOT_RADIUS_SQUARED = (NODE_SLOT_RADIUS*NODE_SLOT_RADIUS);
    const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

    // Create our child canvas
    ImGui::Text("Hold LMB to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
    ImGui::SameLine(ImGui::GetWindowWidth()-300);
    ImGui::Checkbox("Show connection names", &show_connection_names);
    ImGui::SameLine(ImGui::GetWindowWidth()-100);
    ImGui::Checkbox("Show grid", &show_grid);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1,1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(60,60,70,200));
    ImGui::BeginChild("scrolling_region", ImVec2(0,0), true, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(120.0f);

    ImVec2 offset = ImGui::GetCursorScreenPos() - scrolling;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSplit(2);

    // Display grid
    if (show_grid)
    {
        ImU32 GRID_COLOR = ImColor(200,200,200,40);
        float GRID_SZ = 64.0f;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        for (float x = fmodf(offset.x,GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
            draw_list->AddLine(ImVec2(x,0.0f)+win_pos, ImVec2(x,canvas_sz.y)+win_pos, GRID_COLOR);
        for (float y = fmodf(offset.y,GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
            draw_list->AddLine(ImVec2(0.0f,y)+win_pos, ImVec2(canvas_sz.x,y)+win_pos, GRID_COLOR);
    }

    //
    //updateDragging(scrolling);

    // Display links
    draw_list->ChannelsSetCurrent(0); // Background
    for (int link_idx = 0; link_idx < links.Size; link_idx++)
    {
        NodeLink& link = links[link_idx];
        Node* node_inp = link.InputNode;
        Node* node_out = link.OutputNode;
        ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link.InputSlot);
        ImVec2 p2 = offset + node_out->GetInputSlotPos(link.OutputSlot);
        draw_list->AddBezierCurve(p1, p1+ImVec2(+50,0), p2+ImVec2(-50,0), p2, ImColor(200,200,100), 3.0f);
    }
    // Display dragging link
    bool isLMBDraggingForMakingLinks = ImGui::IsMouseDragging(0, 0.0f);
    bool isDragNodeValid = dragNode.isValid();
    if (isLMBDraggingForMakingLinks && isDragNodeValid)   {
        if (dragNode.inputSlotIdx!=-1)  {
            ImVec2 p1 = offset + dragNode.node->GetOutputSlotPos(dragNode.inputSlotIdx);
            const ImVec2& p2 = io.MousePos;//offset + node_out->GetInputSlotPos(link.OutputSlot);
            draw_list->AddBezierCurve(p1, p1+ImVec2(+50,0), p2+ImVec2(-50,0), p2, ImColor(200,200,100), 3.0f);
        }
        else if (dragNode.outputSlotIdx!=-1)  {
            const ImVec2& p1 = io.MousePos;//
            ImVec2 p2 = offset + dragNode.node->GetInputSlotPos(dragNode.outputSlotIdx);
            draw_list->AddBezierCurve(p1, p1+ImVec2(+50,0), p2+ImVec2(-50,0), p2, ImColor(200,200,100), 3.0f);
        }
    }

    // Display nodes
    bool isSomeNodeMoving = false;
    for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
    {
        Node* node = nodes[node_idx];
        ImGui::PushID((const void*) node);
        ImVec2 node_rect_min = offset + node->Pos;

        // Display node contents first
        draw_list->ChannelsSetCurrent(1); // Foreground
        bool old_any_active = ImGui::IsAnyItemActive();
        ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
        ImGui::BeginGroup(); // Lock horizontal position
        ImGui::Text("%s", node->Name);
        ImGui::SliderFloat("##value", &node->Value, 0.0f, 1.0f, "Alpha %.2f");
        ImGui::ColorEdit3("##color", &node->Color.x);
        ImGui::EndGroup();

        // Save the size of what we have emitted and whether any of the widgets are being used
        bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
        node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
        ImVec2 node_rect_max = node_rect_min + node->Size;

        // Display node box
        draw_list->ChannelsSetCurrent(0); // Background
        ImGui::SetCursorScreenPos(node_rect_min);
        ImGui::InvisibleButton("node", node->Size);
        if (ImGui::IsItemHovered()) {
            node_hovered_in_scene = node;
            open_context_menu |= ImGui::IsMouseClicked(1);
        }
        bool node_moving_active = ImGui::IsItemActive();
        if (node_widgets_active || node_moving_active)  selectedNode = node;
        if (node_moving_active && !isDragNodeValid && ImGui::IsMouseDragging(0, 8.0f)) {node->Pos = node->Pos + io.MouseDelta;isSomeNodeMoving=true;}

        ImU32 node_bg_color = (node_hovered_in_list == node || node_hovered_in_scene == node || (node_hovered_in_list == NULL && selectedNode == node)) ? ImColor(75,75,75) : ImColor(60,60,60);
        draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f); 
        draw_list->AddRect(node_rect_min, node_rect_max, ImColor(100,100,100), 4.0f); 
        // Display connectors
        const ImVec2 oldCursorScreenPos = ImGui::GetCursorScreenPos();
        const ImVec2 mouseScreenPos = io.MousePos;;
        ImVec2 connectorScreenPos,deltaPos;const bool canDeleteLinks = true;
        const bool mustDeleteLinkIfSlotIsHovered = canDeleteLinks && io.MouseDoubleClicked[0];
        const bool mustDetectIfSlotIsHoveredForDragNDrop = !isSomeNodeMoving && (!isDragNodeValid || isLMBDraggingForMakingLinks);
        for (int slot_idx = 0; slot_idx < node->InputsCount; slot_idx++)    {
            connectorScreenPos = offset + node->GetInputSlotPos(slot_idx);
            draw_list->AddCircleFilled(connectorScreenPos, NODE_SLOT_RADIUS, ImColor(150,150,150,150));
            if (show_connection_names && node->InputNames)   {
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
                    else if (isDragNodeValid && dragNode.node!=node) {
                        // verify compatibility
                        if (dragNode.inputSlotIdx!=-1)  {
                            if (allowOnlyOneLinkPerInputSlot)   {
                                // Remove all existing node links to node-slot_idx before adding new link:
                                for (int link_idx=0;link_idx<links.size();link_idx++)   {
                                    NodeLink& link = links[link_idx];
                                    if (link.OutputNode == node && slot_idx == link.OutputSlot)   {
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
        for (int slot_idx = 0; slot_idx < node->OutputsCount; slot_idx++)   {
            connectorScreenPos = offset + node->GetOutputSlotPos(slot_idx);
            draw_list->AddCircleFilled(offset + node->GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, ImColor(150,150,150,150));
            if (show_connection_names && node->OutputNames)   {
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
                    else if (isDragNodeValid && dragNode.node!=node) {
                        // verify compatibility
                        if (dragNode.outputSlotIdx!=-1)  {
                            if (allowOnlyOneLinkPerInputSlot)   {
                                // Remove all existing node links to dragNode before adding new link:
                                for (int link_idx=0;link_idx<links.size();link_idx++)   {
                                    NodeLink& link = links[link_idx];
                                    if (link.OutputNode == dragNode.node && dragNode.outputSlotIdx == link.OutputSlot)   {
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
        if (!isLMBDraggingForMakingLinks) {
            dragNode.node = NULL; // clear dragNode
            //printf("Quit dragging.\n");fflush(stdout);
        }

        /*if (node == selectedNode) selected_nodeIdx = node_idx;
        if (node == node_hovered_in_list) node_hovered_in_list_idx = node_idx;
        if (node == node_hovered_in_scene) node_hovered_in_scene_idx = node_idx;*/
        ImGui::SetCursorScreenPos(oldCursorScreenPos);
        ImGui::PopID();
    }
    draw_list->ChannelsMerge();

    // Open context menu
    if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow())   {
        if (ImGui::IsMouseClicked(1))   {   // Open context menu
            selectedNode = node_hovered_in_list = node_hovered_in_scene = NULL;
            open_context_menu = true;
        }
    }
    if (open_context_menu)
    {
        ImGui::OpenPopup("context_menu");
        if (node_hovered_in_list) selectedNode = node_hovered_in_list;
        if (node_hovered_in_scene) selectedNode = node_hovered_in_scene;
    }

    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8,8));
    if (ImGui::BeginPopup("context_menu"))  {
        Node* node = selectedNode;
        ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
        if (node)   {
            ImGui::Text("Node '%s'", node->Name);
            ImGui::Separator();
            //if (ImGui::MenuItem("Rename..", NULL, false, false)) {}
            if (ImGui::MenuItem("Delete", NULL, false, true)) {
                //printf("Current nodes.size()=%d; Deleting node %s.\n",nodes.size(),node->Name);fflush(stdout);
                deleteNode(node);
            }
            //if (ImGui::MenuItem("Copy", NULL, false, false)) {}
        }
        else    {
            if (ImGui::MenuItem("Add")) {
                addNode("New node",scene_pos,0.5f, ImColor(100,100,200), 2, 2);
            }
            if (ImGui::MenuItem("Paste", NULL, false, false)) {}
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

    // Scrolling
    if (!isSomeNodeMoving && !dragNode.node && ImGui::IsWindowHovered() &&  ImGui::IsMouseDragging(0, 6.0f)) scrolling = scrolling - io.MouseDelta;

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();

    ImGui::End();
}



void NodeGraphEditor::removeAnyLinkFromNode(Node* node, bool removeInputLinks, bool removeOutputLinks)  {
    for (int link_idx=0;link_idx<links.size();link_idx++)    {
        NodeLink& link = links[link_idx];
        if ((removeOutputLinks && link.InputNode==node) || (removeInputLinks && link.OutputNode==node))    {
            // remove link
            if (link_idx+1 < links.size()) link = links[links.size()-1];    // swap with the last link
            links.resize(links.size()-1);
            --link_idx;
        }
    }
}

bool NodeGraphEditor::isLinkPresent(Node *inputNode, int input_slot, Node *outputNode, int output_slot) const  {
    for (int link_idx=0;link_idx<links.size();link_idx++)    {
        const NodeLink& l = links[link_idx];
        if (l.InputNode==inputNode && l.InputSlot==input_slot &&
            l.OutputNode==outputNode && l.OutputSlot==output_slot) return true;
    }
    return false;
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


}	//nmespace ImGui

