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


#include "imguipdfviewer.h"

#include <imgui_internal.h>
#include <imgui.h>  // intellisense

#include <poppler/glib/poppler-document.h>
#include <poppler/glib/poppler-page.h>

namespace ImGui {

void PdfViewer::FreeDocument(PopplerDocument *&document)    {
    if (document) {g_object_unref(document);document=NULL;}
}
void PdfViewer::FreePage(PopplerPage *&page)    {
    if (page) {g_object_unref(page);page=NULL;}
}

// Consider moving this to imguibindings.cpp
static void FreeTexture(ImTextureID& imtexid) {
    GLuint& texid = reinterpret_cast<GLuint&>(imtexid);
    if (texid) {glDeleteTextures(1,&texid);texid=0;}
}
// Consider moving this to imguibindings.cpp
static void GenerateOrUpdateTexture(ImTextureID& imtexid,int width,int height,int channels,unsigned char* pixels,bool useMipmapsIfPossible,bool wraps=true,bool wrapt=true) {
        IM_ASSERT(pixels);
        IM_ASSERT(channels>=3 && channels<=4);  // For now we exclude channels == 1
        GLuint& texid = reinterpret_cast<GLuint&>(imtexid);
        if (texid==0) glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmapsIfPossible ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wraps ? GL_REPEAT : GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrapt ? GL_REPEAT : GL_CLAMP);
        //const GLfloat borderColor[]={0.f,0.f,0.f,1.f};glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderColor);
        GLenum type = channels==3 ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D,0,type,width,height,0,type,GL_UNSIGNED_BYTE,pixels);
        if (useMipmapsIfPossible) glGenerateMipmap(GL_TEXTURE_2D);
    }


PdfViewer::FreeTextureDelegate PdfViewer::FreeTextureCb =
#ifdef IMGUI_USE_AUTO_BINDING
&FreeTexture;
#else //IMGUI_USE_AUTO_BINDING
NULL;
#endif //IMGUI_USE_AUTO_BINDING
PdfViewer::GenerateOrUpdateTextureDelegate PdfViewer::GenerateOrUpdateTextureCb =
#ifdef IMGUI_USE_AUTO_BINDING
&GenerateOrUpdateTexture;
#else //IMGUI_USE_AUTO_BINDING
NULL;
#endif //IMGUI_USE_AUTO_BINDING

PdfViewer::PdfViewer()  {
    texid = NULL;TWidth=THeight=0;aspectRatio=zoom=1;zoomCenter.x=zoomCenter.y=.5f;
    uv0.x=uv0.y=0;uv1.x=uv1.y=1;zoomedImageSize.x=zoomedImageSize.y=0;init=false;
}

void PdfViewer::loadFromFile(const char *path) {
    IM_ASSERT(path);
    destroy();

    gchar *absolute, *uri;
    if (g_path_is_absolute(path)) absolute = g_strdup (path);
    else {
        gchar *dir = g_get_current_dir ();
        absolute = g_build_filename (dir, path, (gchar *) 0);
        g_free (dir);
    }

    uri = g_filename_to_uri (absolute, NULL, NULL);
    g_free (absolute);
    if (uri == NULL) {
        fprintf(stderr,"Error: can't load %s\n",path);
        return;
    }

    document =  poppler_document_new_from_file(uri,NULL,NULL);
    g_free(uri);
    if (!document) fprintf(stderr,"Error: can't load %s\n",path);
    setPage(0);
}

bool PdfViewer::setPage(int pageNum) {
    if (!document) return false;

    IM_ASSERT(PdfViewer::FreeTextureCb);                // Please use PdfViewer::SetFreeTextureCallback(...) at init time
    IM_ASSERT(PdfViewer::GenerateOrUpdateTextureCb);    // Please use PdfViewer::SetGenerateOrUpdateTextureCallback(...) at init time

    const int numPages = poppler_document_get_n_pages (document);
    if (pageNum<0 || pageNum>=numPages) return false;
    FreePage(page);
    page = poppler_document_get_page(document,pageNum);
    if (page) {
        cairo_surface_t *surface;
        cairo_t *cr;
        cairo_status_t status;
        ImVector<unsigned char> surfData;

        double width, height;
        poppler_page_get_size (page, &width, &height);

        // For correct rendering of PDF, the PDF is first rendered to a
        // transparent image (all alpha = 0).
        //create cairo-surface/context to act as OpenGL-texture source
        TWidth = IMAGE_DPI*width/72.0;
        THeight = IMAGE_DPI*height/72.0;
        if (THeight==0) THeight=1;
        aspectRatio = float(TWidth/THeight);
        surfData.resize(TWidth*THeight*4);
        memset(&surfData[0],255,4 * TWidth * THeight);

        surface = cairo_image_surface_create_for_data(&surfData[0],CAIRO_FORMAT_ARGB32,TWidth,THeight,4 * TWidth);
        if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS) {
            fprintf (stderr,"create_cairo_context() - Couldn't create surface\n");
        }

        cr = cairo_create (surface);
        cairo_scale (cr, IMAGE_DPI/72.0, IMAGE_DPI/72.0);
        cairo_save (cr);
        poppler_page_render(page, cr);
        cairo_restore (cr);

        /* Then the image is painted on top of a white "page". Instead of
     * creating a second image, painting it white, then painting the
     * PDF image over it we can use the CAIRO_OPERATOR_DEST_OVER
     * operator to achieve the same effect with the one image. */
        //cairo_set_operator (cr, CAIRO_OPERATOR_DEST_OVER);
        //cairo_set_source_rgb (cr, 1, 1, 1);
        //cairo_paint (cr);

        status = cairo_status(cr);
        if (status)
            {printf("%s\n", cairo_status_to_string (status));fflush(stdout);}

        cairo_destroy (cr);

        GenerateOrUpdateTextureCb(texid,TWidth,THeight,4,&surfData[0],true,true,true);

        cairo_surface_destroy (surface);

        IM_ASSERT(texid);
    }
    return true;
}

void PdfViewer::destroy() {
    FreeTextureCb(texid);
    FreePage(page);
    FreeDocument(document);
}

// zoomCenter is panning in [(0,0),(1,1)]
// returns true if some user interaction have been processed
bool ImageZoomAndPan(ImTextureID user_texture_id, const ImVec2& size,float aspectRatio,float& zoom,ImVec2& zoomCenter,int panMouseButtonDrag=1,int resetZoomAndPanMouseButton=2,const ImVec2& zoomMaxAndZoomStep=ImVec2(16.f,1.025f))
{
    bool rv = false;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return rv;
    IM_ASSERT(size.x!=0 && size.y!=0 && zoom!=0);

    // Here we use the whole size (although it can be partially empty)
    ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x,window->DC.CursorPos.y + size.y));
    ItemSize(bb);
    if (!ItemAdd(bb, NULL)) return rv;

    ImVec2 imageSz = size;
    ImVec2 remainingWndSize(0,0);
    if (aspectRatio!=0) {
        const float wndAspectRatio = size.x/size.y;
        if (aspectRatio >= wndAspectRatio) {imageSz.y = imageSz.x/aspectRatio;remainingWndSize.y = size.y - imageSz.y;}
        else {imageSz.x = imageSz.y*aspectRatio;remainingWndSize.x = size.x - imageSz.x;}
    }

    if (ImGui::IsItemHovered()) {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.MouseWheel!=0) {
            if (io.KeyCtrl) {
                const float zoomStep = zoomMaxAndZoomStep.y;
                const float zoomMin = 1.f;
                const float zoomMax = zoomMaxAndZoomStep.x;
                if (io.MouseWheel < 0) {zoom/=zoomStep;if (zoom<zoomMin) zoom=zoomMin;}
                else {zoom*=zoomStep;if (zoom>zoomMax) zoom=zoomMax;}
                rv = true;
            }
            else  {
                const bool scrollDown = io.MouseWheel <= 0;
                const float zoomFactor = .5/zoom;
                if ((!scrollDown && zoomCenter.y > zoomFactor) || (scrollDown && zoomCenter.y <  1.f - zoomFactor))  {
                    const float slideFactor = zoomMaxAndZoomStep.y*0.1f*zoomFactor;
                    if (scrollDown) {
                        zoomCenter.y+=slideFactor;///(imageSz.y*zoom);
                        if (zoomCenter.y >  1.f - zoomFactor) zoomCenter.y =  1.f - zoomFactor;
                    }
                    else {
                        zoomCenter.y-=slideFactor;///(imageSz.y*zoom);
                        if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
                    }
                    rv = true;
                }
            }
        }
        if (io.MouseClicked[resetZoomAndPanMouseButton]) {zoom=1.f;zoomCenter.x=zoomCenter.y=.5f;rv = true;}
        if (ImGui::IsMouseDragging(panMouseButtonDrag,1.f))   {
            zoomCenter.x-=io.MouseDelta.x/(imageSz.x*zoom);
            zoomCenter.y-=io.MouseDelta.y/(imageSz.y*zoom);
            rv = true;
            ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
        }
    }

    const float zoomFactor = .5/zoom;
    if (rv) {
        if (zoomCenter.x < zoomFactor) zoomCenter.x = zoomFactor;
        else if (zoomCenter.x > 1.f - zoomFactor) zoomCenter.x = 1.f - zoomFactor;
        if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
        else if (zoomCenter.y > 1.f - zoomFactor) zoomCenter.y = 1.f - zoomFactor;
    }

    ImVec2 uvExtension(2.f*zoomFactor,2.f*zoomFactor);
    if (remainingWndSize.x > 0) {
        const float remainingSizeInUVSpace = 2.f*zoomFactor*(remainingWndSize.x/imageSz.x);
        const float deltaUV = uvExtension.x;
        const float remainingUV = 1.f-deltaUV;
        if (deltaUV<1) {
            float adder = (remainingUV < remainingSizeInUVSpace ? remainingUV : remainingSizeInUVSpace);
            uvExtension.x+=adder;
            remainingWndSize.x-= adder * zoom * imageSz.x;
            imageSz.x+=adder * zoom * imageSz.x;

            if (zoomCenter.x < uvExtension.x*.5f) zoomCenter.x = uvExtension.x*.5f;
            else if (zoomCenter.x > 1.f - uvExtension.x*.5f) zoomCenter.x = 1.f - uvExtension.x*.5f;
        }
    }
    if (remainingWndSize.y > 0) {
        const float remainingSizeInUVSpace = 2.f*zoomFactor*(remainingWndSize.y/imageSz.y);
        const float deltaUV = uvExtension.y;
        const float remainingUV = 1.f-deltaUV;
        if (deltaUV<1) {
            float adder = (remainingUV < remainingSizeInUVSpace ? remainingUV : remainingSizeInUVSpace);
            uvExtension.y+=adder;
            remainingWndSize.y-= adder * zoom * imageSz.y;
            imageSz.y+=adder * zoom * imageSz.y;

            if (zoomCenter.y < uvExtension.y*.5f) zoomCenter.y = uvExtension.y*.5f;
            else if (zoomCenter.y > 1.f - uvExtension.y*.5f) zoomCenter.y = 1.f - uvExtension.y*.5f;
        }
    }

    ImVec2 uv0((zoomCenter.x-uvExtension.x*.5f),(zoomCenter.y-uvExtension.y*.5f));
    ImVec2 uv1((zoomCenter.x+uvExtension.x*.5f),(zoomCenter.y+uvExtension.y*.5f));


    /* // Here we use just the window size, but then ImGui::IsItemHovered() should be moved below this block. How to do it?
    ImVec2 startPos=window->DC.CursorPos;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    ImVec2 endPos(startPos.x+imageSz.x,startPos.y+imageSz.y);
    ImRect bb(startPos, endPos);
    ItemSize(bb);
    if (!ItemAdd(bb, NULL)) return rv;*/

    ImVec2 startPos=bb.Min,endPos=bb.Max;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    endPos.x = startPos.x + imageSz.x;
    endPos.y = startPos.y + imageSz.y;

    window->DrawList->AddImage(user_texture_id, startPos, endPos, uv0, uv1);

    return rv;
}


void PdfViewer::render(const ImVec2& size) {
    init = true;
    ImVec2 curPos = ImGui::GetCursorPos();
    const ImVec2 wndSz(size.x>0 ? size.x : ImGui::GetWindowSize().x-curPos.x,size.y>0 ? size.y : ImGui::GetWindowSize().y-curPos.y);

    if (!ImGui::ImageZoomAndPan(reinterpret_cast<ImTextureID>(texid),wndSz,aspectRatio,zoom,zoomCenter))  {
        float mw = ImGui::GetIO().MouseWheel;
        if (mw) {
            const bool pageUp = mw<0;
            const float zoomFactor = .5/zoom;
            if (!pageUp && zoomCenter.y <= zoomFactor) {
                if (setPage(poppler_page_get_index(page)-1)) zoomCenter.y = 1.f - zoomFactor;
            }
            else if (pageUp && zoomCenter.y >= 1.f - zoomFactor) {
                if (setPage(poppler_page_get_index(page)+1)) zoomCenter.y = zoomFactor;
            }
        }
    }
}







} // namespace ImGui
