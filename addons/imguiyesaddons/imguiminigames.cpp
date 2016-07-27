// LICENSE: see "imguiminigames.h"

#include "../../imgui.h"
#define IMGUI_DEFINE_PLACEMENT_NEW
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../imgui_internal.h"

#include "imguiminigames.h"

#include <stdlib.h> // rand()

namespace ImGuiMiniGames {
    FreeTextureDelegate FreeTextureCb =
#       ifdef IMGUI_USE_AUTO_BINDING
        &ImImpl_FreeTexture;
#       else //IMGUI_USE_AUTO_BINDING
        NULL;
#       endif //IMGUI_USE_AUTO_BINDING
    GenerateOrUpdateTextureDelegate GenerateOrUpdateTextureCb =
#       ifdef IMGUI_USE_AUTO_BINDING
        &ImImpl_GenerateOrUpdateTexture;
#       else //IMGUI_USE_AUTO_BINDING
        NULL;
#       endif //IMGUI_USE_AUTO_BINDING


    /*static bool GetGlyphData(unsigned short glyph,ImVec2* pSizeOut=NULL,float* pXAdvanceOut=NULL, ImVec2* pUV0Out=NULL, ImVec2* pUV1Out=NULL) {
        if (!GImGui->Font) return false;
        const ImFont::Glyph* g = GImGui->Font->FindGlyph(glyph);
        if (g)  {
            if (pSizeOut) {pSizeOut->x = g->X1-g->X0;pSizeOut->y = g->Y1-g->Y0;}
            if (pXAdvanceOut) *pXAdvanceOut = g->XAdvance;
            if (pUV0Out) {pUV0Out->x = g->U0; pUV0Out->y = g->V0;}
            if (pUV1Out) {pUV1Out->x = g->U1; pUV1Out->y = g->V1;}
            return true;
        }
        return false;
    }*/
    static void ImDrawListPathFillAndStroke(ImDrawList *dl, const ImU32 &fillColor, const ImU32 &strokeColor, bool strokeClosed, float strokeThickness, bool antiAliased)    {
        if (!dl) return;
        if ((fillColor >> 24) != 0) dl->AddConvexPolyFilled(dl->_Path.Data, dl->_Path.Size, fillColor, antiAliased);
        if ((strokeColor>> 24)!= 0 && strokeThickness>0) dl->AddPolyline(dl->_Path.Data, dl->_Path.Size, strokeColor, strokeClosed, strokeThickness, antiAliased);
        dl->PathClear();
    }
    static void ImDrawListAddRect(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, const ImU32 &strokeColor, float rounding, int rounding_corners, float strokeThickness, bool antiAliased) {
        if (!dl || (((fillColor >> 24) == 0) && ((strokeColor >> 24) == 0)))  return;
        dl->PathRect(a, b, rounding, rounding_corners);
        ImDrawListPathFillAndStroke(dl,fillColor,strokeColor,true,strokeThickness,antiAliased);
    }



#   ifndef NO_IMGUIMINIGAMES_MINE

    struct MineHS {
        static const int MAX_CELL_SIZE = 32;
        static const char* Title;
        enum CellState {
            CS_MINE=1,
            CS_FLAG=1<<1,
            CS_OPEN=1<<2,
            CS_DUMMY=1<<3
        };
        unsigned char cells[MAX_CELL_SIZE][MAX_CELL_SIZE];
        ImU32 numGridRows,numGridColumns,numMines,numGridHollowRows,numGridHollowColumns;
        ImU32 numClicks,numFlags,numOpenCells;
        bool paused;
        int frameCount;float startTime,currentTime,currentPenaltyTime,nextPenaltyTime;
        bool inited;
        int comboSelectedIndex;bool fitToScreen;
        enum GamePhase {
            GP_Titles=0,
            GP_Playing,
            GP_GameOver
        };
        unsigned char gamePhase;bool gameWon;
        void resetVariables() {
            numGridRows=numGridColumns=8;numMines=10;numGridHollowRows=numGridHollowColumns=0;
            numClicks=numFlags=numOpenCells=0;
            paused=false;frameCount=ImGui::GetFrameCount();
            startTime=0;currentTime=0;currentPenaltyTime=0;nextPenaltyTime=5;
            gamePhase=GP_Titles;gameWon=false;
            for (ImU32 y=0;y<MAX_CELL_SIZE;y++) {
                for (ImU32 x=0;x<MAX_CELL_SIZE;x++) {
                    cells[x][y]=0;
                }
            }
        }
        MineHS() {resetVariables();inited=false;comboSelectedIndex=0;fitToScreen=true;}
        ~MineHS() {}
        inline static const unsigned char* GetCellData(unsigned char cellContent,unsigned char* pCellStateOut,unsigned char* pNumAdjacentMinesOut=NULL) {
            if (pNumAdjacentMinesOut) *pNumAdjacentMinesOut = (cellContent&0x0F);
            if (pCellStateOut) {*pCellStateOut = (CellState) (cellContent>>4);return pCellStateOut;}
            return NULL;
        }
        inline static void SetCellData(unsigned char& cellContentOut,const unsigned char* pCellStateIn,const unsigned char* pNumAdjacentMinesIn=NULL) {
            if (pCellStateIn) cellContentOut = ((*pCellStateIn)<<4)|(cellContentOut&0x0F);
            if (pNumAdjacentMinesIn) cellContentOut = ((*pNumAdjacentMinesIn)&0x0F)|(cellContentOut&0xF0);
        }
        inline unsigned char calculateNumNeighborsWithState(ImU32 x,ImU32 y,unsigned char flag,bool excludeDummyCells=true) const {
            unsigned char numNeig = 0, state = 0;
            if (x>0)    {
                if                      (((*GetCellData(cells[x-1][y],&state))&flag)    && (excludeDummyCells || !(state&CS_DUMMY))) ++numNeig;
                if (y>0 &&              (((*GetCellData(cells[x-1][y-1],&state))&flag)  && (excludeDummyCells || !(state&CS_DUMMY)))) ++numNeig;
                if (y<numGridRows-1 &&  (((*GetCellData(cells[x-1][y+1],&state))&flag)  && (excludeDummyCells || !(state&CS_DUMMY)))) ++numNeig;
            }
            if (x<numGridColumns-1) {
                if                      (((*GetCellData(cells[x+1][y],&state))&flag)    && (excludeDummyCells || !(state&CS_DUMMY))) ++numNeig;
                if (y>0 &&              (((*GetCellData(cells[x+1][y-1],&state))&flag)  && (excludeDummyCells || !(state&CS_DUMMY)))) ++numNeig;
                if (y<numGridRows-1 &&  (((*GetCellData(cells[x+1][y+1],&state))&flag)  && (excludeDummyCells || !(state&CS_DUMMY)))) ++numNeig;
            }
            if (y>0 &&                  (((*GetCellData(cells[x][y-1],&state))&flag)  && (excludeDummyCells || !(state&CS_DUMMY)))) ++numNeig;
            if (y<numGridRows-1 &&      (((*GetCellData(cells[x][y+1],&state))&flag)  && (excludeDummyCells || !(state&CS_DUMMY)))) ++numNeig;
            return numNeig;
        }
        inline bool areNeighborsWithoutFlagsAllOpen(ImU32 x,ImU32 y) {
            unsigned char state = 0;
            if (x>0)    {                
                GetCellData(cells[x-1][y],&state);                          if (!(state&CS_DUMMY) && !((state&CS_OPEN) || (state&CS_FLAG))) return false;
                if (y>0) {GetCellData(cells[x-1][y-1],&state);              if (!(state&CS_DUMMY) && !((state&CS_OPEN) || (state&CS_FLAG))) return false;}
                if (y<numGridRows-1) {GetCellData(cells[x-1][y+1],&state);  if (!(state&CS_DUMMY) && !((state&CS_OPEN) || (state&CS_FLAG))) return false;}
            }
            if (x<numGridColumns-1) {
                GetCellData(cells[x+1][y],&state);                          if (!(state&CS_DUMMY) && !((state&CS_OPEN) || (state&CS_FLAG))) return false;
                if (y>0) {GetCellData(cells[x+1][y-1],&state);              if (!(state&CS_DUMMY) && !((state&CS_OPEN) || (state&CS_FLAG))) return false;}
                if (y<numGridRows-1) {GetCellData(cells[x+1][y+1],&state);  if (!(state&CS_DUMMY) && !((state&CS_OPEN) || (state&CS_FLAG))) return false;}
            }
                if (y>0) {GetCellData(cells[x][y-1],&state);                if (!(state&CS_DUMMY) && !((state&CS_OPEN) || (state&CS_FLAG))) return false;}
                if (y<numGridRows-1) {GetCellData(cells[x][y+1],&state);    if (!(state&CS_DUMMY) && !((state&CS_OPEN) || (state&CS_FLAG))) return false;}
            return true;
        }
        // Returns false if player hits a mine
        inline bool openCell(ImU32 x,ImU32 y,ImU32* pNumCellsJustOpenedOut=NULL,bool isFirstClick=true) {
            unsigned char state = 0,adj = 0;
            GetCellData(cells[x][y],&state,&adj);
            if (state&CS_FLAG || state&CS_DUMMY) return true; // already flagged or dummy cell
            if (state&CS_OPEN)  {
                if (!isFirstClick) return true; // already open
                // First click here:
                if (adj==0
                    || calculateNumNeighborsWithState(x,y,CS_FLAG)!=adj
                    || areNeighborsWithoutFlagsAllOpen(x,y)) {
                    //fprintf(stderr,"Nothing to do for the %d neighbors [CalculateNumNeighborsWithState(CS_FLAG)=%d][AreNeighborsWithoutFlagsAllOpen()=%s]\n",adj,CalculateNumNeighborsWithState(x,y,CS_FLAG,cells,numGridColumns,numGridRows),AreNeighborsWithoutFlagsAllOpen(x,y,cells,numGridColumns,numGridRows)?"true":"false");
                    return true; // already open
                }
                //else fprintf(stderr,"We can open all the %d neighbors\n",adj); // We must open all the closed neighbors
            }
            else {
                // state is closed here
                state|=CS_OPEN;
                SetCellData(cells[x][y],&state);
                if (pNumCellsJustOpenedOut) (*pNumCellsJustOpenedOut)++;
                if (isFirstClick && adj>0 && !(state&CS_MINE)) return true;
            }
            if (state&CS_MINE) return false;// false => game over

            if (adj==0 || isFirstClick) {
                // recurse in 8 directions
                if (x>0)    {
                    if (                    !openCell(x-1,y,pNumCellsJustOpenedOut,false)) return false;
                    if (y>0 &&              !openCell(x-1,y-1,pNumCellsJustOpenedOut,false)) return false;
                    if (y<numGridRows-1 &&  !openCell(x-1,y+1,pNumCellsJustOpenedOut,false)) return false;
                }
                if (x<numGridColumns-1) {
                    if (                    !openCell(x+1,y,pNumCellsJustOpenedOut,false)) return false;
                    if (y>0 &&              !openCell(x+1,y-1,pNumCellsJustOpenedOut,false)) return false;
                    if (y<numGridRows-1 &&  !openCell(x+1,y+1,pNumCellsJustOpenedOut,false)) return false;
                }
                if (y>0 &&                  !openCell(x,y-1,pNumCellsJustOpenedOut,false)) return false;
                if (y<numGridRows-1 &&      !openCell(x,y+1,pNumCellsJustOpenedOut,false)) return false;
            }

            return true;
        }
        // Sets up the grid quantities, but does not fills it yet with mines
        void initNewGame(ImU32 _numGridRows,ImU32 _numGridColumns,ImU32 _numMines=-1,ImU32 _numGridHollowRows=0,ImU32 _numGridHollowColumns=0)  {
            numGridColumns = _numGridColumns;
            numGridRows = _numGridRows;
            for (int x=0;x<MAX_CELL_SIZE;x++) {
                for (int y=0;y<MAX_CELL_SIZE;y++)   {
                    cells[x][y]=0;
                }
            }
            numGridHollowColumns = (_numGridHollowColumns>2 && _numGridHollowColumns<numGridColumns-4) ? _numGridHollowColumns : 0;
            numGridHollowRows = (_numGridHollowRows>2 && _numGridHollowRows<numGridRows-4) ? _numGridHollowRows : 0;
            if (numGridHollowRows==0 || numGridHollowColumns==0) {numGridHollowRows=numGridHollowColumns=0;}
            if (numGridHollowRows>0 && numGridHollowColumns>0) {
                const unsigned char dummyCellState = CS_DUMMY;
                const int startCol = (numGridColumns-numGridHollowColumns)/2;
                const int startRow = (numGridRows-numGridHollowRows)/2;
                for (int x=startCol,xsz=startCol+numGridHollowColumns;x<xsz;x++) {
                    for (int y=startRow,ysz=startRow+numGridHollowRows;y<ysz;y++) {
                        SetCellData(cells[x][y],&dummyCellState);
                    }
                }
            }

            const ImU32 area = numGridColumns*numGridRows-(numGridHollowColumns*numGridHollowRows);
            numMines = (_numMines>0 && _numMines<area) ? _numMines :
                                                         area==8*8 ? 10 :
                                                                     area==16*16 ? 40 :
                                                                                   area==30*16 ? 99 :
                                                                                                 (area/5<=0 ? 1 : area/5);
            gameWon = false;
            IM_ASSERT(numMines>0 && numMines<area);
        }
        // Fills an inited grid with mines, leaving emptyCellRow and emptyCellColumn free
        void restartGame(int emptyCellColumn, int emptyCellRow)   {
            int c=0,r=0; unsigned char state=0; const unsigned char mineState = CS_MINE;
            const float c_rand = (float)numGridColumns/(float)RAND_MAX;
            const float r_rand = (float)numGridRows/(float)RAND_MAX;
            for (ImU32 i=0;i<numMines;i++)    {
                c = ((float)rand()*c_rand);
                r = ((float)rand()*r_rand);
                while (c<0 || c>=(int)numGridColumns || r<0 || r>=(int)numGridRows || (c==emptyCellColumn && r==emptyCellRow) || ((*GetCellData(cells[c][r],&state))&(CS_MINE|CS_DUMMY)))   {
                    c = ((float)rand()*c_rand);
                    r = ((float)rand()*r_rand);
                }
                SetCellData(cells[c][r],&mineState,NULL);
            }

            // Fill adjacency
            unsigned char numNeig = 0;
            for (ImU32 x=0;x<numGridColumns;x++) {
                for (ImU32 y=0;y<numGridRows;y++)   {
                    numNeig = calculateNumNeighborsWithState(x,y,CS_MINE);
                    if (numNeig>0) SetCellData(cells[x][y],NULL,&numNeig);
                }
            }

            numOpenCells = 0;gameWon = false;
            currentTime = 0;currentPenaltyTime=0;nextPenaltyTime=5;
            frameCount = ImGui::GetFrameCount();
            startTime = ImGui::GetTime();

        }

    void render() {
        Mine::Style& style = Mine::Style::Get();

        if (!inited) {
            inited = true;
            srand(ImGui::GetTime()*10.f);
        }

        ImU32 colorText = style.colors[Mine::Style::Color_Text];
        if (colorText>>24==0) colorText = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);

        ImGui::PushID(this);
        bool mustReInit = false;
        ImGui::BeginGroup();
        if (gamePhase == GP_Playing) {
            if (ImGui::Button("Quit Game##MinesQuitGame")) {gamePhase = GP_Titles;mustReInit=true;}
        }
        else if (gamePhase == GP_GameOver) {
            if (ImGui::Button("New Game##MinesQuitGame")) {gamePhase = GP_Titles;mustReInit=true;}
        }
        if (gamePhase == GP_Titles || mustReInit) {
            static const char* Types[] = {"Easy (8x8)","Medium (16x16)","Hard (16x30)","Hard (30x16)","Impossible"};
            ImGui::PushItemWidth(ImGui::GetWindowWidth()*0.35f);
            if (mustReInit || ImGui::Combo("Game Type##MinesGameType",&comboSelectedIndex,Types,sizeof(Types)/sizeof(Types[0]),sizeof(Types)/sizeof(Types[0])))   {
                resetVariables();   // does not touch comboSelectedIndex
                switch (comboSelectedIndex) {
                case 0: numGridColumns = numGridRows = 8; numMines = 10;break;
                case 2: numGridColumns = 30; numGridRows = 16;numMines = 99;break;
                case 3: numGridColumns = 16; numGridRows = 30;numMines = 99;break;
                case 4: numGridColumns = numGridRows = 32;numMines = 0;numGridHollowColumns= numGridHollowRows = 8;break;
                case 1:
                default:
                    numGridColumns = numGridRows = 16; numMines = 40;
                    break;
                }
                mustReInit = true;
                initNewGame(numGridRows,numGridColumns,numMines,numGridHollowRows,numGridHollowColumns);
            }
            ImGui::PopItemWidth();
        }
        ImGui::Checkbox("Auto Zoom##MinesAutoZoom",&fitToScreen);
        if (ImGui::IsItemHovered() && !fitToScreen) ImGui::SetTooltip("%s","When false, use CTRL+MW to zoom\nand CTRL+MWB to auto-zoom.");
        ImGui::EndGroup();

        ImGui::SameLine(ImGui::GetWindowWidth()*0.35f);

        ImGui::BeginGroup();
        if (gamePhase != GP_Titles && !mustReInit) {
            if (gamePhase == GP_Playing) {
                int newFrame = ImGui::GetFrameCount();
                if (newFrame==frameCount+1 && !paused
#               ifdef IMGUI_USE_AUTO_BINDING
                && !gImGuiPaused && !gImGuiWereOutsideImGui
#               endif //IMGUI_USE_AUTO_BINDING
                ) {
                    currentTime = ImGui::GetTime() - startTime + currentPenaltyTime;
                }
                else startTime = ImGui::GetTime() - currentTime + currentPenaltyTime;
                frameCount = newFrame;
            }
            const unsigned int minutes = (unsigned int)currentTime/60;
            const unsigned int seconds = (unsigned int)currentTime%60;
            ImGui::Text("Time:  %um:%2us",minutes,seconds);
            ImGui::Text("Flags: %d/%d",numFlags,numMines);
        }
        ImGui::EndGroup();

        ImGui::PopID();

        ImVec2 gridSize(numGridColumns,numGridRows);

        // Mine, Flag, [1,8]
        float glyphWidths[10] = {-1.f,-1.f,-1.f,-1.f,-1.f,-1.f,-1.f,-1.f,-1.f,-1.f};

        ImGuiIO& io = ImGui::GetIO();
        if (mustReInit) ImGui::SetNextWindowFocus();
        ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImGui::ColorConvertU32ToFloat4(style.colors[Mine::Style::Color_Background]));
        ImGui::BeginChild("Mine Game Scrolling Region", ImVec2(0,0), false,fitToScreen ? ImGuiWindowFlags_NoScrollbar : (ImGuiWindowFlags_HorizontalScrollbar/*|ImGuiWindowFlags_AlwaysHorizontalScrollbar|ImGuiWindowFlags_AlwaysVerticalScrollbar*/));

        const bool isFocused = ImGui::IsWindowFocused() || ImGui::IsRootWindowFocused();
        const bool isHovered = ImGui::IsWindowHovered();
        ImGuiWindow* window = ImGui::GetCurrentWindow();

        bool LMBclick = false, RMBclick = false, isPclicked = false;
        if (isFocused && isHovered && !mustReInit) {
            LMBclick = ImGui::IsMouseClicked(0);
            RMBclick = ImGui::IsMouseReleased(1);
            isPclicked = ImGui::IsKeyPressed(style.keyPause,false);
            if (isPclicked && gamePhase == GP_Playing) paused=!paused;
        }

        // Zoom / Scale window
        if (!fitToScreen)   {
            if (isFocused && isHovered && !io.FontAllowUserScaling && io.KeyCtrl && window==GImGui->HoveredWindow && (io.MouseWheel || io.MouseClicked[2]))   {
                float new_font_scale = ImClamp(window->FontWindowScale + io.MouseWheel * 0.10f, window->FontWindowScale*0.1f, window->FontWindowScale * 2.50f);
                if (io.MouseClicked[2]) new_font_scale = 1.f;   // MMB = RESET ZOOM
                float scale = new_font_scale / window->FontWindowScale;
                if (scale!=1)	window->FontWindowScale = new_font_scale;
            }
        }

        float textLineHeight = ImGui::GetTextLineHeight();
        ImVec2 gridDimensions = gridSize * (textLineHeight+window->FontWindowScale)  + ImVec2(0.1f*textLineHeight,0.1f*textLineHeight);
        ImVec2 gridOffset(0,0);
        if (fitToScreen || (gridDimensions.x<window->Size.x && gridDimensions.y<window->Size.y))   {
            if (gridDimensions.x!=window->Size.x && gridDimensions.y!=window->Size.y) {
                ImVec2 ratios(gridDimensions.x/window->Size.x,gridDimensions.y/window->Size.y);
                // Fill X or Y Window Size
                window->FontWindowScale/= (ratios.x>=ratios.y) ? ratios.x : ratios.y;

                textLineHeight = ImGui::GetTextLineHeight();
                gridDimensions = gridSize * (textLineHeight+window->FontWindowScale) + ImVec2(0.1f*textLineHeight,0.1f*textLineHeight);
            }
        }
        if (gridDimensions.x<window->Size.x) gridOffset.x = (window->Size.x-gridSize.x*(textLineHeight+window->FontWindowScale))*0.5f;
        if (gridDimensions.y<window->Size.y) gridOffset.y = (window->Size.y-gridSize.y*(textLineHeight+window->FontWindowScale))*0.5f;



        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 canvasSize = window->Size;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();

        if (gamePhase==GP_Playing && (/*!isFocused ||*/ paused /*|| !ImGui::IsMouseHoveringWindow()*/
#       ifdef IMGUI_USE_AUTO_BINDING
                || gImGuiPaused || gImGuiWereOutsideImGui
#       endif //IMGUI_USE_AUTO_BINDING
        ))   {
            // Game Paused here

            if (((unsigned)(ImGui::GetTime()*10.f))%10<5)  {
                // Display "PAUSED":-------------------
                static const char pausedText[] = "PAUSED";
                const ImVec2 textSize = ImGui::CalcTextSize(pausedText);
                const ImVec2 start = win_pos+ImVec2((canvasSize.x-textSize.x)*0.5f+ImGui::GetScrollX(),(canvasSize.y-textSize.y)*0.15f+ImGui::GetScrollY());
                const ImVec2 enlargement(textLineHeight*0.25f,0.f);
                ImDrawListAddRect(draw_list,start-enlargement,start+textSize+enlargement,style.colors[Mine::Style::Color_OpenCellBackground],style.colors[Mine::Style::Color_Grid],4.f,0x0F,window->FontWindowScale,true);
                draw_list->AddText(start,colorText,pausedText);
                //--------------------------------------
            }

            // Display controls in a smaller font:
            static const char controlsText[] = "CONTROLS:\n\nRMB: places/removes a flag.\nLMB: opens a cell\n     (on a number-cell, opens its neighbors).\nCTRL+LMB: safely opens a cell\n    (some penalty time is added).";
            const float fontScaling = 0.35f;
            const ImVec2 textSize = ImGui::CalcTextSize(controlsText)*fontScaling;
            const ImVec2 start = win_pos+ImVec2((canvasSize.x-textSize.x)*0.5f+ImGui::GetScrollX(),(canvasSize.y-textSize.y)*0.65f+ImGui::GetScrollY());
            const ImVec2 enlargement(textLineHeight*0.25f,0.f);
            ImDrawListAddRect(draw_list,start-enlargement,start+textSize+enlargement,style.colors[Mine::Style::Color_OpenCellBackground],style.colors[Mine::Style::Color_Grid],4.f,0x0F,window->FontWindowScale,true);
            draw_list->AddText(GImGui->Font,GImGui->FontSize*fontScaling,start,colorText,controlsText);


        }
        else {


            const ImU32& GRID_COLOR = style.colors[Mine::Style::Color_Grid];
            const float GRID_SZ = textLineHeight+window->FontWindowScale;//32.f;
            const ImVec2 grid_len = gridSize * GRID_SZ;
            const float grid_Line_width = window->FontWindowScale;

            // Display Closed Cell Background
            draw_list->AddRectFilled(win_pos+gridOffset,win_pos+gridOffset+grid_len+ImVec2(grid_Line_width,grid_Line_width),style.colors[Mine::Style::Color_ClosedCellBackground]);

            // Display grid ---------------------------------------------------------------------------------------------
            // Draw Y lines
            int cnt = 0;
            for (float x = gridOffset.x,xsz=gridOffset.x+grid_len.x+GRID_SZ;    x<=xsz;  x+=GRID_SZ)    {
                draw_list->AddLine(ImVec2(x,gridOffset.y)+win_pos, ImVec2(x,gridOffset.y+grid_len.y)+win_pos, GRID_COLOR,grid_Line_width);
                if (cnt++>=gridSize.x) break;
            }
            // Draw X lines
            cnt = 0;
            for (float y = gridOffset.y,ysz=gridOffset.y+grid_len.y+GRID_SZ;    y<=ysz; y+=GRID_SZ)    {
                draw_list->AddLine(ImVec2(gridOffset.x,y)+win_pos, ImVec2(gridOffset.x+grid_len.x,y)+win_pos, GRID_COLOR,grid_Line_width);
                if (cnt++>=gridSize.y) break;
            }
            // Fraw Hollow Square
            if (numGridHollowColumns>0 && numGridHollowRows>0) {
                ImVec2 start = gridOffset + win_pos + ImVec2((numGridColumns-numGridHollowColumns)/2,(numGridRows-numGridHollowRows)/2)*GRID_SZ;
                draw_list->AddRectFilled(start,start+ImVec2(numGridHollowColumns,numGridHollowRows)*GRID_SZ,style.colors[Mine::Style::Color_HollowSpace]);
            }
            //------------------------------------------------------------------------------------------------------------

            // Detect the cell under the mouse.
            int mouseCellColumn = -1, mouseCellRow = -1;
            unsigned char mouseCellState = 0;
            bool isMouseCellValid = false;
            if (isFocused && isHovered && !mustReInit) {
                ImVec2 mp = ImGui::GetMousePos() - win_pos - gridOffset;
                if (mp.x>0 && mp.y>0
                        && (fitToScreen || (mp.x+gridOffset.x<window->Size.x-window->ScrollbarSizes.x+window->Scroll.x && mp.y+gridOffset.y<window->Size.y-window->ScrollbarSizes.y+window->Scroll.y))
                        )   {
                    mouseCellColumn = mp.x/GRID_SZ;
                    mouseCellRow = mp.y/GRID_SZ;
                    if (mouseCellRow>=gridSize.y || mouseCellColumn>=gridSize.x || ((*GetCellData(cells[mouseCellColumn][mouseCellRow],&mouseCellState))&CS_DUMMY)) {mouseCellColumn=mouseCellRow=-1;mouseCellState=0;}
                    else isMouseCellValid = true;
                }
                if (isMouseCellValid) {
                    if (gamePhase == GP_Playing && !LMBclick && !RMBclick && !(mouseCellState&CS_OPEN) && !(mouseCellState&CS_FLAG))  {
                        // Let's draw the hovered cell:
                        ImVec2 start(win_pos+gridOffset+ImVec2(grid_Line_width,grid_Line_width)+ImVec2(mouseCellColumn*GRID_SZ,mouseCellRow*GRID_SZ));
                        draw_list->AddRectFilled(start,start+ImVec2(textLineHeight,textLineHeight),style.colors[Mine::Style::Color_HoveredCellBackground]);
                    }
                    //fprintf(stderr,"Clicked cell[c:%d][r:%d].\n",mouseCellColumn,mouseCellRow);
                }

            }

            if (gamePhase!=GP_Playing)  {
                if (gamePhase == GP_Titles) {
                    if (LMBclick && isMouseCellValid)   {
                        ++numClicks;
                        restartGame(mouseCellColumn,mouseCellRow);  // First click in the game (the clicked cell must be empty)
                        const bool mineAvoided = openCell(mouseCellColumn,mouseCellRow,&numOpenCells);
                        IM_ASSERT(mineAvoided);
                        gamePhase = GP_Playing;
                    }
                }
                LMBclick = RMBclick = false;   // prevents double processing
            }
            else if (gamePhase==GP_Playing && isMouseCellValid) {
                bool mustCheckForGameWon = false;
                if (RMBclick) {
                    if (!(mouseCellState&CS_OPEN)) {
                        ++numClicks;    // Should we skip flag clicks from counting ?
                        mouseCellState^=CS_FLAG;
                        SetCellData(cells[mouseCellColumn][mouseCellRow],&mouseCellState);
                        if (mouseCellState&CS_FLAG) numFlags++;
                        else if (numFlags>0) numFlags--;
                        mustCheckForGameWon = true;
                    }
                }
                else if (LMBclick)  {                    
                    if (io.KeyCtrl) {
                        // Help mode:
                        if (!(mouseCellState&CS_OPEN)) {
                            ++numClicks;
                            if (mouseCellState&CS_MINE) {
                                if (!(mouseCellState&CS_FLAG))    {
                                    mouseCellState|=CS_FLAG;
                                    SetCellData(cells[mouseCellColumn][mouseCellRow],&mouseCellState);
                                    numFlags++;
                                    mustCheckForGameWon = true;
                                }
                            }
                            else {
                                if (mouseCellState&CS_FLAG)    {
                                    mouseCellState&=~CS_FLAG;
                                    SetCellData(cells[mouseCellColumn][mouseCellRow],&mouseCellState);
                                    numFlags--;
                                    mustCheckForGameWon = true;
                                }
                                if (!openCell(mouseCellColumn,mouseCellRow,&numOpenCells)) gamePhase = GP_GameOver;
                                else mustCheckForGameWon = true;
                            }
                            // Add penalty time
                            currentPenaltyTime+=nextPenaltyTime;
                            nextPenaltyTime+=10;    // Next time penalty time will be 10 s bigger
                        }
                    }
                    else {
                        // Normal mode
                        ++numClicks;
                        if (!openCell(mouseCellColumn,mouseCellRow,&numOpenCells)) gamePhase = GP_GameOver;
                        else mustCheckForGameWon = true;
                    }
                }
                if (mustCheckForGameWon) {
                    const ImU32 area = numGridColumns*numGridRows-(numGridHollowColumns*numGridHollowRows);
                    if (numMines == numFlags && numOpenCells == area-numFlags) {
                        gamePhase = GP_GameOver;
                        gameWon = true;
                    }
                }
                // Check what square is under the mouse
                /*ImVec2 start(win_pos+gridOffset+ImVec2(grid_Line_width,grid_Line_width)+ImVec2(C*GRID_SZ,R*GRID_SZ));
                        ImRect rect(start,start+ImVec2(textLineHeight,textLineHeight));
                        draw_list->AddRectFilled(rect.GetTL(),rect.GetBR(),0xAAFF0000);*/
            }

            // draw cells:
            {
                unsigned char state=0,adj=0;
                static char charNum[2] = "0";
                const ImVec2 baseStart = win_pos+gridOffset+ImVec2(0.f/*grid_Line_width*/,grid_Line_width);
                ImVec2 start(0,0);
                for (ImU32 c=0;c<numGridColumns;c++)  {
                    for (ImU32 r=0;r<numGridRows;r++)  {
                        GetCellData(cells[c][r],&state,&adj);

                        start = baseStart+ImVec2(c*GRID_SZ,r*GRID_SZ);

                        if (state&CS_OPEN) {
                            draw_list->AddRectFilled(start+ImVec2(grid_Line_width,0.f),start+ImVec2(grid_Line_width+textLineHeight,textLineHeight),style.colors[Mine::Style::Color_OpenCellBackground]);
                        }
                        if (state&CS_FLAG) {
                            if (gamePhase==GP_GameOver && !(state&CS_MINE)) draw_list->AddRectFilled(start+ImVec2(grid_Line_width,0.f),start+ImVec2(grid_Line_width+textLineHeight,textLineHeight),style.colors[Mine::Style::Color_WrongFlagBackground]);

                            float& glyphWidth = glyphWidths[1];if (glyphWidth==-1.f) glyphWidth = ImGui::CalcTextSize(style.characters[Mine::Style::Character_Flag]).x;
                            draw_list->AddText(start+ImVec2((textLineHeight-glyphWidth)*0.5f,0.f),style.colors[Mine::Style::Color_Flag],style.characters[Mine::Style::Character_Flag]);
                        }
                        else if (state&CS_OPEN || gamePhase==GP_GameOver) {
                            if (state&CS_MINE) {
                                float& glyphWidth = glyphWidths[0];if (glyphWidth==-1.f) glyphWidth = ImGui::CalcTextSize(style.characters[Mine::Style::Character_Mine]).x;
                                draw_list->AddText(start+ImVec2((textLineHeight-glyphWidth)*0.5f,0.f),style.colors[Mine::Style::Color_Mine],style.characters[Mine::Style::Character_Mine]);

                                if (state&CS_OPEN) {
                                    draw_list->AddCircleFilled(start+ImVec2(grid_Line_width+textLineHeight*0.5f,textLineHeight*0.5f),textLineHeight*0.4f,style.colors[Mine::Style::Color_WrongMineOverlay]);
                                    draw_list->AddCircle(start+ImVec2(grid_Line_width+textLineHeight*0.5f,textLineHeight*0.5f),textLineHeight*0.4f,style.colors[Mine::Style::Color_WrongMineOverlayBorder],12,grid_Line_width);
                                }
                            }
                            else if (state&CS_OPEN && adj>0 && adj<9)  {
                                charNum[0] = '0'+adj;
                                float& glyphWidth = glyphWidths[1+adj];if (glyphWidth==-1.f) glyphWidth = ImGui::CalcTextSize(charNum).x;
                                const ImU32& glyphColor = style.colors[(Mine::Style::Color_1+adj-1)];
                                draw_list->AddText(start+ImVec2((textLineHeight-glyphWidth)*0.5f,0.f),glyphColor,charNum);
                            }
                        }                        
                    }
                }
            }

            // Draw end game messages
            if (gamePhase==GP_GameOver) {
                const float elapsedSeconds = (float)(ImGui::GetTime()-currentTime-startTime+currentPenaltyTime);
                if (gameWon) {
                    static char gameWonText[256] = "";
                    sprintf(gameWonText,"GAME COMPLETED\nTIME: %um : %us",((unsigned)currentTime)/60,((unsigned)currentTime)%60);
                    const ImVec2 textSize = ImGui::CalcTextSize(gameWonText);
                    ImVec2 deltaPos(0.f,0.f);
                    if (elapsedSeconds<10.f) {
                        deltaPos.x = canvasSize.x * sin(2.f*elapsedSeconds) * (10.f-elapsedSeconds)*0.025f;
                        deltaPos.y = canvasSize.y * cos(2.f*elapsedSeconds) * (10.f-elapsedSeconds)*0.025f;
                    }
                    const ImVec2 start = win_pos+ImVec2((canvasSize.x-textSize.x)*0.5f+ImGui::GetScrollX()+deltaPos.x,(canvasSize.y-textSize.y)*0.5f+ImGui::GetScrollY()-deltaPos.y);
                    const ImVec2 enlargement(textLineHeight*0.25f,0.f);
                    ImDrawListAddRect(draw_list,start-enlargement,start+textSize+enlargement,style.colors[Mine::Style::Color_OpenCellBackground],style.colors[Mine::Style::Color_Grid],4.f,0x0F,window->FontWindowScale,true);
                    draw_list->AddText(start,colorText,gameWonText);
                }
                else {
                    static const char gameOverText[] = "GAME\nOVER";
                    const ImVec2 textSize = ImGui::CalcTextSize(gameOverText);
                    ImVec2 deltaPos(0.f,0.f);
                    if (elapsedSeconds<10.f) {
                        deltaPos.x = canvasSize.x * sin(2.f*elapsedSeconds) * (10.f-elapsedSeconds)*0.025f;
                        deltaPos.y = canvasSize.y * cos(2.f*elapsedSeconds) * (10.f-elapsedSeconds)*0.025f;
                    }
                    const ImVec2 start = win_pos+ImVec2((canvasSize.x-textSize.x)*0.5f+ImGui::GetScrollX()+deltaPos.x,(canvasSize.y-textSize.y)*0.5f+ImGui::GetScrollY()-deltaPos.y);
                    const ImVec2 enlargement(textLineHeight*0.25f,0.f);
                    ImDrawListAddRect(draw_list,start-enlargement,start+textSize+enlargement,style.colors[Mine::Style::Color_OpenCellBackground],style.colors[Mine::Style::Color_Grid],4.f,0x0F,window->FontWindowScale,true);
                    draw_list->AddText(start,colorText,gameOverText);
                }
            }
            /*const ImVec2 textSize = ImGui::CalcTextSize(title);
            const ImU32 col = 0xFF00FF00;
            draw_list->AddText(win_pos+ImVec2((canvasSize.x-textSize.x)*0.5f+ImGui::GetScrollX(),(canvasSize.y-textSize.y)*0.5f+ImGui::GetScrollY()),col,title);*/

            //if (isMouseDraggingForScrolling) scrolling = scrolling - io.MouseDelta;
        }

        // Sets scrollbars properly:
        ImGui::SetCursorPos(ImGui::GetCursorPos() + gridSize * (textLineHeight+window->FontWindowScale) + ImVec2(0.1f*textLineHeight,0.1f*textLineHeight));

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    };
    const char* MineHS::Title = "Mine Game";

    Mine::Mine() : imp(NULL) {
        imp = (MineHS*) ImGui::MemAlloc(sizeof(MineHS));
        IM_PLACEMENT_NEW(imp) MineHS();
        IM_ASSERT(imp);
    }
    Mine::~Mine() {
        if (imp)    {
            imp->~MineHS();
            ImGui::MemFree(imp);
        }
    }
    void Mine::render() {
        if (imp) imp->render();
    }

    Mine::Style::Style()    {
        colors[Style::Color_Text] =                     0xFF008800;
        colors[Style::Color_Background] =               0x00000000;//0xFFF0F1F2;
        colors[Style::Color_ClosedCellBackground] =     0xFFB6BDBA;
        colors[Style::Color_OpenCellBackground] =       0xFFDCDEDE;
        colors[Style::Color_1] =                        0xFF83594B;
        colors[Style::Color_2] =                        0xFF46A046;
        colors[Style::Color_3] =                        0xFF1E42E2;
        colors[Style::Color_4] =                        0xFF815B62;
        colors[Style::Color_5] =                        0xFF314688;
        colors[Style::Color_6] =                        0xFFD2B89D;
        colors[Style::Color_7] =                        0xFF80D6EE;
        colors[Style::Color_8] =                        0xFF60E6FF;
        colors[Style::Color_WrongFlagBackground] =      0xFF0000CC;
        colors[Style::Color_WrongMineOverlay] =         0x8800CCFF;
        colors[Style::Color_WrongMineOverlayBorder] =   0x99004455;
        colors[Style::Color_HollowSpace]        =       0xFF000000;

        colors[Style::Color_Mine] = 0xFF000000;
        colors[Style::Color_Flag] = 0xFF0000FF;

        colors[Style::Color_Grid] = 0xFFF0F1F2;//colors[Style::Color_Background];

        const ImVec4 tmp1 = ImGui::ColorConvertU32ToFloat4(colors[Style::Color_ClosedCellBackground]);
        const ImVec4 tmp2 = ImGui::ColorConvertU32ToFloat4(colors[Style::Color_OpenCellBackground]);
        const ImVec4 tmp((tmp1.x+tmp2.x)*0.5f,(tmp1.y+tmp2.y)*0.5f,(tmp1.z+tmp2.z)*0.5f,(tmp1.w+tmp2.w)*0.5f);
        colors[Color_HoveredCellBackground] = ImGui::ColorConvertFloat4ToU32(tmp);

        // Warning: we can't comment them out, we must initialize the memory somehow [Consider setting them to "" if we'll support manual drawing].
        strcpy(characters[Character_Mine],"M");
        strcpy(characters[Character_Flag],"F");

        keyPause = (int) 'p';

    }
    Mine::Style Mine::Style::style;




#   endif //NO_IMGUIMINIGAMES_MINE

} // namespace ImGuiMiniGames



// KeyboardWindow by Floooh (Oryol) - Could be helpful if we implement hi-score support
// #define REFERENCE_CODE
#ifdef REFERENCE_CODE
class KeyboardWindow : public WindowBase {
    OryolClassDecl(KeyboardWindow);
public:
    /// setup the window
    virtual void Setup(yakc::kc85& kc) override;
    /// draw method
    virtual bool Draw(yakc::kc85& kc) override;

    bool shift = false;
    bool caps_lock = false;
};
struct key {
    key() : pos(0.0f), name(nullptr), code(0), shift_code(0) { };
    key(float p, const char* n, ubyte c, ubyte sc) : pos(p), name(n), code(c), shift_code(sc) { };
    float pos;
    const char* name;
    ubyte code;
    ubyte shift_code;
};

// the 5 'main section' rows of keys:
// see: http://www.mpm-kc85.de/dokupack/KC85_3_uebersicht.pdf
static const int num_rows = 5;
static const int num_cols = 13;
static struct key layout[num_rows][num_cols] = {
{
    // function keys row
    {4,"F1",0xF1,0xF7}, {0,"F2",0xF2,0xF8}, {0,"F3",0xF3,0xF9}, {0,"F4",0xF4,0xFA}, {0,"F5",0xF5,0xFB}, {0,"F6",0xF6,0xFC},
    {0,"BRK",0x03,0x03}, {0,"STP",0x13,0x13}, {0,"INS",0x1A,0x14}, {0,"DEL",0x1F,0x02}, {0,"CLR",0x01,0x0F}, {0,"HOM",0x10,0x0C}
},
{
    // number keys row
    {4,"1 !",'1','!'}, {0,"2 \"",'2','\"'}, {0,"3 #",'3','#'}, {0,"4 $",'4','$'}, {0,"5 %",'5','%'}, {0,"6 &",'6','&'},
    {0,"7 '",'7',0x27}, {0,"8 (",'8','('}, {0,"9 )",'9',')'}, {0,"0 @",'0','@'}, {0,": *",':','*'}, {0,"- =",'-','='},
    {2,"CUU",0x0B,0x11}
},
{
    // QWERT row
    {16,"Q",'Q','q'}, {0,"W",'W','w'}, {0,"E",'E','e'}, {0,"R",'R','r'}, {0,"T",'T','t'}, {0,"Z",'Z','z'},
    {0,"U",'U','u'}, {0,"I",'I','i'}, {0,"O",'O','o'}, {0,"P",'P','p'}, {0,"\x5E \x5D",0x5E,0x5D},
    {10,"CUL",0x08,0x19},{0,"CUR",0x09,0x18}
},
{
    // ASDF row
    {0,"CAP",0x16,0x16}, {0,"A",'A','a'}, {0,"S",'S','s'}, {0,"D",'D','d'}, {0,"F",'F','f'}, {0,"G",'G','g'},
    {0,"H",'H','h'}, {0,"J",'J','j'}, {0,"K",'K','k'}, {0,"L",'L','l'}, {0,"+ ;",'+',';'}, {0,"\x5F |",0x5F,0x5C},
    {14,"CUD",0x0A,0x12}
},
{
    // YXCV row (NOTE: shift-key has special code which is not forwarded as key!
    {10,"SHI",0xFF,0xFF}, {0,"Y",'Y','y'}, {0,"X",'X','y'}, {0,"C",'C','c'}, {0,"V",'V','v'}, {0,"B",'B','b'},
    {0,"N",'N','n'}, {0,"M",'M','m'}, {0,", <",',','<'}, {0,". >",'.','>'}, {0,"/ ?",'/','?'},
    {36,"RET",0x0D,0x0D}
}

};

//------------------------------------------------------------------------------
void
KeyboardWindow::Setup(kc85& kc) {
    this->setName("Keyboard");
}

//------------------------------------------------------------------------------
bool
KeyboardWindow::Draw(kc85& kc) {
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.75f);
    ImGui::SetNextWindowSize(ImVec2(572, 196));
    if (ImGui::Begin(this->title.AsCStr(), &this->Visible, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_ShowBorders)) {

        // main section keys
        const ImVec2 size(32,24);
        for (int row = 0; row < num_rows; row++) {
            for (int col = 0; col < num_cols; col++) {
                const key& k = layout[row][col];
                if (k.name) {
                    if (col != 0) {
                        ImGui::SameLine();
                    }
                    if (k.pos > 0.0f) {
                        ImGui::Dummy(ImVec2(k.pos,0.0f)); ImGui::SameLine();
                    }
                    if (ImGui::Button(k.name, size)) {
                        // caps lock?
                        if (k.code == 0x16) {
                            this->caps_lock = !this->caps_lock;
                            this->shift = this->caps_lock;
                        }
                        // shift?
                        if (k.code == 0xFF) {
                            this->shift = true;
                        }
                        else {
                            kc.put_key(this->shift ? k.shift_code:k.code);

                            // clear shift state after one key, unless caps_lock is on
                            if (!this->caps_lock) {
                                this->shift = false;
                            }
                        }
                    }
                }
            }
        }

        // space bar
        ImGui::Dummy(ImVec2(80,0)); ImGui::SameLine();
        if (ImGui::Button("SPACE", ImVec2(224, 0))) {
            kc.put_key(this->caps_lock ? 0x5B : 0x20);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    return this->Visible;
}
#endif //REFERENCE_CODE


