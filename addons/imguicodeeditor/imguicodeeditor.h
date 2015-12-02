/*
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 This permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/
#ifndef IMGUICODEEDITOR_H_
#define IMGUICODEEDITOR_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

#ifdef NO_IMGUISTRING
#error imguistring is required for imguicodeeditor to work
#endif //NO_IMGUISTRING

// STATE: NOT USABLE
// TODO/ROADMAP:
/*
SYNTAX HIGHLIGHTING:
->  Fix language initialization             ? What did I mean?
->  Use the HashMap for language keywords -> DONE
->  See if we can remove strtok (maybe we need a profiler)
->  Implement file load/save methods (does we need to read/write BOMs or not ?) -> DONE (without BOM)
->  Test (and fix) the SH with other files (that's the main reason of the previous point) -> DONE (more or less)
->  Implement horizontal scrolling if possible (... ATM even vertical scrolling has problems...) -> DONE

EDITOR:
->  Implement an editor from scratch (how to do it? For sure I have to use a single line editor, but how to integrate it with the existing SH-text-drawing system ?)
->  See the best strategy to modify the folding tags while editing, trying to keep the modifications local if possible. (This is going to be a huge issue that will take up a lot of time)
->  Implement the possibility to select text through multiple lines (and what about selecting while scrolling. Would it be possible?)
->  Implement copy and paste.
->  Implement an Undo/Redo stack.

While reading all this points, I'm seriously thinking about coding it again from scratch... maybe getting inspiration from the source of other free editors.
I'm really thinking that the CodeEditor has about 70% probability of being aborted during development...
...and in any case it will never be as good and reliable as other editors people is already using with ImGui, like:
https://github.com/ocornut/imgui/issues/108
https://github.com/ocornut/imgui/issues/200
*/

 #include "imhashmap.h" // TODO: move to .cpp


namespace ImGuiCe {

// TODO: Hide 65% of the stuff of this header file in the cpp file (e.g.FoldingTag classes)
// TODO: see if it's possible to hide ImString,ImVectorEx and ImHashMaps instances in the cpp file too

enum FontStyle {
    FONT_STYLE_NORMAL=0,
    FONT_STYLE_BOLD,
    FONT_STYLE_ITALIC,
    FONT_STYLE_BOLD_ITALIC,
    FONT_STYLE_COUNT
};
enum Language {
    LANG_NONE=0,
    LANG_CPP,
    LANG_CS,
    LANG_LUA,
    LANG_PYTHON,
    LANG_COUNT
};

enum FoldingType {
    FOLDING_TYPE_PARENTHESIS = 0,
    FOLDING_TYPE_COMMENT,
    FOLDING_TYPE_REGION
};
class FoldingTag   {
public:
    // ex: FoldingTag("{","}","{...}",FOLDING_TYPE_PARENTHESIS,true);   // last arg should be true only for paranthesis AFAIR
    FoldingTag(const ImString& s,const ImString& e,const ImString& _title,FoldingType t,bool _gainOneLineWhenPossible=false);

protected:
    FoldingTag() : start(""),end(""),title(""),kind(FOLDING_TYPE_PARENTHESIS),gainOneLineWhenPossible(false) {}

public: // but please don't touch them
    ImString start,end,title;
    FoldingType kind;
    bool gainOneLineWhenPossible;
};

enum SyntaxHighlightingType {
    //----------------------------------------------------------------------------------------
    SH_KEYWORD_ACCESS=0,              // These can be a multiple strings
    SH_KEYWORD_CONSTANT,
    SH_KEYWORD_CONTEXT,
    SH_KEYWORD_DECLARATION,
    SH_KEYWORD_EXCEPTION,
    SH_KEYWORD_ITERATION,
    SH_KEYWORD_JUMP,
    SH_KEYWORD_KEYWORD_USER1,            // user
    SH_KEYWORD_KEYWORD_USER2,            // user
    SH_KEYWORD_METHOD,
    SH_KEYWORD_MODIFIER,
    SH_KEYWORD_NAMESPACE,
    SH_KEYWORD_OPERATOR,
    SH_KEYWORD_OTHER,
    SH_KEYWORD_PARAMENTER,
    SH_KEYWORD_PREPROCESSOR,
    SH_KEYWORD_PROPERTY,
    SH_KEYWORD_SELECTION,
    SH_KEYWORD_TYPE,
    //-----------------------------------------------------------------------------------------
    //------------------ From now on each token is a vector of single chars -------------------
    SH_LOGICAL_OPERATORS,       //      "&!|~^"
    SH_MATH_OPERATORS,          //      "+-*/<>="
    SH_BRACKETS_CURLY,          //      "{}"
    SH_BRACKETS_SQUARE,         //      "[]"
    SH_BRACKETS_ROUND,           //      "()"
    SH_PUNCTUATION,             //      ".:,;?%"

    SH_STRING,                  //      "\"'"
    SH_NUMBER,
    SH_COMMENT,                 //      "//"

    SH_FOLDED_PARENTHESIS,
    SH_FOLDED_COMMENT,
    SH_FOLDED_REGION,
    SH_COUNT,
};


struct Line;
class Lines : public ImVector<Line*> {
protected:
    typedef ImVector<Line*> Base;
public:
    Lines(const ImString& text="") {cr="\n";clear();setText(text);}
    ~Lines() {destroy();}
    void clear() {destroy(true);}
    Line* add(int lineNum=-1);
    bool remove(int lineNum);
    inline const Line* get(int LineNum) const {return (LineNum<0 || LineNum>=size())?NULL:(*this)[LineNum];}
    inline Line* get(int lineNum) {return (lineNum<0 || lineNum>=size())?NULL:(*this)[lineNum];}

    void getText(ImString& rv,int startLineNum=0,int startLineOffsetBytes=0,int endLineNum=-1,int endLineOffsetBytes=-1) const;
    void getText(ImString& rv,int startTotalOffsetInBytes,int endTotalOffsetBytes) const;
    void setText(const char* text);
    void setText(const ImString& text) {setText(text.c_str());}

protected:
    void destroy(bool keepFirstLine=false);
    void push_back(const Line*) {}
    static void SplitText(const char* text, ImVector<Line *> &lines, ImString *pOptionalCRout=NULL);
    ImString cr;  // "\n" or "\r\n"
    friend class CodeEditor;
    friend class FoldSegment;
    friend class FoldingString;
};

class CodeEditor {
    protected:
    Lines lines;
    bool inited;
    ImVector<Line*> visibleLines;  // just references, recreated every render() call.

public:
    static const ImFont* ImFonts[FONT_STYLE_COUNT];
    bool showIconMargin;
    bool showLineNumbers;
    bool enableTextFolding;
    int scrollToLine;
    bool show_left_pane;
    bool show_style_editor;
    bool show_load_save_buttons;            // TODO
    mutable ImGuiColorEditMode colorEditMode;

    CodeEditor() : inited(false),showIconMargin(true),
    showLineNumbers(true),enableTextFolding(true),scrollToLine(-1),
    show_left_pane(false),show_style_editor(true),show_load_save_buttons(true),colorEditMode(ImGuiColorEditMode_RGB),
    lang(LANG_NONE) {}
    void render();
    static void SetFonts(const ImFont* normal,const ImFont* bold=NULL,const ImFont* italic=NULL,const ImFont* boldItalic=NULL);

    bool isInited() const {return inited;}
    void init();    // optional call

    void setText(const char* text,Language _lang=LANG_NONE);
    void setText(const ImString& text,Language _lang=LANG_NONE) {setText(text.c_str(),_lang);}


#   ifndef NO_IMGUICODEEDITOR_SAVE
    bool save(const char* filename);
#   endif //NO_IMGUICODEEDITOR_SAVE
#   ifndef NO_IMGUICODEEDITOR_LOAD
    bool load(const char* filename,Language optionalLanguage=LANG_COUNT);
#   endif //NO_IMGUICODEEDITOR_LOAD


    // Static Stuff -----------------------------------------------------------------------------------------
    struct Style {
        ImVec4 color_background;
        ImVec4 color_text;
        int font_text;
        ImU32 color_line_numbers_background;
        ImVec4 color_line_numbers;
        int font_line_numbers;
        ImU32 color_icon_margin_background;
        ImU32 color_icon_margin_error;
        ImU32 color_icon_margin_warning;
        ImU32 color_icon_margin_breakpoint;
        ImU32 color_icon_margin_bookmark;
        ImU32 color_icon_margin_contour;
        float icon_margin_contour_thickness;
        ImU32 color_folding_margin_background;

        ImU32 color_syntax_highlighting[SH_COUNT];
        int font_syntax_highlighting[SH_COUNT];

        ImU32 color_folded_parenthesis_background;
        ImU32 color_folded_comment_background;
        ImU32 color_folded_region_background;
        float folded_region_contour_thickness;

        Style();

        static bool Edit(Style& style);
        static void Reset(Style& style) {style = Style();}

#       if (!defined(NO_IMGUIHELPER) && !defined(NO_IMGUIHELPER_SERIALIZATION))
#       ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
        static bool Save(const Style& style,const char* filename);
#       endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#       ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
        static bool Load(Style& style,const char* filename);
#       endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#       endif //NO_IMGUIHELPER_SERIALIZATION

    };
    static Style& GetStyle() {return style;}




    // Folding Support And Syntax Highlighting
    // --------
    // WARNING: This method must be called BEFORE any instance of CodeEditor is initialized (otherwise the program might crash due to ImVectorEx reallocations)    
    static bool SetFoldingSupportForLanguage(Language language,const ImVectorEx<FoldingTag>& foldingTags,bool mergeAdditionalTrailingCharIfPossible=false,char additionalTrailingChar=';');
    static bool HasFoldingSupportForLanguage(Language language);
    // Warning: tokens must be persistent (e.g. pointers to static strings)
    static bool AddSyntaxHighlightingTokens(Language language,SyntaxHighlightingType type,const char** tokens,int numTokens);
    static bool ClearSyntaxHighlightingTokens(Language language,SyntaxHighlightingType type);
    // Warning: strings must be persistent (e.g. pointers to static strings)
    static bool SetSyntaxHighlightingExtraStuff(Language language,const char* singleLineComment=NULL,const char* stringDelimiters=NULL,const char* logicalOperators=NULL,const char* mathOperators=NULL,const char* punctuation=NULL);
    static Language GetLanguageFromFilename(const char* filename);
    // End Static Stuff -------------------------------------------------------------------------------------

    static Style style;

    protected:
    void ParseTextForFolding(bool forceAllSegmentsFoldedOrNot = false, bool foldingStateToForce = true);
    Language lang;
    // We could have stored a pair of these in each "language struct", but this way we save memory and can make per-instance changes
    ImHashMapString shTypeKeywordMap;       // Map from a (persistent) string token to a SyntaxHighlightingType
    ImHashMapChar   shTypePunctuationMap;
private:
    static bool StaticInited;
    void RenderTextLineWrappedWithSH(ImVec2 &pos, const char *text, const char *text_end, bool skipLineCommentProcessing=false);
    void TextLineUnformattedWithSH(const char *text, const char *text_end);
    void TextLineWithSHV(const char *fmt, va_list args);
    void TextLineWithSH(const char *fmt...);

    float CalcTextWidth(const char* text,const char* text_end=NULL,int* pNumUTF8CharsOut=NULL);

};


} // namespace ImGui


#endif //IMGUICODEEDITOR_H_
