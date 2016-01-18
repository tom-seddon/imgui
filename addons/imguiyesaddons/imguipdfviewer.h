// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.


#ifndef IMGUIPDFVIEWER_H_
#define IMGUIPDFVIEWER_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API

#include <poppler/glib/poppler.h>   // I wanted to hide it, but my low C++ knowledge makes the namespace ImGui "hide" the forward declareted classes otherwise...

namespace ImGui {

class PdfViewer {
public:
    PdfViewer();
    ~PdfViewer() {destroy();}

    bool isInited() const {return init;}

    void loadFromFile(const char* path);

    bool setPage(int pageNum);

    void destroy();

    void render(const ImVec2 &size=ImVec2(0,0));    // to be called inside an ImGui::Window. Makes isInited() return true;

    static const double IMAGE_DPI = 150;
    typedef void (*FreeTextureDelegate)(ImTextureID& texid);
    typedef void (*GenerateOrUpdateTextureDelegate)(ImTextureID& imtexid,int width,int height,int channels,unsigned char* pixels,bool useMipmapsIfPossible,bool wraps,bool wrapt);
    void SetFreeTextureCallback(FreeTextureDelegate freeTextureCb) {FreeTextureCb=freeTextureCb;}
    void setGenerateOrUpdateTextureCallback(GenerateOrUpdateTextureDelegate generateOrUpdateTextureCb) {GenerateOrUpdateTextureCb=generateOrUpdateTextureCb;}

protected:
    PopplerDocument* document;
    PopplerPage* page;
    static FreeTextureDelegate FreeTextureCb;
    static GenerateOrUpdateTextureDelegate GenerateOrUpdateTextureCb;

    ImTextureID texid;
    double TWidth,THeight;
    float aspectRatio,zoom;
    ImVec2 zoomCenter;

    mutable ImVec2 uv0;
    mutable ImVec2 uv1;
    mutable ImVec2 zoomedImageSize;
    mutable bool init;

    static void FreePage(PopplerPage*& page);
    static void FreeDocument(PopplerDocument*& document);

};


} // namespace ImGui



#endif //IMGUIPDFVIEWER_H_
