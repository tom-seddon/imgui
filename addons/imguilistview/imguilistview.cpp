#include "imguilistview.h"

bool ImGui::ListViewBase::render(float listViewHeight, const ImVector<int> *pOptionalColumnReorderVector, int maxNumColumnToDisplay) const {
    //if (listViewHeight<0) listViewHeight = getMaxPossibleHeight();  // Hack to fix an issue, but I leave it to the user, and comment it out
    ImGui::PushID(this);

    const int numColumns = (int) getNumColumns();
    const size_t numRows = getNumRows();

    if (maxNumColumnToDisplay<0) maxNumColumnToDisplay = numColumns;
    if (pOptionalColumnReorderVector && (int)pOptionalColumnReorderVector->size()<maxNumColumnToDisplay) maxNumColumnToDisplay = (int)pOptionalColumnReorderVector->size();
    int col = 0;

    //ImVector<HeaderData> m_headerData;        // We can remove this ImVector, if we call getHeaderData(...) 2X times (not sure if it's faster). UPDATE: used member variable (but this way we can't update it at runtime!)
    const bool mustFetchHeaderData = (int)m_headerData.size()<numColumns;
    if (mustFetchHeaderData) updateHeaderData();

    int columnSortingIndex = -1;

    static ImColor transparentColor(1,1,1,0);
    const bool useFullHeight = listViewHeight <0;
    const ImGuiStyle& style = ImGui::GetStyle();
    const float columnHeaderDeltaOffsetX = style.WindowPadding.x;

    // Column headers
    float columnWidthSum = 0;
    ImGui::PushStyleColor(ImGuiCol_Border,style.Colors[ImGuiCol_Column]);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Columns(maxNumColumnToDisplay);
    ImGui::PushStyleColor(ImGuiCol_Button,transparentColor);
    ImGui::PushStyleColor(ImGuiCol_Border,transparentColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow,transparentColor);
    bool mustDisplayTooltip=false;
    for (int colID=0;colID<maxNumColumnToDisplay;colID++)   {
        col = pOptionalColumnReorderVector ? (*pOptionalColumnReorderVector)[colID] : colID;
        HeaderData& hd = m_headerData[col];
        if (mustFetchHeaderData)    {
            m_columnOffsets[col] = (colID>0 && !useFullHeight) ? (columnWidthSum-columnHeaderDeltaOffsetX) : columnWidthSum;
            if (hd.formatting.columnWidth>0) {
                ImGui::SetColumnOffset(colID,m_columnOffsets[col]);
                columnWidthSum+=hd.formatting.columnWidth;
            }
            else columnWidthSum+=ImGui::GetColumnWidth(colID);
        }
        else if (!useFullHeight) ImGui::SetColumnOffset(colID,m_columnOffsets[col]+columnHeaderDeltaOffsetX);//useFullHeight ? 0 : columnHeaderDeltaOffsetX);
        mustDisplayTooltip = hd.formatting.headerTooltip && strlen(hd.formatting.headerTooltip)>0;
        if (!hd.sorting.sortable) {
            ImGui::Text("%s",hd.name);
            if (mustDisplayTooltip && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",hd.formatting.headerTooltip);
        }
        else {
            if (ImGui::SmallButton(hd.name)) {
                lastSortedColumn = columnSortingIndex = col;
            }
            if (mustDisplayTooltip && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",hd.formatting.headerTooltip);
            if (lastSortedColumn == col)    {
                if (!mustFetchHeaderData) {
                    hd.reset();
                    getHeaderData(col,hd);  // Needed because we must update "hd.sorting.sortingAscending"
                }
                ImGui::SameLine(0,ImGui::GetColumnWidth(colID)-ImGui::CalcTextSize(hd.name).x-ImGui::CalcTextSize(hd.sorting.sortingAscending ? " v" : " ^").x-style.FramePadding.x*2.0-style.ItemSpacing.x);
                ImGui::Text(hd.sorting.sortingAscending ? "v" : "^");
            }
        }
        if (colID!=maxNumColumnToDisplay-1) ImGui::NextColumn();
    }
    ImGui::PopStyleColor(3);
    ImGui::Columns(1);
    ImGui::PushStyleColor(ImGuiCol_Border,style.Colors[ImGuiCol_Column]);
    ImGui::Separator();
    ImGui::PopStyleColor();

    // Rows
    bool rowSelectionChanged = false;bool colSelectionChanged = false;  // The latter is not exposed but might turn useful
    bool skipDisplaying = false;
    if (!useFullHeight) {
        //ImGui::SetNextWindowContentWidth(ImGui::GetWindowContentRegionWidth() + 50);    // Last number is hard-coded! Bad!
        skipDisplaying = !ImGui::BeginChild("##ListViewRows",ImVec2(0,listViewHeight));//,false,ImGuiWindowFlags_HorizontalScrollbar);
    }
    if (!skipDisplaying) {
        float itemHeight = ImGui::GetTextLineHeightWithSpacing();
        int displayStart = 0, displayEnd = (int) numRows;

        ImGui::CalcListClipping(numRows, itemHeight, &displayStart, &displayEnd);

        if (scrollToRow>=0) {
            if (displayStart>scrollToRow)  displayStart = scrollToRow;
            else if (displayEnd<=scrollToRow)   displayEnd = scrollToRow+1;
            else scrollToRow = -1;   // we reset it now
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (displayStart * itemHeight));

        bool isThisRowSelected = false;const char* txt=NULL;bool mustDisplayEditor = false;
        editingModePresent = false;

        const ImVec4 ImGuiColHeader = ImGui::GetStyle().Colors[ImGuiCol_Header];
        HeaderData* hd;CellData cd;columnWidthSum=0;
        ImGui::Columns(maxNumColumnToDisplay);
        for (int colID=0;colID<maxNumColumnToDisplay;colID++)   {
            col = pOptionalColumnReorderVector ? (*pOptionalColumnReorderVector)[colID] : colID;
            hd = &m_headerData[col];

            if (firstTimeDrawingRows)    {
                m_columnOffsets[col] = columnWidthSum;
                if (hd->formatting.columnWidth>0) columnWidthSum+=hd->formatting.columnWidth;
                else columnWidthSum+=ImGui::GetColumnWidth(colID);
                ImGui::SetColumnOffset(colID,m_columnOffsets[col]);
            }
            else m_columnOffsets[col] = ImGui::GetColumnOffset(colID);

            const HeaderData::Type& hdType              = hd->type;
            //const HeaderData::Formatting& hdFormatting  = hd->formatting;
            //const HeaderData::Editing& hdEditing        = hd->editing;
            const bool hdEditable = hd->editing.editable;
            bool partOfTheCellClicked = false;  // currently used only when the quad that displays the HT_COLOR is clicked

            if (hdType.headerType==HT_COLOR || hdType.headerType==HT_ICON)    {
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,transparentColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,transparentColor);
            }
            if (!hdEditable) {
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered,transparentColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive,transparentColor);
            }
            for (int row = displayStart; row < displayEnd; ++row) {
                isThisRowSelected = (selectedRow == row);
                mustDisplayEditor = isThisRowSelected && hdEditable && selectedColumn==col && hd->type.headerType!=HT_CUSTOM  && hd->type.headerType!=HT_ICON && editorAllowed;

                if (colID==0 && row==scrollToRow) ImGui::SetScrollPosHere();

                cd.reset();
                getCellData((size_t)row,col,cd);
                ImGui::PushID(cd.fieldPtr);


                if (mustDisplayEditor)  {
                    editingModePresent = true;
                    const HeaderData::Editing& hdEditing = hd->editing;

                    // Draw editor here--------------------------------------------
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
                    const int hdPrecision = hdEditing.precisionOrStringBufferSize;
                    static const int precisionStrSize = 16;static char precisionStr[precisionStrSize];int precisionLastCharIndex;
                    if (hdPrecision>0) {
                        strcpy(precisionStr,"%.");
                        snprintf(&precisionStr[2], precisionStrSize-2,"%ds",hdPrecision);
                        precisionLastCharIndex = strlen(precisionStr)-1;
                    }
                    else {
                        strcpy(precisionStr,"%s");
                        precisionLastCharIndex = 1;
                    }

                    switch (hdType.headerType) {
                    case HT_DOUBLE: {
                        precisionStr[precisionLastCharIndex]='f';
                        const float minValue = (float) hdEditing.minValue;
                        const float maxValue = (float) hdEditing.maxValue;
                        const double rtd = hdType.needsRadiansToDegs ? GetRadiansToDegs<double>() : 1.f;
                        const double dtr = hdType.needsRadiansToDegs ? GetDegsToRadians<double>() : 1.f;
                        double* pField = (double*)cd.fieldPtr;
                        float value[4] = {0,0,0,0};
                        for (int vl=0;vl<hdType.numArrayElements;vl++) {
                            value[vl] = (float) ((*(pField+vl))*rtd);
                        }
                        bool changed = false;
                        switch (hdType.numArrayElements)    {
                        case 2: changed = ImGui::SliderFloat2("##SliderDouble2Editor",value,minValue,maxValue,precisionStr);break;
                        case 3: changed = ImGui::SliderFloat3("##SliderDouble3Editor",value,minValue,maxValue,precisionStr);break;
                        case 4: changed = ImGui::SliderFloat4("##SliderDouble4Editor",value,minValue,maxValue,precisionStr);break;
                        default: changed = ImGui::SliderFloat("##SliderDoubleEditor",value,minValue,maxValue,precisionStr);break;
                        }
                        if (changed)    {
                            for (int vl=0;vl<hdType.numArrayElements;vl++) {
                                *(pField+vl) = (double) value[vl] * dtr;
                            }
                        }
                    }
                        break;
                    case HT_FLOAT: {
                        precisionStr[precisionLastCharIndex]='f';
                        const float minValue = (float) hdEditing.minValue;
                        const float maxValue = (float) hdEditing.maxValue;
                        const float rtd = hdType.needsRadiansToDegs ? GetRadiansToDegs<float>() : 1.f;
                        const float dtr = hdType.needsRadiansToDegs ? GetDegsToRadians<float>() : 1.f;
                        float* pField = (float*)cd.fieldPtr;
                        float value[4] = {0,0,0,0};
                        for (int vl=0;vl<hdType.numArrayElements;vl++) {
                            value[vl] = (float) ((*(pField+vl))*rtd);
                        }
                        bool changed = false;
                        switch (hdType.numArrayElements)    {
                        case 2: changed = ImGui::SliderFloat2("##SliderFloat2Editor",value,minValue,maxValue,precisionStr);break;
                        case 3: changed = ImGui::SliderFloat3("##SliderFloat3Editor",value,minValue,maxValue,precisionStr);break;
                        case 4: changed = ImGui::SliderFloat4("##SliderFloat4Editor",value,minValue,maxValue,precisionStr);break;
                        default: changed = ImGui::SliderFloat("##SliderFloatEditor",value,minValue,maxValue,precisionStr);break;
                        }
                        if (changed)    {
                            for (int vl=0;vl<hdType.numArrayElements;vl++) {
                                *(pField+vl) = (float) value[vl]*dtr;
                            }
                        }
                    }
                        break;
                    case HT_UNSIGNED: {
                        //precisionStr[precisionLastCharIndex]='d';
                        const int minValue = (int) hdEditing.minValue;
                        const int maxValue = (int) hdEditing.maxValue;
                        unsigned* pField = (unsigned*) cd.fieldPtr;
                        int value[4] = {0,0,0,0};
                        for (int vl=0;vl<hdType.numArrayElements;vl++) {
                            value[vl] = (int) *(pField+vl);
                        }
                        bool changed = false;
                        switch (hdType.numArrayElements)    {
                        case 2: changed = ImGui::SliderInt2("##SliderUnsigned2Editor",value,minValue,maxValue,precisionStr);break;
                        case 3: changed = ImGui::SliderInt3("##SliderUnsigned3Editor",value,minValue,maxValue,precisionStr);break;
                        case 4: changed = ImGui::SliderInt4("##SliderUnsigned4Editor",value,minValue,maxValue,precisionStr);break;
                        default: changed = ImGui::SliderInt("##SliderUnsignedEditor",value,minValue,maxValue,precisionStr);break;
                        }
                        if (changed)    {
                            for (int vl=0;vl<hdType.numArrayElements;vl++) {
                                *(pField+vl) = (unsigned) value[vl];
                            }
                        }
                    }
                        break;
                    case HT_INT: {
                        //precisionStr[precisionLastCharIndex]='d';
                        const int minValue = (int) hdEditing.minValue;
                        const int maxValue = (int) hdEditing.maxValue;
                        int* pField = (int*) cd.fieldPtr;
                        int value[4] = {0,0,0,0};
                        for (int vl=0;vl<hdType.numArrayElements;vl++) {
                            value[vl] = (int) *(pField+vl);
                        }
                        bool changed = false;
                        switch (hdType.numArrayElements)    {
                        case 2: changed = ImGui::SliderInt2("##SliderInt2Editor",value,minValue,maxValue,precisionStr);break;
                        case 3: changed = ImGui::SliderInt3("##SliderInt3Editor",value,minValue,maxValue,precisionStr);break;
                        case 4: changed = ImGui::SliderInt4("##SliderInt4Editor",value,minValue,maxValue,precisionStr);break;
                        default: changed = ImGui::SliderInt("##SliderIntEditor",value,minValue,maxValue,precisionStr);break;
                        }
                        if (changed)    {
                            for (int vl=0;vl<hdType.numArrayElements;vl++) {
                                *(pField+vl) = (int) value[vl];
                            }
                        }
                    }
                        break;
                    case HT_BOOL:   {
                        bool * boolPtr = (bool*) cd.fieldPtr;
                        if (*boolPtr) ImGui::Checkbox("true##CheckboxBoolEditor",boolPtr);    // returns true when pressed
                        else ImGui::Checkbox("false##CheckboxBoolEditor",boolPtr);            // returns true when pressed
                    }
                        break;
                    case HT_ENUM: {
                        ImGui::Combo("##ComboEnumEditor",(int*) cd.fieldPtr,hdType.textFromEnumFunctionPointer,hdType.textFromEnumFunctionPointerUserData,hdType.numEnumElements);
                    }
                        break;
                    case HT_STRING: {
                        char* txtField = (char*)  cd.fieldPtr;
                        ImGui::InputText("##InputTextEditor",txtField,hdPrecision,ImGuiInputTextFlags_EnterReturnsTrue);
                    }
                        break;
                    case HT_COLOR:  {
                        float* pColor = (float*) cd.fieldPtr;
                        ImGui::ColorEditMode(colorEditingMode);
                        if (hdType.numArrayElements==3) ImGui::ColorEdit3("##ColorEdit3Editor",pColor);
                        else ImGui::ColorEdit4("##ColorEdit4Editor",pColor);
                    }
                    default: break;
                    }
                    ImGui::PopStyleVar();
                    // End Draw Editor here----------------------------------------

                }
                else {
                    partOfTheCellClicked = false;
                    txt = NULL;
                    if (hdType.headerType==HT_CUSTOM) {
                        txt = cd.customText;
                        if (txt && txt[0]=='\0') txt=NULL;  // Optional
                    }
                    else {
                        txt = GetTextFromCellFieldDataPtr(*hd,cd.fieldPtr);
                        if (txt && txt[0]=='\0') txt=NULL;  // Optional
                        if (hdType.headerType==HT_COLOR)   {
                            const float *pFloat = (const float*) cd.fieldPtr;
                            const ImVec4 color = ImVec4(*pFloat,*(pFloat+1),*(pFloat+2),hdType.numArrayElements==3?1.f:(*(pFloat+3)));
                            ImGui::PushStyleColor(ImGuiCol_Button,color);
                            partOfTheCellClicked = ImGui::ColorButton(color,true);if (txt) ImGui::SameLine();
                        }
                        else if (hdType.headerType==HT_ICON)    {
                            const CellData::IconData* pIconData = (const CellData::IconData*) cd.fieldPtr;
                            if (pIconData->user_texture_id) {
                                ImVec2 iconSize;
                                iconSize.x = iconSize.y = ImGui::GetTextLineHeight();
                                partOfTheCellClicked = ImGui::ImageButton(pIconData->user_texture_id,iconSize,pIconData->uv0,pIconData->uv1,0,pIconData->bg_col,pIconData->tint_col);
                                if (txt) ImGui::SameLine();
                            }
                        }
                    }

                    if (txt) {
                        if (isThisRowSelected && !hdEditable) {
                            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,ImGuiColHeader);
                            ImGui::PushStyleColor(ImGuiCol_HeaderActive,ImGuiColHeader);
                        }
                        if (ImGui::Selectable(txt,cd.selectedRowPtr) || partOfTheCellClicked)  {
                            if (!*cd.selectedRowPtr) {
                                *cd.selectedRowPtr = true;
                                if (!partOfTheCellClicked) editorAllowed = (selectedColumn==col);
                                else editorAllowed = false;
                            }
                            else editorAllowed = false;
                            if (selectedRow!=row)   {
                                rowSelectionChanged = true;
                                popupMenuOpenAtSelectedColumn = -1;
                                if (selectedRow>=0 && selectedRow<(int)numRows)  {
                                    // remove old selection
                                    CellData cdOld;getCellData((size_t)selectedRow,0,cdOld);  // Note that we use column 0 (since we retrieve a row data it makes no difference)
                                    if (cdOld.selectedRowPtr) *cdOld.selectedRowPtr = false;
                                }
                            }
                            selectedRow = row;
                            if (selectedColumn!=col) colSelectionChanged = true;
                            selectedColumn = col;
                        }
                        if (isThisRowSelected && !hdEditable) {
                            //-------------------------------------------------------------------------
                            if (selectedRowPopupMenu)    {
                                if (ImGui::GetIO().MouseDown[1] && ImGui::IsItemHovered()) popupMenuOpenAtSelectedColumn = col;
                                if (popupMenuOpenAtSelectedColumn==col) {
                                    //static bool open=true;  // Should it be member variable ?
                                    //ImGui::BeginPopup(&open);
                                    ImGui::OpenPopup("MyOwnListViewMenu");
                                    if (ImGui::BeginPopup("MyOwnListViewMenu")) {
                                        if (selectedRowPopupMenu(row,col,selectedRowPopupMenuUserData)) popupMenuOpenAtSelectedColumn = -1;
                                        else {
                                            ImVec2 pos = ImGui::GetWindowPos();pos.x-=5;pos.y-=5;
                                            ImVec2 size = ImGui::GetWindowSize();size.x+=10;size.y+=10;
                                            const ImVec2& mousePos = ImGui::GetIO().MousePos;
                                            if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) popupMenuOpenAtSelectedColumn = -1;
                                        }
                                    }
                                    ImGui::EndPopup();
                                }
                            }
                            //----------------------------------------------------------------------------

                            // must be the same colors as (*)
                            ImGui::PopStyleColor();
                            ImGui::PopStyleColor();
                        }
                    }

                    if (hdType.headerType==HT_COLOR) {
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::PopID();
            }
            if (!hdEditable) {
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }
            if (hdType.headerType==HT_COLOR || hdType.headerType==HT_ICON)    {
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }
            if (colID!=maxNumColumnToDisplay-1) ImGui::NextColumn();
        }
        ImGui::Columns(1);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((numRows - displayEnd) * itemHeight));
        firstTimeDrawingRows = false;
    }   // skipDisplaying
    if (!useFullHeight) ImGui::EndChild();

    ImGui::PushStyleColor(ImGuiCol_Border,style.Colors[ImGuiCol_Column]);
    ImGui::Separator();
    ImGui::PopStyleColor();


    ImGui::PopID();

    scrollToRow = -1;   // we must reset it

    // Sorting:
    if (columnSortingIndex>=0) const_cast<ListViewBase*>(this)->sort((size_t) columnSortingIndex);

    return rowSelectionChanged; // Optional data we might want to expose: local variable: 'colSelectionChanged' and class variable: 'isInEditingMode'.
}

bool ImGui::ListView::sort(size_t column) {
    if ((int)column>=headers.size()) return false;
    Header& h = headers[column];
    HeaderData::Sorting& hds = h.hd.sorting;
    if (!hds.sortable) return false;

    // void qsort( void *ptr, size_t count, size_t size,int (*comp)(const void *, const void *) );
    bool& sortingOrder = hds.sortingAscending;
    ItemBase::SortingHelper sorter((int)column,sortingOrder,&hds.sortableElementsOfPossibleArray[0]);   // This IS actually used!
    typedef int (*CompareDelegate)(const void *, const void *);
    CompareDelegate compareFunction = NULL;

    switch (h.hd.type.headerType)  {
    case HT_BOOL:
        compareFunction = ItemBase::SortingHelper::Compare_HT_BOOL;
        break;
    case HT_CUSTOM:
        compareFunction = ItemBase::SortingHelper::Compare_HT_CUSTOM;
        break;
    case HT_INT:
    case HT_ENUM:
        compareFunction = ItemBase::SortingHelper::Compare<int>;
        break;
    case HT_UNSIGNED:
        compareFunction = ItemBase::SortingHelper::Compare<unsigned>;
        break;
    case HT_FLOAT:
    case HT_COLOR:
        compareFunction = ItemBase::SortingHelper::Compare<float>;
        break;
    case HT_DOUBLE:
        compareFunction = ItemBase::SortingHelper::Compare<double>;
        break;
    case HT_STRING:
        compareFunction = ItemBase::SortingHelper::Compare<char*>;
        break;
    case HT_ICON:
        compareFunction = ItemBase::SortingHelper::Compare_HT_ICON;
        break;
    default:
        return false;
    }
    if (!compareFunction) return false;

    qsort((void *) &items[0],items.size(),sizeof(ItemBase*),compareFunction);
    sortingOrder = !sortingOrder;   // next time it sorts backwards

    updateSelectedRow(); // rows get shuffled after sorting: the visible selection is still correct (the boolean flag ItemBase::selected is stored in our row-item),
    // but the 'selectedRow' field is not updated and must be adjusted
    return true;
}
