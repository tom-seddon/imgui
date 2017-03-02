
 /*	code by Flix (https://github.com/Flix01)
  *
 *	THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
 *	OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *	ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 *	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *	IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *  Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef IMGUIIMAGEEDITOR_H_
#define IMGUIIMAGEEDITOR_H_


#ifndef IMGUI_API
#include <imgui.h>	
#endif //IMGUI_API

// OVERVIEW:
/* It's a raw and very basic image editor (or better an icon editor) for Dear ImGui in a single .cpp file (well.., compilation unit).
 * It has no dependency other than Dear ImGui.
 *
 * It uses:
 * <stb_image.h> to load all image formats-
 * <stb_image_write.h> to save .png, .tga and .bmp formats-
 * <stb_image_resize.h> to resize images (it can be optionally disabled)-
 * Other non-stb plugins can be optionally included (see below).
*/

// USAGE:
/*
1) First of all, before doing any operation with the ImGui::ImageEditor class, you must set its static callbacks:
     ImGui::ImageEditor::SetGenerateOrUpdateTextureCallback(...);   // This will be called only with channels=3 or channels=4
     ImGui::ImageEditor::SetFreeTextureCallback(...);

2a)  // Globally:
     ImGui::ImageEditor imageEditor;
     // In initGL() or similiar:
     if (!imageEditor.loadFromFile("myImage.png")) printf("Loading Failed.\n");
     // In drawGL() or similiar, inside a ImGuiWindow:
     imageEditor.render();
     // In destroyGL() or similiar, to destroy the textures: (*)
     imageEditor.destroy();         // (*) Frees instance textures

2b) //Easier way (that doesn't delete instance textures other than in the destructor):
    // In drawGL() or similiar, inside a ImGuiWindow:
    static ImGui::ImageEditor imageEditor;
    if (!imageEditor.isInited()) if (!imageEditor.loadFromFile("myImage.png")) printf("Loading Failed.\n");
    imageEditor.render();   // makes isInited() return false

3)  // In destroyGL() or similiar
    ImGui::ImageEditor::Destroy(); // Frees shared textures

NOTES:
     -> Steps 1) and 3) are done for you if you use a IMGUI_USE_XXX_BINDING definition of the imgui_addon framework.

     -> (*) Of course the ImageEditor::~ImageEditor() calls destroy(), but it can be too late to destroy the textures.
        Note that you're free to reuse the instance after the destroy() call, but then
        you need to call again: imageEditor.destroy()
*/

// Project Options Optional Definitions:
/*
 * IMGUIIMAGEEDITOR_STBIMAGE_PATH           - (default:"./addons/imguibindings/stb_image.h")
 * IMGUIIMAGEEDITOR_ENABLE_NON_STB_PLUGINS  - Enables support for saving .jpg and .gif and for loading/saving .ico files
 *                                            ===>> WARNING <<==
 *                                            These are all proprietary formats that MIGHT contain PATENTED algorithms.
 *                                            ====>>> USE THEM AT YOUR OWN RISK!!! <<<====
 *                                            You can further filter the non-stb plugins by using:
 *                                            IMGUIIMAGEEDITOR_NO_TINY_JPEG_PLUGIN
 *                                            IMGUIIMAGEEDITOR_NO_JO_GIF_PLUGIN
 *                                            IMGUIIMAGEEDITOR_NO_TINY_ICO_PLUGIN
 *                                            When not used, these plugins can be safely deleted from the plugin folder.
 *                                            [Experimental]: when IMGUIIMAGEEDITOR_ENABLE_NON_STB_PLUGINS is defined,
 *                                            you can further define IMGUI_USE_LIBTIFF
 *                                            (it requires libtiff to be installed in your system) to load/save
 *                                            .tiff files. Still: check out libtiff license and be warned that this
 *                                            format might contain patented algorithms.
 * IMGUIIMAGEEDITOR_LOAD_ONLY_SAVABLE_FORMATS - does what it says
 * stb_image definitions can be used too (for example STBI_NO_GIF,etc.), but please make sure that you can load back
 * all the savable image formats.
 *
 * UPDATE: Added ability to load and browse (but not to save) images inside zip folders,
 * when IMGUI_USE_MINIZIP is defined and imguifilesystem is used (this needs linking to zlib).
*/

// Please post bugs here: https://github.com/Flix01/imgui/issues

/* TODO:
 * Fix bugs
*/

#define IMGUIIMAGEEDITOR_VERSION 0.1

namespace ImGui {

struct StbImage;
class ImageEditor
#if (defined(IMGUITABWINDOW_H_) && !defined(IMGUIIMAGEEDITOR_NO_TABLABEL))
: public TabWindow::TabLabel
#endif //IMGUITABWINDOW_H_
{
public:

	ImageEditor();
#if (defined(IMGUITABWINDOW_H_) && !defined(IMGUIIMAGEEDITOR_NO_TABLABEL))
    virtual ~ImageEditor();
    #else //IMGUITABWINDOW_H_
    ~ImageEditor();
    bool getModified() const;       // so that this method is always available
#   endif //IMGUITABWINDOW_H_

    bool isInited() const {return init;}

    bool loadFromFile(const char* path);
    bool saveAs(const char* path=NULL);

    void render(const ImVec2 &size);    // to be called inside an ImGui::Window. Makes isInited() return true;
    void render() {render(ImVec2(0,0));}

    void destroy();         // clears instance textures and frees memory
    static void Destroy();  // clears shared textures and frees shared memory

    typedef void (*FreeTextureDelegate)(ImTextureID& texid);
    typedef void (*GenerateOrUpdateTextureDelegate)(ImTextureID& imtexid,int width,int height,int channels,const unsigned char* pixels,bool useMipmapsIfPossible,bool wraps,bool wrapt,bool minFilterNearest, bool magFilterNearest);
    static void SetFreeTextureCallback(FreeTextureDelegate freeTextureCb) {FreeTextureCb=freeTextureCb;}
    static void SetGenerateOrUpdateTextureCallback(GenerateOrUpdateTextureDelegate generateOrUpdateTextureCb) {GenerateOrUpdateTextureCb=generateOrUpdateTextureCb;}

    class Style {
        public:
        float splitterSize;      // default: -1
        ImVec4 splitterColor;    // default: .x<0
        char arrowsChars[4][5];   // default: "<",">","^","v"
        int keySave;             // default: (int)'s'. The ImGui key index for saving the image when CTRL is down
        Style();
        static Style style;
        inline static Style& Get() {return style;}        
    };


protected:
	struct StbImage* is;

    bool init;

    friend struct StbImage;

#   ifdef _MSC_VER   // old compilers might complain
public:
#   endif //_MSC_VER

    static FreeTextureDelegate FreeTextureCb;
    static GenerateOrUpdateTextureDelegate GenerateOrUpdateTextureCb;
};

} // namespace ImGui

#endif //IMGUIIMAGEEDITOR_H_

