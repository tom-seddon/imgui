/* 	Refactoring from https://github.com/ocornut/imgui/issues/306
    It's basically the same exact code with a few modifications
*/

#ifndef IMGUINODEGRAPHEDITOR_H_
#define IMGUINODEGRAPHEDITOR_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

/*  // Basic usage:
    static ImGui::NodeGraphEditor nge;
    if (nge.mustInit())	{
        nge.init(); // So nge.mustInit() returns false next time
        static const char* mainTexOutputNames[2] = {"rgb","alpha"}; // Must be static
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

    // Here we create a window (please skip if already in a window)
    ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiSetCond_FirstUseEver);
    if (ImGui::Begin("Example: Custom Node Graph", NULL))
    {
        nge.render();
        ImGui::End();
    }
*/
/*  // Minimal usage from inside a window:
    static ImGui::NodeGraphEditor nge;
    nge.render();
*/


namespace ImGui	{

// TODO: make abstract or just use a ImVector of some powerful generic data structures
struct Node
{
    protected:
    float   Value;
    ImVec4  Color;


    protected:

    char    Name[32];
    ImVec2  Pos, Size;
    int     InputsCount, OutputsCount;
    const char** InputNames;
    const char** OutputNames;
    mutable float startEditingTime; // used for Node Editing Callbacks
    mutable bool isOpen;

    public:
    Node(const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count,const char** input_names=NULL,const char** output_names=NULL) {
        strncpy(Name, name, 31); Name[31] = 0; Pos = pos; Value = value; Color = color;
        InputsCount = inputs_count; OutputsCount = outputs_count;
        InputNames = input_names;
        OutputNames = output_names;
        user_ptr = NULL;
        startEditingTime = 0;
        isOpen = true;
    }
    virtual ~Node() {}
    mutable void* user_ptr;
    const char* getName() const {return Name;}

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
    mutable bool isaNodeInActiveState;  // when user clicks/drags/edits values inside a node


    enum NodeState {NS_ADDED,NS_DELETED,NS_EDITED};
    typedef void (*NodeCallback)(Node*& node,NodeState state,NodeGraphEditor& editor);
    enum LinkState {LS_ADDED,LS_DELETED};
    typedef void (*LinkCallback)(const NodeLink& link,LinkState state,NodeGraphEditor& editor);
    LinkCallback linkCallback;// called after a link is added and before it's deleted
    NodeCallback nodeCallback;// called after a node is added, after it's edited and before it's deleted
    float nodeEditedTimeThreshold; // time in seconds that must elapse after the last "editing touch" before the NS_EDITED callback is called

    public:
    struct Style {
        ImVec4 color_background;
        ImU32 color_grid;
        float grid_line_width,grid_size;
        ImU32 color_node;
        ImU32 color_node_frame;
        ImU32 color_node_selected;
        ImU32 color_node_hovered;
        float node_rounding;
        ImVec2 node_window_padding;
        ImU32 color_node_slots;
        float node_slots_radius;
        ImU32 color_link;
        float link_line_width;
        float link_control_point_distance;
        int link_num_segments;  // in AddBezierCurve(...)
        ImVec4 color_node_title;
        ImVec4 color_node_input_slots_names;
        ImVec4 color_node_output_slots_names;
        Style() {
            color_background =      ImColor(60,60,70,200);
            color_grid =            ImColor(200,200,200,40);
            grid_line_width =       1.f;
            grid_size =             64.f;

            color_node =            ImColor(60,60,60);
            color_node_frame =      ImColor(100,100,100);
            color_node_selected =   ImColor(75,75,85);
            color_node_hovered =    ImColor(85,85,85);
            node_rounding =         4.f;
            node_window_padding =   ImVec2(8.f,8.f);

            color_node_slots = ImColor(150,150,150,150);
            node_slots_radius =     5.f;

            color_link =            ImColor(200,200,100);
            link_line_width =       3.f;
            link_control_point_distance = 50.f;
            link_num_segments =     0;

            color_node_title = ImGui::GetStyle().Colors[ImGuiCol_Text];
            color_node_input_slots_names = ImGui::GetStyle().Colors[ImGuiCol_Text];color_node_input_slots_names.w=0.75f;
            color_node_output_slots_names = ImGui::GetStyle().Colors[ImGuiCol_Text];color_node_output_slots_names.w=0.75f;
        }
    };
    bool show_grid;
    bool show_connection_names;
    bool show_node_list;
    bool show_info;
    mutable void* user_ptr;
    static Style& GetStyle() {return style;}

    NodeGraphEditor(bool show_grid_= true,bool show_connection_names_=true,bool _allowOnlyOneLinkPerInputSlot=true,bool init_in_ctr=false) {
        scrolling = ImVec2(0.0f, 0.0f);
        show_grid = show_grid_;
        show_connection_names = show_connection_names_;
        selectedNode = dragNode.node = NULL;
        allowOnlyOneLinkPerInputSlot = _allowOnlyOneLinkPerInputSlot;
        nodeCallback = NULL;linkCallback=NULL;nodeEditedTimeThreshold=1.5f;
        user_ptr = NULL;
        show_node_list = true;
        show_info = true;
        isaNodeInActiveState = false;
        inited = init_in_ctr;
    }
    virtual ~NodeGraphEditor() {
        clear();
    }
    void clear() {
        if (linkCallback)   {
            for (int i=links.size()-1;i>0;i--)  {
                const NodeLink& link = links[i];
                linkCallback(link,LS_DELETED,*this);
            }
        }
        for (int i=nodes.size()-1;i>0;i--)  {
            Node*& node = nodes[i];
            if (node)   {
                if (nodeCallback) nodeCallback(node,NS_DELETED,*this);
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

        if (node) {
            nodes.push_back(node);
            if (nodeCallback) nodeCallback(nodes[nodes.size()-1],NS_ADDED,*this);
        }
        return node;
    }
    bool deleteNode(Node* node) {
        if (node == selectedNode)  selectedNode = NULL;
        if (node == dragNode.node) dragNode.node = NULL;
        for (int i=0;i<nodes.size();i++)    {
            Node*& n = nodes[i];
            if (n==node)  {
                removeAnyLinkFromNode(n);
                if (nodeCallback) nodeCallback(n,NS_DELETED,*this);
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
        if (insert) {
            links.push_back(NodeLink(inputNode,input_slot,outputNode,output_slot));
            if (linkCallback) linkCallback(links[links.size()-1],LS_ADDED,*this);
        }
        return insert;
    }
    bool removeLink(Node* inputNode, int input_slot, Node* outputNode, int output_slot) {
        int link_idx = -1;
        bool ok = isLinkPresent(inputNode,input_slot,outputNode,output_slot,&link_idx);
        if (ok) ok = removeLinkAt(link_idx);
        return ok;
    }
    void removeAnyLinkFromNode(Node* node,bool removeInputLinks=true,bool removeOutputLinks=true);
    bool isLinkPresent(Node* inputNode, int input_slot, Node* outputNode, int output_slot,int* pOptionalIndexInLinkArrayOut=NULL) const;

    // To be called INSIDE a window
    void render();

    // Optional helper methods:
    void getOutputNodesForNodeAndSlot(const Node* node,int output_slot,ImVector<Node*>& returnValueOut,ImVector<int>* pOptionalReturnValueInputSlotOut=NULL) const;
    void getInputNodesForNodeAndSlot(const Node* node,int input_slot,ImVector<Node*>& returnValueOut,ImVector<int>* pOptionalReturnValueOutputSlotOut=NULL) const;
    // if allowOnlyOneLinkPerInputSlot == true:
    Node* getInputNodeForNodeAndSlot(const Node* node,int input_slot,int* pOptionalReturnValueOutputSlotOut=NULL) const;

    // It should be better not to add/delete node/links in the callbacks... (but all is untested here)
    void setNodeCallback(NodeCallback cb) {nodeCallback=cb;}
    void setLinkCallback(LinkCallback cb) {linkCallback=cb;}
    void setNodeEditedCallbackTimeThreshold(int seconds) {nodeEditedTimeThreshold=seconds;}

    protected:

    struct DragNode {
        ImVec2 pos;
        Node* node;int inputSlotIdx,outputSlotIdx;
        DragNode() : node(NULL),inputSlotIdx(-1),outputSlotIdx(-1) {}
        bool isValid() const {return node && (inputSlotIdx>=0 || outputSlotIdx>=0);}
        void reset() {*this=DragNode();}
    };
    DragNode dragNode;

    bool removeLinkAt(int link_idx);
    static Style style;
};



}	// namespace ImGui



#endif //IMGUINODEGRAPHEDITOR_H_

