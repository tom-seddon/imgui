/* 	Refactoring from https://github.com/ocornut/imgui/issues/306
	It's basically the same exact code
*/

#ifndef IMGUINODEGRAPHEDITOR_H_
#define IMGUINODEGRAPHEDITOR_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

namespace ImGui	{

    struct Node
    {
        int     ID;
        char    Name[32];
        ImVec2  Pos, Size;
        float   Value;
        ImVec4  Color;
        int     InputsCount, OutputsCount;

        Node(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count) { 
	ID = id; strncpy(Name, name, 31); Name[31] = 0; Pos = pos; Value = value; Color = color; 		InputsCount = inputs_count; OutputsCount = outputs_count; 
	}

        ImVec2 GetInputSlotPos(int slot_no) const   { return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no+1) / ((float)InputsCount+1)); }
        ImVec2 GetOutputSlotPos(int slot_no) const  { return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no+1) / ((float)OutputsCount+1)); }
    };

    struct NodeLink
    {
        int     InputIdx, InputSlot, OutputIdx, OutputSlot;

        NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { InputIdx = input_idx; InputSlot = input_slot; OutputIdx = output_idx; OutputSlot = output_slot; }
    };

	struct NodeGraphEditor	{
    	ImVector<Node> nodes;
    	ImVector<NodeLink> links;
    	ImVec2 scrolling;
    	bool show_grid;
    	int node_selected;
	NodeGraphEditor(bool show_grid_= true) {
		scrolling = ImVec2(0.0f, 0.0f);
		show_grid = show_grid_; 	
		node_selected = -1;
	}
	
	bool isEmpty() const {return nodes.size()==0;}
	void addNode(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count)	{
        nodes.push_back(Node(id, name, pos,value,color, inputs_count,outputs_count));
	}
	void addLink(int input_idx, int input_slot, int output_idx, int output_slot)	{
		links.push_back(NodeLink(input_idx, input_slot, output_idx, output_slot));			
}
	// To be called outside any windows ATM
	void render(bool* opened);

};

/*
	static ImGui::NodeGraphEditor nge;
	if (nge.isEmpty())	{
    	nge.addNode(0, "MainTex",  ImVec2(40,50), 0.5f, ImColor(255,100,100), 1, 1);
    	nge.addNode(1, "BumpMap",  ImVec2(40,150), 0.42f, ImColor(200,100,200), 1, 1);
    	nge.addNode(2, "Combine", ImVec2(270,80), 1.0f, ImColor(0,200,100), 2, 2);
		nge.addLink(0, 0, 2, 0);
		nge.addLink(1, 0, 2, 1);
	}
	nge.render();
*/


}	// namespace ImGui



#endif //IMGUINODEGRAPHEDITOR_H_

