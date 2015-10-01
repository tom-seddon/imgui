#ifndef IMGUIHELPER_H_
#define IMGUIHELPER_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

/*
// Math bits
// We are keeping those static in the .cpp file so as not to leak them outside, in the case the user has implicit cast operators between ImVec2 and its own types.
static inline ImVec2 operator*(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x*rhs, lhs.y*rhs); }
//static inline ImVec2 operator/(const ImVec2& lhs, const float rhs)            { return ImVec2(lhs.x/rhs, lhs.y/rhs); }
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x+rhs.x, lhs.y+rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x-rhs.x, lhs.y-rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2 rhs)             { return ImVec2(lhs.x*rhs.x, lhs.y*rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2 rhs)             { return ImVec2(lhs.x/rhs.x, lhs.y/rhs.y); }
static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const float rhs)                  { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
//static inline ImVec2& operator/=(ImVec2& lhs, const float rhs)                { lhs.x /= rhs; lhs.y /= rhs; return lhs; }

static inline int    ImMin(int lhs, int rhs)                                    { return lhs < rhs ? lhs : rhs; }
static inline int    ImMax(int lhs, int rhs)                                    { return lhs >= rhs ? lhs : rhs; }
static inline float  ImMin(float lhs, float rhs)                                { return lhs < rhs ? lhs : rhs; }
static inline float  ImMax(float lhs, float rhs)                                { return lhs >= rhs ? lhs : rhs; }
static inline ImVec2 ImMin(const ImVec2& lhs, const ImVec2& rhs)                { return ImVec2(ImMin(lhs.x,rhs.x), ImMin(lhs.y,rhs.y)); }
static inline ImVec2 ImMax(const ImVec2& lhs, const ImVec2& rhs)                { return ImVec2(ImMax(lhs.x,rhs.x), ImMax(lhs.y,rhs.y)); }
static inline int    ImClamp(int v, int mn, int mx)                             { return (v < mn) ? mn : (v > mx) ? mx : v; }
static inline float  ImClamp(float v, float mn, float mx)                       { return (v < mn) ? mn : (v > mx) ? mx : v; }
static inline ImVec2 ImClamp(const ImVec2& f, const ImVec2& mn, ImVec2 mx)      { return ImVec2(ImClamp(f.x,mn.x,mx.x), ImClamp(f.y,mn.y,mx.y)); }
static inline float  ImSaturate(float f)                                        { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
static inline float  ImLerp(float a, float b, float t)                          { return a + (b - a) * t; }
static inline ImVec2 ImLerp(const ImVec2& a, const ImVec2& b, const ImVec2& t)  { return ImVec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
static inline float  ImLengthSqr(const ImVec2& lhs)                             { return lhs.x*lhs.x + lhs.y*lhs.y; }

struct ImRect           // 2D axis aligned bounding-box
{
    ImVec2      Min;
    ImVec2      Max;

    ImRect()                                        { Min = ImVec2(FLT_MAX,FLT_MAX); Max = ImVec2(-FLT_MAX,-FLT_MAX); }
    ImRect(const ImVec2& min, const ImVec2& max)    { Min = min; Max = max; }
    ImRect(const ImVec4& v)                         { Min.x = v.x; Min.y = v.y; Max.x = v.z; Max.y = v.w; }
    ImRect(float x1, float y1, float x2, float y2)  { Min.x = x1; Min.y = y1; Max.x = x2; Max.y = y2; }

    ImVec2      GetCenter() const                   { return Min + (Max-Min)*0.5f; }
    ImVec2      GetSize() const                     { return Max-Min; }
    float       GetWidth() const                    { return (Max-Min).x; }
    float       GetHeight() const                   { return (Max-Min).y; }
    ImVec2      GetTL() const                       { return Min; }
    ImVec2      GetTR() const                       { return ImVec2(Max.x,Min.y); }
    ImVec2      GetBL() const                       { return ImVec2(Min.x,Max.y); }
    ImVec2      GetBR() const                       { return Max; }
    bool        Contains(const ImVec2& p) const     { return p.x >= Min.x && p.y >= Min.y && p.x <= Max.x && p.y <= Max.y; }
    bool        Contains(const ImRect& r) const     { return r.Min.x >= Min.x && r.Min.y >= Min.y && r.Max.x <= Max.x && r.Max.y <= Max.y; }
    bool        Overlaps(const ImRect& r) const     { return r.Min.y <= Max.y && r.Max.y >= Min.y && r.Min.x <= Max.x && r.Max.x >= Min.x; }
    void        Add(const ImVec2& rhs)              { Min.x = ImMin(Min.x, rhs.x); Min.y = ImMin(Min.y, rhs.y); Max.x = ImMax(Max.x, rhs.x); Max.y = ImMax(Max.x, rhs.x); }
    void        Add(const ImRect& rhs)              { Min.x = ImMin(Min.x, rhs.Min.x); Min.y = ImMin(Min.y, rhs.Min.y); Max.x = ImMax(Max.x, rhs.Max.x); Max.y = ImMax(Max.y, rhs.Max.y); }
    void        Expand(const float amount)          { Min.x -= amount; Min.y -= amount; Max.x += amount; Max.y += amount; }
    void        Expand(const ImVec2& amount)        { Min -= amount; Max += amount; }
    void        Clip(const ImRect& clip)            { Min.x = ImMax(Min.x, clip.Min.x); Min.y = ImMax(Min.y, clip.Min.y); Max.x = ImMin(Max.x, clip.Max.x); Max.y = ImMin(Max.y, clip.Max.y); }
    ImVec2      GetClosestPoint(ImVec2 p, bool on_edge) const
    {
        if (!on_edge && Contains(p))
            return p;
        if (p.x > Max.x) p.x = Max.x;
        else if (p.x < Min.x) p.x = Min.x;
        if (p.y > Max.y) p.y = Max.y;
        else if (p.y < Min.y) p.y = Min.y;
        return p;
    }
};
*/
namespace ImGui {

/*
// returns "pressed"
bool GetButtonBehavior(const ImRect& bb, const ImGuiID& id, bool* out_hovered=NULL, bool* out_held=NULL, bool allow_key_modifiers=true, bool repeat = false, bool pressed_on_click = false);
bool IsWindowDoubleClicked(const ImRect& bb, const ImGuiID& id,int button=0,bool* out_hovered=NULL, bool* out_held=NULL, bool allow_key_modifiers=true, bool repeat = false, bool pressed_on_click = false);
ImRect GetWindowRect();
*/
/*
// Manual resize grip
const ImGuiAabb resize_aabb(window->Aabb().GetBR()-ImVec2(18,18), window->Aabb().GetBR());
const ImGuiID resize_id = window->GetID("#RESIZE");
bool hovered, held;
ButtonBehavior(resize_aabb, resize_id, &hovered, &held, true);
ImU32 resize_col = window->Color(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);

if (g.HoveredWindow == window && held && g.IO.MouseDoubleClicked[0])    {
    //Double click handling
    // Manual auto-fit when double-clicking

    window->SizeFull = size_auto_fit;
    window->Size = window->SizeFull;
    if (!(window->Flags & ImGuiWindowFlags_NoSavedSettings)) MarkSettingsDirty();
}
*/

// Experimental: tested on Ubuntu only. Should work with urls, folders and files.
bool OpenWithDefaultApplication(const char* url,bool exploreModeForWindowsOS=false);

}



#endif //IMGUIHELPER_H_

