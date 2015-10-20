/* 	Refactoring from https://github.com/ocornut/imgui/issues/306
    It's basically the same exact code with a few modifications
*/

#ifndef IMGUINODEGRAPHEDITOR_H_
#define IMGUINODEGRAPHEDITOR_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

namespace ImGui	{


struct Node
{
    protected:

    char    Name[32];
    ImVec2  Pos, Size;
    float   Value;
    ImVec4  Color;
    int     InputsCount, OutputsCount;
    const char** InputNames;
    const char** OutputNames;

    public:
    Node(const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count,const char** input_names=NULL,const char** output_names=NULL) {
        strncpy(Name, name, 31); Name[31] = 0; Pos = pos; Value = value; Color = color; 		InputsCount = inputs_count; OutputsCount = outputs_count;
        InputNames = input_names;
        OutputNames = output_names;
    }
    virtual ~Node() {}

    protected:

    ImVec2 GetInputSlotPos(int slot_no) const   { return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no+1) / ((float)InputsCount+1)); }
    ImVec2 GetOutputSlotPos(int slot_no) const  { return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no+1) / ((float)OutputsCount+1)); }


    friend struct NodeLink;
    friend struct NodeGraphEditor;
};

struct NodeLink
{
    Node*  InputNode;   int InputSlot;
    Node*  OutputNode;  int OutputSlot;

    NodeLink(Node* input_node, int input_slot, Node* output_node, int output_slot) {
        InputNode = input_node; InputSlot = input_slot;
        OutputNode = output_node; OutputSlot = output_slot;
    }

    friend struct NodeGraphEditor;
};


struct NodeGraphEditor	{
    protected:
    ImVector<Node*> nodes;          // used as a garbage collector too
    ImVector<NodeLink> links;
    ImVec2 scrolling;
    Node* selectedNode;
    bool inited;
    bool allowOnlyOneLinkPerInputSlot;  // multiple links can still be connected to single output slots

    public:
    bool show_grid;
    bool show_connection_names;

    NodeGraphEditor(bool show_grid_= true,bool show_connection_names_=true,bool _allowOnlyOneLinkPerInputSlot=true,bool init_in_ctr=false) {
        scrolling = ImVec2(0.0f, 0.0f);
        show_grid = show_grid_;
        show_connection_names = show_connection_names_;
        selectedNode = dragNode.node = NULL;
        allowOnlyOneLinkPerInputSlot = _allowOnlyOneLinkPerInputSlot;
        inited = init_in_ctr;
    }
    virtual ~NodeGraphEditor() {
        clear();
    }
    void clear() {
        for (int i=nodes.size()-1;i>0;i--)  {
            Node*& node = nodes[i];
            if (node)   {
                node->~Node();              // ImVector does not call it
                ImGui::MemFree(node);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
                node = NULL;
            }
        }
        nodes.clear();
        links.clear();
        scrolling = ImVec2(0,0);
        selectedNode = dragNode.node = NULL;
    }

    bool mustInit() const {return !inited;}
    void init() {inited=true;}
    bool isEmpty() const {return nodes.size()==0;}
    Node* addNode(const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count,const char** input_names=NULL,const char** output_names=NULL)	{
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        Node* node = (Node*) ImGui::MemAlloc(sizeof(Node));
        // MANDATORY even with blank ctrs. Requires: #include <new>. Reason: ImVector does not call ctrs/dctrs on items.
        new (node) Node(name, pos,value,color, inputs_count,outputs_count,input_names,output_names);

        if (node) nodes.push_back(node);
        return node;
    }
    bool deleteNode(Node* node) {
        if (node == selectedNode)  selectedNode = NULL;
        if (node == dragNode.node) dragNode.node = NULL;
        for (int i=0;i<nodes.size();i++)    {
            Node*& n = nodes[i];
            if (n==node)  {
                removeAnyLinkFromNode(n);
                n->~Node();              // ImVector does not call it
                ImGui::MemFree(n);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
                if (i+1 < nodes.size()) n = nodes[nodes.size()-1];    // swap with the last node
                nodes.resize(nodes.size()-1);
                return true;
            }
        }
        return false;
    }
    bool addLink(Node* inputNode, int input_slot, Node* outputNode, int output_slot,bool checkIfAlreadyPresent = false)	{
        bool insert = true;
        if (checkIfAlreadyPresent) insert = !isLinkPresent(inputNode,input_slot,outputNode,output_slot);
        if (insert) links.push_back(NodeLink(inputNode,input_slot,outputNode,output_slot));
        return insert;
    }    
    void removeAnyLinkFromNode(Node* node,bool removeInputLinks=true,bool removeOutputLinks=true);
    bool isLinkPresent(Node* inputNode, int input_slot, Node* outputNode, int output_slot) const;

    // To be called outside any windows ATM
    void render(bool* opened);


    // Optional helper methods:
    void getInputNodesForNodeAndSlot(const Node* node,int input_slot,ImVector<Node*>& returnValueOut,ImVector<int>* pOptionalReturnValueOutputSlotOut=NULL) const;
    void getOutputNodesForNodeAndSlot(const Node* node,int output_slot,ImVector<Node*>& returnValueOut,ImVector<int>* pOptionalReturnValueInputSlotOut=NULL) const;




    protected:

    struct DragNode {
        ImVec2 pos;
        Node* node;int inputSlotIdx,outputSlotIdx;
        DragNode() : node(NULL),inputSlotIdx(-1),outputSlotIdx(-1) {}
        bool isValid() const {return node && (inputSlotIdx>=0 || outputSlotIdx>=0);}
        void reset() {*this=DragNode();}
    };
    DragNode dragNode;


};

/*  // Basic usage:
    static ImGui::NodeGraphEditor nge;
    if (nge.mustInit())	{
        nge.init(); // So nge.mustInit() returns false next time
        static const char* mainTexOutputNames[2] = {"rgb","alpha"};
        ImGui::Node* mainTexNode = nge.addNode("MainTex",  ImVec2(40,50), 0.5f, ImColor(255,100,100), 0, 2,NULL,mainTexOutputNames);
        static const char* bumpMapInputNames[1] = {"in"};
        static const char* bumpMapOutputNames[1] = {"bumpOut"};
        ImGui::Node* bumpMapNode = nge.addNode("BumpMap",  ImVec2(40,150), 0.42f, ImColor(200,100,200), 1, 1,bumpMapInputNames,bumpMapOutputNames);
        static const char* combineInputNames[3] = {"c1","c2","c3"};
        static const char* combineOutputNames[1] = {"out"};
        ImGui::Node* combineNode = nge.addNode("Combine", ImVec2(270,80), 1.0f, ImColor(0,200,100), 3, 1,combineInputNames,combineOutputNames);
        nge.addLink(mainTexNode, 0, combineNode, 0);
        nge.addLink(mainTexNode, 1, combineNode, 1);
        nge.addLink(bumpMapNode, 0, combineNode, 2);
    }
    nge.render(&show_node_graph_editor_window);
*/


}	// namespace ImGui



#endif //IMGUINODEGRAPHEDITOR_H_

