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

//- Common Code For All Addons needed just to ease inclusion as separate files in user code ----------------------
#include <imgui.h>
#undef IMGUI_DEFINE_PLACEMENT_NEW
#define IMGUI_DEFINE_PLACEMENT_NEW
#undef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

//#define IMGUIIMAGEEDITOR_DEV_ONLY   // TO COMMENT OUT (mandatory)
#ifdef IMGUIIMAGEEDITOR_DEV_ONLY
#define IMGUIIMAGEEDITOR_ENABLE_NON_STB_PLUGINS
#define IMGUI_USE_LIBTIFF
//-----------------------------------------------------------------------------------------------------------------
// TO REMOVE! (used to force intellisense on Qt Creator)--
#include "../../imgui.h"
#include "../../imgui_internal.h"
#include "./addons/imguibindings/stb_image.h"
//--------------------------------------------------------
#define IMGUI_FILESYSTEM_H_ // TO REMOVE! (used to force intellisense on Qt Creator)--
#warning Something must be removed
#endif //IMGUIIMAGEEDITOR_DEV_ONLY

#include "imguiimageeditor.h"

#ifdef NO_IMGUITOOLBAR
#   error imguiimageeditor needs imguitoolbar (ATM it is header only)
#endif //NO_IMGUITOOLBAR
#include "../imguitoolbar/imguitoolbar.h"


#include <ctype.h>  // tolower
#include <string.h> // memset,...


// stb_image.h is MANDATORY
#ifndef IMGUI_USE_AUTO_BINDING
#ifndef STBI_INCLUDE_STB_IMAGE_H
#ifndef IMGUIIMAGEEDITOR_NO_STB_IMAGE_STATIC
#define STB_IMAGE_STATIC
#endif //IMGUIIMAGEEDITOR_NO_STB_IMAGE_STATIC
#ifndef IMGUIIMAGEEDITOR_NO_STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif //IMGUIIMAGEEDITOR_NO_STB_IMAGE_IMPLEMENTATION
#ifndef IMGUIIMAGEEDITOR_STBIMAGE_PATH
#define IMGUIIMAGEEDITOR_STBIMAGE_PATH "./addons/imguibindings/stb_image.h"
#endif //IMGUIIMAGEEDITOR_STBIMAGE_PATH
#include IMGUIIMAGEEDITOR_STBIMAGE_PATH
#endif //STBI_INCLUDE_STB_IMAGE_H
#endif //IMGUI_USE_AUTO_BINDING
/* image loading/decoding from file/memory: JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC
// Limitations:
//    - no 16-bit-per-channel PNG
//    - no 12-bit-per-channel JPEG
//    - no JPEGs with arithmetic coding
//    - no 1-bit BMP
//    - GIF always returns *comp=4
*/

// Are these necessary ?
#ifndef STBI_MALLOC
#define STBI_MALLOC(sz)           malloc(sz)
#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
#define STBI_FREE(p)              free(p)
#endif



// Next we have optional plugins from the imguiimageeditor_plugins subfolder
#ifndef IMGUIIMAGEEDITOR_NO_PLUGINS

#ifndef IMGUIIMAGEEDITOR_NO_STB_IMAGE_RESIZE_PLUGIN
#ifndef STBI_INCLUDE_STB_IMAGE_RESIZE_H
#ifndef IMGUIIMAGEEDITOR_NO_STB_IMAGE_RESIZE_STATIC
#define STB_IMAGE_RESIZE_STATIC
#endif //IMGUIIMAGEEDITOR_NO_STB_IMAGE_RESIZE_STATIC
#ifndef IMGUIIMAGEEDITOR_NO_STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#endif //IMGUIIMAGEEDITOR_NO_STB_IMAGE_RESIZE_IMPLEMENTATION
#include "imguiimageeditor_plugins/stb_image_resize.h"
#endif //STBI_INCLUDE_STB_IMAGE_RESIZE_H
#endif //IMGUIIMAGEEDITOR_NO_STB_IMAGE_RESIZE_PLUGIN
/*
   QUICKSTART
      stbir_resize_uint8(      input_pixels , in_w , in_h , 0,
                               output_pixels, out_w, out_h, 0, num_channels)
      stbir_resize_float(...)
      stbir_resize_uint8_srgb( input_pixels , in_w , in_h , 0,
                               output_pixels, out_w, out_h, 0,
                               num_channels , alpha_chan  , 0)
      stbir_resize_uint8_srgb_edgemode(
                               input_pixels , in_w , in_h , 0,
                               output_pixels, out_w, out_h, 0,
                               num_channels , alpha_chan  , 0, STBIR_EDGE_CLAMP)
                                                            // WRAP/REFLECT/ZERO
*/


#ifndef IMGUIIMAGEEDITOR_NO_STB_IMAGE_WRITE_PLUGIN
#ifndef INCLUDE_STB_IMAGE_WRITE_H
#ifndef IMGUIIMAGEEDITOR_NO_STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_STATIC
#endif //IMGUIIMAGEEDITOR_NO_STB_IMAGE_WRITE_STATIC
#ifndef IMGUIIMAGEEDITOR_NO_STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif //IMGUIIMAGEEDITOR_NO_STB_IMAGE_WRITE_IMPLEMENTATION
#include "imguiimageeditor_plugins/stb_image_write.h"
#endif //INCLUDE_STB_IMAGE_WRITE_H
#endif //IMGUIIMAGEEDITOR_NO_STB_IMAGE_WRITE_PLUGIN
/*
     image writing to disk: PNG, TGA, BMP

     int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
     int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);

*/


#ifdef IMGUIIMAGEEDITOR_ENABLE_NON_STB_PLUGINS

#ifndef IMGUIIMAGEEDITOR_NO_TINY_JPEG_PLUGIN
#ifndef IMGUIIMAGEEDITOR_NO_TINY_JPEG_IMPLEMENTATION
#define TJE_IMPLEMENTATION
#endif //IMGUIIMAGEEDITOR_NO_TINY_JPEG_IMPLEMENTATION
#ifndef min
#define MUST_UNDEF_MIN
#define min(X,Y) ((X)<(Y)?(X):(Y))
#endif //min
#include "imguiimageeditor_plugins/tiny_jpeg.h"
#define TJE_H   // This is because there's no such a header guard in tiny_jpeg.h
#ifdef MUST_UNDEF_MIN
#undef MUST_UNDEF_MIN
#undef min
#endif //MUST_UNDEF_MIN
#endif //IMGUIIMAGEEDITOR_NO_TINY_JPEG_PLUGIN
/* // TJE_H
    if ( !tje_encode_to_file("out.jpg", width, height, num_components, data) ) {
        fprintf(stderr, "Could not write JPEG\n");
        return EXIT_FAILURE;
    }
*/

// LodePng plugin has been removed (commented out)! Reason:
// When it saves some (all?) RGBA image (as LCT_RGBA), it seems to work (I can open it in my file browser),
// but when I load it back (with stb_image(...) or with lodePng as well) it's detected as RGB
// and thus it does not display correctly.

/*#if (!defined(IMGUIIMAGEEDITOR_NO_LODEPNG_PLUGIN) && !defined(IMGUIIMAGEEDITOR_NO_LODE_PNG_PLUGIN))
#ifndef LODEPNG_H
#if (!defined(IMGUIIMAGEEDITOR_NO_LODEPNG_IMPLEMENTATION) && !defined(IMGUIIMAGEEDITOR_NO_LODE_PNG_IMPLEMENTATION))
#include "imguiimageeditor_plugins/lodepng.cpp"
#else ////IMGUIIMAGEEDITOR_NO_LODEPNG_IMPLEMENTATION
#include "imguiimageeditor_plugins/loadpng.h"
#endif //IMGUIIMAGEEDITOR_NO_LODEPNG_IMPLEMENTATION
#endif //LODEPNG_H
#endif //IMGUIIMAGEEDITOR_NO_LODEPNG_PLUGIN*/
/*
  DECODER (unused):

  unsigned char *png,* image;
  error = lodepng_load_file(&png, &pngsize, filename); // fill png from file
  if(!error) error = lodepng_decode32(&image, &width, &height, png, pngsize);
  //error = lodepng_decode32_file(&image, &width, &height, filename); //all in one
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

  free(png);

  //use image here

  //free(image);
  // LODEPNG_H
  ENCODER:

  unsigned char* png;
  size_t pngsize;

    // _encode24 too
  //unsigned error = lodepng_encode32_file(filename, image, width, height); // all in one
  unsigned error = lodepng_encode32(&png, &pngsize, image, width, height);
  if(!error) lodepng_save_file(png, pngsize, filename);

  //if there's an error, display it
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

  free(png);

*/


#ifndef IMGUIIMAGEEDITOR_NO_JO_GIF_PLUGIN
#ifndef JO_INCLUDE_GIF_H
#ifdef IMGUIIMAGEEDITOR_NO_JO_GIF_IMPLEMENTATION
#define JO_GIF_HEADER_FILE_ONLY
#endif //IMGUIIMAGEEDITOR_NO_JO_GIF_IMPLEMENTATION
#include "imguiimageeditor_plugins/jo_gif.cpp"
#endif //JO_INCLUDE_GIF_H
#endif //IMGUIIMAGEEDITOR_NO_JO_GIF_PLUGIN
/* // JO_INCLUDE_GIF_H
 * Basic usage:
 *	char *frame = new char[128*128*4]; // 4 component. RGBX format, where X is unused
 *	jo_gif_t gif = jo_gif_start("foo.gif", 128, 128, 0, 32);
 *	jo_gif_frame(&gif, frame, 4, false); // frame 1
 *	jo_gif_frame(&gif, frame, 4, false); // frame 2
 *	jo_gif_frame(&gif, frame, 4, false); // frame 3, ...
 *	jo_gif_end(&gif);
*/

#ifndef IMGUIIMAGEEDITOR_NO_TINY_ICO_PLUGIN
#ifndef TINY_ICO_H
#ifndef IMGUIIMAGEEDITOR_NO_TINY_ICO_STATIC
#define TINY_ICO_STATIC
#endif //IMGUIIMAGEEDITOR_NO_TINY_ICO_STATIC
#ifndef IMGUIIMAGEEDITOR_NO_TINY_ICO_IMPLEMENTATION
#define TINY_ICO_IMPLEMENTATION
#endif //IMGUIIMAGEEDITOR_NO_TINY_ICO_IMPLEMENTATION
#include "imguiimageeditor_plugins/tiny_ico.h"
#endif //TINY_ICO_H
#endif //IMGUIIMAGEEDITOR_NO_TINY_ICO_PLUGIN

#ifdef IMGUI_USE_LIBTIFF    // This needs libtiff
extern "C" {
#include <tiffio.h>
}
#endif //IMGUI_USE_LIBTIFF
/* _TIFF_

*/

#endif //IMGUIIMAGEEDITOR_ENABLE_NON_STB_PLUGINS

#endif //IMGUIIMAGEEDITOR_NO_PLUGINS






namespace ImGuiIE {

#ifdef _TIFF_
struct my_tiff_memory_stream {
    ImVector<char>& buf;
    tsize_t pos;
    my_tiff_memory_stream(ImVector<char>& b) : buf(b),pos(0) {}
};
static tsize_t my_tiff_read_proc( thandle_t handle, tdata_t buffer, tsize_t size )  {
    //return fread((void*)buffer, size,(FILE*)handle);
    my_tiff_memory_stream& s = *((my_tiff_memory_stream*)handle);
    IM_ASSERT(s.pos+size<=(tsize_t)s.buf.size());
    _TIFFmemcpy(buffer,&s.buf[s.pos],size);
    s.pos+=size;
    return size;
}
static tsize_t my_tiff_write_proc( thandle_t handle, tdata_t buffer, tsize_t size ) {
    //return fwrite((const void*)buffer,size,(FILE*)handle);
    my_tiff_memory_stream& s = *((my_tiff_memory_stream*)handle);
    if (s.pos+size>s.buf.size()) s.buf.resize(s.pos+size);
    memcpy(&s.buf[s.pos],buffer,size);
    s.pos+=size;
    return size;
}
static toff_t my_tiff_seek_proc( thandle_t handle, toff_t offset, int origin )  {
    //return fseek((FILE*)handle,offset,origin);
    my_tiff_memory_stream& s = *((my_tiff_memory_stream*)handle);
    if (origin==SEEK_SET) {

        s.pos = offset;
        return s.pos;
    }
    else if (origin==SEEK_CUR) {

        s.pos+=offset;
        return s.pos;
    }
    else if (origin==SEEK_END) {

        s.pos=(toff_t)s.buf.size()+offset;
        return s.pos;
    }
    else return 0;
}
static int my_tiff_close_proc( thandle_t )   {
    //return fclose((FILE*)handle);
    return 0;
}
static toff_t my_tiff_size_proc(thandle_t handle) {
    my_tiff_memory_stream& s = *((my_tiff_memory_stream*)handle);
    return (toff_t) s.buf.size();
}
static int my_tiff_map_file_proc(thandle_t, void** , toff_t* ) {return 0;}
static void my_tiff_unmap_file_proc(thandle_t, void* , toff_t ) {}
static void my_tiff_extend_proc(TIFF*) {}

static bool tiff_save_to_memory(const unsigned char* pixels,int w,int h,int c,ImVector<char>& rv) {
    rv.clear();
    my_tiff_memory_stream s(rv);
    TIFF* tif = TIFFClientOpen( "MyMemFs","w",(thandle_t)&s,my_tiff_read_proc,my_tiff_write_proc,my_tiff_seek_proc,my_tiff_close_proc,
                    my_tiff_size_proc,my_tiff_map_file_proc,my_tiff_unmap_file_proc);
    if (tif) {
        // From: http://research.cs.wisc.edu/graphics/Courses/638-f1999/libtiff_tutorial.htm
        TIFFSetField (tif, TIFFTAG_IMAGEWIDTH, w);      // set the width of the image
        TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);      // set the height of the image
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, c);  // set number of channels per pixel
        TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
        TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
        //   Some other essential fields to set that you do not have to understand for now.
        TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

        const tsize_t linebytes = c * w;     // length in memory of one row of pixel in the image.

        unsigned char *buf = NULL;        // buffer used to store the row of pixel information for writing to file
        //    Allocating memory to store the pixels of current row
        buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tif));

        // We set the strip size of the file to be size of one row of pixels
        TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, w*c));

        //fprintf(stderr,"tiff_save_to_memory(...): w=%d,h=%d,c=%d linebytes=%u TIFFDefaultStripSize(tif, w*c)=%u\n",w,h,c,linebytes,TIFFDefaultStripSize(tif, w*c));

        //Now writing image to the file one strip at a time
        for (int row = 0; row < h; row++)   {
            memcpy(buf, &pixels[/*(h-row-1)*/row*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
            if (TIFFWriteScanline(tif, buf, row, 0)< 0)    break;
        }

        TIFFClose(tif);tif=NULL;
        if (buf) {_TIFFfree(buf);buf=NULL;}
        return true;
    }
    //else fprintf(stderr,"Error: tiff_save_to_memory...): !tif.\n");

    return false;
}
static unsigned char* tiff_load_from_memory(const char* buffer,int size,int& w,int& h,int &c) {
    if (!buffer || size<=0) return NULL;
    w=h=0;c=4;
    ImVector<char> rv;rv.resize(size);memcpy(&rv[0],buffer,size);
    my_tiff_memory_stream s(rv);
    unsigned char* data = NULL;

    TIFF* tif = TIFFClientOpen( "MyMemFs","r",(thandle_t)&s,my_tiff_read_proc,my_tiff_write_proc,my_tiff_seek_proc,my_tiff_close_proc,
                my_tiff_size_proc,my_tiff_map_file_proc,my_tiff_unmap_file_proc);
    if (tif) {
        /*int dircount = 0;
        do {
            dircount++;
        } while (TIFFReadDirectory(tif));
        printf("%d directories in tif.\n", dircount);*/

        uint32 W, H, C;
        size_t npixels;
        uint32* raster;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &W);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &H);
        //TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &C);   // This does not work...
        C=4;
        npixels = W * H;

        raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
        if (raster != NULL) {
            if (TIFFReadRGBAImage(tif, W, H, raster, 0)) {
                w=(int)W;h=(int)H;c=(int)C;
                //fprintf(stderr,"tiff_load_from_memory: w=%d h=%d c=%d\n",w,h,c);
                data=(unsigned char*)STBI_MALLOC(w*h*c);

/*
char X=(char )TIFFGetX(raster[i]);  // where X can be the channels R, G, B, and A.
// i is the index of the pixel in the raster.

Important: Remember that the origin of the raster is at the lower left corner.
You should be able to figure out the how the image is stored in the raster given
that the pixel information is stored a row at a time!
*/

                unsigned char* pIm = data;
                const unsigned char* pRasterBase = (const unsigned char*) raster;
                const unsigned char* pRaster = pRasterBase;
                for (unsigned y=0;y<H;y++)  {
                    pIm = &data[y*W*C];
                    pRaster = &pRasterBase[(H-1-y)*W*c];
                    for (unsigned ic=0,icSz=W*C;ic<icSz;ic++)    {
                        *pIm++ = *pRaster++;
                    }
                }
            }
            _TIFFfree(raster);raster=NULL;
        }
        TIFFClose(tif);tif=NULL;
    }
    //else fprintf(stderr,"Error: tiff_load_from_memory(...): !tif.\n");

    return data;
}
#endif //_TIFF_

// Some old compilers don't have round
static float round(float x) {return x >= 0.0f ? floor(x + 0.5f) : ceil(x - 0.5f);}

// Cloned from imguivariouscontrols.cpp [but modified slightly]
bool ImageZoomAndPan(ImTextureID user_texture_id, const ImVec2& size,float aspectRatio,ImTextureID checkersTexID = NULL,float* pzoom=NULL,ImVec2* pzoomCenter=NULL,int panMouseButtonDrag=1,int resetZoomAndPanMouseButton=2,const ImVec2& zoomMaxAndZoomStep=ImVec2(16.f,1.025f))
{
    float zoom = pzoom ? *pzoom : 1.f;
    ImVec2 zoomCenter = pzoomCenter ? *pzoomCenter : ImVec2(0.5f,0.5f);

    bool rv = false;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (!window || window->SkipItems) return rv;
    ImVec2 curPos = ImGui::GetCursorPos();
    const ImVec2 wndSz(size.x>0 ? size.x : ImGui::GetWindowSize().x-curPos.x,size.y>0 ? size.y : ImGui::GetWindowSize().y-curPos.y);

    IM_ASSERT(wndSz.x!=0 && wndSz.y!=0 && zoom!=0);

    // Here we use the whole size (although it can be partially empty)
    ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + wndSz.x,window->DC.CursorPos.y + wndSz.y));
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, NULL)) return rv;

    ImVec2 imageSz = wndSz;
    ImVec2 remainingWndSize(0,0);
    if (aspectRatio!=0) {
        const float wndAspectRatio = wndSz.x/wndSz.y;
        if (aspectRatio >= wndAspectRatio) {imageSz.y = imageSz.x/aspectRatio;remainingWndSize.y = wndSz.y - imageSz.y;}
        else {imageSz.x = imageSz.y*aspectRatio;remainingWndSize.x = wndSz.x - imageSz.x;}
    }

    if (ImGui::IsItemHovered()) {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.MouseWheel!=0) {
            if (!io.KeyCtrl && !io.KeyShift)
            {
                const float zoomStep = zoomMaxAndZoomStep.y;
                const float zoomMin = 1.f;
                const float zoomMax = zoomMaxAndZoomStep.x;
                if (io.MouseWheel < 0) {zoom/=zoomStep;if (zoom<zoomMin) zoom=zoomMin;}
                else {zoom*=zoomStep;if (zoom>zoomMax) zoom=zoomMax;}
                rv = true;
            }
            else if (io.KeyCtrl) {
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
            else if (io.KeyShift) {
                const bool scrollRight = io.MouseWheel <= 0;
                const float zoomFactor = .5/zoom;
                if ((!scrollRight && zoomCenter.x > zoomFactor) || (scrollRight && zoomCenter.x <  1.f - zoomFactor))  {
                    const float slideFactor = zoomMaxAndZoomStep.y*0.1f*zoomFactor;
                    if (scrollRight) {
                        zoomCenter.x+=slideFactor;///(imageSz.x*zoom);
                        if (zoomCenter.x >  1.f - zoomFactor) zoomCenter.x =  1.f - zoomFactor;
                    }
                    else {
                        zoomCenter.x-=slideFactor;///(imageSz.x*zoom);
                        if (zoomCenter.x < zoomFactor) zoomCenter.x = zoomFactor;
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


    ImVec2 startPos=bb.Min,endPos=bb.Max;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    endPos.x = startPos.x + imageSz.x;
    endPos.y = startPos.y + imageSz.y;

    if (checkersTexID) {
        const float m = 24.f;
        //window->DrawList->AddImage(checkersTexID, startPos, endPos, uv0*m, uv1*m);
        window->DrawList->AddImage(checkersTexID, startPos, endPos, ImVec2(0,0), ImVec2(m,m));
    }
    window->DrawList->AddImage(user_texture_id, startPos, endPos, uv0, uv1);

    if (pzoom)  *pzoom = zoom;
    if (pzoomCenter) *pzoomCenter = zoomCenter;


    return rv;
}

// Cloned from imguihelper.cpp
bool IsItemActiveLastFrame()    {
    ImGuiContext& g = *GImGui;
    if (g.ActiveIdPreviousFrame)
        return g.ActiveIdPreviousFrame== GImGui->CurrentWindow->DC.LastItemId;
    return false;
}
bool IsItemJustReleased()   {return IsItemActiveLastFrame() && !ImGui::IsItemActive();}


struct ImageScopedDeleter {
    volatile unsigned char* im;
    ImageScopedDeleter(unsigned char* image=NULL) : im(image) {}
    ~ImageScopedDeleter() {
        if (im) {STBI_FREE((void*)im);im=NULL;}
    }
};

enum ResizeFilter {
    RF_NEAREST = 0,
    RF_GOOD,
    RF_BEST,     // Falls back to good when not available
    RF_COUNT
};
inline static const char** GetResizeFilterNames() {
    static const char* names[] = {"NEAREST","GOOD","BEST"};
    IM_ASSERT(sizeof(names)/sizeof(names[0])==RF_COUNT);
    return names;
}

enum LightEffect {
    LE_LINEAR,
    LE_ROUND,
    LE_SPHERICAL,
    LE_COUNT
};
inline static const char** GetLightEffectNames() {
    static const char* names[] = {"LINEAR","ROUND","SPHERICAL"};
    IM_ASSERT(sizeof(names)/sizeof(names[0])==LE_COUNT);
    return names;
}


static bool GetFileContent(const char *filePath, ImVector<char> &contentOut, bool clearContentOutBeforeUsage=true,const char* modes="rb",bool appendTrailingZeroIfModesIsNotBinary=true)	{
#   if (defined(IMGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
    return ImGuiFs::FileGetContent(filePath,contentOut);    // This gets the content inside a zip file too (if filePath is like: "C://MyDocuments/myzipfile.zip/myzipFile/something.png")
#   endif //IMGUI_USE_MINIZIP
    ImVector<char>& f_data = contentOut;
    if (clearContentOutBeforeUsage) f_data.clear();
//----------------------------------------------------
    if (!filePath) return false;
    const bool appendTrailingZero = appendTrailingZeroIfModesIsNotBinary && modes && strlen(modes)>0 && modes[strlen(modes)-1]!='b';
    FILE* f;
    if ((f = ImFileOpen(filePath, modes)) == NULL) return false;
    if (fseek(f, 0, SEEK_END))  {
        fclose(f);
        return false;
    }
    const long f_size_signed = ftell(f);
    if (f_size_signed == -1)    {
        fclose(f);
        return false;
    }
    size_t f_size = (size_t)f_size_signed;
    if (fseek(f, 0, SEEK_SET))  {
        fclose(f);
        return false;
    }
    f_data.resize(f_size+(appendTrailingZero?1:0));
    const size_t f_size_read = f_size>0 ? fread(&f_data[0], 1, f_size, f) : 0;
    fclose(f);
    if (f_size_read == 0 || f_size_read!=f_size)    return false;
    if (appendTrailingZero) f_data[f_size] = '\0';
//----------------------------------------------------
    return true;
}

static bool SetFileContent(const char *filePath, const unsigned char* content, int contentSize,const char* modes="wb")	{
    if (!filePath || !content) return false;
#   if (defined(IGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
    if (ImGuiFs::PathIsInsideAZipFile(filePath)) return false;  // Not supported
#   endif //IMGUI_USE_MINIZIP
    FILE* f;
    if ((f = ImFileOpen(filePath, modes)) == NULL) return false;
    fwrite(content,contentSize, 1, f);
    fflush(f);
    fclose(f);
    return true;
}

static bool FileExists(const char *filePath)   {
    if (!filePath || strlen(filePath)==0) return false;
#   if (defined(IMGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
    return ImGuiFs::PathExistsWithZipSupport(filePath,true,false);
#   endif //IMGUI_USE_MINIZIP
    FILE* f = ImFileOpen(filePath, "rb");
    if (!f) return false;
    fclose(f);f=NULL;
    return true;
}


static const int MaxSupportedExtensionsSize = 2048;
static char SupportedLoadExtensions[MaxSupportedExtensionsSize] = "";
static char SupportedSaveExtensions[5][MaxSupportedExtensionsSize] = {"","","","",""};
static void InitSupportedFileExtensions() {
    if (SupportedLoadExtensions[0]!='\0') return;
    // Save Extensions:
    {
        char* p[5] = {SupportedSaveExtensions[0],SupportedSaveExtensions[1],SupportedSaveExtensions[2],SupportedSaveExtensions[3],SupportedSaveExtensions[4]};
#       if (!defined(STBI_NO_PNG) && (defined(INCLUDE_STB_IMAGE_WRITE_H) || defined(LODEPNG_H)))
        strcat(p[0],".png;");   // index are channels, except 0 == all
        strcat(p[1],".png;");
        strcat(p[3],".png;");
        strcat(p[4],".png;");
#       endif
#       if (!defined(STBI_NO_TGA) && defined(INCLUDE_STB_IMAGE_WRITE_H))
        strcat(p[0],".tga;");
        strcat(p[1],".tga;");
        strcat(p[3],".tga;");
        strcat(p[4],".tga;");
#       endif
#       if (!defined(STBI_NO_JPEG) && defined(TJE_H))
        strcat(p[0],".jpg;.jpeg;");
        strcat(p[3],".jpg;.jpeg;");
#       endif
#       if (defined(TINY_ICO_H) && !defined(TINY_ICO_NO_ENCODER))
        strcat(p[0],".ico;");
        strcat(p[4],".ico;");
//        strcat(p[0],".ico;.cur;");
//        strcat(p[4],".ico;.cur;");
#       endif
#       if (!defined(STBI_NO_BMP) && defined(INCLUDE_STB_IMAGE_WRITE_H))
        strcat(p[0],".bmp;");
        strcat(p[3],".bmp;");
#       endif
#       if (!defined(STBI_NO_GIF) && defined(JO_INCLUDE_GIF_H))
        strcat(p[0],".gif;");
        strcat(p[3],".gif;");
        strcat(p[4],".gif;");
#       endif
#       ifdef _TIFF_
        strcat(p[0],".tiff;.tif;");
        //strcat(p[3],".tiff;.tif;");
        strcat(p[4],".tiff;.tif;");
#       endif //_TIFF_
        for (int i=0;i<5;i++)   {
            const int len = strlen(p[i]);
            if (len>0) p[i][len-1]='\0';   // trim last ';'
            else if (i==0) IM_ASSERT(true); // Can't load any fomat (better assert)
        }
    }

    // Load Extensions:
    {
        char* p = SupportedLoadExtensions;
#   ifndef IMGUIIMAGEEDITOR_LOAD_ONLY_SAVABLE_FORMATS
#       ifndef STBI_NO_PNG
        strcat(p,".png;");
#       endif
#       ifndef STBI_NO_TGA
        strcat(p,".tga;");
#       endif
#       ifndef STBI_NO_PSD
        strcat(p,".psd;");
#       endif
#       ifndef STBI_NO_JPEG
        strcat(p,".jpg;.jpeg;");
#       endif
#       if (defined(TINY_ICO_H) && !defined(TINY_ICO_NO_DECODER))
        strcat(p,".ico;");
        //strcat(p,".ico;.cur;");
#       endif
#       ifndef STBI_NO_GIF
        strcat(p,".gif;");
#       endif
#       ifdef _TIFF_
        strcat(p,".tiff;.tif;");
#       endif
#       ifndef STBI_NO_BMP
        strcat(p,".bmp;");
#       endif
#       ifndef STBI_NO_PIC
        strcat(p,".pic;");
#       endif
#       ifndef STBI_NO_PNM
        strcat(p,".pnm;.ppm;");
#       endif
#   else //IMGUIIMAGEEDITOR_LOAD_ONLY_SAVABLE_FORMATS
        strcpy(p,&SupportedSaveExtensions[0]);
#   endif //IMGUIIMAGEEDITOR_LOAD_ONLY_SAVABLE_FORMATS
#       if (defined(IMGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
        strcat(p,".zip;");  // This is a hack. It's just an attempt to allow opening images inside zip files
#       endif
        const int len = strlen(p);
        if (len>0) p[len-1]='\0';   // trim last ';'
        else {
            IM_ASSERT(true);    // Can't load any fomat (better assert)
            p[0]=';';p[1]='\0'; // otherwise this method can't be called multiple times.
        }
    }
}


// return image must be freed by the caller
static unsigned char* CreateImage(int w,int h,int c,unsigned char R=255,unsigned char G=255,unsigned char B=255,unsigned char A=255,bool assumeThatOneChannelMeansLuminance=false) {
    IM_ASSERT(w>0 && h>0 && c>0 && c<=4 && c!=2);
    if (!(w>0 && h>0 && c>0 && c<=4 && c!=2)) return NULL;
    const size_t size = w*h*c;
    unsigned char* im = (unsigned char*) STBI_MALLOC(size);
    unsigned char* pim = im;
    if (c==1) {const unsigned char pxl = assumeThatOneChannelMeansLuminance ? ((unsigned char)(((unsigned short)R+(unsigned short)G+(unsigned short)B)/3)) : A;memset(im,pxl,size);}
    else if (c==3)  for (int i=0,isz=w*h;i<isz;i++) {*pim++=R;*pim++=G;*pim++=B;}
    else if (c==4) for (int i=0,isz=w*h;i<isz;i++) {*pim++=R;*pim++=G;*pim++=B;*pim++=A;}
    else {STBI_FREE(im);im=NULL;IM_ASSERT(true);}
    return im;
}

// return image must be freed by the caller (dstX,dstY,dstW,dstH are taken by reference, because they can be corrected when inappropriate)
template <typename T> static T* ExtractImage(int& dstX,int& dstY,int& dstW,int& dstH,const T* im,int w,int h,int c) {
    IM_ASSERT(w>0 && h>0 && c>0 && (c==1 || c==3 || c==4));
    if (!(w>0 && h>0 && c>0 && (c==1 || c==3 || c==4))) return NULL;
    if (dstW<0) dstW=-dstW;
    if (dstH<0) dstH=-dstH;
    if (dstH>h) dstH=h;
    if (dstW>w) dstW=w;
    if (dstH>h) dstH=h;
    if (dstX+dstW>w) dstX = w-dstW;
    if (dstY+dstH>h) dstY = h-dstH;
    if (dstX<0) dstX=0;
    if (dstY<0) dstY=0;
    if (dstW*dstH<=0) return NULL;
    IM_ASSERT(dstX+dstW<=w && dstY+dstH<=h);
    const int size = dstW*dstH*c;
    T* nim = (T*) STBI_MALLOC(size*sizeof(T));
    if (nim)    {
        const T* pim = &im[(dstY*w+dstX)*c];
        T* pnim = nim;
        for (int y=0;y<dstH;y++)    {
            for (int xc=0,xcSz=dstW*c;xc<xcSz;xc++)    {
                *pnim++ = *pim++;
            }
            pim+=(w-dstW)*c;
        }
    }
    return nim;
}
/*template <typename T> inline static T* ExtractImage(const ImRect& selection,const T* im,int w,int h,int c) {
    int dstX = selection.Min.x;int dstY = selection.Min.y;
    int dstW = selection.Max.x - selection.Min.x;int dstH = selection.Max.y-selection.Min.y;
    return ExtractImage<T>(dstX,dstY,dstW,dstH,im,e,h,c);
}*/

template <typename T> static bool PasteImage(int posX,int posY,T* im,int w,int h,int c,const T* im2,int w2,int h2) {
    IM_ASSERT(im && im2 && w>0 && h>0 && c>0 && (c==1 || c==3 || c==4) && w2>0 && h2>0);
    if (!(im && im2 && w>0 && h>0 && c>0 && (c==1 || c==3 || c==4) && w2>0 && h2>0)) return NULL;    

    const int stride2 = w2*c;
    const int stride = w*c;

    const T* pim2 = im2;
    T* pim = im;

    const int yStart = posY;
    const int yEnd = posY+h2;
    const int xcStart = posX*c;
    const int xcEnd = (posX+w2)*c;

    for (int y=yStart;y<yEnd;y++)    {
        if (y<0) continue;
        if (y>=h || (y-posY)>=h2) break;
        pim = &im[y*w*c];
        pim2 = &im2[(y-posY)*stride2];
        for (int xc=xcStart;xc<xcEnd;xc++)    {
            if (xc<0) {
                if (xc-xcStart<0) pim2++;
                continue;
            }
            if (xc>=stride || (xc-xcStart)>=stride2) break;
            pim[xc] = pim2[xc-xcStart];
        }
    }

    return true;
}

// return image must be freed by the caller
static unsigned char* ConvertColorsTo(int dstC,const unsigned char* im,int w,int h,int c,bool assumeThatOneChannelMeansLuminance=true) {
    IM_ASSERT(im && w>0 && h>0 && c>0 && c<=4 && c!=2 && dstC>0 && dstC<=4 && dstC!=2);
    if (!(im && w>0 && h>0 && c>0 && c<=4 && c!=2 && dstC>0 && dstC<=4 && dstC!=2)) return NULL;
    typedef int int_type;
    const int_type wxh = w*h;
    const int_type size =wxh*c;
    const int_type dstSize = wxh*dstC;
    unsigned char* nim = (unsigned char*) STBI_MALLOC(dstSize);
    unsigned char* pni = nim;
    const unsigned char* pim = im;
    if (c==1) {
        // Here wxh == size
        if (dstC==1) memcpy(nim,im,dstSize);
        else if (dstC==3) {for (int_type i=0;i<size;i++)   {*pni++ = *pim;*pni++ = *pim;*pni++ = *pim++;}}
        else if (dstC==4) {
            if (assumeThatOneChannelMeansLuminance) {for (int_type i=0;i<size;i++)   {*pni++ = *pim;*pni++ = *pim;*pni++ = *pim++;*pni++ = 255;}}
            else {for (int_type i=0;i<size;i++)   {*pni++ = 0;*pni++ = 0;*pni++ = 0;*pni++ = *pim++;}}
        }
        else IM_ASSERT(true);
    }
    else if (c==3)   {
        if (dstC==1) {for (int_type i=0;i<wxh;i++)   {*pni++ =(unsigned char) (((unsigned short)(pim[0])+(unsigned short)(pim[1])+(unsigned short)(pim[2]))/3);pim+=3;}}
        else if (dstC==3) memcpy(nim,im,dstSize);
        else if (dstC==4) {for (int_type i=0;i<wxh;i++)   {*pni++=*pim++;*pni++=*pim++;*pni++=*pim++;*pni++=255;}}
        else IM_ASSERT(true);
    }
    else if (c==4) {
        if (dstC==1) {
            if (assumeThatOneChannelMeansLuminance) {for (int_type i=0;i<wxh;i++)   {*pni++ =(unsigned char) (((unsigned short)(pim[0])+(unsigned short)(pim[1])+(unsigned short)(pim[2]))/3);pim+=4;}}
            else {for (int_type i=0;i<wxh;i++)   {pim+=3;*pni++=*pim++;}}
        }
        else if (dstC==3) {for (int_type i=0;i<wxh;i++)   {*pni++=*pim++;*pni++=*pim++;*pni++=*pim++;++pim;}}
        else if (dstC==4) memcpy(nim,im,dstSize);
        else IM_ASSERT(true);
    }
    else {STBI_FREE(nim);nim=NULL;IM_ASSERT(true);}
    return nim;
}

#ifdef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
struct stbir_data_struct {
    int color_space;
    int edge_mode;  // stbir_edge em = (stbir_edge) (edge_mode+1);
    int filter;

    stbir_data_struct() : color_space(0),edge_mode(0),filter(0) {}

    inline void setToDefault() {*this=stbir_data_struct();}
    inline bool isDefault() const {return (color_space==0 && edge_mode==0 && filter==0);}

    inline static const char** GetColorSpaceNames() {
        static const char* names[2] = {"Linear","Srgb"};
        return names;
    }
    inline static const char** GetEdgeModeNames() {
        static const char* names[4] = {"Clamp","Reflect","Wrap","Zero"};
        return names;
    }
    inline static const char** GetFilterNames() {
        static const char* names[6] = {"Default","Box","Triangle","Cubicspline","CatmullRom","Mitchell"};
        return names;
    }
};
#endif //STBIR_INCLUDE_STB_IMAGE_RESIZE_H


static unsigned char* ClipImage(int dstW,int dstH,const unsigned char* im,int w,int h,int c,ImRect* pImageSelectionImOut=NULL) {
    IM_ASSERT(im && w>0 && h>0 && c>0 && c<=4 && c!=2 && dstW>0 && dstH>0);
    if (!(im && w>0 && h>0 && c>0 && c<=4 && c!=2 && dstW>0 && dstH>0)) return NULL;

    unsigned char* nim = CreateImage(dstW,dstH,c);
    const int dstX = (dstW-w)/2;
    const int dstY = (dstH-h)/2;
    bool rv = PasteImage(dstX,dstY,nim,dstW,dstH,c,im,w,h);
    if (rv && pImageSelectionImOut) {
        pImageSelectionImOut->Min.x+=dstX;
        pImageSelectionImOut->Min.y+=dstY;
        pImageSelectionImOut->Max.x+=dstX;
        pImageSelectionImOut->Max.y+=dstY;
        if (pImageSelectionImOut->Min.x<0) pImageSelectionImOut->Min.x=0;
        if (pImageSelectionImOut->Min.y<0) pImageSelectionImOut->Min.y=0;
        if (pImageSelectionImOut->Max.x>dstW) pImageSelectionImOut->Max.x=dstW;
        if (pImageSelectionImOut->Max.y>dstH) pImageSelectionImOut->Max.y=dstH;
    }
    if (!rv) {STBI_FREE(nim);nim=NULL;}

    return nim;
}

// return image must be freed by the caller
static unsigned char* ResizeImage(int dstW,int dstH,const unsigned char* im,int w, int h,int c,ImGuiIE::ResizeFilter filter,void* p_stbir_data_struct=NULL) {
    IM_ASSERT(im && w>0 && h>0 && c>0 && c<=4 && c!=2 && dstW>0 && dstH>0);
    if (!(im && w>0 && h>0 && c>0 && c<=4 && c!=2 && dstW>0 && dstH>0)) return NULL;

    const int dstWxH = dstW*dstH;
    const int dstSize = dstWxH*c;
    unsigned char* nim = (unsigned char*) STBI_MALLOC(dstSize);
    //memset((void*)nim,255,dstSize);
    unsigned char* pni = nim;
    const unsigned char* pim = im;

    if (w<=2 || h<=2) filter = ImGuiIE::RF_NEAREST;  // Otherwise algorithms may fail.

#   ifndef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
    if ((int)filter>ImGuiIE::RF_GOOD) filter = ImGuiIE::RF_GOOD;  // RF_BEST unavailable
#   endif //STBIR_INCLUDE_STB_IMAGE_RESIZE_H

    //typedef float real_t;
    typedef double real_t;

    if  (filter==ImGuiIE::RF_NEAREST) {
        // http://tech-algorithm.com/articles/nearest-neighbor-image-scaling/

        // EDIT: added +1 to account for an early rounding problem
        int x_ratio = (int)((w<<16)/dstW) +1;
        int y_ratio = (int)((h<<16)/dstH) +1;
        int x2=0, y2=0;
        for (int i=0;i<dstH;i++) {
            for (int j=0;j<dstW;j++) {
                x2 = ((j*x_ratio)>>16);
                y2 = ((i*y_ratio)>>16);

                pni = &nim[(i*dstW+j)*c];
                pim = &im[(y2*w+x2)*c];
                for (int ch=0;ch<c;ch++) {*pni++ = *pim++;}
            }
        }
        /*
        const real_t x_ratio = ((real_t)(w-1))/dstW;
        const real_t y_ratio = ((real_t)(h-1))/dstH;
        int x2=0, y2=0;
        for (int i = 0; i<dstH; i++) {
            for (int j = 0; j<dstW; j++) {
                x2 = (int) (x_ratio * j);
                y2 = (int) (y_ratio * i);

                pni = &nim[(i*dstW+j)*c];
                pim = &im[(y2*w+x2)*c];
                for (int ch=0;ch<c;ch++) {*pni++ = *pim++;}
            }
        }*/
    }
    else if (filter==ImGuiIE::RF_GOOD)
    {
        // http://tech-algorithm.com/articles/bilinear-image-scaling/
        int x=0,y=0,index=0;
        const unsigned char *A=NULL;
        const unsigned char *B=NULL;
        const unsigned char *C=NULL;
        const unsigned char *D=NULL;        
        const real_t x_ratio = ((real_t)(w-1))/dstW;
        const real_t y_ratio = ((real_t)(h-1))/dstH;
        real_t x_diff=0, y_diff=0, one_minus_x_diff=0, one_minus_y_diff=0;
        for (int i=0;i<dstH;i++) {
            for (int j=0;j<dstW;j++) {
                x = (int)(x_ratio * j) ;
                y = (int)(y_ratio * i) ;
                x_diff = (x_ratio * j) - x ;
                y_diff = (y_ratio * i) - y ;
                one_minus_x_diff = 1.f - x_diff;
                one_minus_y_diff = 1.f - y_diff;
                index = (y*w+x)*c;
                A = &im[index] ;
                B = &im[index+c] ;
                C = &im[index+w*c] ;
                D = &im[index+(w+1)*c] ;


                for (int ch=0;ch<c;ch++) {
                    // TODO: Alpha now s like all other channels...
                    *pni++= (unsigned char)
                            ((real_t)(*A)*(one_minus_x_diff*one_minus_y_diff) +
                             (real_t)(*B)*(x_diff*one_minus_y_diff) +
                             (real_t)(*C)*(y_diff*one_minus_x_diff) +
                             (real_t)(*D)*(y_diff*x_diff));
                    ++A;++B;++C;++D;
                }
            }
        }

    }
#   ifdef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
    else if (filter==ImGuiIE::RF_BEST)   {
        stbir_colorspace color_space = STBIR_COLORSPACE_LINEAR; // STBIR_COLORSPACE_SRGB
        stbir_edge edge_wrap_mode = STBIR_EDGE_CLAMP;
        stbir_filter filter = STBIR_FILTER_DEFAULT;
        const int alpha_channel = -1;   // what's this for ?
                                  //c==1 ? 0 : c==4 ? 3 : -1;
        const int flags = 0;
        if (p_stbir_data_struct) {
            const stbir_data_struct& ds = *((const stbir_data_struct*) p_stbir_data_struct);
            color_space =  ds.color_space!=1 ? STBIR_COLORSPACE_LINEAR : STBIR_COLORSPACE_SRGB;
            edge_wrap_mode = (stbir_edge) (ds.edge_mode+1);
            filter = (stbir_filter) ds.filter;
        }

        const int rv = stbir_resize_uint8_generic(im,w,h,0,nim,dstW,dstH,0,
                                                  c,alpha_channel,flags,
                                                  edge_wrap_mode,filter, color_space,
                                                  NULL);

        if (rv==0) {
            //fprintf(stderr,"stbir__resize_arbitrary FAILED\n");
            STBI_FREE(nim);nim=NULL;
            // fallback:
            return ResizeImage(dstW,dstH,im,w,h,c,ImGuiIE::RF_GOOD);
        }
    }
#   endif //STBIR_INCLUDE_STB_IMAGE_RESIZE_H
    else {STBI_FREE(nim);nim=NULL;IM_ASSERT(true);}

    return nim;
}

// in place operation
template <typename T> static void FlipX(T* data,int w,int h, int numChannels) {
    T temp;
    T* p;
    T* pInv;
    for (int x=0,hs=w/2;x<hs;x++)	{
        for (int y=0;y<h;y++)	{
            for (int c=0;c<numChannels;c++) {
                p = &data[(w*y+x)*numChannels+c];
                pInv = 	&data[(w*y+(w-x-1))*numChannels+c];
                temp = *p;
                *p = *pInv;
                *pInv = temp;
            }
        }
    }
}

// in place operation
template <typename T> static void FlipY(T* data,int w,int h, int numChannels) {
    int x = w,y=h;
    const int lineByteWidth = x*numChannels;
    T* tempLine = NULL;
    tempLine = (T*) STBI_MALLOC(lineByteWidth*sizeof(T));
    if (tempLine) {
        for (int i = 0; i < y/2; i++)	{
            memcpy(&tempLine[0],&data[i*lineByteWidth],lineByteWidth*sizeof(T));
            memcpy(&data[i*lineByteWidth],&data[(y-i-1)*lineByteWidth],lineByteWidth*sizeof(T));
            memcpy(&data[(y-i-1)*lineByteWidth],&tempLine[0],lineByteWidth*sizeof(T));
        }
        STBI_FREE(tempLine);tempLine=NULL;// or just free(data);
    }
}

// in place operation
template <typename T> static void ShiftX(int offset,bool wrapMode,T* im,int w,int h, int c,const T nullColor=T(255)) {
    if (offset==0 || w==0 || h==0 || c==0) return;
    offset%=w;
    const int lineByteWidth = w*c;
    T* tempLine = (T*) STBI_MALLOC(lineByteWidth*sizeof(T));
    const int off = (offset<0) ? (-offset) : offset;
    const int coff = c*off;
    if (tempLine) {
        const T nullColors[4] = {nullColor,nullColor,nullColor,T(0)};
        for (int y=0;y<h;y++)   {
            // 1) Fill tempLine
            memcpy((void*)&tempLine[0],(const void*)&im[y*w*c],w*c*sizeof(T));
            for (int xc=0,xcSz=w*c;xc<xcSz;xc+=c)    {
                // 2) Fill horizontal line of im with the correct parts of tempLine
                if (offset>0)   {
                    if (xc<coff)   {
                        // We must fill the leftmost 'offset' pixels
                        if (wrapMode) {
                            memcpy((void*)&im[y*w*c+xc],(const void*)&tempLine[(w-off)*c+xc],c*sizeof(T));
                        }
                        else {
                            memcpy((void*)&im[y*w*c+xc],(const void*)&nullColors[0],c*sizeof(T));
                        }
                    }
                    else {
                        memcpy((void*)&im[y*w*c+xc],(const void*)&tempLine[xc-coff],c*sizeof(T));
                    }
                }
                else   {
                    if (xc>=w*c-coff)   {
                        // We must fill the rightmost 'offset' pixels
                        if (wrapMode) {
                            memcpy((void*)&im[y*w*c+xc],(const void*)&tempLine[xc+(-w+off)*c],c*sizeof(T));
                        }
                        else {
                            memcpy((void*)&im[y*w*c+xc],(const void*)&nullColors[0],c*sizeof(T));
                        }
                    }
                    else {
                        memcpy((void*)&im[y*w*c+xc],(const void*)&tempLine[xc+coff],c*sizeof(T));
                    }
                }

            }
        }
        STBI_FREE(tempLine);tempLine=NULL;
    }
}

// in place operation
template <typename T> static void ShiftY(int offset,bool wrapMode,T* im,int w,int h, int c,const T nullColor=T(255)) {
    if (offset==0 || w==0 || h==0 || c==0) return;
    offset%=h;
    const int lineByteWidth = h*c;
    T* tempLine = (T*) STBI_MALLOC(lineByteWidth*sizeof(T));
    const int off = (offset<0) ? (-offset) : offset;
    if (tempLine) {
        const T nullColors[4] = {nullColor,nullColor,nullColor,T(0)};
        for (int xc=0,xcSz=w*c;xc<xcSz;xc+=c) {
            // 1) Fill tempLine
            for (int y=0;y<h;y++)   {
                memcpy((void*)&tempLine[y*c],(const void*)&im[y*w*c+xc],c*sizeof(T));
            }
            // 2) Fill vertical line of im with the correct parts of tempLine
            for (int y=0;y<h;y++)   {
                if (offset>0)   {
                    if (y<off)   {
                        // We must fill the topmost 'offset' pixels
                        if (wrapMode) {
                            memcpy((void*)&im[y*w*c+xc],(const void*)&tempLine[(h-off+y)*c],c*sizeof(T));
                        }
                        else {
                            memcpy((void*)&im[y*w*c+xc],(const void*)&nullColors[0],c*sizeof(T));
                        }
                    }
                    else {
                        memcpy((void*)&im[y*w*c+xc],(const void*)&tempLine[(y-off)*c],c*sizeof(T));
                    }
                }
                else   {
                    if (y>=h-off)   {
                        // We must fill the bottommost 'offset' pixels
                        if (wrapMode) {
                            memcpy((void*)&im[y*w*c+xc],(const void*)&tempLine[(y-h+off)*c],c*sizeof(T));
                        }
                        else {
                            memcpy((void*)&im[y*w*c+xc],(const void*)&nullColors[0],c*sizeof(T));
                        }
                    }
                    else {
                        memcpy((void*)&im[y*w*c+xc],(const void*)&tempLine[(y+off)*c],c*sizeof(T));
                    }
                }
            }
        }
        STBI_FREE(tempLine);tempLine=NULL;
    }
}


// return image must be freed by the caller
template <typename T> T* RotateCW90Deg(const T* im,int w,int h, int c) {
    if (!im || w<=0 || h<=0 || (c!=1 && c!=3 && c!=4)) return NULL;
    T* ni = (T*) STBI_MALLOC(w*h*c*sizeof(T));
    if (ni) {
        const T* pim = im;
        T* pni = ni;
        const int stride2MinusC = (h-1)*c;
        for (int y = 0; y < h; y++)	{
            pni = &ni[stride2MinusC-y*c];
            for (int x = 0; x < w; x++)	{
                for (int i=0;i<c;i++) *pni++=*pim++;
                pni+=stride2MinusC;
            }
        }
    }
    return ni;
}

// return image must be freed by the caller
template <typename T> T* RotateCCW90Deg(const T* im,int w,int h, int c) {
    if (!im || w<=0 || h<=0 || (c!=1 && c!=3 && c!=4)) return NULL;
    T* ni = (T*) STBI_MALLOC(w*h*c*sizeof(T));
    if (ni) {
        const T* pim = im;
        T* pni = ni;
        const int stride2PlusC = (h+1)*c;
        const int offsetStart = (w-1)*h*c;
        for (int y = 0; y < h; y++)	{
            pni = &ni[offsetStart+y*c];
            for (int x = 0; x < w; x++)	{
                for (int i=0;i<c;i++) *pni++=*pim++;
                pni-=stride2PlusC;
            }
        }
    }
    return ni;
}

// in place operation
template <typename T> static void InvertColors(T* im,int w,int h, int c,int numberOfColorChannelsToInvertOnRGBAImages=3,const T fullColorChannel=T(255)) {
    if (!im || w<=0 || h<=0 || (c!=1 && c!=3 && c!=4)) return;
    T *pim = im;const T* cim = im;
    if (c==4) {
        int rgbaMode = numberOfColorChannelsToInvertOnRGBAImages;
        if (rgbaMode!=1 && rgbaMode!=3 && rgbaMode!=4) rgbaMode = 3;
        if (rgbaMode==4) {
            for (int i=0,isz=w*h*c;i<isz;i++) *pim++ = fullColorChannel-(*cim++);
        }
        else if (rgbaMode==3) {
            for (int i=0,isz=w*h*c;i<isz;i+=c) {
                *pim++ = fullColorChannel-(*cim++);*pim++ = fullColorChannel-(*cim++);*pim++ = fullColorChannel-(*cim++);
                *pim++ = *cim++;
            }
        }
        else if (rgbaMode==1) {
            for (int i=0,isz=w*h*c;i<isz;i+=c) {
                *pim++ = *cim++;*pim++ = *cim++;*pim++ = *cim++;
                *pim++ = fullColorChannel-(*cim++);
            }
        }
        else IM_ASSERT(true);
    }
    else if (c==3) {
        for (int i=0,isz=w*h*c;i<isz;i++) *pim++ = fullColorChannel-(*cim++);
    }
    else if (c==1) {
        for (int i=0,isz=w*h*c;i<isz;i++) *pim++ = fullColorChannel-(*cim++);
    }
    else IM_ASSERT(true);
}


// in place operation. Doesn't support c==1. (bad code due to porting)
static bool ApplyLightEffect(unsigned char* im,int w,int h, int c,int lightStrength,ImGuiIE::LightEffect lightEffect,bool clampColorComponentsAtAlpha=true)   {
    if (!im || w<=0 || h<=0 || (c!=3 && c!=4)) return false;
    const bool hasAlpha = (c==4);
    double Dx = 0, Dy = 0;
    double wf = (double)w;
    double hf = (double)h;
    int total = 0;
    double extension = (double)lightStrength;
    int R, G, B, A = 255;
    unsigned char* pim = im;
    if (lightEffect == ImGuiIE::LE_ROUND) {
        for (int y=0; y<h; y++) {
            Dy = (double)y / hf;
            pim = &im[y*w*c];
            for (int x=0; x<w; x++) {
                if (hasAlpha && pim[3]==0) {pim+=c;continue;}
                Dx = (double)x / wf;
                total = (int) ImGuiIE::round (extension * (0.5 - sqrt (Dx * Dy)));  //should be a number between -extension/2 and extension/2
                R = (int)pim[0] + total;G = (int)pim[1] + total;B = (int)pim[2] + total;if (hasAlpha) A = (int)pim[3];
                if (R < 0) R = 0;if (G < 0) G = 0;if (B < 0) B = 0;
                if (!clampColorComponentsAtAlpha) {if (R>255) R=255;if (G>255) G=255;if (B>255) B=255;}
                else {if (B > A) B = A;if (G > A) G = A;if (R > A) R = A;}
                *pim++ = (unsigned char)R;*pim++ = (unsigned char)G;*pim++ = (unsigned char)B;if (hasAlpha) pim++;
            }
        }
    }
    else if (lightEffect == ImGuiIE::LE_SPHERICAL) {
        double hwf = wf*0.5;
        double hhf = hf*0.5;
        double rf = hwf/hhf;
        double sqrtValue;
        for (int y=0; y<h; y++) {
            Dy = ((double)y-hhf)*rf;// / hhf;
            //if (Dy<0) Dy = -Dy;
            pim = &im[y*w*c];
            for (int x=0; x<w; x++) {
                if (hasAlpha && pim[3]==0) {pim+=c;continue;}
                Dx = ((double)x-hwf);// / hwf;
                //if (Dx<0) Dx = -Dx;
                sqrtValue = (double) sqrt (Dx*Dx+Dy*Dy)/(float)hwf;
                total = (int) ImGuiIE::round (extension * (0.5-sqrtValue));  //should be a number between -extension/2 and extension/2
                R = (int)pim[0] + total;G = (int)pim[1] + total;B = (int)pim[2] + total;if (hasAlpha) A = (int)pim[3];
                if (R < 0) R = 0;if (G < 0) G = 0;if (B < 0) B = 0;
                if (!clampColorComponentsAtAlpha) {if (R>255) R=255;if (G>255) G=255;if (B>255) B=255;}
                else {if (B > A) B = A;if (G > A) G = A;if (R > A) R = A;}
                *pim++ = (unsigned char)R;*pim++ = (unsigned char)G;*pim++ = (unsigned char)B;if (hasAlpha) pim++;
            }
        }
    }
    else if (lightEffect== ImGuiIE::LE_LINEAR)
    {
        double dist = 0;
        for (int y=0; y<h; y++) {
            Dy = (double)y / hf;
            pim = &im[y*w*c];
            for (int x=0; x<w; x++) {
                if (hasAlpha && pim[3]==0) {pim+=c;continue;}
                Dx = (double)x / wf;
                dist = 0.5 * (1.0 - (Dx + Dy)); //-0.5<dist<0.5
                total = (int) ImGuiIE::round (dist * extension); //should be a number between -extension/2 and extension/2
                R = (int)pim[0] + total;G = (int)pim[1] + total;B = (int)pim[2] + total;if (hasAlpha) A = (int)pim[3];
                if (R < 0) R = 0;if (G < 0) G = 0;if (B < 0) B = 0;
                if (!clampColorComponentsAtAlpha) {if (R>255) R=255;if (G>255) G=255;if (B>255) B=255;}
                else {if (B > A) B = A;if (G > A) G = A;if (R > A) R = A;}
                *pim++ = (unsigned char)R;*pim++ = (unsigned char)G;*pim++ = (unsigned char)B;if (hasAlpha) pim++;
            }
        }
    }

    return true;
}


// this is needed just for doing Brightness And Contrast
// For now T must be "unsigned char" and R "double" or "float" (=> if you need T "float" you must adjust the code in Channel2Real(...) and RealToChannel(...))
// TODO: Make this work for c==1 too
// (bad code due to porting)
template <typename T,typename R> class ColorMatrixHelper {
    public:
    static bool ApplyBrightnessContrastAndSaturationInPlace(T* im,int w,int h,int c,R brightness,R contrast,R saturation,bool clampRGBToAlpha = true)   {
        if (!im || w<=0 || h<=0 || (c!=1 && c!=3 && c!=4) || (brightness==0 && contrast==0 && saturation==0)) return false;

        R bresult[5][5];
        R cresult[5][5];
        R sresult[5][5];

        brightness *= R(0.025); 					GetBrightnessColorMatrix(brightness,bresult);
        contrast = R(1.0) + contrast * R(0.05);		GetContrastColorMatrix(contrast,cresult);
        saturation = R(1.0) + saturation * R(0.05);	GetSaturationColorMatrix(saturation,sresult);

        R result[5][5];
        MultiplyColorMatrices (cresult,bresult, result);
        MultiplyColorMatrices (result, sresult, result);

        bool ok = TransformByColorMatrix(im,w,h,c,result,true,true,clampRGBToAlpha,true);
        return ok;
    }


    protected:

    // ColorMatrix is a 5x5 affine matrix intended to be multiplied to a RGBAW pixel quintet (W = white).
    // Works for Argb32 and Rgb24 surfaces only.
    // data[] has 5 components per pixel.
    static bool TransformByColorMatrix(T* im,int w,int h,int c,R colorMatrix[5][5],bool excludeAlpha=true,bool excudeFifthComponent = true,bool clampRGBToAlpha=true,bool clampResultIn_0_1 = true)
    {
        if (!colorMatrix || !im)   return false;
        ImVector<R> data,dataResult;
        for (int y=0; y<h; y++) {
            // TODO: this is a nonsense due to porting code, we can probably operate pixels one by one, can we ?
            if (!GetDataLineForColorMatrixOperations (im,w,h,c,y,data)) return false;
            TransformImageLineByColorMatrix (data, colorMatrix, dataResult,excludeAlpha,excudeFifthComponent,clampRGBToAlpha,clampResultIn_0_1);
            if (!SetDataLineForColorMatrixOperations (im,w,h,c,y,dataResult)) return false;
        }
        return true;
    }

    inline static R Channel2Real(T b) {
        static const R _Byte2Double = R(1.0)/R(255.0);
        return (R)b * _Byte2Double;
    }
    inline static T RealToChannel(R d) {
        return (T) (d * R(255.0));
    }
    static bool GetDataLineForColorMatrixOperations(const T* b,int w,int h,int c,int y,ImVector<R>& data)  {
        if (y < 0 || y >= h) return false;
        if (c==1)   {
            // custom code path for c == 1
            const int dataMiniStride = w * 5;
            if ((int)data.size()<dataMiniStride) data.resize(dataMiniStride);
            R tmp(0.f);
            const T* pim = &b[y*w*c];
            R* pR = &data[0];
            for (int x = 0; x < w; x++) {
                tmp = Channel2Real(*pim++);
                *pR++=tmp;*pR++=tmp;*pR++=tmp;*pR++=1;  // RGBA
                *pR++=1;                                // W
            }
            return true;
        }
        if (c<3 || c>4) return false;
        // c==3 || c==4 here
        int tempMiniStride = w * c;
        ImVector<T> temp;temp.resize(tempMiniStride);
        int dataMiniStride = w * 5;
        if ((int)data.size()<dataMiniStride) data.resize(dataMiniStride);

        memcpy((void*)&temp[0],(const void*) (b + y*(w*c)),tempMiniStride);

        //---- temp => data (Convert bytes to decimals)
        int xc,x5;
        for (int x = 0; x < w; x++) {
            xc = c * x;x5 = 5 * x;
            for (int i=0;i<c;i++)	{
                data[x5+i] = Channel2Real(temp[xc+i]);
            }
            if (c==3) data[x5+3] = 1.0;
            data[x5+4] = 1.0;
        }
        //---------------------------------------------------
        return true;
    }

    static bool SetDataLineForColorMatrixOperations(T* im,int w,int h,int c,int y,const ImVector<R>& data)    {
        if (y < 0 || y >= h) return false;
        if (c==1)   {
            // custom code path for c == 1
            IM_ASSERT(data.size()>=w*5);
            T* pim = &im[y*w*c];
            R tmp(0.f);
            const R* pR = &data[0];
            for (int x = 0; x < w; x++) {
                tmp=*pR++;tmp+=*pR++;tmp+=*pR++;tmp/=3.f;pR++;pR++; // The last 2 skip AW
                *pim++=RealToChannel(tmp);
            }
            return true;
        }
        if (c<3 || c>4) return false;
        // c==3 || c==4 here
        int effectiveWidth = w;
        int tempMiniStride = w * c;
        int tempSize = tempMiniStride;
        int dataMiniStride = w * 5;
        int dataSize = dataMiniStride;
        if ((int)data.size() < dataMiniStride) {
            dataSize = data.size();
            effectiveWidth = dataSize/5;
            tempSize = effectiveWidth * c;
        }
        ImVector<T> temp;temp.resize(tempSize);

        // "data" => "temp in RGBA or RGB format"
        int xc,x5;
        for (int x = 0; x < effectiveWidth; x++) {
            xc = c * x;x5 = 5 * x;
            for (int i=0;i<c;i++)	{
                temp[xc+i] = RealToChannel(data[x5+i]);
            }
            //if (c==3) temp[xc+3] = 255;
        }

        memcpy((void*) ((im + y * (w*c))),(const void*) &temp[0],tempSize);
        return true;
    }

    static void TransformImageLineByColorMatrix(const ImVector<R>& l,R m[5][5],ImVector<R>& r,bool excludeAlpha=false,bool excudeFifthComponent = true,bool clampRGBToAlpha=false,bool clampResultIn_0_1 = true)
    {
        if (r.size()<l.size()) r.resize(l.size());
        int x5,x51,x52,x53,x54;
        for (int x=0,sz = (int)l.size()/5; x<sz; x++) {
            x5 = 5*x;x51=x5+1;x52=x5+2;x53=x5+3;x54=x5+4;
            r[x5]  = l[x5]*m[0][0] + l[x51]*m[1][0] + l[x52]*m[2][0] + l[x53]*m[3][0] + l[x54]*m[4][0];
            r[x51] = l[x5]*m[0][1] + l[x51]*m[1][1] + l[x52]*m[2][1] + l[x53]*m[3][1] + l[x54]*m[4][1];
            r[x52] = l[x5]*m[0][2] + l[x51]*m[1][2] + l[x52]*m[2][2] + l[x53]*m[3][2] + l[x54]*m[4][2];
            if (excludeAlpha) r[x53] = l[x53];
            else r[x53] = l[x5]*m[0][3] + l[x51]*m[1][3] + l[x52]*m[2][3] + l[x53]*m[3][3] + l[x54]*m[4][3];
            if (excudeFifthComponent) r[x54] = l[x54];
            else r[x54] = l[x5]*m[0][4] + l[x51]*m[1][4] + l[x52]*m[2][4] + l[x53]*m[3][4] + l[x54]*m[4][4];

            if (clampResultIn_0_1)	{
                if (!excludeAlpha) {
                    if (r[x53]>T(1.0)) r[x53] = T(1.0);
                    else if (r[x53]<0.0)  r[x53] = 0.0;
                }
                if (!excudeFifthComponent)	{
                    if (r[x54]>T(1.0)) r[x54] = T(1.0);
                    else if (r[x54]<0.0)  r[x54] = 0.0;
                }
                R mx = clampRGBToAlpha ? r[x53] : T(1.0);
                if (r[x5]>mx)  r[x5] = mx;
                else if (r[x5]<0.0)  r[x5] = 0.0;
                if (r[x51]>mx) r[x51] = mx;
                else if (r[x51]<0.0)  r[x51] = 0.0;
                if (r[x52]>mx) r[x52] = mx;
                else if (r[x52]<0.0)  r[x52] = 0.0;
            }
            else if (clampRGBToAlpha) {
                R mx = r[x53];
                if (r[x5]>mx)  r[x5] = mx;
                if (r[x51]>mx) r[x51] = mx;
                if (r[x52]>mx) r[x52] = mx;
            }
        }
    }


    inline static void GetIdentityColorMatrix(R mOut[5][5])  {
        // Set the values of the identity matrix
        mOut[0][0] = 1; mOut[0][1] = 0; mOut[0][2] = 0; mOut[0][3] = 0; mOut[0][4] = 0;
        mOut[1][0] = 0; mOut[1][1] = 1; mOut[1][2] = 0; mOut[1][3] = 0; mOut[1][4] = 0;
        mOut[2][0] = 0; mOut[2][1] = 0; mOut[2][2] = 1; mOut[2][3] = 0; mOut[2][4] = 0;
        mOut[3][0] = 0; mOut[3][1] = 0; mOut[3][2] = 0; mOut[3][3] = 1; mOut[3][4] = 0;
        mOut[4][0] = 0; mOut[4][1] = 0; mOut[4][2] = 0; mOut[4][3] = 0; mOut[4][4] = 1;
    }
    inline static void MultiplyColorMatrices(const R f1[5][5],const R f2[5][5],R result[5][5])    {
        R X[5][5];
        int size = 5;
        R column[5];
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                column[k] = f1[k][j];
            }
            for (int i = 0; i < 5; i++) {
                const R* row = &f2[i][0];
                R s = 0;
                for (int k = 0; k < size; k++)  {
                    s += row[k] * column[k];
                }
                X[i][j] = s;
            }
        }

        for (int i=0;i<5;i++)   {
            for (int j=0;j<5;j++)   {
                result[i][j] = X[i][j];
            }
        }
        // Note that the transition throgh X[5][5] is probably mandatory only in cases when result == f1 or result == f2
    }

    static void GetBrightnessColorMatrix(R brightness,R bmOut[5][5])  {
        // Set the values of the brightness matrix
        bmOut[0][0] = 1; bmOut[0][1] = 0; bmOut[0][2] = 0; bmOut[0][3] = 0; bmOut[0][4] = 0;
        bmOut[1][0] = 0; bmOut[1][1] = 1; bmOut[1][2] = 0; bmOut[1][3] = 0; bmOut[1][4] = 0;
        bmOut[2][0] = 0; bmOut[2][1] = 0; bmOut[2][2] = 1; bmOut[2][3] = 0; bmOut[2][4] = 0;
        bmOut[3][0] = 0; bmOut[3][1] = 0; bmOut[3][2] = 0; bmOut[3][3] = 1; bmOut[3][4] = 0;
        bmOut[4][0] = brightness; bmOut[4][1] = brightness; bmOut[4][2] = brightness; bmOut[4][3] = 0; bmOut[4][4] = 1;
    }
    static void GetContrastColorMatrix(R contrast,R cmOut[5][5])  {
        // Set the values of contrast matrix
        R C = R(0.5) * (R(1.0) - contrast);

        cmOut[0][0] = contrast; cmOut[0][1] = 0; cmOut[0][2] = 0; cmOut[0][3] = 0; cmOut[0][4] = 0;
        cmOut[1][0] = 0; cmOut[1][1] = contrast; cmOut[1][2] = 0; cmOut[1][3] = 0; cmOut[1][4] = 0;
        cmOut[2][0] = 0; cmOut[2][1] = 0; cmOut[2][2] = contrast; cmOut[2][3] = 0; cmOut[2][4] = 0;
        cmOut[3][0] = 0; cmOut[3][1] = 0; cmOut[3][2] = 0; cmOut[3][3] = 1; cmOut[3][4] = 0;
        cmOut[4][0] = C; cmOut[4][1] = C; cmOut[4][2] = C; cmOut[4][3] = 0; cmOut[4][4] = 1;
    }
    static void GetSaturationColorMatrix(R saturation,R smOut[5][5])  {
        // Set the values of saturation matrix
        R s = saturation;
        R sr = R(0.3086) * (R(1.0) - s);
        R sg = R(0.6094) * (R(1.0) - s);
        R sb = R(0.0820) * (R(1.0) - s);

        smOut[0][0] = s+sr; smOut[0][1] = sr; smOut[0][2] = sr; smOut[0][3] = 0; smOut[0][4] = 0;
        smOut[1][0] = sg; smOut[1][1] = s+sg; smOut[1][2] = sg; smOut[1][3] = 0; smOut[1][4] = 0;
        smOut[2][0] = sb; smOut[2][1] = sb; smOut[2][2] = s+sb; smOut[2][3] = 0; smOut[2][4] = 0;
        smOut[3][0] = 0; smOut[3][1] = 0; smOut[3][2] = 0; smOut[3][3] = 1; smOut[3][4] = 0;
        smOut[4][0] = 0; smOut[4][1] = 0; smOut[4][2] = 0; smOut[4][3] = 0; smOut[4][4] = 1;
    }
};

inline ImU32 GetPulsingValueInternal(ImU32 solidBase=40,float pulseSpeed=20.f) {
    // To avoid <math.h>. Max amplitude 40. (values in [0,40])
    const static unsigned char sinLookupTable[48] = {20,23,25,28,30,32,34,36,37,38,39,40,40,40,39,38,37,36,34,32,30,28,25,23,20,17,15,12,10,8,6,4,3,2,1,0,0,0,1,2,3,4,6,8,10,12,15,17};
    const ImU32 index = ((ImU32)(ImGui::GetTime()*pulseSpeed))%48;    // speed = coefficient of GetTime()
    return solidBase + sinLookupTable[index];
}
inline void DrawSelection(ImDrawList* drawlist,const ImRect& selection,ImU32 color,float thickness) {
    if (color == 0) {
        const ImU32 comp = GetPulsingValueInternal(40,20.f);
        color = IM_COL32(comp,comp,comp,255);
    }
    if (thickness<=0) thickness = 5.f;
    drawlist->AddRect(selection.Min,selection.Max,color,0,0,thickness);
}
inline ImVec4 GetPulsingColorFrom(const ImVec4& color)   {
    ImVec4 colorNew = color;
    colorNew.w-=0.0875f;
    //if (colorNew.w<0.16f) colorNew.w=0.16f;
    if (colorNew.w<0.f) colorNew.w=0.f;
    if (colorNew.w>0.82f) colorNew.w=0.82f;
    colorNew.w+=(GetPulsingValueInternal(30,50.f)/255.f); // 40 goes from [0.f,0.16f]
    return colorNew;
}

// Derived from his article: http://www.codeproject.com/cs/media/floodfillincsharp.asp
class FillHelperClass
{
private:
//  PRIVATE STATIC VARIABLES
    static int startColorMinusTolerance[4];
    static int startColorPlusTolerance[4];
    static ImVector<unsigned char> PixelsChecked;
    static unsigned char RGBA[4];
    static unsigned char* im;
    static int w;
    static int h;
    static int c;
    static int X;
    static int Y;
    static int W;
    static int H;

public:

// affects only 4 channels images:
inline static void PremultiplyAlpha(unsigned char* col)  {
    const int A = col[3];
    for (int i=0;i<3;i++) {
        *col = (unsigned char)(((int)(*col)*A)/255);
        ++col;
    }
}
// affects only 4 channels images:
inline static void DePremultiplyAlpha(unsigned char* col)  {
    const int A = col[3];
    if (A == 0 || A==255) return;
    int tmp=0;
    for (int i=0;i<3;i++) {
        tmp = (((int)(*col)*255)/A);
        if (tmp>255) tmp=255;
        *col++ = (unsigned char) tmp;
    }
}
inline static void ToByteArray(unsigned char rv[4],const ImVec4& v) {
    const float* pv = &v.x;
    for (int i=0;i<4;i++) rv[i] = (unsigned char) ((*pv++)*255.f);
}

public:
FillHelperClass()   {}

inline static bool Fill(unsigned char* im,int w,int h,int c,const ImVec2& imagePoint,const ImVec4& fillColor,const ImVec4& tolerance,
const ImRect* pImageSelection=NULL,bool mirrorX=false,bool mirrorY=false,bool premultiplyAlphaHere=false)   {
    unsigned char fillColorUC[4];unsigned char toleranceUC[4];
    ToByteArray(fillColorUC,fillColor);ToByteArray(toleranceUC,tolerance);
    return Fill(im,w,h,c, imagePoint, fillColorUC, toleranceUC,pImageSelection,mirrorX,mirrorY,premultiplyAlphaHere);
}
inline static bool Fill(unsigned char* im,int w,int h,int c,const ImVec2& imagePoint,const unsigned char fillColor[4],const unsigned char tolerance[4],
const ImRect* pImageSelection=NULL,bool mirrorX=false,bool mirrorY=false,bool premultiplyAlphaHere=false)   {
    if ((c!=1 && c!=3 && c!=4) || !im || w<=0 || h<=0) return false;
    FillHelperClass::im = im;
    FillHelperClass::w=w;FillHelperClass::h=h;FillHelperClass::c=c;
    X=Y=0;W=w;H=h;
    if (pImageSelection) {
        X=pImageSelection->Min.x;
        Y=pImageSelection->Min.y;
        W=pImageSelection->Max.x-pImageSelection->Min.x;
        H=pImageSelection->Max.y-pImageSelection->Min.y;
        if (X<=0 || Y<=0 || W<=0 || H<=0) return false;
    }
    memcpy(RGBA,fillColor,4);
    if (c==4 && premultiplyAlphaHere) PremultiplyAlpha(RGBA);
    if (c==1) RGBA[0]=RGBA[1]=RGBA[2]=RGBA[3];  // alpha to RGB
    PixelsChecked.resize(W*H);for (int i=0,isz=PixelsChecked.size();i<isz;i++) PixelsChecked[i] = 0;
    unsigned char startcolor[4]={255,255,255,255};
    int ptx = (int) imagePoint.x;
    int pty = (int) imagePoint.y;
    if (ptx<X || ptx>=X+W || pty<Y || pty>=Y+H) return false;

    for (int i=0;i<4;i++)   {
        if (i==1) {
            if (!mirrorX && !mirrorY) break;
            if (!mirrorX) continue;
            // MirrorX
            ptx = (int)((W-1)-((int) imagePoint.x-X)+X);
            pty =  (int) imagePoint.y;
        }
        else if (i==2) {
            if (!mirrorY) break;
            // MirrorY
            ptx =  (int) imagePoint.x;
            pty = (int)((H-1)-((int) imagePoint.y-Y)+Y);
        }
        else if (i==3) {
            if (!mirrorX || !mirrorY) break;
            // Mirror X and Y
            ptx = (int)((W-1)-((int) imagePoint.x-X)+X);
            pty = (int)((H-1)-((int) imagePoint.y-Y)+Y);
        }
        else IM_ASSERT(true);
        IM_ASSERT(ptx>=X && ptx<=X+W-1);
        IM_ASSERT(pty>=Y && pty<=Y+H-1);
        if (GetPixelCheckedAt(ptx,pty)) continue;
        if (c!=1) {
            memcpy(startcolor,GetPixelAt(ptx,pty),c);
            for (int i=0;i<4;i++)   {
                startColorMinusTolerance[i] = (int) startcolor[i]-(int) tolerance[i];
                startColorPlusTolerance[i] = (int) startcolor[i]+(int) tolerance[i];
            }
        }
        else {
            startcolor[0]=startcolor[1]=startcolor[2]=startcolor[3] = *GetPixelAt(ptx,pty);
            for (int i=0;i<4;i++)   {
                startColorMinusTolerance[i] = (int) startcolor[3]-(int) tolerance[3];
                startColorPlusTolerance[i] = (int) startcolor[3]+(int) tolerance[3];
            }
        }
        LinearFloodFill(ptx,pty);
    }
    return true;
}

// FloodFIll Stuff
private:
inline static unsigned char* GetPixelAt(int x,int y) {return &im[(y*w+x)*c];}
inline static void SetPixelCheckedAt(int x,int y,bool v) {PixelsChecked[(y-Y)*W+(x-X)] = v ? 1 : 0;}
inline static bool GetPixelCheckedAt(int x,int y) {return PixelsChecked[(y-Y)*W+(x-X)]!=0;}

inline static bool CheckPixel(int x,int y) {
    bool ret=true;
    const unsigned char* pim = GetPixelAt(x,y);
    for(unsigned char i=0;i<c;i++)  {
        ret&= ((int)(*pim)>=startColorMinusTolerance[i]) && ((int)(*pim)<=startColorPlusTolerance[i]);
        ++pim;
    }
    return ret;
}
static void LinearFloodFill(int x,int y)    {
    //FIND LEFT EDGE OF COLOR AREA
    int LFillLoc=x; //the location to check/fill on the left
    unsigned char* pim = NULL;
    while (true) {
        pim = GetPixelAt(LFillLoc,y);
        for (int i=0;i<c;i++) *pim++ = RGBA[i];
        SetPixelCheckedAt(LFillLoc,y,true);
        LFillLoc--; 		 	 //de-increment counter
        if(LFillLoc<X || !CheckPixel(LFillLoc,y) ||  (GetPixelCheckedAt(LFillLoc,y)))
            break;			 	 //exit loop if we're at edge of image or color area
    }
    LFillLoc++;

    //FIND RIGHT EDGE OF COLOR AREA
    int RFillLoc=x; //the location to check/fill on the left
    while (true) {
        pim = GetPixelAt(RFillLoc,y);
        for (int i=0;i<c;i++) *pim++ = RGBA[i];
        SetPixelCheckedAt(RFillLoc,y,true);
        RFillLoc++; 		 //increment counter
        if(RFillLoc>=(X+W) || !CheckPixel(RFillLoc,y) ||  (GetPixelCheckedAt(RFillLoc,y)))
            break;			 //exit loop if we're at edge of image or color area
    }
    RFillLoc--;

    //START THE LOOP UPWARDS AND DOWNWARDS
    for(int i=LFillLoc;i<=RFillLoc;i++)
    {
        //START LOOP UPWARDS
        //if we're not above the top of the image and the pixel above this one is within the color tolerance
        if (y>Y && CheckPixel(i,y-1) && (!(GetPixelCheckedAt(i,y-1))))
            LinearFloodFill(i,y-1);
        //START LOOP DOWNWARDS
        if(y<(Y+H-1) && CheckPixel(i,y+1) && (!(GetPixelCheckedAt(i,y+1))))
            LinearFloodFill(i,y+1);
    }
}

};
int FillHelperClass::startColorMinusTolerance[4]={0,0,0,0};
int FillHelperClass::startColorPlusTolerance[4]={0,0,0,0};
ImVector<unsigned char> FillHelperClass::PixelsChecked;
unsigned char FillHelperClass::RGBA[4]={0,0,0,0};
unsigned char* FillHelperClass::im=NULL;
int FillHelperClass::w=0;
int FillHelperClass::h=0;
int FillHelperClass::c=0;
int FillHelperClass::X=0;
int FillHelperClass::Y=0;
int FillHelperClass::W=0;
int FillHelperClass::H=0;


// This class is used for "CopiedImage" (the static image that can be cut/copied/paste.
// However it has later be enhanced to be the base for "UndoStack" (some methods has been added only for this purpose)
class StbImageBuffer {
public:
    int w,h,c;  // relative to the stored image (can be calculated from imageSelection)

    int W,H;                // relative to the whole src image
    ImRect imageSelection;  // relative to the whole src image [full image: W==w H==h imageSelection.Min(0,0) imageSelection.Max(w,h)]
    typedef void (*DestroyMemoryBufferCallback)(unsigned char* buffer);

protected:

    int imCompressedSize;    // if > 0, im is a compressed format
    bool mustCompressIt;
    unsigned char* im;
    DestroyMemoryBufferCallback imDestructorCb;

    // returned value must be freed with freePixels(...)
    unsigned char* getPixels() const {
        if (imCompressedSize==0) return im;
        // We use stbi_image to decompress it
        int W=0,H=0,C=0;    // not used
        unsigned char* data = NULL;
#       ifdef LODEPNG_H
        if (imDestructorCb==&DestroyBufferLodePng) lodepng_decode_memory(&data,&W,&H,im,(size_t) compressedSize,c==4 ? LCT_RGBA : (c==3 ? LCT_RGB : (c==1 ? LCT_GREY : LCT_RGBA)), 8);
        else data = stbi_load_from_memory(im,imCompressedSize,&W,&H,&C,c);
#       else // LODEPNG_H
        data = stbi_load_from_memory(im,imCompressedSize,&W,&H,&C,c);
#       endif //LODEPNG_H

        IM_ASSERT(data && W==w && H==h);
        return data;
    }
    void freePixels(unsigned char*& pxl) const {
        if (imCompressedSize>0) {
            IM_ASSERT(pxl!=im);
#           ifdef LODEPNG_H
            if (pxl && imDestructorCb==&DestroyBufferLodePng) {free(pxl);pxl=NULL;}
#           endif //LODEPNG_H
            if (pxl) {STBI_FREE(pxl);pxl=NULL;}
            IM_ASSERT(pxl == NULL);
        }
        else IM_ASSERT(pxl==im);
    }
    friend class ScopedData;

#  ifdef INCLUDE_STB_IMAGE_WRITE_H
   static void DestroyBufferStbiw(unsigned char* buffer) {STBIW_FREE(buffer);}
#  endif // INCLUDE_STB_IMAGE_WRITE_H
#  ifdef LODEPNG_H
   static void DestroyBufferLodePng(unsigned char* buffer) {free(buffer);}
#  endif // LODEPNG_H


public:

    inline bool isSelectionValid() const {return (imageSelection.Min.x<imageSelection.Max.x || imageSelection.Min.y<imageSelection.Max.y);}
    void destroy() {
        if (im) {
            if (imCompressedSize==0) STBI_FREE(im);
            else {
                IM_ASSERT(imDestructorCb);
                imDestructorCb(im);
            }
            im=NULL;
        }
        w=h=c=W=H=0;imCompressedSize=0;
        imageSelection.Min=imageSelection.Max=ImVec2(0,0);
        imDestructorCb = NULL;
    }
    ~StbImageBuffer() {destroy();}
    StbImageBuffer(bool _mustCompressIt=false) : w(0),h(0),c(0),W(0),H(0),mustCompressIt(_mustCompressIt),im(NULL) {imageSelection.Min=imageSelection.Max=ImVec2(0,0);}
    void assign(const unsigned char* srcIm,int srcW,int srcH,int srcC,const ImRect* sel=NULL) {
        destroy();
        if (srcIm && srcW>=0 && srcH>=0 && (srcC==1 || srcC==3 || srcC==4)) {
            if (!sel)   {
                im = (unsigned char*)STBI_MALLOC(srcW*srcH*srcC);
                if (im) {
                    memcpy((void*)im,(const void*)srcIm,srcW*srcH*srcC);
                    w=W=srcW;h=H=srcH;c=srcC;                    
                }
            }
            else {
                const int X = sel->Min.x;
                const int Y = sel->Min.y;
                const int W = sel->Max.x - sel->Min.x;
                const int H = sel->Max.y - sel->Min.y;
                im = (unsigned char*) STBI_MALLOC(W*H*srcC);
                if (im) {
                    this->W = srcW; this->H = srcH;   // full size
                    for (int y=Y;y<Y+H;y++)   {
                       memcpy((void*) &im[(y-Y)*W*srcC],(const void*)&srcIm[(y*srcW+X)*srcC],W*srcC);
                    }
                    w=W;h=H;c=srcC;
                    imageSelection.Min = sel->Min;imageSelection.Max = sel->Max;                    
                }
            }
        }       
        if (mustCompressIt && im) {
#           if (!defined(STBI_NO_PNG) || defined(LODEPNG_H))
#           ifdef INCLUDE_STB_IMAGE_WRITE_H
            if (imCompressedSize==0)    {
                unsigned char* buffer = stbi_write_png_to_mem(im,0,w,h,c,&imCompressedSize);
                if (buffer) {STBI_FREE(im);im=buffer;imDestructorCb=&DestroyBufferStbiw;}
                else imCompressedSize=0;
            }
#           endif // INCLUDE_STB_IMAGE_WRITE_H
#           ifdef LODEPNG_H
            if (imCompressedSize==0)    {
                unsigned char* buffer = NULL;
                size_t bufferSize=0;
                const unsigned error = lodepng_encode_memory(&buffer,&bufferSize,im,w,h,c==4 ? LCT_RGBA : (c==3 ? LCT_RGB : (c==1 ? LCT_GREY : LCT_RGBA)), 8);
                if (error) {free(buffer);buffer=NULL;}
                else {
                    STBI_FREE(im);im=buffer;
                    imCompressedSize=(int)bufferSize;
                    imDestructorCb=&DestroyBufferLodePng;
                }
            }
#           endif // LODEPNG_H
#           endif // (!defined(STBI_NO_PNG) || defined(LODEPNG_H))
        }
    }
    bool isCompressed() const {return (imCompressedSize>0);}
    bool isValid() const {return (im!=NULL);}

    // This wrapping must be used to get (decompressed) image pixels
    class ScopedData {
        protected:
        const StbImageBuffer& buffer;
        unsigned char* im;
        friend class StbImageBuffer;
        public:
        ScopedData(const StbImageBuffer& _buffer) : buffer(_buffer) {im = buffer.getPixels();}
        ~ScopedData() {buffer.freePixels(im);}
        int getW() const {return buffer.w;}
        int getH() const {return buffer.h;}
        int getC() const {return buffer.c;}
        const unsigned char* getImage() const {return im;}
    };

    bool copyTo(unsigned char*& nim,int& nw,int& nh,int nc) const {
        if (!isValid()) return false;
        IM_ASSERT(nim && nc==c);
        const bool selectionValid = isSelectionValid();
        if (nw!=W || nh!=H) {
            if (selectionValid) {
                IM_ASSERT(W == imageSelection.Max.x - imageSelection.Min.x);
                IM_ASSERT(H == imageSelection.Max.y - imageSelection.Min.y);
            }
            STBI_FREE(nim);
            nw=W;nh=H;
            nim = (unsigned char*) STBI_MALLOC(nw*nh*c);

        }
        ScopedData data(*this);

        int XS = selectionValid ? imageSelection.Min.x : 0;
        int YS = selectionValid ? imageSelection.Min.y : 0;
        int WS = selectionValid ? (imageSelection.Max.x - imageSelection.Min.x) : nw;
        int HS = selectionValid ? (imageSelection.Max.y - imageSelection.Min.y) : nh;
        if (XS >= nw) XS= nw-1;
        if (YS >= nh) YS= nh-1;
        if (WS+XS>nw) WS = nw-XS;
        if (HS+YS>nh) HS = nh-YS;

        const unsigned char* im = data.getImage();
        if (im) {
            for (int y=YS;y<YS+HS;y++)   {
                memcpy((void*) &nim[(y*nw+XS)*nc],(const void*)&im[((y-YS)*w)*c],WS*c);
            }
        }
        return (im!=NULL);
    }
};


//#define IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK


// We could have used just something like: StbImageBuffer buffers[stack_max], since we don't resize them at runtime...
class UndoStack : protected ImVector<StbImageBuffer*> {
    protected:
    typedef ImVector<StbImageBuffer*> Base;
    typedef StbImageBuffer ItemType;

    static void DeleteItem(ItemType*& it) {
        if (it) {
            it->~StbImageBuffer();  // it should call it->destroy
            ImGui::MemFree(it);
            it = NULL;
        }
    }
    static void ClearItem(ItemType*& it) {if (it) it->destroy();}

    const int stack_max;    //Number Of Available Undo/Redo (higher=more memory used)
    int stack_cur;          //=0;

    public:

    ~UndoStack() {clear(true);}
    UndoStack(int stackMax=-1) : Base(),stack_max(stackMax>0 ? stackMax : 25),stack_cur(0)  {
        this->resize(stack_max);   // We resize it soon, and we never resize it again
    }

    inline void clear(bool destroyEverything=false) {
        if (Data) {
            if (destroyEverything) {
                for (int i=0;i<Size;i++) {DeleteItem(Data[i]);}
                ImGui::MemFree(Data);Data = NULL;
                Size = Capacity = 0;
            }
            else {for (int i=0;i<Size;i++) {ClearItem(Data[i]);}}            
        }
        stack_cur=0;
    }
    inline int getStackCur() const {return stack_cur;}

    void push(const unsigned char* im,int w,int h,int c,const ImRect* sel=NULL,bool clearNextEntries = false)    {
        if (stack_cur<stack_max)    {
            Data[stack_cur]->assign(im,w,h,c,sel);
#           ifdef IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK
            fprintf(stderr,"%s at %d -> (%d,%d), (%d,%d) [w=%d h=%d c=%d]\n",
                    clearNextEntries ? "Pushing" : "UNDO. Pushing",
                    stack_cur,
                    sel?(int)sel->Min.x:0,
                    sel?(int)sel->Min.y:0,
                    sel?(int)(sel->Max.x-sel->Min.x):w,
                    sel?(int)(sel->Max.y-sel->Min.y):h,
                    w,
                    h,
                    c
                    );
#           endif //IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK
            //if (stack[stack_cur]== null) Console.WriteLine ("\tStackPush(): Error: stack[stack_cur]==null with: stack_cur={0}, stack_max={1}", stack_cur, stack_max);
            stack_cur++;

            if (clearNextEntries) {
                for (int i = stack_cur;i<stack_max;i++) {
                    ItemType*& it = Data[i];
                    if (!it->isValid()) break;
                    ClearItem(it);
                }
            }
            //Console.WriteLine ("\tStackPush(): stack_cur={0}, stack_max={1}", stack_cur, stack_max);
            return;
        }
        //stack_cur == stack_max:
        //if (stack_cur!=stack_max) Console.WriteLine ("\tStackPush(): Error: stack_cur!=stack_max with: stack_cur={0}, stack_max={1}", stack_cur, stack_max);
        ItemType* Data0 = Data[0];
        if (Data0->isValid()) {
            /*stack [0].Dispose ();*/Data0->destroy(); //Discard stack[0]
        }
        for (int t=1;t<stack_max;t++)   {
            Data[t-1]=Data[t];	// Shift stack
        }
        Data[stack_max-1] = Data0;
        Data[stack_cur-1]->assign(im,w,h,c,sel);	// push the last element (remember stack_cur == stack_max here)
        //if (stack[stack_cur-1]== null) Console.WriteLine ("\tStackPush(): Error: stack[stack_cur-1]==null with: stack_cur={0}, stack_max={1}", stack_cur, stack_max);
        //Console.WriteLine ("\tStackPush(): stack_cur={0}, stack_max={1}", stack_cur, stack_max);
    }
    const ItemType* undo(const unsigned char* im,int w,int h,int c) //,const ImRect* sel=NULL)
    {
        //fprintf(stderr,"StackUndo(): stack_cur=%d, stack_max=%d\n", stack_cur, stack_max);
        //IM_ASSERT(sel==NULL);   // if this works we can remove last arg
        if (stack_cur<0) {//Console.WriteLine ("StackUndo(): Error: stack_cur<0, with stack_cur = "+stack_cur);
            stack_cur=0;return NULL;}
        if (stack_cur<1) return NULL;
        if (!Data[stack_cur-1]->isValid() /*|| stack[stack_cur-1]==image*/) {
            //fprintf(stderr,"StackUndo(): Error: stack [stack_cur - 1] == null, with stack_cur = %d\n",stack_cur);
            return NULL;
        }
        const ImRect* sel = Data[stack_cur-1]->isSelectionValid() ? &Data[stack_cur-1]->imageSelection : NULL;
        push(im,w,h,c,sel);
        stack_cur--;
        stack_cur--;
        while (stack_cur>=stack_max) stack_cur--;

        return Data[stack_cur];
    }

    const ItemType* redo(const unsigned char* im,int w,int h,int c)    {
        //Console.WriteLine ("StackRedo(): stack_cur={0}, stack_max={1}", stack_cur, stack_max);
        if (stack_cur+1<stack_max && !Data[stack_cur+1]->isValid()) {
            //Console.WriteLine ("StackRedo(): Nothing to do: 'stack_cur+1<stack_max && stack[stack_cur+1]==null' with: stack_cur={0}, stack_max={1}", stack_cur, stack_max);
            return NULL;
        }
        stack_cur++;
        if (stack_cur>=stack_max) {
            //Console.WriteLine ("StackRedo(): Nothing to do: stack_cur>=stack_max with: stack_cur={0}, stack_max={1} => Setting: stack_cur=stack_max-1", stack_cur, stack_max);
            stack_cur=stack_max-1;return NULL;
        }

        IM_ASSERT(stack_cur>0);
        const ImRect* sel = Data[stack_cur]->isSelectionValid() ? &Data[stack_cur]->imageSelection : NULL;
        Data[stack_cur-1]->assign(im,w,h,c,sel);

#       ifdef IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK
        fprintf(stderr,"REDO. Assigning At %d -> (%d,%d), (%d,%d)  [w=%d h=%d c=%d]\n",
                stack_cur-1,
                sel?(int)sel->Min.x:0,
                sel?(int)sel->Min.y:0,
                sel?(int)(sel->Max.x-sel->Min.x):w,
                sel?(int)(sel->Max.y-sel->Min.y):h,
                w,
                h,
                c
                );
#       endif //IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK
        return Data[stack_cur];
    }

    protected:

    inline void resize(int new_size)            {
        if (new_size > Capacity) reserve(_grow_capacity(new_size));
        if (new_size < Size) {for (int i=new_size;i<Size;i++) DeleteItem(Data[i]);}
        else if (new_size > Size) {
            for (int i=Size;i<new_size;i++) {
                ItemType* it = (ItemType*) ImGui::MemAlloc(sizeof(ItemType));
                if (it) {
                    IM_PLACEMENT_NEW (it) ItemType(true);
                    Data[i] = it;
                }
            }
        }
        Size = new_size;
    }

};


} //namespace ImGuiIE


namespace ImGui {



struct StbImage {

    // We wrap our callback, because we want to force one channel images to be fed as RGB
    static void GenerateOrUpdateTextureCb(ImTextureID& texID,int w,int h,int c,const unsigned char* image,bool useMipmaps,bool wraps,bool wrapt,bool minFilterNearest,bool magFilterNearest) {
        if (image) {
            IM_ASSERT(ImGui::ImageEditor::GenerateOrUpdateTextureCb);   // Please call ImGui::ImageEditor::SetGenerateOrUpdateTextureCallback(...) at InitGL() time.

            const unsigned char* im = image;
            unsigned char* imageRGB = NULL;
            if (c==1)   {
                imageRGB = ImGuiIE::ConvertColorsTo(3,image,w,h,c,true);
                IM_ASSERT(imageRGB);
                im = imageRGB;c=3;
            }
            ImGuiIE::ImageScopedDeleter scopedDeleter(imageRGB);

            ImGui::ImageEditor::GenerateOrUpdateTextureCb(texID,w,h,c,im,useMipmaps,wraps,wrapt,minFilterNearest,magFilterNearest);
        }
    }

    ImageEditor* ie;            // reference
    unsigned char* image;
    int w,h,c;
    ImTextureID texID;

    static ImTextureID IconsTexID;
    static ImTextureID CheckersTexID;

    struct BrightnessWindowData {
        ImTextureID baseTexID,previewTexID;
        unsigned char *baseImage,*previewImage;
        int w,h,c;
        float bri,con,sat;
        bool mustInvalidatePreview;
        float zoom;
        ImVec2 zoomCenter;
        bool mustPrecessSelection;
        int X,Y,W,H;
        BrightnessWindowData(const StbImage& s) {
            resetBCS();
            IM_ASSERT(s.image && s.w>=0 && s.h>=0 && (s.c==1 || s.c==3 || s.c==4));
            baseTexID = previewTexID = NULL;
            baseImage = previewImage = NULL;
            // Fill baseImage and baseTexID here--------------------------
            mustPrecessSelection = s.chbSelectionLimit && s.isImageSelectionValid();
            X = mustPrecessSelection ? ((int)(s.imageSelection.Min.x)) : 0;
            Y = mustPrecessSelection ? ((int)(s.imageSelection.Min.y)) : 0;
            w = W = mustPrecessSelection ? ((int)(s.imageSelection.Max.x-s.imageSelection.Min.x)) : s.w;
            h = H = mustPrecessSelection ? ((int)(s.imageSelection.Max.y-s.imageSelection.Min.y)) : s.h;
            c=s.c;

            unsigned char* im = mustPrecessSelection ? ImGuiIE::ExtractImage(X,Y,W,H,s.image,s.w,s.h,s.c) : s.image;
            ImGuiIE::ImageScopedDeleter scoped(mustPrecessSelection ? im : NULL);

            // baseImage
            const int maxWxH = 128*128;
            if (w*h>maxWxH) {
                // resize s.image
                const float scaling = (float)maxWxH/(float)(w*h);
                w = (int) ((float)w*scaling);if (w<=0) w=1;
                h = (int) ((float)h*scaling);if (h<=0) h=1;

                baseImage = (unsigned char*) STBI_MALLOC(w*h*c);
                void* pStr=NULL;
#               ifdef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
                ImGuiIE::stbir_data_struct str;pStr = (void*) &str;
#               endif // STBIR_INCLUDE_STB_IMAGE_RESIZE_H
                baseImage = ImGuiIE::ResizeImage(w,h,im,W,H,c,ImGuiIE::RF_BEST,pStr);
            }
            else {
                // clone s.image
                baseImage = (unsigned char*) STBI_MALLOC(w*h*c);
                memcpy((void*) baseImage,(const void*) im,w*h*c);
            }
            IM_ASSERT(baseImage);
            // baseTexID
            StbImage::GenerateOrUpdateTextureCb(baseTexID,w,h,c,baseImage,false,false,false,true,true);
            IM_ASSERT(baseTexID);
            //------------------------------------------------------------
            // Now we clone baseImage to previewImage
            previewImage = (unsigned char*) STBI_MALLOC(w*h*c);
            memcpy((void*) previewImage ,(const void*) baseImage,w*h*c);
            IM_ASSERT(previewImage);
            // previewTexID
            StbImage::GenerateOrUpdateTextureCb(previewTexID,w,h,c,previewImage,false,false,false,true,true);
            IM_ASSERT(previewTexID);
            mustInvalidatePreview = false;
            zoom=1.f;zoomCenter.x=zoomCenter.y=0.5f;
        }
        float getAspectRatio() const {return w/h;}
        ImTextureID getTextureID() {
            if (mustInvalidatePreview)  {
                mustInvalidatePreview=false;
                if (previewTexID) {
                    IM_ASSERT(ImageEditor::FreeTextureCb);
                    ImageEditor::FreeTextureCb(previewTexID);previewTexID=NULL;
                }
            }
            if (previewTexID) return previewTexID;
            IM_ASSERT(previewImage);
            StbImage::GenerateOrUpdateTextureCb(previewTexID,w,h,c,previewImage,false,false,false,true,true);
            return previewTexID;
        }
        void invalidatePreview() {
            mustInvalidatePreview = true;
        }
        ~BrightnessWindowData() {
            if (baseImage) {STBI_FREE(baseImage);baseImage=NULL;}
            if (previewImage) {STBI_FREE(previewImage);previewImage=NULL;}
            if (baseTexID) {
                IM_ASSERT(ImageEditor::FreeTextureCb);
                ImageEditor::FreeTextureCb(previewTexID);baseTexID=NULL;
            }if (previewTexID) {
                IM_ASSERT(ImageEditor::FreeTextureCb);
                ImageEditor::FreeTextureCb(previewTexID);previewTexID=NULL;
            }
        }
        void resetBCS() {
            bri=con=sat=0;
        }
        bool applyTo(StbImage& s) {
            bool ok = false;
            if (!mustPrecessSelection) {
                s.pushImage();
                ok = ImGuiIE::ColorMatrixHelper<unsigned char,float>::ApplyBrightnessContrastAndSaturationInPlace(s.image,s.w,s.h,s.c,bri,con,sat,true);
            }
            else {                
                unsigned char* im = ImGuiIE::ExtractImage(X,Y,W,H,s.image,s.w,s.h,s.c);
                ImGuiIE::ImageScopedDeleter scoped(im);
                if (im) {
                    const ImRect sel(ImVec2(X,Y),ImVec2(X+W,Y+H));
                    s.pushImage(&sel);
                    ok = ImGuiIE::ColorMatrixHelper<unsigned char,float>::ApplyBrightnessContrastAndSaturationInPlace(im,W,H,s.c,bri,con,sat,true);
                    if (ok) ImGuiIE::PasteImage(X,Y,s.image,s.w,s.h,s.c,im,W,H);
                }
            }
            if (ok) s.mustInvalidateTexID = true;
            return ok;
        }
        void updatePreview() {
            IM_ASSERT(baseImage);
            invalidatePreview();

            // Now we clone baseImage to previewImage
            if (!previewImage) previewImage = (unsigned char*) STBI_MALLOC(w*h*c);
            memcpy((void*) previewImage ,(const void*) baseImage,w*h*c);
            IM_ASSERT(previewImage);

            ImGuiIE::ColorMatrixHelper<unsigned char,float>::ApplyBrightnessContrastAndSaturationInPlace(previewImage,w,h,c,bri,con,sat,true);
        }
    };
    BrightnessWindowData* brightnessWindowData;

    float zoom;
    ImVec2 zoomCenter;
    ImVec2 zoomMaxAndZoomStep;
    ImVec2 uv0,uv1,uvExtension,imageSz,startPos,endPos; // local data used in renderImage() and in mouseToImageCoords() and in imageToMouseCoords()
    ImRect imageSelection;  // in image space

    char* filePath;             // [owned]
    const char* filePathName;   // Reference into filePath
    char* fileExt;              // [owned]
    bool fileExtCanBeSaved;
    bool fileExtHasFullAlpha;
    char imageDimString[64];
    char fileSizeString[64];

    bool modified;
    ImVec2 slidersFraction;

    int resizeWidth,resizeHeight;
    bool resizeKeepsProportions;
    int resizeFilter;
    int shiftPixels;

    int tolColor,tolAlpha;
    int penWidth;
    ImVec4 penColor;
    ImRect penSelection;
    ImVector<ImVec2> penPointsInImageCoords;

    ImGui::Toolbar modifyImageTopToolbar;
    ImGui::Toolbar modifyImageBottomToolbar;

    // check buttons:
    bool chbShiftImageSelection,chbShiftImageWrap,isShiftPixelsAreaHovered;
    bool chbMirrorX,chbMirrorY,chbSelectionLimit;
    bool chbLightEffectInvert;

    bool mustInvalidateTexID;    
    bool mustUpdateFileListSoon;

    int shiftImageSelectionMode;

#   ifdef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
    ImGuiIE::stbir_data_struct myStbirData;
#   endif //STBIR_INCLUDE_STB_IMAGE_RESIZE_H

    static ImGuiIE::StbImageBuffer CopiedImage; // TODO: delete before exiting ? Or just in its dcr ?

    ImGuiIE::UndoStack undoStack;

    static void ImStrAllocate(char*& p,const char* text) {
        ImStrDelete(p);
        if (text) {
            const size_t len = strlen(text) + 1;
            p = (char*) ImGui::MemAlloc(len);
            memcpy(p, (const void*)text, len);
        }
    }
    static void ImStrDelete(char*& p) {if (p) {ImGui::MemFree(p);p=NULL;}}
    static void ImStrAllocateFileExtension(char*& p,const char* fileName,const char** pOptionalExtensionReference=NULL) {
        const char* filePathExt = strrchr(fileName,'.');
        if (pOptionalExtensionReference) *pOptionalExtensionReference = filePathExt;
        ImStrAllocate(p,filePathExt);
        char* pp = p;
        if (pp) {for (int i=0,sz=strlen(p);i<sz;i++) {*pp = tolower(*pp);++pp;}}
    }
    static const char* ImGetFileNameReference(const char* filePath) {
        const char* filePathName = strrchr(filePath,'/');
        if (!filePathName) filePathName = strrchr(filePath,'\\');
        if (filePathName) filePathName++;
        else filePathName = filePath;
        return filePathName;
    }
    static void ImFormatFileSizeString(char* buffer,int buffer_size,int size_in_bytes=0) {
        IM_ASSERT(buffer_size>0 && buffer);
        if (size_in_bytes<=0) {buffer[0]='\0';return;}
        const int B = size_in_bytes%1024;
        int K = size_in_bytes/1024;
        int M = K/1024;
        K = K%1024;
        int G = M/1024;
        int T = G/1024;
        G = G%1024;
        char ch[6]="\0";
        float sf = 0;
        if (T>0) {strcpy(ch,"Tb");sf=T+(float)G/1024.f;}
        else if (G>0) {strcpy(ch,"Gb");sf=G+(float)M/1024.f;}
        else if (M>0) {strcpy(ch,"Mb");sf=M+(float)K/1024.f;}
        else if (K>0) {strcpy(ch,"Kb");sf=K+(float)B/1024.f;}
        else {strcpy(ch,"b");sf=B;}
        ImFormatString(buffer,buffer_size,"size %1.3f %s",sf,ch);
    }
    static void ImFormatImageDimString(char* buffer,int buffer_size,int w,int h) {
        IM_ASSERT(buffer_size>0 && buffer);
        const int total_pixels = w*h;
        const int B = total_pixels%1000;
        int K = total_pixels/1000;
        int M = K/1000;
        K = K%1000;
        int G = M/1000;
        int T = G/1000;
        G = G%1000;
        char ch[6]="\0";
        float pxls = 0;
        if (T>0) {strcpy(ch,"Tpxl");pxls=T+(float)G/1000.f;}
        else if (G>0) {strcpy(ch,"Gpxl");pxls=G+(float)M/1000.f;}
        else if (M>0) {strcpy(ch,"Mpxl");pxls=M+(float)K/1000.f;}
        else if (K>0) {strcpy(ch,"Kpxl");pxls=K+(float)B/1000.f;}
        else {strcpy(ch,"pxl");pxls=B;}
        ImFormatString(buffer,buffer_size,"%dx%d = %1.3f %s",w,h,pxls,ch);
    }
    struct FileExtensionHelper {
        char* ext;      // [owned]
        const char* fileName; // ref
        const char* filePath; // ref

        FileExtensionHelper(const char* path) {
            fileName = filePath = path;
            ext = NULL;
            fileName = ImGetFileNameReference(path);
            ImStrAllocateFileExtension(ext,fileName);
        }
        ~FileExtensionHelper() {ImStrDelete(ext);}

    };

#   ifdef IMGUI_FILESYSTEM_H_
    ImGuiFs::Dialog LoadDialog;
    ImGuiFs::Dialog SaveDialog;
    ImGuiFs::Dialog SaveOrLoseDialog;

    ImGuiFs::PathStringVector filePaths;
    int filePathsIndex,newFilePathsIndex;
#   endif //IMGUI_FILESYSTEM_H_

#   ifdef INCLUDE_STB_IMAGE_WRITE_H
    static void stbi_write_callback(void *context, void *data, int size) {
        fwrite(data,size,1,(FILE*)context);
    }
#   endif //INCLUDE_STB_IMAGE_WRITE_H

    StbImage(ImageEditor& _ie) : ie(&_ie),image(NULL),w(0),h(0),c(0),texID(NULL),modified(false),
    undoStack(
#       ifndef IMGUIIMAGEEDITOR_UNDO_SIZE
        25
#       else  // IMGUIIMAGEEDITOR_UNDO_SIZE
        IMGUIIMAGEEDITOR_UNDO_SIZE
#       endif //IMGUIIMAGEEDITOR_UNDO_SIZE
    )
     {
        resetZoomAndPan();
        slidersFraction.x=0.25f;slidersFraction.y=0.5f;
        resizeWidth=resizeHeight=resizeFilter=0;
        resizeKeepsProportions=true;
        shiftPixels=4;
        penWidth = 1;
        penColor = ImVec4(0,0,0,1);
        penSelection.Min=penSelection.Max=ImVec2(0,0);
        tolColor = 32; tolAlpha=0;
        filePath = fileExt = NULL;filePathName = NULL;fileExtCanBeSaved = true;
        fileSizeString[0] = imageDimString[0] = '\0';
        chbShiftImageSelection = chbShiftImageWrap = isShiftPixelsAreaHovered = false;
        chbMirrorX = chbMirrorY = chbSelectionLimit = chbLightEffectInvert = false;
        mustInvalidateTexID = mustUpdateFileListSoon = false;
#       ifdef IMGUI_FILESYSTEM_H_
        filePathsIndex=-1;
#       endif // IMGUI_FILESYSTEM_H_
        brightnessWindowData = NULL;
        fileExtHasFullAlpha = true;
        imageSz.x=imageSz.y=0;
        startPos.x=startPos.y=0;
        endPos.x=endPos.y=0;
        imageSelection.Min=imageSelection.Max=ImVec2(0,0);
        shiftImageSelectionMode = 0;

        ImGuiIE::InitSupportedFileExtensions(); // can be called multiple times safely
    }
    ~StbImage() {destroy();}
    static void InitSupportTextures() {
        // Warning: This method inits toolbars too, so it can be refactored as static!
        IM_ASSERT(ImGui::ImageEditor::GenerateOrUpdateTextureCb);
        if (!IconsTexID) {
            const unsigned char data[] = {
                137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,96,0,0,0,96,8,3,0,0,0,213,70,135,10,0,0,1,128,80,76,84,69,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,79,50,9,77,69,0,174,73,8,92,
                87,0,104,109,0,138,117,2,116,127,0,120,128,0,129,131,0,255,255,241,255,255,241,255,255,241,255,255,241,255,255,241,255,255,241,255,255,241,255,255,241,255,255,241,255,255,241,255,
                255,241,255,255,254,255,255,241,255,255,241,255,255,241,255,255,245,255,255,254,255,255,255,255,255,255,255,255,255,255,255,255,255,254,240,254,254,253,238,251,54,242,251,90,249,251,170,248,
                251,163,240,252,70,249,250,248,235,249,38,249,249,167,244,246,243,239,246,105,237,242,116,237,241,237,244,241,148,240,240,175,228,239,22,233,237,179,234,237,123,241,236,143,227,234,229,239,
                230,135,233,231,129,221,229,12,236,229,132,210,225,211,207,221,203,212,222,189,197,219,196,200,219,195,213,217,3,207,210,1,204,203,0,204,196,1,211,170,8,168,171,184,172,169,170,195,
                168,87,170,166,167,168,166,157,175,165,132,186,163,113,142,162,201,168,162,148,162,161,159,161,160,157,202,159,66,159,158,154,159,158,155,208,156,24,156,155,155,207,153,78,106,151,234,100,
                149,238,100,149,238,100,149,238,100,149,238,100,149,238,217,140,49,111,138,186,113,127,151,119,120,123,107,107,108,107,103,102,100,89,83,124,78,59,168,66,31,184,59,20,188,57,15,195,
                56,15,62,45,18,52,41,33,47,39,36,44,39,38,43,38,37,39,31,26,31,19,13,7,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,201,203,80,0,0,0,0,
                24,116,82,78,83,0,0,0,0,0,0,0,0,0,0,56,150,224,232,242,254,252,253,249,246,250,246,245,245,136,238,67,75,0,0,4,211,73,68,65,84,104,222,237,152,91,111,226,
                70,24,134,247,54,146,33,150,74,192,127,193,55,27,76,2,9,160,36,165,136,136,32,69,178,176,91,201,101,21,115,81,201,170,193,75,204,26,124,224,98,254,122,103,198,167,177,199,
                16,27,217,219,86,202,43,133,36,144,188,207,124,135,249,102,224,203,69,197,250,242,19,0,127,157,210,197,233,215,47,62,252,119,4,224,142,11,59,156,253,50,247,159,4,200,178,92,
                28,0,0,64,191,130,228,119,244,72,57,200,50,73,248,87,0,161,75,142,20,101,3,192,105,0,120,129,2,138,34,157,87,3,232,15,8,2,13,192,254,144,32,41,63,21,16,39,
                237,3,0,240,117,10,0,10,0,210,53,192,230,19,158,79,20,49,9,0,60,239,2,23,4,254,231,1,38,124,220,38,89,0,34,128,98,0,236,14,211,67,18,210,0,232,207,251,
                77,196,21,5,248,139,71,0,158,32,80,37,74,6,80,96,31,224,210,78,144,100,146,144,6,164,2,200,223,166,50,6,208,132,36,32,242,151,184,51,1,20,225,8,32,99,22,229,
                1,80,132,12,64,34,65,5,138,44,135,59,44,38,208,17,160,63,64,5,150,184,51,1,50,65,64,253,196,101,0,0,185,254,194,0,57,206,210,36,163,139,176,191,164,112,103,3,
                56,130,144,209,166,240,41,207,35,243,83,168,139,100,56,129,184,152,144,181,209,96,137,83,235,47,52,174,101,4,8,167,117,230,168,64,254,210,241,35,51,62,44,19,63,197,14,24,
                224,19,178,135,93,134,127,33,0,121,226,100,30,23,18,237,127,238,181,165,248,173,162,234,139,87,213,119,211,255,255,245,253,243,253,65,242,41,6,170,86,97,155,50,192,117,92,23,
                80,0,180,61,211,95,231,0,234,192,51,183,91,139,152,85,185,0,54,22,233,19,60,147,6,192,245,67,255,173,105,185,128,2,208,103,65,4,176,237,20,161,30,62,147,2,48,158,
                107,109,177,28,15,20,168,129,109,171,146,164,24,33,0,57,27,202,55,69,73,3,128,191,126,44,211,3,5,34,48,196,213,90,52,130,16,144,189,161,207,102,234,58,5,128,249,177,
                182,145,112,150,114,3,164,189,163,172,253,36,193,71,93,20,37,85,90,167,106,0,14,174,101,110,9,2,176,11,68,160,170,162,164,7,137,215,21,81,92,81,69,174,129,203,205,187,
                105,154,71,0,145,84,53,171,6,186,174,175,84,148,36,168,149,40,42,122,80,244,24,80,247,220,141,70,2,76,55,51,2,245,245,85,165,35,56,120,30,0,134,191,110,93,148,68,
                61,108,41,226,242,237,108,52,77,219,68,4,232,159,85,3,232,31,16,168,125,80,119,141,111,127,174,141,149,36,74,235,168,101,67,64,13,252,208,166,250,229,239,19,68,48,253,18,
                51,25,93,132,253,125,2,181,147,15,0,192,226,194,242,138,74,188,37,130,151,129,231,104,191,141,12,111,118,187,244,99,192,249,201,216,7,186,26,1,168,89,112,2,0,224,250,7,
                51,7,120,51,254,238,15,68,176,2,127,42,2,61,38,20,72,81,13,174,255,87,241,111,99,173,235,34,255,168,189,191,59,161,63,61,139,34,2,40,80,100,198,209,166,35,103,119,
                203,143,118,251,193,35,172,116,228,159,49,236,8,66,238,54,101,140,103,117,15,12,254,197,80,85,227,59,244,63,216,39,206,131,84,25,242,108,52,198,24,233,206,126,245,178,3,207,
                35,247,7,177,254,236,97,23,17,240,150,203,51,42,152,157,246,120,247,85,129,111,192,103,3,237,210,35,252,179,1,33,225,85,205,57,236,24,103,179,156,242,35,152,196,233,242,59,
                185,254,35,227,58,36,16,128,211,227,26,70,160,105,83,248,6,240,110,105,130,132,255,145,243,128,104,214,92,7,14,6,104,143,252,116,153,204,207,241,3,39,13,248,224,200,196,128,
                247,229,116,105,122,117,59,207,229,151,74,209,7,7,158,31,1,236,255,84,126,78,0,2,229,4,212,220,75,168,157,229,82,254,71,107,224,43,239,181,197,102,124,213,236,207,235,251,
                231,245,157,99,61,150,227,90,77,82,37,125,250,206,250,98,0,203,114,173,198,98,30,104,241,86,26,192,11,197,178,173,198,88,16,122,72,66,103,81,30,192,80,208,129,174,238,234,
                44,219,152,8,237,155,118,187,125,51,20,230,229,1,20,254,118,48,248,250,108,129,58,219,24,15,175,31,186,80,247,189,18,1,226,237,122,183,183,246,46,3,35,24,11,67,172,78,
                175,196,20,137,119,59,6,22,249,224,215,224,169,255,244,212,239,151,12,176,96,27,121,184,139,198,195,238,3,82,187,204,20,25,43,135,69,221,202,33,64,231,30,107,88,102,4,245,
                26,218,100,72,40,69,29,44,161,68,0,86,4,232,220,116,175,161,74,236,34,82,24,128,252,175,251,165,237,131,102,235,151,72,87,87,104,39,119,122,61,152,34,161,172,20,53,223,
                230,209,248,105,16,179,104,94,214,44,106,46,224,154,209,162,5,97,220,184,186,138,195,105,149,5,152,11,125,156,245,155,206,184,209,170,224,51,187,230,188,119,143,252,187,149,1,22,
                189,176,245,43,3,12,253,205,91,93,138,218,120,250,116,135,149,69,208,233,163,1,250,84,93,138,58,254,17,32,84,215,69,232,8,123,184,174,42,69,115,97,136,79,225,182,48,169,
                40,69,157,224,30,81,85,138,222,22,196,44,170,2,64,170,10,192,231,251,131,147,250,7,3,226,55,153,10,90,150,76,0,0,0,0,73,69,78,68,174,66,96,130
            };

            int x=0,y=0,c=0;
            unsigned char* im = stbi_load_from_memory(data,sizeof(data),&x,&y,&c,4);
            if (im) {
                StbImage::GenerateOrUpdateTextureCb(IconsTexID,x,y,4,im,true,false,false,false,false);
                STBI_FREE(im);im=NULL;
            }
        }
        if (!CheckersTexID) {
            const unsigned char data[] = {
                137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,8,2,0,0,0,144,145,104,54,0,0,0,3,115,66,73,84,8,8,8,219,225,79,224,
                0,0,0,42,73,68,65,84,40,145,99,252,242,229,11,3,54,176,101,203,22,172,226,76,88,69,241,128,81,13,196,0,22,92,225,237,227,227,67,29,27,70,53,16,3,0,175,205,
                8,26,145,81,231,29,0,0,0,0,73,69,78,68,174,66,96,130
            };
            int x=0,y=0,c=0;
            unsigned char* im = stbi_load_from_memory(data,sizeof(data),&x,&y,&c,3);
            if (im) {
                StbImage::GenerateOrUpdateTextureCb(CheckersTexID,x,y,3,im,true,true,true,false,false);  // last 2 args can be "true,true" here... if we cut down the checker image size (up tp 4 pxls!)
                STBI_FREE(im);im=NULL;
            }
        }
    }
    static void Destroy() {
        IM_ASSERT(ImageEditor::FreeTextureCb);
        if (IconsTexID) {
            IM_ASSERT(ImageEditor::FreeTextureCb);
            ImageEditor::FreeTextureCb(IconsTexID);IconsTexID=NULL;
        }
        if (CheckersTexID) {
            IM_ASSERT(ImageEditor::FreeTextureCb);
            ImageEditor::FreeTextureCb(CheckersTexID);CheckersTexID=NULL;
        }
    }
    void initToolbars() {
        if (!IconsTexID) InitSupportTextures();
        if (IconsTexID) {
            ImVec2 btnSize(32,32);

            if (modifyImageTopToolbar.getNumButtons()==0)   {
                modifyImageTopToolbar.clearButtons();
                modifyImageTopToolbar.addButton(ImGui::Toolbutton("select",IconsTexID,ImVec2(0,0),ImVec2(0.3334f,0.3334f),btnSize));
                modifyImageTopToolbar.addButton(ImGui::Toolbutton("fill",IconsTexID,ImVec2(0.3334f,0.f),ImVec2(0.6667f,0.3334f),btnSize));
                modifyImageTopToolbar.addButton(ImGui::Toolbutton("draw",IconsTexID,ImVec2(0.6667f,0),ImVec2(1.0f,0.3334f),btnSize));
                modifyImageTopToolbar.setProperties(true,false,false,ImVec2(0.0f,0.f),ImVec2(0.85f,0.2f),ImVec4(1.f,0.85f,0.0f,0.8f));
                modifyImageTopToolbar.setSelectedButtonIndex(0);
            }

            if (modifyImageBottomToolbar.getNumButtons()==0)   {
                modifyImageBottomToolbar.clearButtons();
                modifyImageBottomToolbar.addButton(ImGui::Toolbutton("extract",IconsTexID,ImVec2(0,0.3334f),ImVec2(0.3334f,0.6667f),btnSize));
                modifyImageBottomToolbar.addSeparator(btnSize.x*0.5f);
                btnSize.x*=0.8f;btnSize.y*=0.8f;
                modifyImageBottomToolbar.addButton(ImGui::Toolbutton("cut",IconsTexID,ImVec2(0.3334f,0.3334f),ImVec2(0.6667f,0.6667f),btnSize));
                modifyImageBottomToolbar.addButton(ImGui::Toolbutton("copy",IconsTexID,ImVec2(0.f,0.6667f),ImVec2(0.3334f,1.f),btnSize));
                modifyImageBottomToolbar.addButton(ImGui::Toolbutton("paste",IconsTexID,ImVec2(0.6667f,0.3334f),ImVec2(1.f,0.6667f),btnSize));
                modifyImageBottomToolbar.setProperties(false,false,false,ImVec2(0.f,0.f),ImVec2(0.5f,0.5f),ImVec4(1.f,0.85f,0.0f,0.8f));
            }
        }
    }
    void resetZoomAndPan() {
        zoom = 1.f;
        zoomCenter.x=zoomCenter.y=0.5f;
        zoomMaxAndZoomStep.x = 16.f;
        zoomMaxAndZoomStep.y = 1.025f;

        uv0.x=uv0.y=0;
        uv1.x=uv1.y=1;
        uvExtension.x=uvExtension.y=1;
    }
    void updateFileList() {
#       ifdef IMGUI_FILESYSTEM_H_
        filePaths.clear();
        filePathsIndex=-1;
        char parentFolder[ImGuiFs::MAX_PATH_BYTES] = "";
        ImGuiFs::PathStringVector allFilePaths;
#       if (defined(IMGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
        ImGuiFs::PathGetDirectoryNameWithZipSupport(filePath,parentFolder);
        ImGuiFs::DirectoryGetFilesWithZipSupport(parentFolder,allFilePaths);
#       else //IMGUI_USE_MINIZIP
        ImGuiFs::PathGetDirectoryName(filePath,parentFolder);
        ImGuiFs::DirectoryGetFiles(parentFolder,allFilePaths);
#       endif //IMGUI_USE_MINIZIP
        char curFilePathExt[ImGuiFs::MAX_FILENAME_BYTES] = "";
        for (int i=0,isz=allFilePaths.size();i<isz;i++) {
            //fprintf(stderr,"%d) %s\n",i,allFilePaths[i]);
            const char* curFilePathExtRef = strrchr(allFilePaths[i],(int)'.');
            if (curFilePathExtRef)  {
                // We need it lowercase
                strcpy(curFilePathExt,curFilePathExtRef);
                for (int l=0,lsz=strlen(curFilePathExt);l<lsz;l++) curFilePathExt[l] = tolower(curFilePathExt[l]);
                if (strstr(ImGuiIE::SupportedLoadExtensions,curFilePathExt)!=NULL) {
                    filePaths.resize(filePaths.size()+1);
                    strcpy(filePaths[filePaths.size()-1],allFilePaths[i]);
                    if (strcmp(filePath,allFilePaths[i])==0) filePathsIndex = filePaths.size()-1;
                    //fprintf(stderr,"%s\n",filePaths[filePaths.size()-1]);
                }
            }
        }
#       endif // IMGUI_FILESYSTEM_H_
        mustUpdateFileListSoon = false;
    }
    void assignFilePath(const char* path,bool updateFilePathsList=true) {
        IM_ASSERT(path);                                // Can't be NULL; please use "\0"
        IM_ASSERT(path!=filePath);                      // Can't reallocate the same memory      

        if (path==filePath) return;

#       ifdef IMGUI_FILESYSTEM_H_
        char filePathAbsolute[ImGuiFs::MAX_PATH_BYTES] = "";
#       if (defined(IMGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
        ImGuiFs::PathGetAbsoluteWithZipSupport(path,filePathAbsolute);
#       else // IMGUI_USE_MINIZIP
        ImGuiFs::PathGetAbsolute(path,filePathAbsolute);
#       endif //IMGUI_USE_MINIZIP
        ImStrAllocate(filePath,filePathAbsolute);
#       else //IMGUI_FILESYSTEM_H_
        ImStrAllocate(filePath,path);
#       endif //IMGUI_FILESYSTEM_H_
        filePathName = ImGetFileNameReference(filePath);
        ImStrAllocateFileExtension(fileExt,filePathName);
        fileExtCanBeSaved = (strstr(ImGuiIE::SupportedSaveExtensions[c],fileExt)!=NULL);
#       if (defined(IMGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
        if (ImGuiFs::PathIsInsideAZipFile(path)) fileExtCanBeSaved = false;
#       endif // IMGUI_USE_MINIZIP
        fileExtHasFullAlpha = !((strcmp(fileExt,".gif")==0) || (strcmp(fileExt,".ico")==0) || (strcmp(fileExt,".cur")==0));

#       ifdef IMGUITABWINDOW_H_
        ie->setLabel(filePathName);
        ie->setTooltip(filePath);
#       endif //IMGUITABWINDOW_H_

        if (updateFilePathsList || mustUpdateFileListSoon)    {
            updateFileList();
        }
    }
    void clearFilePath(bool clearFilePathsLists=false) {
        ImStrDelete(filePath);ImStrDelete(fileExt);filePathName=NULL;
        if  (clearFilePathsLists)   {
#       ifdef IMGUI_FILESYSTEM_H_
        filePaths.clear();
	filePathsIndex=newFilePathsIndex=-1;
#       endif //IMGUI_FILESYSTEM_H_
        }
    }
    void assignModified(bool flag) {
        modified = flag;
#       ifdef IMGUITABWINDOW_H_
        ie->setModified(flag);
#       endif //IMGUITABWINDOW_H_
        if (modified) fileSizeString[0]='\0';
    }
    void clearModified() {assignModified(false);}
    void destroy() {
        if (image) STBI_FREE(image);
        w=h=c=0;
        if (texID) {
            IM_ASSERT(ImageEditor::FreeTextureCb);
            ImageEditor::FreeTextureCb(texID);texID=NULL;
        }
        if (brightnessWindowData) {
            brightnessWindowData->~BrightnessWindowData();
            ImGui::MemFree(brightnessWindowData);
            brightnessWindowData=NULL;
        }
        resetZoomAndPan();
        clearFilePath();
    }
    void clear(bool keepPath=false) {
        if (image) {STBI_FREE(image);image=NULL;}
        w=h=c=0;
        resizeWidth=resizeHeight=0;
        mustInvalidateTexID = true;
        if (!keepPath) {clearFilePath();fileSizeString[0]=imageDimString[0] = '\0';}
        modified=false;
        resetZoomAndPan();
    }

    void pushImage(const ImRect* pSel=NULL) {undoStack.push(image,w,h,c,pSel,true);}
    bool undo() //const ImRect* pSel=NULL)
    {
        const ImGuiIE::StbImageBuffer* buffer = undoStack.undo(image,w,h,c);//,pSel);
        if (buffer) {
            const int oldW = w;
            const int oldH = h;
            if (buffer->copyTo(image,w,h,c))   {
#               ifdef IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK
                // Debug---------------------------------------
                const ImRect* sel = &buffer->imageSelection;
                fprintf(stderr,"UNDO. Copying image from: %d -> (%d,%d), (%d,%d) [w=%d h=%d c=%d]\n",
                    undoStack.getStackCur(),
                    sel?(int)sel->Min.x:0,
                    sel?(int)sel->Min.y:0,
                    sel?(int)(sel->Max.x-sel->Min.x):w,
                    sel?(int)(sel->Max.y-sel->Min.y):h,
                    buffer->W,
                    buffer->H,
                    buffer->c
                    );
                //-----------------------------------------------
#               endif //IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK
                mustInvalidateTexID = true;
                assignModified(true);
                if (oldW!=w || oldH!=h) {
                    ImFormatImageDimString(imageDimString,64,w,h);
                    imageSelection.Min = imageSelection.Max = ImVec2(0,0);
                }
                return true;
            }
        }
        return false;
    }
    bool redo() {
        const ImGuiIE::StbImageBuffer* buffer = undoStack.redo(image,w,h,c);
        if (buffer) {
            const int oldW = w;
            const int oldH = h;
            if (buffer->copyTo(image,w,h,c))   {
#               ifdef IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK
                // Debug---------------------------------------
                const ImRect* sel = &buffer->imageSelection;
                fprintf(stderr,"REDO. Copying image from: %d -> (%d,%d), (%d,%d)[w=%d h=%d c=%d]\n",
                    undoStack.getStackCur(),
                    sel?(int)sel->Min.x:0,
                    sel?(int)sel->Min.y:0,
                    sel?(int)(sel->Max.x-sel->Min.x):w,
                    sel?(int)(sel->Max.y-sel->Min.y):h,
                    buffer->W,
                    buffer->H,
                    buffer->c
                    );
                //-----------------------------------------------
#               endif //IMGUIIMAGEEDITOR_DEBUG_UNDO_STACK
                mustInvalidateTexID = true;
                assignModified(true);
                if (oldW!=w || oldH!=h) {
                    ImFormatImageDimString(imageDimString,64,w,h);
                    imageSelection.Min = imageSelection.Max = ImVec2(0,0);
                }
                return true;
            }
        }
        return false;
    }

    bool convertImageToColorMode(int newC,bool assumeThatOneChannelMeansLuminance=true) {
        if (!image) return false;
        unsigned char* im = ImGuiIE::ConvertColorsTo(newC,image,w,h,c,assumeThatOneChannelMeansLuminance);
        if (!im) return false;
        STBI_FREE(image);
        mustInvalidateTexID = true;
        image = im;c=newC;
        return true;
    }

    bool clipImage(int newW,int newH) {
        if (!image || newW<0 || newH<0) return false;
        ImRect sel = imageSelection;
        if (!isImageSelectionValid(sel)) {
            sel.Min.x = sel.Min.y = 0;
            sel.Max.x = w; sel.Max.y= h;
        }
        unsigned char* im = ImGuiIE::ClipImage(newW,newH,image,w,h,c,&sel);
        if (!im) return false;
        pushImage();
        STBI_FREE(image);
        imageSelection = sel;
        mustInvalidateTexID = true;
        image = im;w=newW;h=newH;
        ImFormatImageDimString(imageDimString,64,w,h);
        return true;
    }

    bool resizeImage(int newW,int newH,ImGuiIE::ResizeFilter filter) {
        if (!image) return false;
        void* lastArg = NULL;
#       ifdef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
        lastArg = (void*)&myStbirData;
#       endif //STBIR_INCLUDE_STB_IMAGE_RESIZE_H
        unsigned char* im = ImGuiIE::ResizeImage(newW,newH,image,w,h,c,filter,lastArg);
        if (!im) return false;
        pushImage();
        STBI_FREE(image);
        mustInvalidateTexID = true;
        image = im;w=newW;h=newH;
        ImFormatImageDimString(imageDimString,64,w,h);
        return true;
    }

    bool pasteImage(ImGuiIE::ResizeFilter filter) {
        if (!isImageSelectionValid() || !CopiedImage.isValid()) return false;
        unsigned char *cim = NULL,*cim2 = NULL;
        ImGuiIE::StbImageBuffer::ScopedData copiedImage(CopiedImage);   // This wrapping allow the pixels to be stored inside CopiedImage in a compressed format
        if (c!=CopiedImage.c) cim = ImGuiIE::ConvertColorsTo(c,copiedImage.getImage(),CopiedImage.w,CopiedImage.h,CopiedImage.c,true);
        ImGuiIE::ImageScopedDeleter scoped(cim);

        // we must scale the image
        const int X = imageSelection.Min.x;
        const int Y = imageSelection.Min.y;
        const int W = imageSelection.Max.x-imageSelection.Min.x;
        const int H = imageSelection.Max.y-imageSelection.Min.y;
        if (W!=CopiedImage.w || H!=CopiedImage.h) {
            void* lastArg = NULL;
#           ifdef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
            lastArg = (void*)&myStbirData;
#           endif //STBIR_INCLUDE_STB_IMAGE_RESIZE_H
            cim2 = ImGuiIE::ResizeImage(W,H,cim ? cim : copiedImage.getImage(),CopiedImage.w,CopiedImage.h,c,filter,lastArg);
        }
        ImGuiIE::ImageScopedDeleter scoped2(cim2);

        pushImage(&imageSelection);
        const bool ok = ImGuiIE::PasteImage(X,Y,image,w,h,c,cim2 ? cim2 : (cim ? cim : (copiedImage.getImage())),W,H);
        if (ok) mustInvalidateTexID = true;
        return ok;
    }

    bool blankSelection(const ImRect& sel,const ImVec4& color=ImVec4(1,1,1,1),bool mustPushImage = true) {
        if (!image || !isImageSelectionValid(sel)) return false;
        if (mustPushImage) pushImage(&sel);
        const int X = sel.Min.x;
        const int Y = sel.Min.y;
        const int W = sel.Max.x-sel.Min.x;
        const int H = sel.Max.y-sel.Min.y;
        unsigned char col[4] = {
            (unsigned char)(color.x*255.f),
            (unsigned char)(color.y*255.f),
            (unsigned char)(color.z*255.f),
            (unsigned char)(color.w*255.f)
            };
        if (c==1) col[0]=col[1]=col[2]=col[3];
        unsigned char* pim = image;
        for (int y=Y;y<Y+H;y++) {
            for (int x=X;x<X+W;x++) {
                pim = &image[(y*w+x)*c];
                for (int i=0;i<c;i++) *pim++ = col[i];
            }
        }
        mustInvalidateTexID = true;        
        return true;
    }

    bool blankSelection(const ImVec4& color=ImVec4(1,1,1,1),bool mustPushImage=true) {return blankSelection(imageSelection,color,mustPushImage);}

    bool extractSelection(const ImRect& sel) {
        if (!image || !isImageSelectionValid(sel)) return false;
        int X = sel.Min.x;
        int Y = sel.Min.y;
        int W = sel.Max.x-sel.Min.x;
        int H = sel.Max.y-sel.Min.y;
        unsigned char* extracted = ImGuiIE::ExtractImage(X,Y,W,H,image,w,h,c);
        if (extracted) {
            pushImage();
            STBI_FREE(image);image=extracted;
            mustInvalidateTexID = true;
            w=W;h=H;
            ImFormatImageDimString(imageDimString,64,w,h);
        }
        return (extracted!=NULL);
    }

    bool extractSelection() {return extractSelection(imageSelection);}

    bool flipImageX(const ImRect* pOptionalImageSelection=NULL) {
        bool ok = false;
        if (image) {
            if (pOptionalImageSelection) {
                if (isImageSelectionValid(*pOptionalImageSelection)) {
                    int dstX = pOptionalImageSelection->Min.x;int dstY = pOptionalImageSelection->Min.y;
                    int dstW = pOptionalImageSelection->Max.x - dstX;int dstH = pOptionalImageSelection->Max.y-dstY;
                    unsigned char* selectedImage = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                    if (selectedImage) {
                        pushImage(pOptionalImageSelection);
                        ImGuiIE::ImageScopedDeleter scoped(selectedImage);
                        ImGuiIE::FlipX(selectedImage,dstW,dstH,c);
                        ImGuiIE::PasteImage(dstX,dstY,image,w,h,c,selectedImage,dstW,dstH);
                        ok = true;
                    }
                }
            }
            else {pushImage();ImGuiIE::FlipX(image,w,h,c);ok = true;}
        }
        if (ok) mustInvalidateTexID = true;
        return ok;
    }

    bool flipImageY(const ImRect* pOptionalImageSelection=NULL) {
        bool ok = false;
        if (image) {
            if (pOptionalImageSelection) {
                if (isImageSelectionValid(*pOptionalImageSelection)) {
                    int dstX = pOptionalImageSelection->Min.x;int dstY = pOptionalImageSelection->Min.y;
                    int dstW = pOptionalImageSelection->Max.x - dstX;int dstH = pOptionalImageSelection->Max.y-dstY;
                    unsigned char* selectedImage = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                    if (selectedImage) {
                        pushImage(pOptionalImageSelection);
                        ImGuiIE::ImageScopedDeleter scoped(selectedImage);
                        ImGuiIE::FlipY(selectedImage,dstW,dstH,c);
                        ImGuiIE::PasteImage(dstX,dstY,image,w,h,c,selectedImage,dstW,dstH);
                        ok = true;
                    }
                }
            }
            else {pushImage();ImGuiIE::FlipY(image,w,h,c);ok = true;}
        }
        if (ok) mustInvalidateTexID = true;
        return ok;
    }

    bool rotateImageCW90Deg(ImRect* pOptionalImageSelection=NULL) {
        if (!image) return false;
        if (pOptionalImageSelection) {
            if (isImageSelectionValid(*pOptionalImageSelection)) {
                int dstX = pOptionalImageSelection->Min.x;int dstY = pOptionalImageSelection->Min.y;
                int dstW = pOptionalImageSelection->Max.x - dstX;int dstH = pOptionalImageSelection->Max.y-dstY;
                unsigned char* selectedImage = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                if (selectedImage) {
                    unsigned char* im=NULL;
                    {
                        ImGuiIE::ImageScopedDeleter scoped(selectedImage);
                        im=ImGuiIE::RotateCW90Deg<unsigned char>(selectedImage,dstW,dstH,c);
                        if (!im) return false;
                    }
                    ImGuiIE::ImageScopedDeleter scoped(im);
                    const int nX = dstX+(dstW-dstH)/2;const int nY = dstY+(dstH-dstW)/2;
                    pushImage(pOptionalImageSelection);
                    ImGuiIE::PasteImage(nX,nY,image,w,h,c,im,dstH,dstW);
                    mustInvalidateTexID = true;
                    // rotate selection:
                    pOptionalImageSelection->Min.x = nX;pOptionalImageSelection->Min.y = nY;
                    pOptionalImageSelection->Max.x = (nX+dstH);pOptionalImageSelection->Max.y = (nY+dstW);
                    clampImageSelection(imageSelection);
                }
            }
        }
        else {
            unsigned char* im=ImGuiIE::RotateCW90Deg<unsigned char>(image,w,h,c);
            if (!im) return false;
            pushImage();
            STBI_FREE(image);
            mustInvalidateTexID = true;
            image = im;int t=w;w=h;h=t;
            ImFormatImageDimString(imageDimString,64,w,h);
        }
        return true;
    }

    bool rotateImageCCW90Deg(ImRect* pOptionalImageSelection=NULL) {
        if (!image) return false;
        if (pOptionalImageSelection) {
            if (isImageSelectionValid(*pOptionalImageSelection)) {
                int dstX = pOptionalImageSelection->Min.x;int dstY = pOptionalImageSelection->Min.y;
                int dstW = pOptionalImageSelection->Max.x - dstX;int dstH = pOptionalImageSelection->Max.y-dstY;
                unsigned char* selectedImage = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                if (selectedImage) {
                    unsigned char* im=NULL;
                    {
                        ImGuiIE::ImageScopedDeleter scoped(selectedImage);
                        im=ImGuiIE::RotateCCW90Deg<unsigned char>(selectedImage,dstW,dstH,c);
                        if (!im) return false;
                    }
                    ImGuiIE::ImageScopedDeleter scoped(im);
                    const int nX = dstX+(dstW-dstH)/2;const int nY = dstY+(dstH-dstW)/2;
                    pushImage(pOptionalImageSelection);
                    ImGuiIE::PasteImage(nX,nY,image,w,h,c,im,dstH,dstW);
                    mustInvalidateTexID = true;
                    // rotate selection:
                    pOptionalImageSelection->Min.x = nX;pOptionalImageSelection->Min.y = nY;
                    pOptionalImageSelection->Max.x = (nX+dstH);pOptionalImageSelection->Max.y = (nY+dstW);
                    clampImageSelection(imageSelection);
                }
            }
        }
        else {
            unsigned char* im=ImGuiIE::RotateCCW90Deg<unsigned char>(image,w,h,c);
            if (!im) return false;
            pushImage();
            STBI_FREE(image);
            mustInvalidateTexID = true;
            image = im;int t=w;w=h;h=t;
            ImFormatImageDimString(imageDimString,64,w,h);
        }
        return true;
    }

    bool shiftImageX(int offset, bool wrapMode,ImRect* pOptionalImageSelection=NULL,int shiftSelectionMode=2) {
        bool ok = false;
        if (image && offset!=0) {
            offset%=w;
            if (pOptionalImageSelection)    {
                if (isImageSelectionValid(*pOptionalImageSelection))    {
                    int dstX = pOptionalImageSelection->Min.x;int dstY = pOptionalImageSelection->Min.y;
                    int dstW = pOptionalImageSelection->Max.x - dstX;int dstH = pOptionalImageSelection->Max.y-dstY;
                    ImRect finalSelection(pOptionalImageSelection->Min,pOptionalImageSelection->Max);
                    if (shiftSelectionMode==0) {
                        const int W = finalSelection.Max.x - finalSelection.Min.x;
                        const int H = finalSelection.Max.y - finalSelection.Min.y;
                        int X = finalSelection.Min.x+offset,Y = finalSelection.Min.y;if (wrapMode) X%=w;
                        if (X+W>w) X = w-W;if (Y+H>h) Y = h-H;
                        if (X<0) X=0;if (Y<0) Y=0;
                        finalSelection.Min.x=X;finalSelection.Min.y=Y;
                        finalSelection.Max.x=X+W;finalSelection.Max.y=Y+H;
                    }
                    else {
                        finalSelection.Min.x = (dstX+offset)%w;finalSelection.Min.y = dstY;
                        finalSelection.Max.x = (dstX+dstW+offset)%w;finalSelection.Max.y = dstY+dstH;
                    }
                    if (shiftSelectionMode>0 && shiftSelectionMode<3)   {
                        unsigned char* selectedImage = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                        if (selectedImage) {
                            ImGuiIE::ImageScopedDeleter scoped(selectedImage);
                            // We must paste image to dstX+offset;
                            ImRect totalSelection(pOptionalImageSelection->Min,pOptionalImageSelection->Max);
                            totalSelection.Add(finalSelection);
                            if (wrapMode) {
                                if (totalSelection.Min.x<0) {
                                    totalSelection.Max.x = totalSelection.Max.x > (w-totalSelection.Min.x) ? totalSelection.Max.x : (w-totalSelection.Min.x);
                                    totalSelection.Min.x = 0;
                                }
                                if (totalSelection.Min.y<0) {
                                    totalSelection.Max.y = totalSelection.Max.y > (h-totalSelection.Min.y) ? totalSelection.Max.y : (h-totalSelection.Min.y);
                                    totalSelection.Min.y = 0;
                                }
                                if (totalSelection.Max.x>=w) {
                                    totalSelection.Min.x = totalSelection.Min.x < (w-totalSelection.Max.x) ? totalSelection.Min.x : (w-totalSelection.Max.x);
                                    totalSelection.Max.x = w;
                                }
                                if (totalSelection.Max.y>=h) {
                                    totalSelection.Min.y = totalSelection.Min.y < (h-totalSelection.Max.y) ? totalSelection.Min.y : (h-totalSelection.Max.y);
                                    totalSelection.Max.y = h;
                                }
                            }
                            clampImageSelection(totalSelection);
                            pushImage(&totalSelection);
                            if (shiftSelectionMode==1) blankSelection(*pOptionalImageSelection,ImVec4(1,1,1,1),false);
                            if (wrapMode) {
                                ImGuiIE::ShiftX(-offset,true,image,w,h,c);
                                ImGuiIE::PasteImage(dstX,dstY,image,w,h,c,selectedImage,dstW,dstH);
                                ImGuiIE::ShiftX(offset,true,image,w,h,c);
                            }
                            else {ImGuiIE::PasteImage((dstX+offset)%w,dstY,image,w,h,c,selectedImage,dstW,dstH);}
                            ok = true;
                        }
                    }
                    // increment selection:
                    if (shiftSelectionMode==0) {
                        const int W = finalSelection.Max.x - finalSelection.Min.x;
                        const int H = finalSelection.Max.y - finalSelection.Min.y;
                        int X = finalSelection.Min.x,Y = finalSelection.Min.y;
                        if (X+W>w) X = w-W;if (Y+H>h) Y = h-H;
                        finalSelection.Min.x=X;finalSelection.Min.y=Y;
                        finalSelection.Max.x=X+W;finalSelection.Max.y=Y+H;
                    }
                    clampImageSelection(finalSelection);
                    pOptionalImageSelection->Min = finalSelection.Min;
                    pOptionalImageSelection->Max = finalSelection.Max;
                }
            }
            else {pushImage();ImGuiIE::ShiftX(offset,wrapMode,image,w,h,c);ok = true;}
        }

        if (ok) mustInvalidateTexID = true;
        return ok;
    }

    bool shiftImageY(int offset, bool wrapMode,ImRect* pOptionalImageSelection=NULL,int shiftSelectionMode=2) {
        bool ok = false;
        if (image && offset!=0) {
            offset%=h;
            if (pOptionalImageSelection) {
                if (isImageSelectionValid(*pOptionalImageSelection)) {
                    int dstX = pOptionalImageSelection->Min.x;int dstY = pOptionalImageSelection->Min.y;
                    int dstW = pOptionalImageSelection->Max.x - dstX;int dstH = pOptionalImageSelection->Max.y-dstY;
                    ImRect finalSelection(pOptionalImageSelection->Min,pOptionalImageSelection->Max);
                    if (shiftSelectionMode==0) {
                        const int W = finalSelection.Max.x - finalSelection.Min.x;
                        const int H = finalSelection.Max.y - finalSelection.Min.y;
                        int X = finalSelection.Min.x,Y = finalSelection.Min.y+offset;if (wrapMode) Y%=h;
                        if (X+W>w) X = w-W;if (Y+H>h) Y = h-H;
                        if (X<0) X=0;if (Y<0) Y=0;
                        finalSelection.Min.x=X;finalSelection.Min.y=Y;
                        finalSelection.Max.x=X+W;finalSelection.Max.y=Y+H;
                    }
                    else {
                        finalSelection.Min.x = dstX;finalSelection.Min.y = (dstY+offset)%h;
                        finalSelection.Max.x = dstX+dstW;finalSelection.Max.y = (dstY+dstH+offset)%h;
                    }
                    if (shiftSelectionMode>0 && shiftSelectionMode<3) {
                        unsigned char* selectedImage = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                        if (selectedImage) {
                            ImGuiIE::ImageScopedDeleter scoped(selectedImage);
                            // We must paste image to dstY+offset;
                            ImRect totalSelection(pOptionalImageSelection->Min,pOptionalImageSelection->Max);
                            totalSelection.Add(finalSelection);
                            if (wrapMode) {
                                if (totalSelection.Min.x<0) {
                                    totalSelection.Max.x = totalSelection.Max.x > (w-totalSelection.Min.x) ? totalSelection.Max.x : (w-totalSelection.Min.x);
                                    totalSelection.Min.x = 0;
                                }
                                if (totalSelection.Min.y<0) {
                                    totalSelection.Max.y = totalSelection.Max.y > (h-totalSelection.Min.y) ? totalSelection.Max.y : (h-totalSelection.Min.y);
                                    totalSelection.Min.y = 0;
                                }
                                if (totalSelection.Max.x>=w) {
                                    totalSelection.Min.x = totalSelection.Min.x < (w-totalSelection.Max.x) ? totalSelection.Min.x : (w-totalSelection.Max.x);
                                    totalSelection.Max.x = w;
                                }
                                if (totalSelection.Max.y>=h) {
                                    totalSelection.Min.y = totalSelection.Min.y < (h-totalSelection.Max.y) ? totalSelection.Min.y : (h-totalSelection.Max.y);
                                    totalSelection.Max.y = h;
                                }
                            }
                            clampImageSelection(totalSelection);
                            pushImage(&totalSelection);
                            if (shiftSelectionMode==1) blankSelection(*pOptionalImageSelection,ImVec4(1,1,1,1),false);
                            if (wrapMode) {
                                ImGuiIE::ShiftY(-offset,true,image,w,h,c);
                                ImGuiIE::PasteImage(dstX,dstY,image,w,h,c,selectedImage,dstW,dstH);
                                ImGuiIE::ShiftY(offset,true,image,w,h,c);
                            }
                            else ImGuiIE::PasteImage(dstX,(dstY+offset)%h,image,w,h,c,selectedImage,dstW,dstH);
                            ok = true;
                        }
                    }
                    // increment selection:
                    clampImageSelection(finalSelection);
                    pOptionalImageSelection->Min = finalSelection.Min;
                    pOptionalImageSelection->Max = finalSelection.Max;
                }
            }
            else {pushImage();ImGuiIE::ShiftY(offset,wrapMode,image,w,h,c);ok = true;}
        }
        if (ok) mustInvalidateTexID = true;
        return ok;
    }

    bool invertImageColors(const ImRect* pOptionalImageSelection=NULL,int numberOfColorChannelsToInvertOnRGBAImages=3) {
        bool ok = false;
        if (image) {
            if (pOptionalImageSelection) {
                if (isImageSelectionValid(*pOptionalImageSelection)) {
                    int dstX = pOptionalImageSelection->Min.x;int dstY = pOptionalImageSelection->Min.y;
                    int dstW = pOptionalImageSelection->Max.x - dstX;int dstH = pOptionalImageSelection->Max.y-dstY;
                    unsigned char* selectedImage = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                    if (selectedImage) {
                        ImGuiIE::ImageScopedDeleter scoped(selectedImage);
                        ImGuiIE::InvertColors(selectedImage,dstW,dstH,c,numberOfColorChannelsToInvertOnRGBAImages);
                        pushImage(pOptionalImageSelection);
                        ImGuiIE::PasteImage(dstX,dstY,image,w,h,c,selectedImage,dstW,dstH);
                        ok = true;
                    }
                }
            }
            else {pushImage();ImGuiIE::InvertColors(image,w,h,c,numberOfColorChannelsToInvertOnRGBAImages);ok=true;}
            if (ok) mustInvalidateTexID = true;
        }
        return ok;
    }

    bool applyImageLightEffect(int lightStrength,ImGuiIE::LightEffect lightEffect,const ImRect* pOptionalImageSelection=NULL,bool clampColorComponentsAtAlpha=true) {
        if (c==1) return false;
        bool ok = false;
        if (image) {
            if (pOptionalImageSelection)   {
                if (isImageSelectionValid(*pOptionalImageSelection)) {
                    int dstX = pOptionalImageSelection->Min.x;int dstY = pOptionalImageSelection->Min.y;
                    int dstW = pOptionalImageSelection->Max.x - dstX;int dstH = pOptionalImageSelection->Max.y-dstY;
                    unsigned char* selectedImage = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                    if (selectedImage) {
                        ImGuiIE::ImageScopedDeleter scoped(selectedImage);
                        ImGuiIE::ApplyLightEffect(selectedImage,dstW,dstH,c,lightStrength,lightEffect,clampColorComponentsAtAlpha);
                        pushImage(pOptionalImageSelection);
                        ImGuiIE::PasteImage(dstX,dstY,image,w,h,c,selectedImage,dstW,dstH);
                        ok = true;
                    }
                }
            }
            else {pushImage();ImGuiIE::ApplyLightEffect(image,w,h,c,lightStrength,lightEffect,clampColorComponentsAtAlpha);ok=true;}
            if (ok) mustInvalidateTexID = true;
        }
        return ok;
    }

    bool loadFromMemory(const unsigned char* buffer,int size,bool reloadMode = false,const char* ext=NULL) {
        clear(reloadMode);
        if (!buffer || size<=0) return false;
        IM_ASSERT(!image);        
        if (ext && ((strcmp(ext,".ico")==0) || (strcmp(ext,".cur")==0)))   {
#           ifdef TINY_ICO_H
            image = tiny_ico_load_from_memory(buffer,size,&w,&h,&c,0);
#           endif  //TINY_ICO_H
        }
        else if (ext && ((strcmp(ext,".tiff")==0) || (strcmp(ext,".tif")==0)))   {
#           ifdef _TIFF_
            image = ImGuiIE::tiff_load_from_memory((const char*) buffer,size,w,h,c);
#           endif //_TIFF_
        }
        else if (!image) image = stbi_load_from_memory(buffer,size,&w,&h,&c,0);
        if (!image) return false;
        if (c!=1 && c!=3 && c<4) {
            clear(reloadMode);
            image = stbi_load_from_memory(buffer,size,&w,&h,&c,3);
            c=3;
        }
        else if (c>4)	{
            clear(reloadMode);
            image = stbi_load_from_memory(buffer,size,&w,&h,&c,4);
            c=4;
        }
        if (image) {
            ImFormatFileSizeString(fileSizeString,64,size);
            ImFormatImageDimString(imageDimString,64,w,h);
            resizeWidth = w; resizeHeight = h;
        }
        // Note that we don't load "texID" here if "image" is valid.
        // This way we can call loadFromMemory(...)/loadFromFile(...) even without a rendering context.
        return (image!=NULL);
    }

    bool loadFromFile(const char* path,bool updateFilePathsList=true) {
        // Fetch file extension soon, because we could not use stbi_load in loadFromMemory(...):
        char ext[6] = "";
        if (path) {
            const char* dot = strrchr(path,'.');
            const int len = (int) strlen(path);
            if (dot && ((dot-path)==len-4 || (dot-path)==len-5)) {
                strcpy(ext,dot);
                for (int i=0;i<6;i++) ext[i]=tolower(ext[i]);
            }
        }
        if (strcmp(ext,".zip")==0) return false;    // not an image file

        const bool reloadMode = (path==filePath);
        clear(reloadMode);
        ImVector<char> content;
#       if (defined(IMGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
        if (!ImGuiFs::FileGetContent(path,content)) return false;    // supports path inside zip files
#       else // IMGUI_USE_MINIZIP
        if (!ImGuiIE::GetFileContent(path,content)) return false;
#       endif //IMGUI_USE_MINIZIP
        if (content.size()==0) return false;
        const bool ok = loadFromMemory((const unsigned char*)&content[0],content.size(),reloadMode,ext);
        if (ok) {
            if (!reloadMode) assignFilePath(path,updateFilePathsList);
            else if (mustUpdateFileListSoon) updateFileList();
            assignModified(false);
        }
        else clearFilePath();
        if (ok) undoStack.clear();
        return ok;
    }

    bool saveAs(const char* path=NULL,int numChannels=0,bool assumeThatOneChannelMeansLuminance=false) {
        bool rv = false;
        if (!path || path[0]=='\0') path = filePath;
        if (!image || !path) return rv;
#       if (defined(IMGUI_USE_MINIZIP) && defined(IMGUI_FILESYSTEM_H_) && !defined(IMGUIFS_NO_EXTRA_METHODS))
        if (ImGuiFs::PathIsInsideAZipFile(path)) return false;  // Not supported
#       endif // IMGUI_USE_MINIZIP

        // Get Extension and save it
        FileExtensionHelper feh(path);
        if (numChannels==0) {
            // We must fetch the number of channels
            numChannels=c;
            if (strstr(ImGuiIE::SupportedSaveExtensions[numChannels],feh.ext)==NULL) {
                numChannels = 0;
                for (int i=4;i>=1;--i) {
                    if (strstr(ImGuiIE::SupportedSaveExtensions[i],feh.ext)!=NULL) {
                        numChannels = i;break;
                    }
                }
                if (numChannels == 0) return false;
            }
//            // Correct bad numChannels based on feh.ext
//            if (strcmp(feh.ext,".bmp")==0 || strcmp(feh.ext,".jpg")==0 || strcmp(feh.ext,".jpeg")==0) numChannels=3;
//            else if (strcmp(feh.ext,".ico")==0 || strcmp(feh.ext,".cur")==0) numChannels=4;
//            else if (numChannels==1 && (strcmp(feh.ext,".gif")==0)) numChannels=4;
        }
        if (numChannels!=1 && numChannels!=3 && numChannels!=4) return rv;



        if (numChannels!=c && !convertImageToColorMode(numChannels,assumeThatOneChannelMeansLuminance)) return false;
        IM_ASSERT(c==numChannels);

#       ifndef IMGUIIMAGEEDITOR_KEEP_RGB_WHEN_ALPHA_IS_ZERO
        if (c==4) {
            unsigned char* pim = image;
            for (int i=0,isz=w*h;i<isz;i++) {
                if (pim[3]==0)  {*pim++=255;*pim++=255;*pim++=255;++pim;}
                else pim+=4;
            }
        }
#       endif //IMGUIIMAGEEDITOR_KEEP_RGB_WHEN_ALPHA_IS_ZERO

        //fprintf(stderr,"Save as \"%s\" [ext: %s Num Channels: %d]\n",path,feh.ext,c);


        if (strcmp(feh.ext,".png")==0) {
#           ifdef INCLUDE_STB_IMAGE_WRITE_H
            if (!rv) {
                FILE* f = ImFileOpen(path,"wb");
                if (f) {
                    rv = stbi_write_png_to_func(&StbImage::stbi_write_callback,(void*)f,w,h,c,image,w*c)==1;
                    fflush(f);fclose(f);
                }
            }
#           endif // INCLUDE_STB_IMAGE_WRITE_H
#           ifdef LODEPNG_H
            // I had problems putting this branch above stbiw.
            // The saved png was valid (and a bit smaller in size), but then stbi loaded it incorrectly
            // (and actually even lodePng did the same! It worked if we knew the correct number of color channels before loading the png image).
            // In any case fo me correct loading through stb_image is a priority.
            // In any case we can still use lodePng for storing the CopiedImage and the UndoStack (to test).
            if (!rv) {
                unsigned char* png=NULL;size_t pngSize=0;unsigned error = 1;
                error = lodepng_encode_memory(&png,&pngSize, image, w, h,c==4 ? LCT_RGBA : (c==3 ? LCT_RGB : (c==1 ? LCT_GREY : LCT_RGBA)), 8);
                if (!error) rv = ImGuiIE::SetFileContent(path,png,pngSize);
                else rv = false;
                if (png) free(png);
            }
#           endif // LODEPNG_H
        }
        else if (strcmp(feh.ext,".tga")==0) {
#           ifdef INCLUDE_STB_IMAGE_WRITE_H
            if (!rv) {
                FILE* f = ImFileOpen(path,"wb");
                if (f) {
                    rv = stbi_write_tga_to_func(&StbImage::stbi_write_callback,(void*)f,w,h,c,image)==1;
                    fflush(f);fclose(f);
                }
            }
#           endif // INCLUDE_STB_IMAGE_WRITE_H
        }
        else if (strcmp(feh.ext,".bmp")==0) {
            IM_ASSERT(c==3);
#           ifdef INCLUDE_STB_IMAGE_WRITE_H
            if (!rv) {
                FILE* f = ImFileOpen(path,"wb");
                if (f) {
                    rv = stbi_write_bmp_to_func(&StbImage::stbi_write_callback,(void*)f,w,h,c,image)==1;
                    fflush(f);fclose(f);
                }
            }
#           endif // INCLUDE_STB_IMAGE_WRITE_H
        }
        else if (strcmp(feh.ext,".jpg")==0 || strcmp(feh.ext,".jpeg")==0) {
            IM_ASSERT(c==3);
#           ifdef TJE_H
            if (!rv) rv = tje_encode_to_file(path, w, h, c, image);
#           endif //TJE_H
        }
        else if (strcmp(feh.ext,".gif")==0) {
            IM_ASSERT(c==3 || c==4);
#           ifdef JO_INCLUDE_GIF_H
            if (!rv) {
                unsigned char* imageRGBX = (c!=4) ? ImGuiIE::ConvertColorsTo(4,image,w,h,c) : image;
                if (imageRGBX) {
                    jo_gif_t gif = jo_gif_start(path, w, h, 0, 255);
                    jo_gif_frame(&gif, imageRGBX, 4, false,c==4);
                    jo_gif_end(&gif);
                    if (c!=4) STBI_FREE(imageRGBX);
                    rv = ImGuiIE::FileExists(path);
                }
            }
#           endif // JO_INCLUDE_GIF_H
        }
        else if (strcmp(feh.ext,".ico")==0 || strcmp(feh.ext,".cur")==0) {
            IM_ASSERT(c==4);
#           if (defined(TINY_ICO_H) && !defined(TINY_ICO_NO_ENCODER) && defined(INCLUDE_STB_IMAGE_WRITE_H))
            if (!rv) {
                if (w>256 || h>256) resizeImage(256,256,ImGuiIE::RF_NEAREST);
                IM_ASSERT(w<=256 && h<=256);
                int iconSize = 0;
                unsigned char* ico = tiny_ico_save_from_memory(image,w,h,c,&iconSize);
                if (ico) {
                    rv = ImGuiIE::SetFileContent(path,ico,iconSize);
                    STBIW_FREE(ico);ico=NULL;
                }
            }
#           endif //TINY_ICO_H
        }
        else if (strcmp(feh.ext,".tiff")==0 || strcmp(feh.ext,".tif")==0) {
            IM_ASSERT(/*c==3 || */c==4);
#           ifdef _TIFF_
            if (!rv) {
                ImVector<char> outBuf;
                if (ImGuiIE::tiff_save_to_memory(image,w,h,c,outBuf)) {
                    rv = ImGuiIE::SetFileContent(path,(const unsigned char*)&outBuf[0],outBuf.size());
                }
            }
#           endif //_TIFF_
        }

        if (rv) {
            if (strstr(ImGuiIE::SupportedLoadExtensions,feh.ext)!=NULL) loadFromFile(path); // Better reload, so that we can see the real saved quality soon.
            else {
                if (path!=filePath) assignFilePath(path);
                assignModified(false);
            }
        }
        else fprintf(stderr,"Error: cannot save file: \"%s\"\n",path);
        return rv;
    }

    struct MyRenderStruct {
        ImVec4 CheckButtonColor;
        ImVec4 CheckButtonHoveredColor;
        ImVec4 CheckButtonActiveColor;
        bool leftPanelHovered,centralPanelHovered,rightPanelHovered,anyPanelHovered;
        bool mustUndo,mustRedo,mustSave;

        MyRenderStruct() {
            //const ImGui::ImageEditor::Style& ies(ImGui::ImageEditor::Style::Get());
            const ImGuiStyle& style(ImGui::GetStyle());
            CheckButtonColor = style.Colors[ImGuiCol_Button];CheckButtonColor.w*=0.5f;
            CheckButtonHoveredColor = style.Colors[ImGuiCol_ButtonHovered];CheckButtonHoveredColor.w*=0.5f;
            CheckButtonActiveColor = style.Colors[ImGuiCol_ButtonActive];CheckButtonActiveColor.w*=0.5f;
            leftPanelHovered=centralPanelHovered=rightPanelHovered=anyPanelHovered=false;
            mustUndo=mustRedo=mustSave=false;
        }
    };

    void render(const ImVec2& size) {
        bool rv = false;
        ImGuiWindow* window = GetCurrentWindow();
        if (!window || window->SkipItems) return;

        if (modifyImageTopToolbar.getNumButtons()==0) initToolbars();   // This should call InitSupportTextures() if necessary

        if (!texID || mustInvalidateTexID) {
            if (mustInvalidateTexID) {
                if (texID) {
                    IM_ASSERT(ImGui::ImageEditor::FreeTextureCb);   // Please call ImGui::ImageEditor::SetFreeTextureCallback(...) at InitGL() time.
                    ImGui::ImageEditor::FreeTextureCb(texID);texID=NULL;
                }
                mustInvalidateTexID = false;
            }
            if (image) {
                IM_ASSERT(ImGui::ImageEditor::GenerateOrUpdateTextureCb);   // Please call ImGui::ImageEditor::SetGenerateOrUpdateTextureCallback(...) at InitGL() time.
                StbImage::GenerateOrUpdateTextureCb(texID,w,h,c,image,true,false,false,true,true);
                if (!texID) return;
            }
        }

        ImGuiContext& g = *GImGui;
        const ImGuiIO& io = ImGui::GetIO();
        //ImGuiStyle& style = ImGui::GetStyle();

        ImVec2 labelSize(0,0);
        {
            // Set the correct font scale (2 lines)
            g.FontBaseSize = io.FontGlobalScale * g.Font->Scale * g.Font->FontSize*2;
            g.FontSize = window->CalcFontSize();

            static const char* typeNames[5]={"NONE","A","LA","RGB","RGBA"};
            static const char* noImageString = "NO IMAGE";
            const char* displayName = filePathName?filePathName:noImageString;
            IM_ASSERT(c>=0 && c<=4);
            labelSize.x = ImGui::CalcTextSize(displayName).x+ImGui::CalcTextSize("\t").x+ImGui::CalcTextSize(typeNames[c]).x+(modified?ImGui::CalcTextSize("*").x:0.f);
            labelSize.y = g.FontSize;
            if (labelSize.x<window->Size.x) ImGui::SetCursorPosX((window->Size.x-labelSize.x)*0.5f);
            else {
                // Reset the font scale (2 lines)
                g.FontBaseSize = io.FontGlobalScale * g.Font->Scale * g.Font->FontSize;
                g.FontSize = window->CalcFontSize();
                labelSize*=0.5f; // We don't calculate it again...
                if (labelSize.x<window->Size.x) ImGui::SetCursorPosX((window->Size.x-labelSize.x)*0.5f);
                else ImGui::SetCursorPosX(0);
            }
            ImGui::Text("%s%s\t%s",displayName,modified?"*":"",typeNames[c]);

            // Reset the font scale (2 lines)
            g.FontBaseSize = io.FontGlobalScale * g.Font->Scale * g.Font->FontSize;
            g.FontSize = window->CalcFontSize();

            if (texID && image)  {
                if (imageDimString[0]!='\0' || fileSizeString[0]!='\0') {
                    labelSize.x = ImGui::CalcTextSize((imageDimString[0]!='\0' && fileSizeString[0]!='\0') ? "(\t)" : "()").x+ImGui::CalcTextSize(imageDimString).x+ImGui::CalcTextSize(fileSizeString).x;
                    labelSize.y+=g.FontSize;
                    if (labelSize.x<window->Size.x) ImGui::SetCursorPosX((window->Size.x-labelSize.x)*0.5f);
                    else ImGui::SetCursorPosX(0);
                    ImGui::Text("(%s",imageDimString);
                    if (imageDimString[0]!='\0' && fileSizeString[0]!='\0') {ImGui::SameLine(0,0);ImGui::Text("%s","\t");}
                    ImGui::SameLine(0,0);
                    ImGui::Text("%s)",fileSizeString);
                }
            }
            else {
                labelSize.x = ImGui::CalcTextSize(noImageString).x;
                labelSize.y+=g.FontSize;
                if (labelSize.x<window->Size.x) ImGui::SetCursorPosX((window->Size.x-labelSize.x)*0.5f);
                else ImGui::SetCursorPosX(0);
                ImGui::Text("%s",noImageString);
            }

            //ImGui::Separator();
            //labelSize.y+=1.0f;
        }

        //const ImGuiWindowFlags showBorders = (window->Flags&ImGuiWindowFlags_ShowBorders);
        //ImGui::PushStyleColor(ImGuiCol_ChildWindowBg,ImVec4(1.f,1.f,1.f,0.25f));
        if (ImGui::BeginChild("ImageEditorChildWindow",size.y>0 ? ImVec2(size.x,size.y-labelSize.y) : size,false,ImGuiWindowFlags_NoScrollbar))  {
            window = GetCurrentWindow();

            const ImGui::ImageEditor::Style& ies = ImGui::ImageEditor::Style::Get();
#           ifndef IMGUITABWINDOW_H_
            const float splitterSize = ies.splitterSize>0 ? ies.splitterSize: 8;
            ImVec4 splitterColor = ies.splitterColor.x<0 ? ImVec4(1,1,1,1) : ies.splitterColor;
#           else //IMGUITABWINDOW_H_
            const ImGui::TabLabelStyle& ts = ImGui::TabLabelStyle::Get();
            const float splitterSize = ies.splitterSize>0 ? ies.splitterSize: ts.tabWindowSplitterSize;
            ImVec4 splitterColor           = ies.splitterColor.x<0 ? ts.tabWindowSplitterColor : ies.splitterColor;
#           endif //IMGUITABWINDOW_H_
            ImVec4 splitterColorHovered = splitterColor;
            ImVec4 splitterColorActive  = splitterColor;
            splitterColor.w *= 0.4f;
            splitterColorHovered.w *= 0.55f;
            splitterColorActive.w *= 0.7f;
            const float minSplitSize = 10;  // If size is smaller, the panel won't be displayed

            bool isASplitterActive = false;
            ImVec2 ws = window->Size;
            float splitterPercToPixels = 0.f,splitterDelta = 0.f;
            if (ws.x>2.*splitterSize+minSplitSize && ws.y>minSplitSize) {
                MyRenderStruct mrs;

                ws.x-=2.*splitterSize;
                splitterPercToPixels = ws.x*slidersFraction.x;
                if (splitterPercToPixels>minSplitSize)  {
                    if (ImGui::BeginChild("ImageEditorChildWindowPanelLeft",ImVec2(splitterPercToPixels,ws.y),true))    {
                        renderLeftPanel(mrs);
                        mrs.leftPanelHovered = ImGui::IsWindowHovered() && (ImGui::IsWindowFocused() || !io.WantTextInput);
                        mrs.anyPanelHovered|=mrs.leftPanelHovered;
                    }
                    ImGui::EndChild();  //"ImageEditorChildWindowPanelLeft"
                }
                else ImGui::SameLine(0,minSplitSize);
                // Vertical Splitter ------------------------------------------
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
                ImGui::PushStyleColor(ImGuiCol_Button,splitterColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,splitterColorHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,splitterColorActive);
                ImGui::PushID(this);
                ImGui::SameLine(0,0);

                ImGui::Button("##splitter1", ImVec2(splitterSize,ws.y));
                bool splitter1Active = ImGui::IsItemActive();
                if (splitter1Active || ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

                isASplitterActive |= splitter1Active;
                if (splitter1Active)  splitterDelta = io.MouseDelta.x;
                else splitterDelta = 0.f;
                if (splitter1Active)  {
                    float& w = splitterPercToPixels;
                    const float minw = splitterSize;
                    const float maxw = ws.x*(slidersFraction.x+slidersFraction.y)-splitterSize;
                    if (w + splitterDelta>maxw)         splitterDelta = (w!=maxw) ? (maxw-w) : 0.f;
                    else if (w + splitterDelta<minw)    splitterDelta = (w!=minw) ? (minw-w) : 0.f;
                    w+=splitterDelta;
                    slidersFraction.x = splitterPercToPixels/ws.x;
                    if (slidersFraction.x<0) slidersFraction.x=0;

                    splitterPercToPixels = ws.x*slidersFraction.y;
                    splitterPercToPixels-=splitterDelta;
                    slidersFraction.y = splitterPercToPixels/ws.x;
                    if (slidersFraction.y<0) {slidersFraction.x-=slidersFraction.y;slidersFraction.y=0;}
                }
                ImGui::SameLine(0,0);
                ImGui::PopID();
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();
                //------------------------------------------------------
                splitterPercToPixels = ws.x*slidersFraction.y;
                if (splitterPercToPixels>2.f*splitterSize) {
                    if (ImGui::BeginChild("ImageEditorChildWindowPanelCenter",ImVec2(splitterPercToPixels,ws.y),false,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse)) {
                        rv|=renderImage(ImVec2(0,0),mrs);
                        mrs.centralPanelHovered = ImGui::IsWindowHovered() && (ImGui::IsWindowFocused() || !io.WantTextInput);
                        mrs.anyPanelHovered|=mrs.centralPanelHovered;
                    }
                    ImGui::EndChild();  //"ImageEditorChildWindowPanelCenter"
                }
                // Vertical Splitter ------------------------------------------
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
                ImGui::PushStyleColor(ImGuiCol_Button,splitterColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,splitterColorHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,splitterColorActive);
                ImGui::PushID(this);
                ImGui::SameLine(0,0);

                ImGui::Button("##splitter2", ImVec2(splitterSize,ws.y));
                bool splitter2Active = ImGui::IsItemActive();
                if (splitter2Active || ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

                isASplitterActive |= splitter2Active;
                if (splitter2Active)  splitterDelta = io.MouseDelta.x;
                else splitterDelta = 0.f;
                if (splitter2Active)  {
                    float& w = splitterPercToPixels;
                    const float minw = splitterSize;
                    const float maxw = ws.x*(1.f-slidersFraction.x)-splitterSize;
                    if (w + splitterDelta>maxw)         splitterDelta = (w!=maxw) ? (maxw-w) : 0.f;
                    else if (w + splitterDelta<minw)    splitterDelta = (w!=minw) ? (minw-w) : 0.f;
                    w+=splitterDelta;
                    slidersFraction.y = splitterPercToPixels/ws.x;
                    if (slidersFraction.y<0) slidersFraction.y=0;
                }
                ImGui::SameLine(0,0);
                ImGui::PopID();
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();
                //------------------------------------------------------
                splitterPercToPixels = ws.x*(1.f-slidersFraction.x-slidersFraction.y);
                if (splitterPercToPixels>2.f*splitterSize) {
                    if (ImGui::BeginChild("ImageEditorChildWindowPanelRight",ImVec2(splitterPercToPixels,ws.y),true)) {
                        renderRightPanel(mrs);
                        mrs.rightPanelHovered = ImGui::IsWindowHovered() && (ImGui::IsWindowFocused() || !io.WantTextInput);
                        mrs.anyPanelHovered|=mrs.rightPanelHovered;
                    }
                    ImGui::EndChild();
                }

                if (mrs.anyPanelHovered && io.KeyCtrl)  {
                    if (ImGui::IsKeyPressed(ies.keySave,false)) mrs.mustSave=true;
                    const bool zPressed = ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Z],false);
                    if ((io.KeyShift && zPressed) || ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Y],false)) mrs.mustRedo=true;
                    else if (zPressed) mrs.mustUndo=true;

                }

                if (mrs.mustSave && image) {
                    if (modified)   {
                        if(fileExtCanBeSaved) saveAs();
                    }
                }
                if (mrs.mustUndo) undo();
                else if (mrs.mustRedo) redo();
            }

        }
        ImGui::EndChild(); // "ImageEditorChildWindow"
        //ImGui::PopStyleColor();



    }

    void renderLeftPanel(MyRenderStruct& mrs) {
        const ImGui::ImageEditor::Style& ies(ImGui::ImageEditor::Style::Get());
        const ImGuiStyle& style(ImGui::GetStyle());

#       ifdef IMGUI_FILESYSTEM_H_
        if (filePathsIndex>=0 && filePathsIndex<filePaths.size() && filePaths.size()>1)   {
            if (ImGui::TreeNodeEx("Browse Directory:",ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
                ImGui::TreePop();
                ImGui::PushID("Browse Directory Group");
                bool mustLoadNewImage = false;
                static ImVec2 dlgPos(0,0);
                static int dlgFrames = 0;
                static const ImGui::StbImage* instance = NULL;   // Probably just an optimization...
                if (ImGui::Button(ies.arrowsChars[0])) {
                    newFilePathsIndex = filePathsIndex;
                    --newFilePathsIndex;if (newFilePathsIndex<0) newFilePathsIndex=filePaths.size()-1;
                    mustLoadNewImage = true;
                }
                ImGui::SameLine();
                ImGui::Text("%d/%d",filePathsIndex+1,filePaths.size());
                ImGui::SameLine();
                if (ImGui::Button(ies.arrowsChars[1])) {
                    newFilePathsIndex = filePathsIndex;
                    ++newFilePathsIndex;if (newFilePathsIndex>=filePaths.size()) newFilePathsIndex=0;
                    mustLoadNewImage = true;
                }
                if (modified) {
                    //  Save Dialog
                    if (mustLoadNewImage) {
                        ImGui::OpenPopup("###ImGuiImageEditor_SaveChangesDialog");
                        dlgFrames = 2;
                        instance = this;

                        ImVec2 dlgSize = ImGui::CalcTextSize("The current file has been modified.");
                        dlgSize.y*= 6.f;   // approximated...
                        dlgPos = ImGui::GetMousePos()-dlgSize*0.5f;
                        const ImGuiIO& io = ImGui::GetIO();
                        if (dlgPos.x+dlgSize.x>io.DisplaySize.x) dlgPos.x = io.DisplaySize.x - dlgSize.x;
                        if (dlgPos.x<0) dlgPos.x = 0;
                        if (dlgPos.y+dlgSize.y>io.DisplaySize.y) dlgPos.y = io.DisplaySize.y - dlgSize.y;
                        if (dlgPos.y<0) dlgPos.y = 0;

                    }
                    if (instance==this) {
                        if (dlgFrames>0) ImGui::SetNextWindowPos(dlgPos);
                        bool open = true;
                        const char* savePath = NULL;
                        bool mustSaveSavePath = false;
                        bool mustSkipSavingSavePath = false;
                        bool onDialogClosing = false;

                        if (ImGui::BeginPopupModal("Save Current Changes###ImGuiImageEditor_SaveChangesDialog",&open,ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
                            if (dlgFrames>0) --dlgFrames;

                            ImGui::Text("The current file has been modified.\n");
                            ImGui::Spacing();
                            ImGui::Spacing();
                            ImGui::Text("Do you want to save it ?\n");
                            ImGui::Spacing();
                            float lastButtonsWidth = 0;
                            if (fileExtCanBeSaved) lastButtonsWidth = ImGui::CalcTextSize("Save Don't Save Cancel").x+3.0f*(style.FramePadding.x+style.ItemSpacing.x)+style.WindowPadding.x;
                            else lastButtonsWidth = ImGui::CalcTextSize("Don't Save Cancel").x+2.0f*(style.FramePadding.x+style.ItemSpacing.x)+style.WindowPadding.x;
                            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()-lastButtonsWidth);
                            if (fileExtCanBeSaved) {if (ImGui::Button("Save##dlg1")) { onDialogClosing = mustSaveSavePath = true;}ImGui::SameLine();}
                            if (ImGui::Button("Don't Save##dlg1")) {onDialogClosing = mustSkipSavingSavePath = true;}
                            ImGui::SameLine();
                            if (ImGui::Button("Cancel##dlg1")) onDialogClosing = true;
                            ImGui::PopItemWidth();

                            if (onDialogClosing) ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                        }
                        if (!open) onDialogClosing = true;

                        if (onDialogClosing)    {
                            instance = NULL;    // MANDATORY!
                            //fprintf(stderr,"mustSaveSavePath=%s mustSkipSavingSavePath=%s\n",mustSaveSavePath?"true":"false",mustSkipSavingSavePath?"true":"false");

                            if (mustSaveSavePath) {
                                //fprintf(stderr,"Save image as \"%s\"\n",savePath[0]=='\0' ? filePath:savePath);
                                saveAs(savePath);
                            }

                            if (mustSaveSavePath || mustSkipSavingSavePath) {
                                filePathsIndex = newFilePathsIndex;
                                loadFromFile(filePaths[filePathsIndex],false);
                            }
                        }
                    }
                }
                else if (mustLoadNewImage)   {
                    // load the new image
                    filePathsIndex = newFilePathsIndex;
                    loadFromFile(filePaths[filePathsIndex],false);
                }
                ImGui::PopID();
            }
        }
#       endif //IMGUI_FILESYSTEM_H_
        if (image && ImGui::TreeNodeEx("Image Size:",ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
            ImGui::TreePop();
            ImGui::PushID("Image Size Group");
            if (ImGui::InputInt("Width",&resizeWidth,1,10)) {
                if (resizeWidth<1) resizeWidth=1;
                if (resizeKeepsProportions && w!=0) resizeHeight = resizeWidth * h/w;
            }
            if (ImGui::InputInt("Height",&resizeHeight,1,10)) {
                if (resizeHeight<1) resizeHeight=1;
                if (resizeKeepsProportions && h!=0) resizeWidth = resizeHeight * w/h;
            }
            // These is a check button
            bool tmp = resizeKeepsProportions;
            if (tmp) {
                ImGui::PushStyleColor(ImGuiCol_Button,mrs.CheckButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mrs.CheckButtonHoveredColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,mrs.CheckButtonActiveColor);
            }
            if (ImGui::SmallButton("Keep Proportions")) {
                resizeKeepsProportions = !resizeKeepsProportions;
                    if (resizeKeepsProportions && w!=0 && h!=0) {
                    if (resizeWidth<=resizeHeight) resizeWidth = resizeHeight * w/h;
                    else resizeHeight = resizeWidth * h/w;
                }
            }
            if (tmp) ImGui::PopStyleColor(3);

            ImGui::Combo("Resize Filter",&resizeFilter,ImGuiIE::GetResizeFilterNames(),ImGuiIE::RF_COUNT);
#           ifdef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
            if (resizeFilter==(int)ImGuiIE::RF_BEST)    {
                if (ImGui::TreeNode("Best Filter Control")) {
                    ImGui::PushItemWidth(ImGui::GetWindowWidth()*0.475f);
                    ImGui::Combo("Color Space",&myStbirData.color_space,ImGuiIE::stbir_data_struct::GetColorSpaceNames(),2);
                    ImGui::Combo("Edge Mode",&myStbirData.edge_mode,ImGuiIE::stbir_data_struct::GetEdgeModeNames(),4,4);
                    ImGui::Combo("Filter",&myStbirData.filter,ImGuiIE::stbir_data_struct::GetFilterNames(),6,6);
                    ImGui::PopItemWidth();
                    if (!myStbirData.isDefault() && ImGui::SmallButton("Reset")) myStbirData.setToDefault();
                    ImGui::TreePop();
                }
            }
#           endif //STBIR_INCLUDE_STB_IMAGE_RESIZE_H

            if (resizeWidth>0 && resizeHeight>0)    {
                if (ImGui::Button("Clip") && clipImage(resizeWidth,resizeHeight)) {assignModified(true);}
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Set new size without\nresizing the image");
                ImGui::SameLine();
                if (ImGui::Button("Resize") && resizeImage(resizeWidth,resizeHeight,(ImGuiIE::ResizeFilter)resizeFilter)) {assignModified(true);}
            }
            ImGui::PopID();
        }
        if (image && ImGui::TreeNodeEx("Shift/Flip/Rotate Image:",ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
            ImGui::TreePop();
            ImGui::PushID("Shift Group");

            ImGui::BeginGroup();    // Parent
            ImGui::PushItemWidth(100);
            ImGui::InputInt("Shift Pixels",&shiftPixels);
            ImGui::PopItemWidth();

            const bool hasImageSelection = isImageSelectionValid();
            const float curPosYoffset = ImGui::GetTextLineHeightWithSpacing()*0.5f;
            const float startCurPosY = ImGui::GetCursorPosY();

            ImGui::BeginGroup();

            ImGui::SetCursorPosY(startCurPosY+curPosYoffset);
            if (ImGui::Button(ies.arrowsChars[0])&& shiftImageX(-shiftPixels,chbShiftImageWrap,(chbShiftImageSelection && hasImageSelection) ? &imageSelection : NULL,shiftImageSelectionMode)) {if (shiftImageSelectionMode!=0) assignModified(true);}

            ImGui::EndGroup();
            ImGui::SameLine(0,6);
            ImGui::BeginGroup();

            if (ImGui::Button(ies.arrowsChars[2]) && shiftImageY(-shiftPixels,chbShiftImageWrap,(chbShiftImageSelection && hasImageSelection) ? &imageSelection : NULL,shiftImageSelectionMode)) {if (shiftImageSelectionMode!=0) assignModified(true);}
            if (ImGui::Button(ies.arrowsChars[3]) && shiftImageY(shiftPixels,chbShiftImageWrap,(chbShiftImageSelection && hasImageSelection) ? &imageSelection : NULL,shiftImageSelectionMode)) {if (shiftImageSelectionMode!=0) assignModified(true);}

            ImGui::EndGroup();
            ImGui::SameLine(0,6);
            ImGui::BeginGroup();

            ImGui::SetCursorPosY(startCurPosY+curPosYoffset);
            if (ImGui::Button(ies.arrowsChars[1]) && shiftImageX(shiftPixels,chbShiftImageWrap,(chbShiftImageSelection && hasImageSelection) ? &imageSelection : NULL,shiftImageSelectionMode)) {if (shiftImageSelectionMode!=0) assignModified(true);}

            ImGui::EndGroup();
            ImGui::SameLine(0,8);
            ImGui::BeginGroup();
            {
                // These are check buttons
                {
                    bool tmp = chbShiftImageWrap;
                    if (tmp) {
                        ImGui::PushStyleColor(ImGuiCol_Button,mrs.CheckButtonColor);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mrs.CheckButtonHoveredColor);
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,mrs.CheckButtonActiveColor);
                    }
                    if (ImGui::SmallButton("Wrap")) chbShiftImageWrap=!chbShiftImageWrap;
                    if (tmp) ImGui::PopStyleColor(3);
                }
                if (hasImageSelection)    {
                    ImGui::SameLine();
                    bool tmp = chbShiftImageSelection;
                    if (tmp) {
                        ImGui::PushStyleColor(ImGuiCol_Button,
                                              //ImGuiIE::GetPulsingColorFrom(
                                              mrs.CheckButtonColor
                                              //)
                                              );
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mrs.CheckButtonHoveredColor);
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,mrs.CheckButtonActiveColor);
                    }
                    if (ImGui::SmallButton("Selection")) chbShiftImageSelection=!chbShiftImageSelection;
                    if (tmp) ImGui::PopStyleColor(3);
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Limit the Shift/Flip/Rotate\noperations to the selected area");
                    if (chbShiftImageSelection) {
                        ImGui::Spacing();
                        static const char* names[3] = {"Move","Cut","Clone"};
                        ImGui::PushItemWidth(ImGui::GetWindowWidth()*0.25f);
                        ImGui::Combo("Selection Mode",&shiftImageSelectionMode,names,3,3);
                        ImGui::PopItemWidth();
                    }
                }

            }
            ImGui::EndGroup();

            ImGui::BeginGroup();
            if (ImGui::SmallButton("FlipX") && flipImageX((chbShiftImageSelection && hasImageSelection) ? &imageSelection : NULL)) {assignModified(true);}
            if (ImGui::SmallButton("FlipY") && flipImageY((chbShiftImageSelection && hasImageSelection) ? &imageSelection : NULL)) {assignModified(true);}
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            if (ImGui::SmallButton("RotateCCW") && rotateImageCCW90Deg((chbShiftImageSelection && hasImageSelection) ? &imageSelection : NULL)) {assignModified(true);}
            if (ImGui::SmallButton("RotateCW") && rotateImageCW90Deg((chbShiftImageSelection && hasImageSelection) ? &imageSelection : NULL)) {assignModified(true);}
            ImGui::EndGroup();
            ImGui::EndGroup();  // Parent
            isShiftPixelsAreaHovered = ImGui::IsItemHovered();

            ImGui::PopID();
        }
#   ifdef IMGUI_FILESYSTEM_H_
        static const bool canSaveAlpha = ImGuiIE::SupportedSaveExtensions[1][0]!='\0';
        static const bool canSaveRGB = ImGuiIE::SupportedSaveExtensions[3][0]!='\0';
        static const bool canSaveRGBA = ImGuiIE::SupportedSaveExtensions[4][0]!='\0';
        if (image && (canSaveAlpha || canSaveRGB || canSaveRGBA)) {
            if (ImGui::TreeNodeEx("Save As...",ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
                ImGui::TreePop();
                ImGui::PushID("Save As...");

                static int numChannels = 0; // Hope static is OK
                bool anyPressed = false;
                if (canSaveRGBA && !anyPressed) {
                    if (ImGui::Button("Save RGBA Image As...")) {numChannels=4;anyPressed=true;}
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",ImGuiIE::SupportedSaveExtensions[4]);
                }
                if (canSaveRGB && !anyPressed) {
                    if (ImGui::Button("Save RGB Image As...")) {numChannels=3;anyPressed=true;}
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",ImGuiIE::SupportedSaveExtensions[3]);
                }
                if (canSaveAlpha && !anyPressed) {
                    if (ImGui::Button("Save ALPHA Image As...")) {numChannels=1;anyPressed=true;}
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",ImGuiIE::SupportedSaveExtensions[1]);
                }

                const char* path = SaveDialog.saveFileDialog(anyPressed,SaveDialog.getLastDirectory(),filePathName,ImGuiIE::SupportedSaveExtensions[numChannels]);
                // const char* startingFileNameEntry=NULL,const char* fileFilterExtensionString=NULL,const char* windowTitle=NULL,const ImVec2& windowSize=ImVec2(-1,-1),const ImVec2& windowPos=ImVec2(-1,-1),const float windowAlpha=0.875f);
                if (strlen(path)>0) {
                    const int nC = numChannels==0 ? c : numChannels;
                    saveAs(path,nC);
                }

                ImGui::PopID();
            }
        }
#   endif //IMGUI_FILESYSTEM_H_

    }

    void renderRightPanel(MyRenderStruct& mrs) {
        const ImGuiStyle& style = ImGui::GetStyle();

        const bool hasSelection = isImageSelectionValid();
        if (image && ImGui::TreeNodeEx("Modify Image:",ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
            ImGui::TreePop();
            ImGui::PushID("Modify Image Group");
            modifyImageTopToolbar.render();
            const int selectedTopItem = modifyImageTopToolbar.getSelectedButtonIndex();
            IM_ASSERT(selectedTopItem>=0 && selectedTopItem<3);
            ImGui::Separator();
            if (selectedTopItem==0) {
                bool mustDisplaySelectionSizeTooltip = false;
                if (ImGui::TreeNode("Adjust Selection:"))  {
                    if (hasSelection && ImGui::IsItemHovered()) mustDisplaySelectionSizeTooltip = true;
                    ImGui::TreePop();
                    const float step = 1.f;
                    const float stepFast = 2.f;
                    const int precision = 0;
                    const float windowWidth = ImGui::GetWindowWidth();
                    const float itemSize = windowWidth*0.35f;
                    ImGui::BeginGroup();
                    ImGui::PushItemWidth(itemSize);
                    if (ImGui::InputFloat("X0##sel",&imageSelection.Min.x,step,stepFast,precision))    {
                        if (imageSelection.Min.x>=(float)w) imageSelection.Min.x=(float)(w-1);
                        else if (imageSelection.Min.x<0) imageSelection.Min.x=0;
                        if (imageSelection.Min.x>imageSelection.Max.x) imageSelection.Min.x=imageSelection.Max.x;
                    }
                    if (ImGui::InputFloat("Y0##sel",&imageSelection.Min.y,step,stepFast,precision))    {
                        if (imageSelection.Min.y>=(float)h) imageSelection.Min.y=(float)(h-1);
                        else if (imageSelection.Min.y<0) imageSelection.Min.y=0;
                        if (imageSelection.Min.y>imageSelection.Max.y) imageSelection.Min.y=imageSelection.Max.y;
                    }
                    ImGui::PopItemWidth();
                    ImGui::EndGroup();
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::PushItemWidth(itemSize);
                    if (ImGui::InputFloat("X1##sel",&imageSelection.Max.x,step,stepFast,precision))    {
                        if (imageSelection.Max.x>(float)w) imageSelection.Max.x=(float)(w);
                        else if (imageSelection.Max.x<0) imageSelection.Max.x=0;
                        if (imageSelection.Min.x>imageSelection.Max.x) imageSelection.Min.x=imageSelection.Max.x;
                    }
                    if (ImGui::InputFloat("Y1##sel",&imageSelection.Max.y,step,stepFast,precision))    {
                        if (imageSelection.Max.y>(float)h) imageSelection.Max.y=(float)(h);
                        else if (imageSelection.Max.y<0) imageSelection.Max.y=0;
                        if (imageSelection.Min.y>imageSelection.Max.y) imageSelection.Min.y=imageSelection.Max.y;
                    }
                    ImGui::PopItemWidth();
                    ImGui::EndGroup();
                    const float buttonTextWidth = ImGui::CalcTextSize(hasSelection ? "Clear Select All" : "Select All").x+(hasSelection ? 4.f : 2.f)*(style.FramePadding.x+style.ItemSpacing.x)+style.WindowPadding.x;
                    ImGui::Text("%s","");
                    ImGui::SameLine(0,(windowWidth-buttonTextWidth)*0.5f);
                    if (hasSelection)   {
                        if (ImGui::SmallButton("Clear")) {imageSelection.Min.x=imageSelection.Min.y=imageSelection.Max.x=imageSelection.Max.y=0.f;}
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","clear selection");
                        ImGui::SameLine();
                    }
                    if (ImGui::SmallButton("Select All")) {imageSelection.Min.x=imageSelection.Min.y=0.f;imageSelection.Max.x=w;imageSelection.Max.y=h;}
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","select the whole image");
                }
                else if (hasSelection && ImGui::IsItemHovered()) mustDisplaySelectionSizeTooltip = true;
                if (mustDisplaySelectionSizeTooltip) ImGui::SetTooltip("(size: %dx%d)",(int)(imageSelection.Max.x-imageSelection.Min.x),(int)(imageSelection.Max.y-imageSelection.Min.y));

                if (hasSelection && image)   {
                    const int pressedItem = modifyImageBottomToolbar.render(false,CopiedImage.isValid() ? -1 : modifyImageBottomToolbar.getNumButtons()-1);
                    if (pressedItem!=-1)  {
                        if (pressedItem==0)   {
                            // Extract Image
                            if (extractSelection()) {
                                assignModified(true);
                                // Change the file name
                                {
                                const size_t len = strlen(filePath) + 1 + 12; // "Extracted000"
                                const char* filePathExt = strrchr(filePathName,'.');
                                char* p = (char*) ImGui::MemAlloc(len);p[0]='\0';
                                if (filePathExt) {
                                    memcpy((void*)p,(const void*)filePath,(filePathExt-filePath));
                                    p[filePathExt-filePath]='\0';
                                }
                                else strcpy(p,filePath);
                                const size_t baseLen = strlen(p);
                                int num = 1;char numChars[4]="000";
                                while (baseLen>0) {
                                    p[baseLen]='\0';
                                    strcat(p,"Extracted");
                                    sprintf(numChars,"%.3d",num++);
                                    strcat(p,numChars);
                                    if (filePathExt) strcat(p,filePathExt);
                                    if (num>999 || !ImGuiIE::FileExists(p)) break;
                                }
                                // Assign the path we've found-------------------
                                ImStrAllocate(filePath,p);
                                filePathName = ImGetFileNameReference(filePath);
                                ImStrAllocateFileExtension(fileExt,filePathName);
                                //------------------------------------------------
                                ImGui::MemFree(p);
                                mustUpdateFileListSoon = true;
                                }
                            }
                        }
                        else if (pressedItem==2) {
                            // CUT
                            CopiedImage.assign(image,w,h,c,&imageSelection);
                            blankSelection(imageSelection);
                            assignModified(true);
                        }
                        else if (pressedItem==3) {
                            // COPY
                            CopiedImage.assign(image,w,h,c,&imageSelection);
                        }
                        else if (pressedItem==4) {
                            // PASTE
                            if (CopiedImage.isValid() && pasteImage((ImGuiIE::ResizeFilter)resizeFilter)) assignModified(true);

                        }
                        else IM_ASSERT(true);   // we have skipped 1 because it's a separator!
                    }
                }
            }
            else if (selectedTopItem==1) {
                const int step = 1;
                const int stepFast = 20;
                if (c>=3)   {
                    static bool useSlider = false;
                    bool pressed = false;
                    if (useSlider)  pressed = ImGui::SliderInt("Tol.Col##2",&tolColor,0,255,"%.0f");
                    else            pressed = ImGui::InputInt("Tol.Col##1",&tolColor,step,stepFast);
                    if (ImGui::IsItemHovered()) {
                        if (ImGui::IsMouseReleased(1)) useSlider=!useSlider;
                        ImGui::SetTooltip("%s","Fill tolerance RGB\nright-click to\nchange widget");
                    }
                    if (pressed) {
                        if (tolColor<0) tolColor=0;
                        else if (tolColor>255) tolColor=255;
                    }
                }
                if (c!=3)   {
                    static bool useSlider = false;
                    bool pressed = false;
                    if (useSlider)  pressed = ImGui::SliderInt("Tol.Alpha##2",&tolAlpha,0,255,"%.0f");
                    else            pressed = ImGui::InputInt("Tol.Alpha##1",&tolAlpha,step,stepFast);
                    if (ImGui::IsItemHovered()) {
                        if (ImGui::IsMouseReleased(1)) useSlider=!useSlider;
                        ImGui::SetTooltip("%s","Fill tolerance A\nright-click to\nchange widget");
                    }
                    if (pressed) {
                        if (tolAlpha<0) tolAlpha=0;
                        else if (tolAlpha>255) tolAlpha=255;
                    }
                }
            }
            else if (selectedTopItem==2)    {
                if (ImGui::InputInt("Pen Width",&penWidth)) {if (penWidth<1.f) penWidth=1.f;}
            }
            ImGui::Separator();

            // These are check buttons
            bool tmp = chbMirrorX;
            if (tmp) {
                ImGui::PushStyleColor(ImGuiCol_Button,mrs.CheckButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mrs.CheckButtonHoveredColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,mrs.CheckButtonActiveColor);
            }
            if (ImGui::Button("  X  ")) chbMirrorX=!chbMirrorX;
            if (tmp) ImGui::PopStyleColor(3);

            ImGui::SameLine(0,2);

            tmp = chbMirrorY;
            if (tmp) {
                ImGui::PushStyleColor(ImGuiCol_Button,mrs.CheckButtonColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mrs.CheckButtonHoveredColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,mrs.CheckButtonActiveColor);
            }
            if (ImGui::Button("  Y  ")) chbMirrorY=!chbMirrorY;
            if (tmp) ImGui::PopStyleColor(3);

            ImGui::SameLine();ImGui::Text("Mirror");

            if (hasSelection)   {
                ImGui::SameLine();
                tmp = chbSelectionLimit;
                if (tmp) {
                    ImGui::PushStyleColor(ImGuiCol_Button,ImGuiIE::GetPulsingColorFrom(mrs.CheckButtonColor));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mrs.CheckButtonHoveredColor);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,mrs.CheckButtonActiveColor);
                }
                if (ImGui::Button("Selection")) chbSelectionLimit=!chbSelectionLimit;
                if (tmp) ImGui::PopStyleColor(3);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Limit most of the operations\nin this panel to selection");

                //ImGui::SameLine();ImGui::Text("Target");
            }

            ImGui::Separator();
            if (c>=3 && (c!=4 || fileExtHasFullAlpha || penColor.w!=0)) {
                //#       define IMGUIIMAGEEDITOR_NO_COLORCOMBO
#       if (defined(IMGUIVARIOUSCONTROLS_H_) && !defined(IMGUIIMAGEEDITOR_NO_COLORCOMBO))
                static bool useColorCombo = true;
                if (useColorCombo)  ImGui::ColorCombo("Color###PenColorRGB1",&penColor,c!=3,ImGui::GetWindowWidth()*0.85f);
                else ImGui::ColorEdit3("Color###PenColorRGB2",&penColor.x);
                if (ImGui::IsItemHovered()) {
                    if (ImGui::IsMouseReleased(1)) useColorCombo=!useColorCombo;
                    ImGui::SetTooltip("%s","right-click to\nchange widget");
                }
#       else //IMGUIVARIOUSCONTROLS_H_
                ImGui::ColorEdit3("Color###PenColorRGB2",&penColor.x);
#       endif //IMGUIVARIOUSCONTROLS_H_
            }
            if (c!=3 && c!=0)   {
                if (c!=1) {
                    if (!fileExtHasFullAlpha) {
                        bool transparent = penColor.w<0.5f;
                        if (ImGui::Checkbox("Transparent",&transparent)) penColor.w = transparent ? 0.f : 1;
                    }
                    else ImGui::SliderFloat("Alpha###PenColorAlpha",&penColor.w,0,1,"%.3f",1.f);
                }
                else ImGui::SliderFloat("Color###PenColorAlpha",&penColor.w,0,1,"%.3f",1.f);
            }
            else penColor.w=1.f;    // c==3

            ImGui::PopID();
        }
        if (image && ImGui::TreeNodeEx("Adjust Image:",ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
            ImGui::TreePop();
            ImGui::PushID("Adjust Image Group");

            ImGui::PushItemWidth(-1);
            const bool mustOpenBrightnessAndContrast = ImGui::Button("Brightness And Contrast") && image;
            if (ImGui::Button("Invert Image") && image && invertImageColors((chbSelectionLimit && hasSelection) ? &imageSelection : NULL)) {assignModified(true);}
            ImGui::PopItemWidth();

            if (c!=1) {
                ImGui::Text("Icon Light Effect:");
                const double lightEffectStep = chbLightEffectInvert ? -48 : 48;
                if (ImGui::SmallButton("Add Linear") && applyImageLightEffect(lightEffectStep,ImGuiIE::LE_LINEAR,(chbSelectionLimit && hasSelection) ? &imageSelection : NULL)) {assignModified(true);}
                ImGui::SameLine();
                if (ImGui::SmallButton("Add Round")  && applyImageLightEffect(lightEffectStep,ImGuiIE::LE_ROUND,(chbSelectionLimit && hasSelection) ? &imageSelection : NULL)) {assignModified(true);}
                if (ImGui::SmallButton("Add Spheric") && applyImageLightEffect(lightEffectStep,ImGuiIE::LE_SPHERICAL,(chbSelectionLimit && hasSelection) ? &imageSelection : NULL)) {assignModified(true);}
                ImGui::SameLine();
                // These is a check button
                bool tmp = chbLightEffectInvert;
                if (tmp) {
                    ImGui::PushStyleColor(ImGuiCol_Button,mrs.CheckButtonColor);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,mrs.CheckButtonHoveredColor);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,mrs.CheckButtonActiveColor);
                }
                if (ImGui::SmallButton("Inversion")) chbLightEffectInvert=!chbLightEffectInvert;
                if (tmp) ImGui::PopStyleColor(3);

            }

            ImGui::PopID();

            static ImVec2 dlgPos(0,0);
            static ImVec2 dlgSize(0,0);
            static int dlgFrames = 0;
            static const ImGui::StbImage* instance = NULL;   // Probably just an optimization...

            if (mustOpenBrightnessAndContrast) {
                ImGui::OpenPopup("###ImGuiImageEditor_BrightnessAndContrastDialog");
                dlgFrames = 2;
                instance = this;
                const ImGuiIO& io = ImGui::GetIO();

                const bool firstTime = (dlgSize.x==0);
                if (firstTime) {
                    dlgSize = io.DisplaySize*0.35f;   //TODO: Change if needed
                }
                dlgPos = ImGui::GetMousePos()-dlgSize*0.5f;
                if (dlgPos.x+dlgSize.x>io.DisplaySize.x) dlgPos.x = io.DisplaySize.x - dlgSize.x;
                if (dlgPos.x<0) dlgPos.x = 0;
                if (dlgPos.y+dlgSize.y>io.DisplaySize.y) dlgPos.y = io.DisplaySize.y - dlgSize.y;
                if (dlgPos.y<0) dlgPos.y = 0;

                // Make sure we create a new instance of brightnessWindowData here:-------------
                if (brightnessWindowData) {
                    brightnessWindowData->~BrightnessWindowData();
                    ImGui::MemFree(brightnessWindowData);
                    brightnessWindowData = NULL;
                }
                IM_ASSERT(!brightnessWindowData);
                brightnessWindowData = (BrightnessWindowData*) ImGui::MemAlloc(sizeof(BrightnessWindowData));
                IM_PLACEMENT_NEW (brightnessWindowData) BrightnessWindowData(*this);
                // ---------------------------------------------------------------------------------

                if (firstTime) dlgSize.x = -dlgSize.x;  // conventional hack to allow keeping last user window resize
            }

            if (instance==this) {
                if (dlgFrames>0) {
                    ImGui::SetNextWindowPos(dlgPos);
                    if (dlgSize.x<0) ImGui::SetNextWindowSize(ImVec2(-dlgSize.x,dlgSize.y));
                }
                bool open = true;
                bool onDialogClosing = false;
                bool mustApply = false;
                bool mustCancel = false;
                if (ImGui::BeginPopupModal("Brightness And Contrast###ImGuiImageEditor_BrightnessAndContrastDialog",&open,ImGuiWindowFlags_NoCollapse)) {
                    IM_ASSERT(brightnessWindowData);
                    if (dlgFrames>0) --dlgFrames;   // MANDATORY!
                    if (dlgSize.x<0) dlgSize.x = -dlgSize.x;    // Optional (if we need to use this value below)


                    const bool horizontalLayout = (dlgSize.x>=dlgSize.y);
                    ImGui::BeginGroup();
                    ImVec2 imageSize = dlgSize;imageSize.y = (dlgSize.y-4.f*style.WindowPadding.y-style.ItemSpacing.y);
                    if (horizontalLayout) imageSize.x=dlgSize.x*0.5f;
                    else imageSize.y=imageSize.y*0.5f;
                    ImGuiIE::ImageZoomAndPan(brightnessWindowData->getTextureID(),imageSize,brightnessWindowData->getAspectRatio(),(c==4)?CheckersTexID:NULL,&brightnessWindowData->zoom,&brightnessWindowData->zoomCenter);
                    ImGui::EndGroup();

                    if (horizontalLayout) ImGui::SameLine();

                    ImGui::BeginGroup();
                    //ImGui::Text("TODO: \"Brightness And Contrast\" dialog here.\n");
                    bool anyModified = false;
                    ImGui::Text("Brightness:");
                    {
                        bool justModified = false;
                        float& tv = brightnessWindowData->bri;
                        ImGui::PushItemWidth(dlgSize.x*(horizontalLayout? 0.3f : 0.6f));
                        ImGui::SliderFloat("###BRI_SL",&tv,-20,20,"%.0f");
                        ImGui::PopItemWidth();
                        justModified |= ImGuiIE::IsItemJustReleased();
                        ImGui::SameLine(0,2);if (ImGui::Button("-##BRI")) {tv-=1.f;justModified=true;}
                        ImGui::SameLine(0,2);if (ImGui::Button("0##BRI")) {tv=0.f;justModified=true;}
                        ImGui::SameLine(0,2);if (ImGui::Button("+##BRI")) {tv+=1.f;justModified=true;}
                        if (justModified) {
                            if (tv<-20.f) tv = -20.f;
                            else if (tv>20.f) tv = 20.f;
                        }
                        anyModified|=justModified;
                        ImGui::Spacing();
                    }
                    ImGui::Text("Contrast:");
                    {
                        bool justModified = false;
                        float& tv = brightnessWindowData->con;
                        ImGui::PushItemWidth(dlgSize.x*(horizontalLayout? 0.3f : 0.6f));
                        ImGui::SliderFloat("###CON_SL",&tv,-20,20,"%.0f");
                        ImGui::PopItemWidth();
                        justModified |= ImGuiIE::IsItemJustReleased();
                        ImGui::SameLine(0,2);if (ImGui::Button("-##CON")) {tv-=1.f;justModified=true;}
                        ImGui::SameLine(0,2);if (ImGui::Button("0##CON")) {tv=0.f;justModified=true;}
                        ImGui::SameLine(0,2);if (ImGui::Button("+##CON")) {tv+=1.f;justModified=true;}
                        if (justModified) {
                            if (tv<-20.f) tv = -20.f;
                            else if (tv>20.f) tv = 20.f;
                        }
                        anyModified|=justModified;
                        ImGui::Spacing();
                    }
                    ImGui::Text("Saturation:");
                    {
                        bool justModified = false;
                        float& tv = brightnessWindowData->sat;
                        ImGui::PushItemWidth(dlgSize.x*(horizontalLayout? 0.3f : 0.6f));
                        ImGui::SliderFloat("###SAT_SL",&tv,-20,20,"%.0f");
                        ImGui::PopItemWidth();
                        justModified |= ImGuiIE::IsItemJustReleased();
                        ImGui::SameLine(0,2);if (ImGui::Button("-##SAT")) {tv-=1.f;justModified=true;}
                        ImGui::SameLine(0,2);if (ImGui::Button("0##SAT")) {tv=0.f;justModified=true;}
                        ImGui::SameLine(0,2);if (ImGui::Button("+##SAT")) {tv+=1.f;justModified=true;}
                        if (justModified) {
                            if (tv<-20.f) tv = -20.f;
                            else if (tv>20.f) tv = 20.f;
                        }
                        anyModified|=justModified;
                        ImGui::Spacing();
                    }

                    ImGui::Text("Reset All:");
                    if (ImGui::Button("RESET##dlg3")) {
                        brightnessWindowData->resetBCS();
                        anyModified = true;
                    }
                    ImGui::Spacing();

                    if (horizontalLayout) dlgSize.x*=0.5f;

                    if (anyModified) brightnessWindowData->updatePreview();

                    ImGui::Spacing();
                    ImGui::Separator();
                    const float lastButtonsWidth = ImGui::CalcTextSize("Apply Cancel").x+2.0f*(style.FramePadding.x+style.ItemSpacing.x)+style.WindowPadding.x+style.ScrollbarSize;
                    ImGui::Text("%s","");
                    ImGui::SameLine(0,dlgSize.x-lastButtonsWidth);
                    if (ImGui::Button("Apply##dlg3")) {onDialogClosing = mustApply = true;}
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel##dlg3")) {onDialogClosing = mustCancel = true;}
                    ImGui::EndGroup();

                    dlgSize = ImGui::GetWindowSize();   // MANDATORY!
                    if (onDialogClosing) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
                if (!open) onDialogClosing = true;

                if (onDialogClosing)    {
                    instance = NULL;    // MANDATORY!

                    // If necessary apply changes to image here
                    if (mustApply)  {
                        if (brightnessWindowData->applyTo(*this)) {assignModified(true);}
                    }

                    if (brightnessWindowData) {
                        brightnessWindowData->~BrightnessWindowData();
                        ImGui::MemFree(brightnessWindowData);
                        brightnessWindowData = NULL;
                    }
                }
            }
        }
        if (image && ImGui::TreeNodeEx("Image Controls:",ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
            ImGui::TreePop();
            ImGui::PushID("Image Controls Group");

            ImGui::BeginGroup();
            ImGui::Text("Undo:");
            //bool undoRedoHover = false;
            if (ImGui::Button("Undo")) {mrs.mustUndo=true;}
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Ctrl+Z");
            //undoRedoHover|=ImGui::IsItemHovered();
            if (ImGui::Button("Redo")) {mrs.mustRedo=true;}
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Ctrl+Y or\nCtrl+Shift+Z");
            //undoRedoHover|=ImGui::IsItemHovered();
            ImGui::EndGroup();

            //if (undoRedoHover) ImGui::SetTooltip("undoStack.cur_stack = %d",undoStack.getStackCur());

            ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::Text("Zoom:");
            if (ImGui::InputFloat("%",&zoom,1.f/16.f,1.f/8.f,2)) {if (zoom<1) zoom=1.f;}
            if (ImGui::Button("Auto")) {zoom = 1.f;zoomCenter=ImVec2(0.5f,0.5f);}
            ImGui::EndGroup();

            ImGui::PopID();
        }
        if (ImGui::TreeNodeEx("Current Image:",ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
            ImGui::TreePop();
            ImGui::PushID("Current Image Group");

            if (image)  {
                if (fileExtCanBeSaved) {
                    bool canDownload = false;
#                   if (defined(__EMSCRIPTEN__) && defined(EMSCRIPTEN_SAVE_SHELL))
                    canDownload = !modified && filePath && filePathName;
#                   endif // EMSCRIPTEN_SAVE_SHELL
                    const bool pressed = canDownload ? ImGui::Button("Download###save2") : ImGui::Button("Save###save1");
                    if (pressed) {
                        if (!canDownload) mrs.mustSave=true;
                        else {
#                           if (defined(__EMSCRIPTEN__) && defined(EMSCRIPTEN_SAVE_SHELL))
                            if (ImGuiIE::FileExists(filePath)) {
                                ImGuiTextBuffer buffer;
                                buffer.append("saveFileFromMemoryFSToDisk(\"%s\",\"%s\")",filePath,filePathName);
                                emscripten_run_script(&buffer.Buf[0]);
                            }
#                           endif // EMSCRIPTEN_SAVE_SHELL
                        }
                    }
                    if (!canDownload && ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Ctrl+S");
                    ImGui::SameLine();
                }
                if (ImGui::Button("Reload")) {loadFromFile(filePath);}
            }
#       ifdef IMGUI_FILESYSTEM_H_
            bool loadNewImage = false;
            if (ImGuiIE::SupportedLoadExtensions[0]!='\0')  {
                ImGui::PushItemWidth(-1);
                loadNewImage = ImGui::Button("Load New Image");
                ImGui::PopItemWidth();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",ImGuiIE::SupportedLoadExtensions);
                const char* path = LoadDialog.chooseFileDialog(loadNewImage,
                                                               //LoadDialog.getLastDirectory(),
                                                               filePath,
                                                               ImGuiIE::SupportedLoadExtensions
                                                               );
                if (strlen(path)>0) loadFromFile(path);
            }
#       endif //IMGUI_FILESYSTEM_H_

            ImGui::PopID();
        }
    }

    inline bool isImageSelectionValid(const ImRect& imageSelection) const {return ((imageSelection.Max.x-imageSelection.Min.x)>0 && (imageSelection.Max.y-imageSelection.Min.y)>0 && imageSelection.Min.x>=0 && imageSelection.Min.y>=0 && imageSelection.Max.x<=w && imageSelection.Max.y<=h);}
    inline bool isImageSelectionValid() const {return isImageSelectionValid(imageSelection);}
    inline void clampImageSelection(ImRect& imageSelection) {
        if (imageSelection.Min.x>=w) imageSelection.Min.x=(w-1);
        else if (imageSelection.Min.x<0) imageSelection.Min.x=0;
        if (imageSelection.Min.y>=h) imageSelection.Min.y=(h-1);
        else if (imageSelection.Min.y<0) imageSelection.Min.y=0;
        if (imageSelection.Max.x<imageSelection.Min.x) imageSelection.Max.x=imageSelection.Min.x;
        else if (imageSelection.Max.x>w) imageSelection.Max.x=w;
        if (imageSelection.Max.y<imageSelection.Min.y) imageSelection.Max.y=imageSelection.Min.y;
        else if (imageSelection.Max.y>h) imageSelection.Max.y=h;
    }
    inline void clampImageSelection() {clampImageSelection(imageSelection);}


    inline ImVec2 mouseToImageCoords(const ImVec2& mousePos,bool checkVisibility=false) const {
        ImVec2 pos(-1,-1);
        if (imageSz.x>0 && imageSz.y>0 && (!checkVisibility || (mousePos.x>=startPos.x && mousePos.x<endPos.x && mousePos.y>=startPos.y && mousePos.y<endPos.y))) {
                // MouseToImage here:-------------------------------
                pos = mousePos-startPos;
                pos.x/=imageSz.x;pos.y/=imageSz.y;  // Note that imageSz is the size of the displayed image in screen coords
                pos.x*=uvExtension.x;pos.y*=uvExtension.y;
                pos.x+=uv0.x;pos.y+=uv0.y;
                pos.x*=w;pos.y*=h;
                // it should be:
                // 0 <= (int)pos.x < w
                // 0 <= (int)pos.y < h
        }
        return pos;
    }

    inline ImVec2 imageToMouseCoords(const ImVec2& imagePos,bool* pIsOutputValidOut=NULL) const {
            ImVec2 pos(-1,-1);
            //if (pIsOutputValidOut) {*pIsOutputValidOut = (imagePos.x>=0 && imagePos.x<w && imagePos.y>=0 && imagePos.y<h) ? true : false;}
            if (pIsOutputValidOut) *pIsOutputValidOut = false;
            if (w>0 && h>0) {
                pos = imagePos;
                pos.x/=w;pos.y/=h;
                pos.x-=uv0.x;pos.y-=uv0.y;
                pos.x/=uvExtension.x;pos.y/=uvExtension.y;
                pos.x*=imageSz.x;pos.y*=imageSz.y;
                pos+=startPos;
                if (pIsOutputValidOut && pos.x>=startPos.x && pos.x<endPos.x &&
                        pos.y>=startPos.y && pos.y<endPos.y)    *pIsOutputValidOut = true;
            }
            return pos;
    }

    inline float getImageToMouseCoordsRatio() const {return imageSz.x/(uvExtension.x*w);}

    inline bool getImageColorAtPixel(int x, int y,ImVec4& cOut) {
        if (!image || x<0 || x>=w || y<0 || y>=h || c<=0 || c==2 || c>4) return false;
        cOut.x=cOut.y=cOut.z=0.f;cOut.w=1.f;
        const unsigned char* pim = &image[(y*w+x)*c];
        if (c==1) cOut.w= (float)(*pim)/255.f;
        else if (c==3) {cOut.x=(float)(*pim++)/255.f;cOut.y=(float)(*pim++)/255.f;cOut.z=(float)(*pim)/255.f;}
        else if (c==4) {cOut.x=(float)(*pim++)/255.f;cOut.y=(float)(*pim++)/255.f;cOut.z=(float)(*pim++)/255.f;cOut.w=(float)(*pim)/255.f;}
        return true;
    }
    inline bool getImageColorAtPixel(const ImVec2& pxl,ImVec4& cOut) {
        const int x = (int) pxl.x; const int y = (int) pxl.y;
        return getImageColorAtPixel(x,y,cOut);
    }
    inline bool getImageColorAtMousePosition(const ImVec2& mousePos,ImVec4& cOut) {
        const ImVec2 pxl = mouseToImageCoords(mousePos);
        return getImageColorAtPixel(pxl,cOut);
    }

    bool renderImage(const ImVec2& size,MyRenderStruct& mrs) {
        if (!texID) return false;
        bool rv = false;
        const ImGuiIO& io = ImGui::GetIO();

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!texID || !window || window->SkipItems || h<=0) return rv;
        ImVec2 curPos = ImGui::GetCursorPos();
        const ImVec2 wndSz(size.x>0 ? size.x : ImGui::GetWindowSize().x-curPos.x,size.y>0 ? size.y : ImGui::GetWindowSize().y-curPos.y);

        IM_ASSERT(wndSz.x!=0 && wndSz.y!=0 && zoom!=0);

        const ImGuiID id = (ImGuiID) ((unsigned long)this)+1;
        ImGui::PushID(id);

        // Here we use the whole size (although it can be partially empty)
        ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + wndSz.x,window->DC.CursorPos.y + wndSz.y));
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, NULL)) {ImGui::PopID();return rv;}

        imageSz = wndSz;
        ImVec2 remainingWndSize(0,0);
        const float aspectRatio=(float)w/(float)h;

        if (aspectRatio!=0) {
            const float wndAspectRatio = wndSz.x/wndSz.y;
            if (aspectRatio >= wndAspectRatio) {imageSz.y = imageSz.x/aspectRatio;remainingWndSize.y = wndSz.y - imageSz.y;}
            else {imageSz.x = imageSz.y*aspectRatio;remainingWndSize.x = wndSz.x - imageSz.x;}
        }

        bool itemHovered = false;//ImGui::IsItemHoveredRect() && ImGui::IsWindowHovered();
        ImGui::ButtonBehavior(bb,id,&itemHovered,NULL);

        static short int isMousePanning = 0;

        bool secondaryKeyPressed = false;   // We use this to pick image color, or to reset zoom if CTRL is pressed
        if (itemHovered) {
            if (io.MouseWheel!=0) {
                // Zoom and Pan
                if (!io.KeyCtrl && !io.KeyShift)
                {
                    //Zoom
                    const float zoomStep = zoomMaxAndZoomStep.y;
                    const float zoomMin = 1.f;
                    const float zoomMax = zoomMaxAndZoomStep.x;
                    if (io.MouseWheel < 0) {zoom/=zoomStep;if (zoom<zoomMin) zoom=zoomMin;}
                    else {zoom*=zoomStep;if (zoom>zoomMax) zoom=zoomMax;}
                    rv = true;
                }
                else if (io.KeyCtrl)  { // Scroll down
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
                else if (io.KeyShift) {
                    const bool scrollRight = io.MouseWheel <= 0;
                    const float zoomFactor = .5/zoom;
                    if ((!scrollRight && zoomCenter.x > zoomFactor) || (scrollRight && zoomCenter.x <  1.f - zoomFactor))  {
                        const float slideFactor = zoomMaxAndZoomStep.y*0.1f*zoomFactor;
                        if (scrollRight) {
                            zoomCenter.x+=slideFactor;///(imageSz.x*zoom);
                            if (zoomCenter.x >  1.f - zoomFactor) zoomCenter.x =  1.f - zoomFactor;
                        }
                        else {
                            zoomCenter.x-=slideFactor;///(imageSz.x*zoom);
                            if (zoomCenter.x < zoomFactor) zoomCenter.x = zoomFactor;
                        }
                        rv = true;
                    }
                }
            }
            else {
                if (ImGui::IsMouseDragging(1) || ImGui::IsMouseDragging(2))   {
                    // Pan
                    if (zoom!=1.f)  {
                        zoomCenter.x-=io.MouseDelta.x/(imageSz.x*zoom);
                        zoomCenter.y-=io.MouseDelta.y/(imageSz.y*zoom);
                        rv = true;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
                    }
                    isMousePanning = 2;
                }
                else if (isMousePanning>0) --isMousePanning;
                secondaryKeyPressed = !isMousePanning && (io.MouseReleased[1] || io.MouseReleased[2]);
                if (io.KeyCtrl && secondaryKeyPressed) {
                    // Reset zoom
                    zoom=1.f;zoomCenter.x=zoomCenter.y=.5f;rv = true;
                }
            }
        }
        else isMousePanning = 0;

        const float zoomFactor = .5/zoom;
        if (rv) {
            if (zoomCenter.x < zoomFactor) zoomCenter.x = zoomFactor;
            else if (zoomCenter.x > 1.f - zoomFactor) zoomCenter.x = 1.f - zoomFactor;
            if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
            else if (zoomCenter.y > 1.f - zoomFactor) zoomCenter.y = 1.f - zoomFactor;
        }

        uvExtension = ImVec2(2.f*zoomFactor,2.f*zoomFactor);
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

        uv0 = ImVec2((zoomCenter.x-uvExtension.x*.5f),(zoomCenter.y-uvExtension.y*.5f));
        uv1 = ImVec2((zoomCenter.x+uvExtension.x*.5f),(zoomCenter.y+uvExtension.y*.5f));

        startPos=bb.Min;endPos=bb.Max;
        startPos.x+= remainingWndSize.x*.5f;
        startPos.y+= remainingWndSize.y*.5f;
        endPos.x = startPos.x + imageSz.x;
        endPos.y = startPos.y + imageSz.y;

        const ImVec2 CheckersRepeatFactor(24.f,24.f);
        if (c==4) {
            //window->DrawList->AddImage(CheckersTexID, startPos, endPos, uv0*m, uv1*m);
            window->DrawList->AddImage(CheckersTexID, startPos, endPos, ImVec2(0,0), CheckersRepeatFactor);
        }
        window->DrawList->AddImage(texID, startPos, endPos, uv0, uv1);


        static bool mouseSelectionStarted = false;
        static bool penSelectionStarted = false;

        // Pick Color
        if (!io.KeyCtrl && secondaryKeyPressed && !mouseSelectionStarted && !penSelectionStarted) getImageColorAtMousePosition(io.MousePos,penColor);


        const float selectionThickness = zoom*2.f;
        const ImU32 selectionColor=0;
        if (modifyImageTopToolbar.getSelectedButtonIndex()==0) {
            static ImRect mouseSelection;
            static ImRect absSelection;

            if (itemHovered && ImGui::IsMouseDragging(0,1.f))   {
                if (!mouseSelectionStarted) {
                    mouseSelectionStarted=true;
                    mouseSelection.Min=mouseSelection.Max=io.MousePos;
                }
                mouseSelection.Max=io.MousePos;

                absSelection = mouseSelection;
                if (absSelection.Min.x>absSelection.Max.x)  {float tmp = absSelection.Min.x;absSelection.Min.x=absSelection.Max.x;absSelection.Max.x=tmp;}
                if (absSelection.Min.y>absSelection.Max.y)  {float tmp = absSelection.Min.y;absSelection.Min.y=absSelection.Max.y;absSelection.Max.y=tmp;}

                ImGuiIE::DrawSelection(window->DrawList,absSelection,selectionColor,selectionThickness);
            }
            else if (mouseSelectionStarted) {
                mouseSelectionStarted = false;
                mouseSelection = absSelection;

                imageSelection.Min = mouseToImageCoords(mouseSelection.Min);
                imageSelection.Max = mouseToImageCoords(mouseSelection.Max);

                imageSelection.Min.x = floor(imageSelection.Min.x);
                imageSelection.Min.y = floor(imageSelection.Min.y);
                imageSelection.Max.x = ceil(imageSelection.Max.x);
                imageSelection.Max.y = ceil(imageSelection.Max.y);

                if (imageSelection.Min.x<0) imageSelection.Min.x=0;
                else if (imageSelection.Min.x>=w) imageSelection.Min.x=w-1;
                if (imageSelection.Min.y<0) imageSelection.Min.y=0;
                else if (imageSelection.Min.y>=h) imageSelection.Min.y=h-1;

                if (imageSelection.Max.x<imageSelection.Min.x) imageSelection.Max.x=imageSelection.Min.x;
                else if (imageSelection.Max.x>=w) imageSelection.Max.x=w-1;
                if (imageSelection.Max.y<imageSelection.Min.y) imageSelection.Max.y=imageSelection.Min.y;
                else if (imageSelection.Max.y>=h) imageSelection.Max.y=h-1;
            }

            if (!mouseSelectionStarted) {
                if (isImageSelectionValid()) {
                    mouseSelection.Min = imageToMouseCoords(imageSelection.Min);
                    mouseSelection.Max = imageToMouseCoords(imageSelection.Max);

                    ImGuiIE::DrawSelection(window->DrawList,mouseSelection,selectionColor,selectionThickness);
                }
            }
        }
        else {
            if (((chbSelectionLimit && !mrs.leftPanelHovered) || (isShiftPixelsAreaHovered && chbShiftImageSelection)) && isImageSelectionValid()) {
                // Draw selection
                const ImRect mouseSelection(imageToMouseCoords(imageSelection.Min),imageToMouseCoords(imageSelection.Max));

                if (!(isShiftPixelsAreaHovered && chbShiftImageSelection)) {
                    const ImU32 greyOutColor = IM_COL32(0,0,0,ImGuiIE::GetPulsingValueInternal(100,4.f));
                    window->DrawList->AddRectFilled(bb.Min,ImVec2(bb.Max.x,mouseSelection.Min.y),greyOutColor);

                    window->DrawList->AddRectFilled(ImVec2(bb.Min.x,mouseSelection.Min.y),ImVec2(mouseSelection.Min.x,mouseSelection.Max.y),greyOutColor);
                    window->DrawList->AddRectFilled(ImVec2(mouseSelection.Max.x,mouseSelection.Min.y),ImVec2(bb.Max.x,mouseSelection.Max.y),greyOutColor);

                    window->DrawList->AddRectFilled(ImVec2(bb.Min.x,mouseSelection.Max.y),bb.Max,greyOutColor);
                    //ImGuiIE::DrawSelection(window->DrawList,mouseSelection,selectionColor,2.f);
                }
                else ImGuiIE::DrawSelection(window->DrawList,mouseSelection,selectionColor,selectionThickness);
            }

            // DRAW tool
            if (modifyImageTopToolbar.getSelectedButtonIndex()==2) {
                //----------------------------------------------------------------------------------------------------                
                if (itemHovered && ImGui::IsMouseDragging(0,0.f))  {
                    if (!penSelectionStarted) {
                        penSelectionStarted=true;
                        penPointsInImageCoords.resize(0);
                    }

                    // Add current point                    
                    ImVec2 imagePoint = mouseToImageCoords(io.MousePos,false);
                    int numPoints = penPointsInImageCoords.size();                    
                    const bool penWidthIsEven = (((int) penWidth)%2)==0;    // When even, we store the pixel so that (p.x-1,p.y-1) must be filled too
                    int X=0,Y=0,W=w,H=h;
                    const bool mustLimitPenToImageSelection = chbSelectionLimit && isImageSelectionValid(imageSelection);
                    if (mustLimitPenToImageSelection) {
                        X = imageSelection.Min.x;
                        Y = imageSelection.Min.y;
                        W = imageSelection.Max.x-imageSelection.Min.x;
                        H = imageSelection.Max.y-imageSelection.Min.y;
                    }
                    if (imagePoint.x>=X && imagePoint.x<X+W && imagePoint.y>=Y && imagePoint.y<Y+H) {
                        const ImVec2 imagePointFloorBase((int) imagePoint.x,(int)imagePoint.y);
                        const ImVec2 imagePointDeltaBase = imagePoint - imagePointFloorBase;                        
                        for (int i=0;i<4;i++) {
                            ImVec2 imagePointFloor=imagePointFloorBase;
                            ImVec2 imagePointDelta=imagePointDeltaBase;
                            if (i==1) {
                                if (!chbMirrorX && !chbMirrorY) break;
                                if (!chbMirrorX) continue;
                                // MirrorX
                                imagePointFloor.x = (int)((W-1)-(imagePointFloorBase.x-X)+X);
                                imagePointFloor.y = imagePointFloorBase.y;
                                imagePointDelta.x = 1.f-imagePointDeltaBase.x;
                            }
                            else if (i==2) {
                                if (!chbMirrorY) break;
                                // MirrorY
                                imagePointFloor.x = imagePointFloorBase.x;
                                imagePointFloor.y = (int)((H-1)-(imagePointFloorBase.y-Y)+Y);
                                imagePointDelta.y = 1.f-imagePointDeltaBase.y;
                            }
                            else if (i==3) {
                                if (!chbMirrorX || !chbMirrorY) break;
                                // Mirror X and Y
                                imagePointFloor.x = (int)((W-1)-(imagePointFloorBase.x-X)+X);
                                imagePointDelta.x = 1.f-imagePointDeltaBase.x;
                                imagePointFloor.y = (int)((H-1)-(imagePointFloorBase.y-Y)+Y);
                                imagePointDelta.y = 1.f-imagePointDeltaBase.y;
                            }
                            else IM_ASSERT(true);
                            IM_ASSERT(imagePointFloor.x>=X && imagePointFloor.x<=X+W-1);
                            IM_ASSERT(imagePointFloor.y>=Y && imagePointFloor.y<=Y+H-1);

                            if (penWidthIsEven) {
                                if (imagePointDelta.x<=0.5f) {
                                    if (imagePointDelta.y<=0.5f) {imagePoint.x = imagePointFloor.x,imagePoint.y = imagePointFloor.y;}
                                    else {imagePoint.x = imagePointFloor.x,imagePoint.y = imagePointFloor.y+1;}
                                }
                                else {
                                    if (imagePointDelta.y<=0.5f) {imagePoint.x = imagePointFloor.x+1,imagePoint.y = imagePointFloor.y;}
                                    else {imagePoint.x = imagePointFloor.x+1,imagePoint.y = imagePointFloor.y+1;}
                                }
                            }
                            else {imagePoint = imagePointFloor;}
                            // Skip early loop
                            if (i==0 && numPoints>0)    {
                                bool skip = false;
                                for (int i=numPoints-1,iEnd=numPoints-5>0?i-4:0;i>=iEnd;i--) {
                                    if (penPointsInImageCoords[i].x==imagePoint.x && penPointsInImageCoords[i].y==imagePoint.y) {skip=true;break;}
                                }
                                if (skip) break;
                            }
                            // Add imagePoint to penPointsInImageCoords
                            penPointsInImageCoords.push_back(imagePoint);
                            ++numPoints;

                            // Update penSelection
                            const float halfPenWidth = (float)((int) (penWidth*0.5f));
                            const ImVec2 imagePointMin(imagePoint.x-halfPenWidth,imagePoint.y-halfPenWidth);
                            const ImVec2 imagePointMax(imagePointMin.x+penWidth,imagePointMin.y+penWidth);
                            if (numPoints==1) {
                                penSelection.Min=imagePointMin;
                                penSelection.Max=imagePointMax;
                            }
                            else {
                                if (penSelection.Min.x>imagePointMin.x) penSelection.Min.x=imagePointMin.x;
                                if (penSelection.Min.y>imagePointMin.y) penSelection.Min.y=imagePointMin.y;
                                if (penSelection.Max.x<imagePointMax.x) penSelection.Max.x=imagePointMax.x;
                                if (penSelection.Max.y<imagePointMax.y) penSelection.Max.y=imagePointMax.y;
                            }
                            clampImageSelection(penSelection);
                        }
                    }

                    // Display points
                    if (numPoints>0)    {
                        ImVec2 tmp(0,0),tmp2(0,0);
                        const ImVec2* pPenPoints = &penPointsInImageCoords[0];
                        ImU32 penColori = 0;
                        if (c==1)   {
                            const ImU32 alpha = (ImU32)(penColor.w*255.f);
                            penColori = ((255<<IM_COL32_A_SHIFT) |(alpha<<IM_COL32_B_SHIFT) | (alpha<<IM_COL32_G_SHIFT) | (alpha<<IM_COL32_R_SHIFT));
                        }
                        else penColori = ImGui::ColorConvertFloat4ToU32(penColor);
                        const float penThicknessPixel = getImageToMouseCoordsRatio();
                        const float penThicknessTotal = penThicknessPixel*penWidth;
                        const float deltaPenWidth = penThicknessPixel * (int) (penWidth*0.5f);
                        ImRect penSelectionInMouseCoords(imageToMouseCoords(penSelection.Min),imageToMouseCoords(penSelection.Max));
                        penSelectionInMouseCoords.Clip(bb);
                        if (mustLimitPenToImageSelection) {
                            const ImRect imageSelectionInMouseCoords(imageToMouseCoords(imageSelection.Min),imageToMouseCoords(imageSelection.Max));
                            penSelectionInMouseCoords.Clip(imageSelectionInMouseCoords);
                        }
                        window->DrawList->PushClipRect(penSelectionInMouseCoords.Min,penSelectionInMouseCoords.Max,false);
                        const bool mustPaintCheckersBg = (c==4 && penColor.w<1);
                        // We must avoid alpha blending (without changing the blend function)
                        // The best way of doing it is to draw the solid background we use for transparency
                        // under all the pixels we're about to draw
                        // Please note that to be 100% correct we must fetch the EXACT SAME tex coords we have
                        // below the image!
                        const ImVec2 cfactor(CheckersRepeatFactor.x/(endPos.x-startPos.x),CheckersRepeatFactor.y/(endPos.y-startPos.y));
                        ImVec2 cuv0(0,0),cuv1(0,0);
                        const ImVec2 puv0(0.1f,0.1f);
                        const ImVec2 puv1(0.4f,0.4f);
                        for (int i=0;i<numPoints;i++)  {
                            tmp = imageToMouseCoords(*pPenPoints++);
                            tmp.x-= deltaPenWidth; tmp.y-=deltaPenWidth;
                            tmp2.x = tmp.x+penThicknessTotal;tmp2.y = tmp.y+penThicknessTotal;
                            if (tmp2.x>=startPos.x && tmp.x<endPos.x && tmp2.y>=startPos.y && tmp.y<endPos.y)   {
                                if (mustPaintCheckersBg)    {
                                    cuv0.x = (tmp.x-startPos.x)*cfactor.x;
                                    cuv0.y = (tmp.y-startPos.y)*cfactor.y;
                                    cuv1.x = (tmp2.x-startPos.x)*cfactor.x;
                                    cuv1.y = (tmp2.y-startPos.y)*cfactor.y;
                                    window->DrawList->AddImage(CheckersTexID,tmp,tmp2,cuv0,cuv1);
                                    window->DrawList->AddImage(CheckersTexID,tmp,tmp2,puv0,puv1,penColori); // we must reuse the same (checker) texture to display the penColor, to prevent hundreds of texture changes
                                }
                                else window->DrawList->AddRectFilled(tmp,tmp2,penColori);
                            }
                        }
                        window->DrawList->PopClipRect();

                        // Display penSelection (Debug Only)
                        //ImGuiIE::DrawSelection(window->DrawList,penSelectionInMouseCoords,IM_COL32(255,0,0,255),2.f);
                        //ImGui::SetTooltip("%d points\nmin(%d,%d) max(%d,%d)\nimage size(%d,%d)\n mousePos (%1.f,%1.f)",numPoints,(int)penSelection.Min.x,(int)penSelection.Min.y,(int)penSelection.Max.x,(int)penSelection.Max.y,w,h,io.MousePos.x,io.MousePos.y);
                    }                    
                }
                else if (penSelectionStarted) {
                    penSelectionStarted = false;

                    const int numPoints = penPointsInImageCoords.size();

                    if (numPoints>0)    {
                        int X=0,Y=0,W=w,H=h;
                        const bool mustLimitPenToImageSelection = chbSelectionLimit && isImageSelectionValid(imageSelection);
                        if (mustLimitPenToImageSelection) {
                            X = imageSelection.Min.x;
                            Y = imageSelection.Min.y;
                            W = imageSelection.Max.x-imageSelection.Min.x;
                            H = imageSelection.Max.y-imageSelection.Min.y;
                        }

                        // Display points [copied here from above to prevent one frame artifact (it must be exactly the same code)]
                        ImVec2 tmp(0,0),tmp2(0,0);
                        const ImVec2* pPenPoints = &penPointsInImageCoords[0];
                        ImU32 penColori = 0;
                        if (c==1)   {
                            const ImU32 alpha = (ImU32)(penColor.w*255.f);
                            penColori = ((255<<IM_COL32_A_SHIFT) |(alpha<<IM_COL32_B_SHIFT) | (alpha<<IM_COL32_G_SHIFT) | (alpha<<IM_COL32_R_SHIFT));
                        }
                        else penColori = ImGui::ColorConvertFloat4ToU32(penColor);
                        const float penThicknessPixel = getImageToMouseCoordsRatio();
                        const float penThicknessTotal = penThicknessPixel*penWidth;
                        const float deltaPenWidth = penThicknessPixel * (int) (penWidth*0.5f);
                        ImRect penSelectionInMouseCoords(imageToMouseCoords(penSelection.Min),imageToMouseCoords(penSelection.Max));
                        penSelectionInMouseCoords.Clip(bb);
                        if (mustLimitPenToImageSelection) {
                            const ImRect imageSelectionInMouseCoords(imageToMouseCoords(imageSelection.Min),imageToMouseCoords(imageSelection.Max));
                            penSelectionInMouseCoords.Clip(imageSelectionInMouseCoords);
                        }
                        window->DrawList->PushClipRect(penSelectionInMouseCoords.Min,penSelectionInMouseCoords.Max,false);
                        const bool mustPaintCheckersBg = (c==4 && penColor.w<1);
                        // We must avoid alpha blending (without changing the blend function)
                        // The best way of doing it is to draw the solid background we use for transparency
                        // under all the pixels we're about to draw
                        // Please note that to be 100% correct we must fetch the EXACT SAME tex coords we have
                        // below the image!
                        const ImVec2 cfactor(CheckersRepeatFactor.x/(endPos.x-startPos.x),CheckersRepeatFactor.y/(endPos.y-startPos.y));
                        ImVec2 cuv0(0,0),cuv1(0,0);
                        const ImVec2 puv0(0.1f,0.1f);
                        const ImVec2 puv1(0.4f,0.4f);
                        for (int i=0;i<numPoints;i++)  {
                            tmp = imageToMouseCoords(*pPenPoints++);
                            tmp.x-= deltaPenWidth; tmp.y-=deltaPenWidth;
                            tmp2.x = tmp.x+penThicknessTotal;tmp2.y = tmp.y+penThicknessTotal;
                            if (tmp2.x>=startPos.x && tmp.x<endPos.x && tmp2.y>=startPos.y && tmp.y<endPos.y)   {
                                if (mustPaintCheckersBg)    {
                                    cuv0.x = (tmp.x-startPos.x)*cfactor.x;
                                    cuv0.y = (tmp.y-startPos.y)*cfactor.y;
                                    cuv1.x = (tmp2.x-startPos.x)*cfactor.x;
                                    cuv1.y = (tmp2.y-startPos.y)*cfactor.y;
                                    window->DrawList->AddImage(CheckersTexID,tmp,tmp2,cuv0,cuv1);
                                    window->DrawList->AddImage(CheckersTexID,tmp,tmp2,puv0,puv1,penColori); // we must reuse the same (checker) texture to display the penColor, to prevent hundreds of texture changes
                                }
                                else window->DrawList->AddRectFilled(tmp,tmp2,penColori);
                            }
                        }
                        window->DrawList->PopClipRect();

                        // Modify image
                        if (isImageSelectionValid(penSelection))
                        {
                            //IM_ASSERT(isImageSelectionValid(penSelection));

                            /*// extract selection (technically only needed for penWidth>1, penColor.w<1 and c==4):
                         * // NO WAY! We just want to replace color, not to alpha blend it! Otherwise we could never reset an alpha value to zero!
                        int dstX = penSelection.Min.x;
                        int dstY = penSelection.Min.y;
                        int dstW = penSelection.Max.x-penSelection.Min.x;
                        int dstH = penSelection.Max.y-penSelection.Min.y;
                        unsigned char* sim = NULL;
                        if (penWidth>1 && penColor.w<1 && c==4) sim = ImGuiIE::ExtractImage(dstX,dstY,dstW,dstH,image,w,h,c);
                        ImGuiIE::ImageScopedDeleter scoped(sim);
                        const unsigned char* psim = sim;*/

                            pushImage(&penSelection);

                            int x=-1,y=-1;
                            const ImVec2* pPenPoints = &penPointsInImageCoords[0];
                            unsigned char* pim = image;
                            const int penWidthi = (int) penWidth;
                            const int deltaPenWidth = (int) (penWidth*0.5f);
                            unsigned char penColorUC[4] = {(unsigned char)(penColor.x*255.f),(unsigned char)(penColor.y*255.f),(unsigned char)(penColor.z*255.f),(unsigned char)(penColor.w*255.f)};
                            if (c==1) penColorUC[0]=penColorUC[1]=penColorUC[2]=penColorUC[3];
                            int numPixelsSet = 0;
                            for (int i=0;i<numPoints;i++)  {
                                x = (int) pPenPoints->x-deltaPenWidth;y = (int) pPenPoints->y-deltaPenWidth;
                                pPenPoints++;
                                for (int yy=y,yySz=y+penWidthi;yy<yySz;yy++) {
                                    if (yy<Y || yy>=Y+H) continue;
                                    for (int xx=x,xxSz=x+penWidthi;xx<xxSz;xx++) {
                                        if (xx<X || xx>=X+W) continue;
                                        pim = &image[(yy*w+xx)*c];
                                        if (c>=3) {for (int i=0;i<c;i++) *pim++ = penColorUC[i];}
                                        else if (c==1) *pim++ = penColorUC[3];
                                        else IM_ASSERT(true);


                                        //fprintf(stderr,"(%d,%d)\n",xx,yy);
                                        ++numPixelsSet;
                                    }
                                }
                            }
                            if (numPixelsSet>0) {
                                mustInvalidateTexID = true;
                                assignModified(true);
                            }
                        }
                    }

                }
                //----------------------------------------------------------------------------------------------------
            }


            // FILL tool
            if (modifyImageTopToolbar.getSelectedButtonIndex()==1 && itemHovered && io.MouseClicked[0]) {
                const ImVec2 imagePos = mouseToImageCoords(io.MousePos,true);
                if (imagePos.x>=0 && imagePos.y>=0 && imagePos.x<w && imagePos.y<h) {
                    const ImRect* pSelection = (chbSelectionLimit && isImageSelectionValid(imageSelection))? &imageSelection : NULL;
                    pushImage(pSelection);
                    if (ImGuiIE::FillHelperClass::Fill(image,w,h,c,imagePos,penColor,ImVec4((float)tolColor/255.f,(float)tolColor/255.f,(float)tolColor/255.f,(float)tolAlpha/255.f),
                    pSelection,chbMirrorX,chbMirrorY,false)) {
                        mustInvalidateTexID = true;
                        assignModified(true);
                    }
                }
            }
        }

        ImGui::PopID();

        return rv;
    }

}; //StbImage

ImGuiIE::StbImageBuffer StbImage::CopiedImage(true);    // if true the image is stored as .png internally

ImTextureID StbImage::IconsTexID=NULL;
ImTextureID StbImage::CheckersTexID=NULL;



// Callbacks
ImageEditor::FreeTextureDelegate ImageEditor::FreeTextureCb =
#ifdef IMGUI_USE_AUTO_BINDING
&ImImpl_FreeTexture;
#else //IMGUI_USE_AUTO_BINDING
NULL;
#endif //IMGUI_USE_AUTO_BINDING
ImageEditor::GenerateOrUpdateTextureDelegate ImageEditor::GenerateOrUpdateTextureCb =
#ifdef IMGUI_USE_AUTO_BINDING
&ImImpl_GenerateOrUpdateTexture;
#else //IMGUI_USE_AUTO_BINDING
NULL;
#endif //IMGUI_USE_AUTO_BINDING

static StbImage* CreateStbImage(ImageEditor& ie) {
    StbImage* is = (StbImage*) ImGui::MemAlloc(sizeof(StbImage));
    IM_PLACEMENT_NEW (is) StbImage(ie);
    return is;
}

ImageEditor::ImageEditor() : is(NULL),init(false) {}

ImageEditor::~ImageEditor() {destroy();}

void ImageEditor::destroy(){
    if (is) {
        is->~StbImage();
        ImGui::MemFree(is);
        is = NULL;
    }
    init = false;
}
bool ImageEditor::loadFromFile(const char *path)  {
    if (!is) is = CreateStbImage(*this);
	IM_ASSERT(is);	// You can't call loadFromFile() after destroy()!
    const bool rv = is->loadFromFile(path);
    if (rv) {is->resizeWidth=is->w;is->resizeHeight=is->h;}
    return rv;
}
bool ImageEditor::saveAs(const char* path) {
    if (!is) is = CreateStbImage(*this);
    IM_ASSERT(is);	// You can't call saveAs() after destroy()!
    const bool rv = is->saveAs(path);
    return rv;
}

void ImageEditor::render(const ImVec2 &size)  {
    if (!is) is = CreateStbImage(*this);
	IM_ASSERT(is);	// You can't call render() after destroy()!    
    init = true;
    is->render(size);
}

void ImageEditor::Destroy() {StbImage::Destroy();}

#if (!defined(IMGUITABWINDOW_H_) || defined(IMGUIIMAGEEDITOR_NO_TABLABEL))
bool ImageEditor::getModified() const {return is ? is->modified : false;}
#endif //(!defined(IMGUITABWINDOW_H_) || defined(IMGUIIMAGEEDITOR_NO_TABLABEL))

ImageEditor::Style ImageEditor::Style::style;
ImageEditor::Style::Style() : splitterSize(-1),splitterColor(-1.f,1.f,1.f,1.f) {
    strcpy(&arrowsChars[0][0],"<");
    strcpy(&arrowsChars[1][0],">");
    strcpy(&arrowsChars[2][0],"^");
    strcpy(&arrowsChars[3][0],"v");
    keySave = (int) 's';
}



} // namespace ImGui


