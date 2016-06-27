#ifndef IMGUIHELPER_H_
#define IMGUIHELPER_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API


namespace ImGui {

// Experimental: tested on Ubuntu only. Should work with urls, folders and files.
bool OpenWithDefaultApplication(const char* url,bool exploreModeForWindowsOS=false);

void CloseAllPopupMenus();  // Neve Tested

#ifndef NO_IMGUIHELPER_FONT_METHODS
void InitPushFontOverload();            // Internally fills a static ImFont* vector, consolidating the Font order, so that the following Font Methods are available:

const ImFont* GetFont(int fntIndex);
void PushFont(int fntIndex);    // using the index of the font instead of a ImFont* is easier (you can set up an enum).
void TextColoredV(int fntIndex,const ImVec4& col, const char* fmt, va_list args);
void TextColored(int fntIndex,const ImVec4& col, const char* fmt, ...) IM_PRINTFARGS(3);
void TextV(int fntIndex,const char* fmt, va_list args);
void Text(int fntIndex,const char* fmt, ...) IM_PRINTFARGS(2);

// Handy if we want to use ImGui::Image(...) or ImGui::ImageButton(...) with a glyph
bool GetTexCoordsFromGlyph(unsigned short glyph,ImVec2& uv0,ImVec2& uv1);
// Returns the height of the main menu based on the current font and style
float CalcMainMenuHeight();
#endif //NO_IMGUIHELPER_FONT_METHODS

#ifndef NO_IMGUIHELPER_DRAW_METHODS
// Extensions to ImDrawList
void ImDrawListAddConvexPolyFilledWithVerticalGradient(ImDrawList* dl, const ImVec2* points, const int points_count, ImU32 colTop, ImU32 colBot, bool anti_aliased, float miny=-1.f, float maxy=-1.f);
void ImDrawListPathFillWithVerticalGradientAndStroke(ImDrawList* dl, const ImU32& fillColorTop, const ImU32& fillColorBottom, const ImU32& strokeColor, bool strokeClosed=false, float strokeThickness = 1.0f, bool antiAliased = true, float miny=-1.f, float maxy=-1.f);
void ImDrawListPathFillAndStroke(ImDrawList* dl,const ImU32& fillColor,const ImU32& strokeColor,bool strokeClosed=false, float strokeThickness = 1.0f, bool antiAliased = true);
void ImDrawListAddRect(ImDrawList* dl,const ImVec2& a, const ImVec2& b,const ImU32& fillColor,const ImU32& strokeColor,float rounding = 0.0f, int rounding_corners = 0x0F,float strokeThickness = 1.0f,bool antiAliased = true);
void ImDrawListAddRectWithVerticalGradient(ImDrawList* dl,const ImVec2& a, const ImVec2& b,const ImU32& fillColorTop,const ImU32& fillColorBottom,const ImU32& strokeColor,float rounding = 0.0f, int rounding_corners = 0x0F,float strokeThickness = 1.0f,bool antiAliased = true);
void ImDrawListAddRectWithVerticalGradient(ImDrawList* dl,const ImVec2& a, const ImVec2& b,const ImU32& fillColor,float fillColorGradientDeltaIn0_05,const ImU32& strokeColor,float rounding = 0.0f, int rounding_corners = 0x0F,float strokeThickness = 1.0f,bool antiAliased = true);
void ImDrawListPathArcTo(ImDrawList* dl,const ImVec2& centre,const ImVec2& radii, float amin, float amax, int num_segments = 10);
void ImDrawListAddEllipse(ImDrawList* dl,const ImVec2& centre, const ImVec2& radii,const ImU32& fillColor,const ImU32& strokeColor,int num_segments = 12,float strokeThickness = 1.f,bool antiAliased = true);
void ImDrawListAddEllipseWithVerticalGradient(ImDrawList* dl,const ImVec2& centre, const ImVec2& radii,const ImU32& fillColorTop,const ImU32& fillColorBottom,const ImU32& strokeColor,int num_segments = 12,float strokeThickness = 1.f,bool antiAliased = true);
void ImDrawListAddCircle(ImDrawList* dl,const ImVec2& centre, float radius,const ImU32& fillColor,const ImU32& strokeColor,int num_segments = 12,float strokeThickness = 1.f,bool antiAliased = true);
void ImDrawListAddCircleWithVerticalGradient(ImDrawList* dl,const ImVec2& centre, float radius,const ImU32& fillColorTop,const ImU32& fillColorBottom,const ImU32& strokeColor,int num_segments = 12,float strokeThickness = 1.f,bool antiAliased = true);

void ImDrawListAddConvexPolyFilledWithHorizontalGradient(ImDrawList *dl, const ImVec2 *points, const int points_count, ImU32 colLeft, ImU32 colRight, bool anti_aliased,float minx=-1.f,float maxx=-1.f);
void ImDrawListPathFillWithHorizontalGradientAndStroke(ImDrawList *dl, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, bool strokeClosed=false, float strokeThickness = 1.0f, bool antiAliased = true,float minx=-1.f,float maxx=-1.f);
void ImDrawListAddRectWithHorizontalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColorLeft, const ImU32 &fillColoRight, const ImU32 &strokeColor, float rounding = 0.0f, int rounding_corners = 0x0F, float strokeThickness = 1.0f, bool antiAliased = true);
void ImDrawListAddEllipseWithHorizontalGradient(ImDrawList *dl, const ImVec2 &centre, const ImVec2 &radii, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, int num_segments = 12, float strokeThickness = 1.0f, bool antiAliased = true);
void ImDrawListAddCircleWithHorizontalGradient(ImDrawList *dl, const ImVec2 &centre, float radius, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, int num_segments = 12, float strokeThickness = 1.0f, bool antiAliased = true);
void ImDrawListAddRectWithHorizontalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, float fillColorGradientDeltaIn0_05, const ImU32 &strokeColor, float rounding = 0.0f, int rounding_corners = 0x0F, float strokeThickness = 1.0f, bool antiAliased = true);


#ifndef NO_IMGUIHELPER_VERTICAL_TEXT_METHODS
#   ifdef IMGUIHELPER_HAS_VERTICAL_TEXT_SUPPORT
#       warning Don't define IMGUIHELPER_HAS_VERTICAL_TEXT_SUPPORT yorself! It's a read-only definition!
#   else //IMGUIHELPER_HAS_VERTICAL_TEXT_SUPPORT
#       define IMGUIHELPER_HAS_VERTICAL_TEXT_SUPPORT
#   endif //IMGUIHELPER_HAS_VERTICAL_TEXT_SUPPORT
ImVec2 CalcVerticalTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);
void RenderTextVertical(const ImFont* font,ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end=NULL, float wrap_width=0.0f, bool cpu_fine_clip=false, bool rotateCCW=false);
void AddTextVertical(ImDrawList* drawList,const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end=NULL, float wrap_width=0.0f, const ImVec4* cpu_fine_clip_rect=NULL,bool rotateCCW = false);
void AddTextVertical(ImDrawList* drawList,const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end=NULL,bool rotateCCW = false);
void RenderTextVerticalClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, ImGuiAlign align =  ImGuiAlign_Default, const ImVec2* clip_min=NULL, const ImVec2* clip_max=NULL,bool rotateCCW=false);
#endif //NO_IMGUIHELPER_VERTICAL_TEXT_METHODS
#endif //NO_IMGUIHELPER_DRAW_METHODS

// These two methods are inspired by imguidock.cpp
// if optionalRootWindowName==NULL, they refer to the current window
// P.S. This methods are never used anywhere, and it's not clear to me when
// PutInForeground() is better then ImGui::SetWindowFocus()
void PutInBackground(const char* optionalRootWindowName=NULL);
void PutInForeground(const char* optionalRootWindowName=NULL);


#   ifdef IMGUI_USE_ZLIB	// requires linking to library -lZlib
// Two methods that fill rv and return true on success
#       ifndef NO_IMGUIHELPER_SERIALIZATION
#           ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
bool GzDecompressFromFile(const char* filePath,ImVector<char>& rv,bool clearRvBeforeUsage=true);
#           endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#       endif //NO_IMGUIHELPER_SERIALIZATION
bool GzDecompressFromMemory(const char* memoryBuffer,int memoryBufferSize,ImVector<char>& rv,bool clearRvBeforeUsage=true);
#   endif //IMGUI_USE_ZLIB

// IMPORTANT: FT_INT,FT_UNSIGNED,FT_FLOAT,FT_DOUBLE,FT_BOOL support from 1 to 4 components.
enum FieldType {
    FT_INT=0,
    FT_UNSIGNED,
    FT_FLOAT,
    FT_DOUBLE,
    //--------------- End types that support 1 to 4 array components ----------
    FT_STRING,      // an arbitrary-length string (or a char blob that can be used as custom type)
    FT_ENUM,        // serialized/deserialized as FT_INT
    FT_BOOL,
    FT_COLOR,       // serialized/deserialized as FT_FLOAT (with 3 or 4 components)
    FT_TEXTLINE,    // a (series of) text line(s) (separated by '\n') that are fed one at a time in the Deserializer callback
    FT_CUSTOM,      // a custom type that is served like FT_TEXTLINE (=one line at a time).
    FT_COUNT
};

}   // ImGui


// These classed are supposed to be used internally
namespace ImGuiHelper {
typedef ImGui::FieldType FieldType;

#ifndef NO_IMGUIHELPER_SERIALIZATION

#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
bool GetFileContent(const char* filePath,ImVector<char>& contentOut,bool clearContentOutBeforeUsage=true,const char* modes="rb",bool appendTrailingZeroIfModesIsNotBinary=true);
bool FileExists(const char* filePath);

class Deserializer {
    char* f_data;
    size_t f_size;
    void clear();
    bool loadFromFile(const char* filename);
    bool allocate(size_t sizeToAllocate,const char* optionalTextToCopy=NULL,size_t optionalTextToCopySize=0);
    public:
    Deserializer() : f_data(NULL),f_size(0) {}
    Deserializer(const char* filename);                     // From file
    Deserializer(const char* text,size_t textSizeInBytes);  // From memory (and optionally from file through GetFileContent(...))
    ~Deserializer() {clear();}
    bool isValid() const {return (f_data && f_size>0);}

    // returns whether to stop parsing or not
    typedef bool (*ParseCallback)(FieldType ft,int numArrayElements,void* pValue,const char* name,void* userPtr);   // (*)
    // returns a pointer to "next_line" if the callback has stopped parsing or NULL.
    // returned value can be refeed as optionalBufferStart
    const char *parse(ParseCallback cb,void* userPtr,const char* optionalBufferStart=NULL) const;

    // (*)
    /*
    FT_CUSTOM and FT_TEXTLINE are served multiple times (one per text line) with numArrayElements that goes from 0 to numTextLines-1.
    All the other field types are served once.
    */

protected:
    void operator=(const Deserializer&) {}
    Deserializer(const Deserializer&) {}
};
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD

#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
class ISerializable;
class Serializer {

    ISerializable* f;
    void clear();

    public:
    Serializer(const char* filename);               // To file
    Serializer(int memoryBufferCapacity=2048);      // To memory (and optionally to file through WriteBufferToFile(...))
    ~Serializer();
    bool isValid() const {return (f);}

    bool save(FieldType ft, const float* pValue, const char* name, int numArrayElements=1,int prec=3);
    bool save(FieldType ft, const int* pValue, const char* name, int numArrayElements=1,int prec=-1);
    bool save(const float* pValue,const char* name,int numArrayElements=1,int prec=3)    {
        return save(ImGui::FT_FLOAT,pValue,name,numArrayElements,prec);
    }
    bool save(const int* pValue,const char* name,int numArrayElements=1,int prec=-1)  {
        return save(ImGui::FT_INT,pValue,name,numArrayElements,prec);
    }
    bool save(const char* pValue,const char* name,int pValueSize=-1);
    bool save(const unsigned* pValue, const char* name, int numArrayElements=1,int prec=-1);
    bool save(const double* pValue, const char* name, int numArrayElements=1,int prec=-1);
    bool save(const bool* pValue, const char* name, int numArrayElements=1);
    bool saveTextLines(const char* pValue,const char* name); // Splits the string into N lines: each line is passed by the deserializer into a single element in the callback

    // To serialize FT_CUSTOM:
    bool saveCustomFieldTypeHeader(const char* name, int numTextLines=1); //e.g. for 4 lines "[CUSTOM-4:MyCustomFieldTypeName]\n". Then add 4 lines using getPointer() below.

    // These 2 are only available when this class is constructed with the
    // Serializer(int memoryBufferCapacity) constructor
    const char* getBuffer() const;
    int getBufferSize() const;
    static bool WriteBufferToFile(const char* filename, const char* buffer, int bufferSize);

protected:
    void operator=(const Serializer&) {}
    Serializer(const Serializer&) {}

};
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#endif //NO_IMGUIHELPER_SERIALIZATION

// Optional String Helper methods:
// "destText" must be released with ImGui::MemFree(destText). It should always work.
void StringSet(char*& destText,const char* text,bool allowNullDestText=true);
// "destText" must be released with ImGui::MemFree(destText). It should always work.
void StringAppend(char*& destText, const char* textToAppend, bool allowNullDestText=true, bool prependLineFeedIfDestTextIsNotEmpty = true, bool mustAppendLineFeed = false);
// Appends a formatted string to a char vector (= no need to free memory)
// v can't be empty (it must at least be: v.size()==1 && v[0]=='\0')
// returns the number of chars appended.
int StringAppend(ImVector<char>& v,const char* fmt, ...);

} // ImGuiHelper

#ifndef NO_IMGUIKNOWNCOLOR_DEFINITIONS
#define KNOWNIMGUICOLOR_ALICEBLUE ImColor(240,248,255)
#define KNOWNIMGUICOLOR_ANTIQUEWHITE ImColor(250,235,215)
#define KNOWNIMGUICOLOR_AQUA ImColor(0,255,255)
#define KNOWNIMGUICOLOR_AQUAMARINE ImColor(127,255,212)
#define KNOWNIMGUICOLOR_AZURE ImColor(240,255,255)
#define KNOWNIMGUICOLOR_BEIGE ImColor(245,245,220)
#define KNOWNIMGUICOLOR_BISQUE ImColor(255,228,196)
#define KNOWNIMGUICOLOR_BLACK ImColor(0,0,0)
#define KNOWNIMGUICOLOR_BLANCHEDALMOND ImColor(255,235,205)
#define KNOWNIMGUICOLOR_BLUE ImColor(0,0,255)
#define KNOWNIMGUICOLOR_BLUEVIOLET ImColor(138,43,226)
#define KNOWNIMGUICOLOR_BROWN ImColor(165,42,42)
#define KNOWNIMGUICOLOR_BURLYWOOD ImColor(222,184,135)
#define KNOWNIMGUICOLOR_CADETBLUE ImColor(95,158,160)
#define KNOWNIMGUICOLOR_CHARTREUSE ImColor(127,255,0)
#define KNOWNIMGUICOLOR_CHOCOLATE ImColor(210,105,30)
#define KNOWNIMGUICOLOR_CORAL ImColor(255,127,80)
#define KNOWNIMGUICOLOR_CORNFLOWERBLUE ImColor(100,149,237)
#define KNOWNIMGUICOLOR_CORNSILK ImColor(255,248,220)
#define KNOWNIMGUICOLOR_CRIMSON ImColor(220,20,60)
#define KNOWNIMGUICOLOR_CYAN ImColor(0,255,255)
#define KNOWNIMGUICOLOR_DARKBLUE ImColor(0,0,139)
#define KNOWNIMGUICOLOR_DARKCYAN ImColor(0,139,139)
#define KNOWNIMGUICOLOR_DARKGOLDENROD ImColor(184,134,11)
#define KNOWNIMGUICOLOR_DARKGRAY ImColor(169,169,169)
#define KNOWNIMGUICOLOR_DARKGREEN ImColor(0,100,0)
#define KNOWNIMGUICOLOR_DARKKHAKI ImColor(189,183,107)
#define KNOWNIMGUICOLOR_DARKMAGENTA ImColor(139,0,139)
#define KNOWNIMGUICOLOR_DARKOLIVEGREEN ImColor(85,107,47)
#define KNOWNIMGUICOLOR_DARKORANGE ImColor(255,140,0)
#define KNOWNIMGUICOLOR_DARKORCHID ImColor(153,50,204)
#define KNOWNIMGUICOLOR_DARKRED ImColor(139,0,0)
#define KNOWNIMGUICOLOR_DARKSALMON ImColor(233,150,122)
#define KNOWNIMGUICOLOR_DARKSEAGREEN ImColor(143,188,139)
#define KNOWNIMGUICOLOR_DARKSLATEBLUE ImColor(72,61,139)
#define KNOWNIMGUICOLOR_DARKSLATEGRAY ImColor(47,79,79)
#define KNOWNIMGUICOLOR_DARKTURQUOISE ImColor(0,206,209)
#define KNOWNIMGUICOLOR_DARKVIOLET ImColor(148,0,211)
#define KNOWNIMGUICOLOR_DEEPPINK ImColor(255,20,147)
#define KNOWNIMGUICOLOR_DEEPSKYBLUE ImColor(0,191,255)
#define KNOWNIMGUICOLOR_DIMGRAY ImColor(105,105,105)
#define KNOWNIMGUICOLOR_DODGERBLUE ImColor(30,144,255)
#define KNOWNIMGUICOLOR_FIREBRICK ImColor(178,34,34)
#define KNOWNIMGUICOLOR_FLORALWHITE ImColor(255,250,240)
#define KNOWNIMGUICOLOR_FORESTGREEN ImColor(34,139,34)
#define KNOWNIMGUICOLOR_FUCHSIA ImColor(255,0,255)
#define KNOWNIMGUICOLOR_GAINSBORO ImColor(220,220,220)
#define KNOWNIMGUICOLOR_GHOSTWHITE ImColor(248,248,255)
#define KNOWNIMGUICOLOR_GOLD ImColor(255,215,0)
#define KNOWNIMGUICOLOR_GOLDENROD ImColor(218,165,32)
#define KNOWNIMGUICOLOR_GRAY ImColor(128,128,128)
#define KNOWNIMGUICOLOR_GREEN ImColor(0,128,0)
#define KNOWNIMGUICOLOR_GREENYELLOW ImColor(173,255,47)
#define KNOWNIMGUICOLOR_HONEYDEW ImColor(240,255,240)
#define KNOWNIMGUICOLOR_HOTPINK ImColor(255,105,180)
#define KNOWNIMGUICOLOR_INDIANRED ImColor(205,92,92)
#define KNOWNIMGUICOLOR_INDIGO ImColor(75,0,130)
#define KNOWNIMGUICOLOR_IVORY ImColor(255,255,240)
#define KNOWNIMGUICOLOR_KHAKI ImColor(240,230,140)
#define KNOWNIMGUICOLOR_LAVENDER ImColor(230,230,250)
#define KNOWNIMGUICOLOR_LAVENDERBLUSH ImColor(255,240,245)
#define KNOWNIMGUICOLOR_LAWNGREEN ImColor(124,252,0)
#define KNOWNIMGUICOLOR_LEMONCHIFFON ImColor(255,250,205)
#define KNOWNIMGUICOLOR_LIGHTBLUE ImColor(173,216,230)
#define KNOWNIMGUICOLOR_LIGHTCORAL ImColor(240,128,128)
#define KNOWNIMGUICOLOR_LIGHTCYAN ImColor(224,255,255)
#define KNOWNIMGUICOLOR_LIGHTGOLDENRODYELLOW ImColor(250,250,210)
#define KNOWNIMGUICOLOR_LIGHTGRAY ImColor(211,211,211)
#define KNOWNIMGUICOLOR_LIGHTGREEN ImColor(144,238,144)
#define KNOWNIMGUICOLOR_LIGHTPINK ImColor(255,182,193)
#define KNOWNIMGUICOLOR_LIGHTSALMON ImColor(255,160,122)
#define KNOWNIMGUICOLOR_LIGHTSEAGREEN ImColor(32,178,170)
#define KNOWNIMGUICOLOR_LIGHTSKYBLUE ImColor(135,206,250)
#define KNOWNIMGUICOLOR_LIGHTSLATEGRAY ImColor(119,136,153)
#define KNOWNIMGUICOLOR_LIGHTSTEELBLUE ImColor(176,196,222)
#define KNOWNIMGUICOLOR_LIGHTYELLOW ImColor(255,255,224)
#define KNOWNIMGUICOLOR_LIME ImColor(0,255,0)
#define KNOWNIMGUICOLOR_LIMEGREEN ImColor(50,205,50)
#define KNOWNIMGUICOLOR_LINEN ImColor(250,240,230)
#define KNOWNIMGUICOLOR_MAGENTA ImColor(255,0,255)
#define KNOWNIMGUICOLOR_MAROON ImColor(128,0,0)
#define KNOWNIMGUICOLOR_MEDIUMAQUAMARINE ImColor(102,205,170)
#define KNOWNIMGUICOLOR_MEDIUMBLUE ImColor(0,0,205)
#define KNOWNIMGUICOLOR_MEDIUMORCHID ImColor(186,85,211)
#define KNOWNIMGUICOLOR_MEDIUMPURPLE ImColor(147,112,219)
#define KNOWNIMGUICOLOR_MEDIUMSEAGREEN ImColor(60,179,113)
#define KNOWNIMGUICOLOR_MEDIUMSLATEBLUE ImColor(123,104,238)
#define KNOWNIMGUICOLOR_MEDIUMSPRINGGREEN ImColor(0,250,154)
#define KNOWNIMGUICOLOR_MEDIUMTURQUOISE ImColor(72,209,204)
#define KNOWNIMGUICOLOR_MEDIUMVIOLETRED ImColor(199,21,133)
#define KNOWNIMGUICOLOR_MIDNIGHTBLUE ImColor(25,25,112)
#define KNOWNIMGUICOLOR_MINTCREAM ImColor(245,255,250)
#define KNOWNIMGUICOLOR_MISTYROSE ImColor(255,228,225)
#define KNOWNIMGUICOLOR_MOCCASIN ImColor(255,228,181)
#define KNOWNIMGUICOLOR_NAVAJOWHITE ImColor(255,222,173)
#define KNOWNIMGUICOLOR_NAVY ImColor(0,0,128)
#define KNOWNIMGUICOLOR_OLDLACE ImColor(253,245,230)
#define KNOWNIMGUICOLOR_OLIVE ImColor(128,128,0)
#define KNOWNIMGUICOLOR_OLIVEDRAB ImColor(107,142,35)
#define KNOWNIMGUICOLOR_ORANGE ImColor(255,165,0)
#define KNOWNIMGUICOLOR_ORANGERED ImColor(255,69,0)
#define KNOWNIMGUICOLOR_ORCHID ImColor(218,112,214)
#define KNOWNIMGUICOLOR_PALEGOLDENROD ImColor(238,232,170)
#define KNOWNIMGUICOLOR_PALEGREEN ImColor(152,251,152)
#define KNOWNIMGUICOLOR_PALETURQUOISE ImColor(175,238,238)
#define KNOWNIMGUICOLOR_PALEVIOLETRED ImColor(219,112,147)
#define KNOWNIMGUICOLOR_PAPAYAWHIP ImColor(255,239,213)
#define KNOWNIMGUICOLOR_PEACHPUFF ImColor(255,218,185)
#define KNOWNIMGUICOLOR_PERU ImColor(205,133,63)
#define KNOWNIMGUICOLOR_PINK ImColor(255,192,203)
#define KNOWNIMGUICOLOR_PLUM ImColor(221,160,221)
#define KNOWNIMGUICOLOR_POWDERBLUE ImColor(176,224,230)
#define KNOWNIMGUICOLOR_PURPLE ImColor(128,0,128)
#define KNOWNIMGUICOLOR_RED ImColor(255,0,0)
#define KNOWNIMGUICOLOR_ROSYBROWN ImColor(188,143,143)
#define KNOWNIMGUICOLOR_ROYALBLUE ImColor(65,105,225)
#define KNOWNIMGUICOLOR_SADDLEBROWN ImColor(139,69,19)
#define KNOWNIMGUICOLOR_SALMON ImColor(250,128,114)
#define KNOWNIMGUICOLOR_SANDYBROWN ImColor(244,164,96)
#define KNOWNIMGUICOLOR_SEAGREEN ImColor(46,139,87)
#define KNOWNIMGUICOLOR_SEASHELL ImColor(255,245,238)
#define KNOWNIMGUICOLOR_SIENNA ImColor(160,82,45)
#define KNOWNIMGUICOLOR_SILVER ImColor(192,192,192)
#define KNOWNIMGUICOLOR_SKYBLUE ImColor(135,206,235)
#define KNOWNIMGUICOLOR_SLATEBLUE ImColor(106,90,205)
#define KNOWNIMGUICOLOR_SLATEGRAY ImColor(112,128,144)
#define KNOWNIMGUICOLOR_SNOW ImColor(255,250,250)
#define KNOWNIMGUICOLOR_SPRINGGREEN ImColor(0,255,127)
#define KNOWNIMGUICOLOR_STEELBLUE ImColor(70,130,180)
#define KNOWNIMGUICOLOR_TAN ImColor(210,180,140)
#define KNOWNIMGUICOLOR_TEAL ImColor(0,128,128)
#define KNOWNIMGUICOLOR_THISTLE ImColor(216,191,216)
#define KNOWNIMGUICOLOR_TOMATO ImColor(255,99,71)
#define KNOWNIMGUICOLOR_TURQUOISE ImColor(64,224,208)
#define KNOWNIMGUICOLOR_VIOLET ImColor(238,130,238)
#define KNOWNIMGUICOLOR_WHEAT ImColor(245,222,179)
#define KNOWNIMGUICOLOR_WHITE ImColor(255,255,255)
#define KNOWNIMGUICOLOR_WHITESMOKE ImColor(245,245,245)
#define KNOWNIMGUICOLOR_YELLOW ImColor(255,255,0)
#define KNOWNIMGUICOLOR_YELLOWGREEN ImColor(154,205,50)
#endif // NO_IMGUIKNOWNCOLOR_DEFINITIONS

#endif //IMGUIHELPER_H_

