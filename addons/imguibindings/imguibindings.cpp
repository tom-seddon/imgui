#include "imguibindings.h"


// These variables can be declared extern and set at runtime-----------------------------------------------------
bool gImGuiPaused = false;
float gImGuiInverseFPSClamp = -1.0f;    // CAN'T BE 0. < 0 = No clamping.
bool gImGuiCapturesInput = false;
// --------------------------------------------------------------------------------------------------------------

struct ImImpl_PrivateParams  {
#ifndef IMIMPL_SHADER_NONE

#ifndef IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS
#define IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS 1
#elif (IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS<=0)
#undef IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS
#define IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS 1
#endif //IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS

    GLuint vertexBuffers[IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS];
    GLuint indexBuffers[IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS];
    GLuint program;
    // gProgram uniform locations:
    GLint uniLocOrthoMatrix;
    GLint uniLocTexture;
    // gProgram attribute locations:
    GLint attrLocPosition;
    GLint attrLocUV;
    GLint attrLocColour;
    // font texture
    GLuint fontTex;
    ImImpl_PrivateParams() :program(0),uniLocOrthoMatrix(-1),uniLocTexture(-1),
        attrLocPosition(-1),attrLocUV(-1),attrLocColour(-1),fontTex(0)
    {for (int i=0;i<IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS;i++) {vertexBuffers[i]=0;indexBuffers[i]=0;}}
#else //IMIMPL_SHADER_NONE
    // font texture
    GLuint fontTex;
    ImImpl_PrivateParams() :fontTex(0) {}
#endif //IMIMPL_SHADER_NONE
};
static ImImpl_PrivateParams gImImplPrivateParams;


#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif //STBI_INCLUDE_STB_IMAGE_H

//#define IMIMPL_USES_STB_COMPRESSED_TTF_IN_MEMORY
#ifdef IMIMPL_USES_STB_COMPRESSED_TTF_IN_MEMORY
static unsigned char *stb__barrier;static unsigned char *stb__barrier2;
static unsigned char *stb__barrier3;static unsigned char *stb__barrier4;
static unsigned char *stb__dout;
struct StbDecompressor {
static unsigned int stb_decompress_length(unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}
static void stb__match(unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    assert (stb__dout + length <= stb__barrier);
    if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
    if (data < stb__barrier4) { stb__dout = stb__barrier+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(unsigned char *data, unsigned int length)
{
    assert (stb__dout + length <= stb__barrier);
    if (stb__dout + length > stb__barrier) { stb__dout += length; return; }
    if (data < stb__barrier2) { stb__dout = stb__barrier+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static unsigned char *stb_decompress_token(unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (s2 << 16) + s1;
}

static unsigned int stb_decompress(unsigned char *output, unsigned char *i, unsigned int length)
{
    unsigned int olen;
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    olen = stb_decompress_length(i);
    stb__barrier2 = i;
    stb__barrier3 = i+length;
    stb__barrier = output + olen;
    stb__barrier4 = output;
    i += 16;

    stb__dout = output;
    while (1) {
        unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                assert(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                assert(0); /* NOTREACHED */
                return 0;
            }
        }
        assert(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}
};
#endif //IMIMPL_USES_STB_COMPRESSED_TTF_IN_MEMORY

void InitImGuiFontTexture(const ImImpl_InitParams* pOptionalInitParams) {
    const char* OptionalTTFFilePath=NULL;
    if (pOptionalInitParams && pOptionalInitParams->gOptionalTTFFilePath[0]!='\0') OptionalTTFFilePath = (const char*) &pOptionalInitParams->gOptionalTTFFilePath[0];

    ImGuiIO& io = ImGui::GetIO();
    DestroyImGuiFontTexture();	// reentrant

    ImFont* my_font = NULL; // must be freed at the end of the program ??? I'm not doing it!

    unsigned char* image = NULL;
//#   define TEST_TO_REMOVE
#   ifdef  TEST_TO_REMOVE
//#   define TEST_SDF_FONT    // tweakable
    {
#   ifndef TEST_SDF_FONT
    //const char* fntPath = "./fonts/DejaVuSerifCondensedBoldOutlineRGBAtxt18.fnt";
    //const char* imgPath = "./fonts/DejaVuSerifCondensedBoldOutlineRGBAtxt18_0.png";
    const char* fntPath = "./fonts/DejaVuSerifCondensed-Bold.ttf_outline.txt";
    const char* imgPath = "./fonts/DejaVuSerifCondensed-Bold.ttf_outline.png";
#   else // TEST_SDF_FONT
    const char* fntPath = "./fonts/DejaVuSerifCondensed-Bold.ttf_sdf.txt";
    const char* imgPath = "./fonts/DejaVuSerifCondensed-Bold.ttf_sdf.png";
#   endif //TEST_SDF_FONT
    int imageWidth,imageHeight,imageN;
    image = stbi_load(imgPath,&imageWidth,&imageHeight,&imageN,0);
    if (image)  {
        ImFont* my_font = io.Fonts->AddFontFromFileFnt(fntPath,image,imageWidth,imageHeight,imageN);
        if (!my_font) fprintf(stderr,"Error loading: \"%s\"\n",fntPath);
    }
    else fprintf(stderr,"Error loading: \"%s\"\n",imgPath);
    }
#   endif //TEST_TO_REMOVE


    if (OptionalTTFFilePath) {
        my_font = io.Fonts->AddFontFromFileTTF(OptionalTTFFilePath,pOptionalInitParams->gOptionalTTFFileFontSizeInPixels,NULL,pOptionalInitParams->gOptionalTTFFileGlyphRanges);
        if (!my_font) fprintf(stderr,"Error loading: \"%s\"\n",OptionalTTFFilePath);
    }
    if (!my_font && pOptionalInitParams && pOptionalInitParams->pOptionalReferenceToTTFFileInMemory && pOptionalInitParams->pOptionalSizeOfTTFFileInMemory>0)   {
        char* tempBuffer = NULL;size_t tempBufferSize=0;
        const bool cloneBuffer = true;//false;//true;  //usable when IMIMPL_USES_STB_COMPRESSED_TTF_IN_MEMORY is NOT defined
#       ifdef  IMIMPL_USES_STB_COMPRESSED_TTF_IN_MEMORY
        const bool decompress = true;
        if (decompress) {
            // Decompress
            const size_t buf_decompressed_size = StbDecompressor::stb_decompress_length((unsigned char*)pOptionalInitParams->pOptionalReferenceToTTFFileInMemory);
            unsigned char* buf_decompressed = (unsigned char *)ImGui::MemAlloc(buf_decompressed_size);
            StbDecompressor::stb_decompress(buf_decompressed, (unsigned char*)pOptionalInitParams->pOptionalReferenceToTTFFileInMemory, pOptionalInitParams->gOptionalTTFFileFontSizeInPixels);

            my_font = io.Fonts->AddFontFromMemoryTTF((void*)buf_decompressed,buf_decompressed_size,pOptionalInitParams->gOptionalTTFFileFontSizeInPixels,pOptionalInitParams->gOptionalTTFFileGlyphRanges,0);
        }
        else {
            if (!cloneBuffer) my_font = io.Fonts->AddFontFromMemoryTTF((void*)pOptionalInitParams->pOptionalReferenceToTTFFileInMemory,pOptionalInitParams->pOptionalSizeOfTTFFileInMemory,pOptionalInitParams->gOptionalTTFFileFontSizeInPixels,pOptionalInitParams->gOptionalTTFFileGlyphRanges,0);
            else {
                tempBufferSize = pOptionalInitParams->pOptionalSizeOfTTFFileInMemory;
                tempBuffer = (char*)ImGui::MemAlloc(tempBufferSize);
                memcpy(tempBuffer,(void*)pOptionalInitParams->pOptionalReferenceToTTFFileInMemory,tempBufferSize);

                my_font = io.Fonts->AddFontFromMemoryTTF(tempBuffer,tempBufferSize,pOptionalInitParams->gOptionalTTFFileFontSizeInPixels,pOptionalInitParams->gOptionalTTFFileGlyphRanges,0);
            }
        }
#       undef IMIMPL_USES_STB_COMPRESSED_TTF_IN_MEMORY
#       else //IMIMPL_USES_STB_COMPRESSED_TTF_IN_MEMORY
        if (!cloneBuffer) my_font = io.Fonts->AddFontFromMemoryTTF((void*)pOptionalInitParams->pOptionalReferenceToTTFFileInMemory,pOptionalInitParams->pOptionalSizeOfTTFFileInMemory,pOptionalInitParams->gOptionalTTFFileFontSizeInPixels,NULL,pOptionalInitParams->gOptionalTTFFileGlyphRanges);
        else {
            tempBufferSize = pOptionalInitParams->pOptionalSizeOfTTFFileInMemory;
            tempBuffer = (char*)ImGui::MemAlloc(tempBufferSize);
            memcpy(tempBuffer,(void*)pOptionalInitParams->pOptionalReferenceToTTFFileInMemory,tempBufferSize);

            my_font = io.Fonts->AddFontFromMemoryTTF(tempBuffer,tempBufferSize,pOptionalInitParams->gOptionalTTFFileFontSizeInPixels,NULL,pOptionalInitParams->gOptionalTTFFileGlyphRanges);
        }
#       endif //IMIMPL_USES_STB_COMPRESSED_TTF_IN_MEMORY
        if (!my_font) fprintf(stderr,"Error loading ttf file from memory\n");
        //else fprintf(stderr,"Loaded ttf file from memory\n");
    }

    if (!my_font) {
        my_font = io.Fonts->AddFontDefault();
        //if (!pOptionalInitParams) my_font = io.Fonts->AddFontDefault();//
        //else my_font = io.Fonts->AddFontDefault(pOptionalInitParams->gOptionalTTFFileFontSizeInPixels,pOptionalInitParams->gOptionalTTFFileGlyphRanges);
    }


    // Load font texture
    unsigned char* pixels;
    int width, height;
    //fprintf(stderr,"Loading font texture\n");
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    glGenTextures(1, &gImImplPrivateParams.fontTex);
    glBindTexture(GL_TEXTURE_2D, gImImplPrivateParams.fontTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)gImImplPrivateParams.fontTex;
    //fprintf(stderr,"Loaded font texture\n");


    if (image) stbi_image_free(image);image=NULL;

}

void DestroyImGuiFontTexture()	{
    if (gImImplPrivateParams.fontTex)	{
        glDeleteTextures( 1, &gImImplPrivateParams.fontTex );
        gImImplPrivateParams.fontTex = 0;
    }
}

#ifndef _WIN32
#include <unistd.h>
#else //_WIN32
// Is there a header with ::Sleep(...) ?
#endif //_WIN32
void WaitFor(unsigned int ms)    {
#ifdef _WIN32
  if (ms > 0) Sleep(ms);
#else
  // delta in microseconds
  useconds_t delta = (useconds_t) ms * 1000;
  // On some systems, the usleep argument must be < 1000000
  while (delta > 999999L)   {
    usleep(999999);
    delta -= 999999L;
  }
  if (delta > 0L) usleep(delta);
#endif
}

GLuint ImImpl_LoadTexture(const char* filename,int req_comp,GLenum magFilter,GLenum minFilter,GLenum wrapS,GLenum wrapT)  {
    int w,h,n;
    unsigned char* pixels = stbi_load(filename,&w,&h,&n,req_comp);
    if (!pixels) {
        fprintf(stderr,"Error: can't load texture: \"%s\".\n",filename);
        return 0;
    }
    if (req_comp>0 && req_comp<=4) n = req_comp;
    GLuint texId=0;

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,wrapS);

    const int byteAlignement = 1;
    glPixelStorei(GL_UNPACK_ALIGNMENT, byteAlignement);

    const GLenum ifmt = n==1 ? GL_ALPHA : n==2 ? GL_LUMINANCE_ALPHA : n==3 ? GL_RGB : GL_RGBA;
    const GLenum fmt = ifmt;
    glTexImage2D(GL_TEXTURE_2D, 0, ifmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, pixels);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
    if (minFilter==GL_NEAREST_MIPMAP_NEAREST || minFilter==GL_NEAREST_MIPMAP_LINEAR || minFilter==GL_LINEAR_MIPMAP_NEAREST || minFilter==GL_LINEAR_MIPMAP_LINEAR)
        glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(pixels);

    return texId;
}

GLuint ImImpl_LoadTextureFromMemory(const unsigned char* filenameInMemory,int filenameInMemorySize,int req_comp,GLenum magFilter,GLenum minFilter,GLenum wrapS,GLenum wrapT)  {
    int w,h,n;
    unsigned char* pixels = stbi_load_from_memory(filenameInMemory,filenameInMemorySize,&w,&h,&n,req_comp);
    if (!pixels) {
        fprintf(stderr,"Error: can't load texture from memory\n");
        return 0;
    }
    if (req_comp>0 && req_comp<=4) n = req_comp;
    GLuint texId=0;

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,wrapS);

    const int byteAlignement = 1;
    glPixelStorei(GL_UNPACK_ALIGNMENT, byteAlignement);

    const GLenum ifmt = n==1 ? GL_ALPHA : n==2 ? GL_LUMINANCE_ALPHA : n==3 ? GL_RGB : GL_RGBA;
    const GLenum fmt = ifmt;
    glTexImage2D(GL_TEXTURE_2D, 0, ifmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, pixels);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
    if (minFilter==GL_NEAREST_MIPMAP_NEAREST || minFilter==GL_NEAREST_MIPMAP_LINEAR || minFilter==GL_LINEAR_MIPMAP_NEAREST || minFilter==GL_LINEAR_MIPMAP_LINEAR)
        glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(pixels);

    return texId;
}

#ifndef IMIMPL_SHADER_NONE
static GLuint CompileShaders(const GLchar** vertexShaderSource, const GLchar** fragmentShaderSource )
{
    //Compile vertex shader
    GLuint vertexShader( glCreateShader( GL_VERTEX_SHADER ) );
    glShaderSource( vertexShader, 1, vertexShaderSource, NULL );
    glCompileShader( vertexShader );


    // check
    GLint bShaderCompiled;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &bShaderCompiled);

    if (!bShaderCompiled)        {
        int i32InfoLogLength, i32CharsWritten;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

        char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(vertexShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
        printf("********VertexShader %s\n", pszInfoLog);

        delete[] pszInfoLog;
    }

    //Compile fragment shader
    GLuint fragmentShader( glCreateShader( GL_FRAGMENT_SHADER ) );
    glShaderSource( fragmentShader, 1, fragmentShaderSource, NULL );
    glCompileShader( fragmentShader );

    //check
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &bShaderCompiled);

    if (!bShaderCompiled)        {
        int i32InfoLogLength, i32CharsWritten;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

        char* pszInfoLog = new char[i32InfoLogLength];
        glGetShaderInfoLog(fragmentShader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
        printf("********FragmentShader %s\n", pszInfoLog);

        delete[] pszInfoLog;
    }

    //Link vertex and fragment shader together
    GLuint program( glCreateProgram() );
    glAttachShader( program, vertexShader );
    glAttachShader( program, fragmentShader );
    glLinkProgram( program );

    //check
    GLint bLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &bLinked);
    if (!bLinked)        {
        int i32InfoLogLength, i32CharsWritten;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

        char* pszInfoLog = new char[i32InfoLogLength];
        glGetProgramInfoLog(program, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
        printf("%s",pszInfoLog);

        delete[] pszInfoLog;
    }

    //Delete shaders objects
    glDeleteShader( vertexShader );
    glDeleteShader( fragmentShader );

    return program;
}
#endif //IMIMPL_SHADER_NONE


void InitImGuiProgram()  {
#ifndef IMIMPL_SHADER_NONE
// -----------------------------------------------------------------------
// START SHADER CODE
//------------------------------------------------------------------------
// shaders
#ifdef IMIMPL_SHADER_GL3
static const GLchar* gVertexShaderSource[] = {
#ifdef IMIMPL_SHADER_GLES
      "#version 300 es\n"
#else  //IMIMPL_SHADER_GLES
      "#version 330\n"
#endif //IMIMPL_SHADER_GLES
      "precision highp float;\n"
      "uniform mat4 ortho;\n"
      "layout (location = 0 ) in vec2 Position;\n"
      "layout (location = 1 ) in vec2 UV;\n"
      "layout (location = 2 ) in vec4 Colour;\n"
      "out vec2 Frag_UV;\n"
      "out vec4 Frag_Colour;\n"
      "void main()\n"
      "{\n"
      " Frag_UV = UV;\n"
      " Frag_Colour = Colour;\n"
      "\n"
      " gl_Position = ortho*vec4(Position.xy,0,1);\n"
      "}\n"
    };

static const GLchar* gFragmentShaderSource[] = {
#ifdef IMIMPL_SHADER_GLES
      "#version 300 es\n"
#else //IMIMPL_SHADER_GLES
      "#version 330\n"
#endif //IMIMPL_SHADER_GLES
      "precision mediump float;\n"
      "uniform lowp sampler2D Texture;\n"
      "in vec2 Frag_UV;\n"
      "in vec4 Frag_Colour;\n"
      "out vec4 FragColor;\n"
      "void main()\n"
      "{\n"
      " FragColor = Frag_Colour * texture( Texture, Frag_UV.st);\n"
      "}\n"
    };
#else //NO IMIMPL_SHADER_GL3
static const GLchar* gVertexShaderSource[] = {
#ifdef IMIMPL_SHADER_GLES
      "#version 100\n"
      "precision highp float;\n"
#endif //IMIMPL_SHADER_GLES
      "uniform mat4 ortho;\n"
      "attribute vec2 Position;\n"
      "attribute vec2 UV;\n"
      "attribute vec4 Colour;\n"
      "varying vec2 Frag_UV;\n"
      "varying vec4 Frag_Colour;\n"
      "void main()\n"
      "{\n"
      " Frag_UV = UV;\n"
      " Frag_Colour = Colour;\n"
      "\n"
      " gl_Position = ortho*vec4(Position.xy,0,1);\n"
      "}\n"
    };

#ifndef TEST_SDF_FONT
static const GLchar* gFragmentShaderSource[] = {
#ifdef IMIMPL_SHADER_GLES
      "#version 100\n"
      "precision mediump float;\n"
      "uniform lowp sampler2D Texture;\n"
#else //IMIMPL_SHADER_GLES
      "uniform sampler2D Texture;\n"
#endif //IMIMPL_SHADER_GLES
      "varying vec2 Frag_UV;\n"
      "varying vec4 Frag_Colour;\n"
      "void main()\n"
      "{\n"
      " gl_FragColor = Frag_Colour * texture2D( Texture, Frag_UV.st);\n"
      "}\n"
    };
#else //TEST_SDF_FONT
static const GLchar* gFragmentShaderSource[] = {
      "#define SPECIAL_EFFECT 1 // 0 = WORKS; 1 = OUTLINE; 2 = PULSATE; 3 = SQUIGGLE\n"
      "#define SUPERSAMPLED_VERSION 0\n"
#ifdef IMIMPL_SHADER_GLES
      "#version 100\n"
      "precision mediump float;\n"
      "uniform lowp sampler2D Texture;\n"
#else //IMIMPL_SHADER_GLES
      "#version 120 // GLSL 1.2 to correspond with OpenGL 2.0\n"
      "uniform sampler2D Texture;\n"
#endif //IMIMPL_SHADER_GLES
      "varying vec2 Frag_UV;\n"
      "varying vec4 Frag_Colour;\n"
      "float contour(in float d, in float w) {\n"
      "return smoothstep(0.5 - w, 0.5 + w, d); // smoothstep(lower edge0, upper edge1, x)\n"
      "}\n"
      "float samp(in vec2 uv, float w) {\n"
      "return contour(texture2D(Texture, Frag_UV).a, w);\n"
      "}\n"
      "\n"
      "void main(void) {\n"
      "float dist = texture2D(Texture, Frag_UV.st).a; // retrieve distance from texture\n"
      "// fwidth helps keep outlines a constant width irrespective of scaling\n"
      "// GLSL's fwidth = abs(dFdx(uv)) + abs(dFdy(uv))\n"
      "float width = fwidth(dist);"
      "// Stefan Gustavson's fwidth\n"
      "//float width = 0.7 * length(vec2(dFdx(dist), dFdy(dist)));\n"
      "\n"
      "float alphaThreshold = 0.5;\n"
      "\n"
      "#if SPECIAL_EFFECT==0\n"
      "vec3 fragcolor = Frag_Colour.rgb;\n"
      "#elif SPECIAL_EFFECT==1 // OUTLINE EFFECT \n"
      "float outlineDarkeningFactor = 0.3f;\n"
      "alphaThreshold = 0.2f;\n"
      "float outlineThreshold = 0.5f;"
      "float inside = smoothstep(outlineThreshold - width, outlineThreshold + width, dist) ;\n"
      "//float glow = 1.0-inside;//smoothstep (0.0 , 20.0 , dist ) ;\n"
      "float glow = smoothstep (0.0 , 20.0 , dist ) ; // I don't understand this...\n"
      "vec3 insidecolor = Frag_Colour.rgb;\n"
      "vec3 outlinecolor = Frag_Colour.rgb*outlineDarkeningFactor;\n"
      "vec3 fragcolor = mix ( glow * outlinecolor , insidecolor , inside ) ;\n"
      "#elif SPECIAL_EFFECT==2 // PULSATE EFFECT \n"
      "dist = dist - 2.0 + 2.0 * sin ( Frag_UV.s * 10.0) ;\n"
      "vec3 fragcolor = vec3 ( smoothstep ( -0.5 , 0.5 , dist ) )*Frag_Colour.rgb ;\n"
      "#elif SPECIAL_EFFECT==3 // SQUIGGLE EFFECT \n"
      "dist = dist + 2.0 * noise(20.0* Frag_UV ) ;\n"
      "vec3 fragcolor = vec3 (1.0 - smoothstep ( -2.0 , -1.0 , dist ) + smoothstep (1.0 , 2.0 , dist ) ) ;\n"
      "#endif //SPECIAL_EFFECT\n"
      "\n"
      "#if !SUPERSAMPLED_VERSION\n"
      "float alpha = smoothstep(alphaThreshold - width, alphaThreshold + width, dist);\n"
      "#else //SUPERSAMPLED_VERSION\n\n"
      "float alpha = contour( dist, width );\n"
      "//float alpha = aastep( 0.5, dist );\n"
      "// Supersample, 4 extra points\n"
      "float dscale = 0.354; // half of 1/sqrt2; you can play with this\n"
      "vec2 duv = dscale * (dFdx(Frag_UV) + dFdy(Frag_UV));\n"
      "vec4 box = vec4(Frag_UV-duv, Frag_UV+duv);\n"
      "float asum = samp( box.xy, width ) + samp( box.zw, width ) + samp( box.xw, width ) + samp( box.zy, width );\n"
      "// weighted average, with 4 extra points having 0.5 weight each, so 1 + 0.5*4 = 3 is the divisor\n"
      "alpha = (alpha + 0.5 * asum) / 3.0;\n"
      "#endif //SUPERSAMPLED_VERSION\n\n"
      "\n"
      "gl_FragColor = vec4(fragcolor, alpha);\n"
      "}\n"
    };
#endif //TEST_SDF_FONT
#endif //IMIMPL_SHADER_GL3
//------------------------------------------------------------------------
// END SHADER CODE
//------------------------------------------------------------------------

    if (gImImplPrivateParams.program==0)    {
        gImImplPrivateParams.program = CompileShaders(gVertexShaderSource, gFragmentShaderSource );
        if (gImImplPrivateParams.program==0) {
            fprintf(stderr,"Error compiling shaders.\n");
            return;
        }
        //Get Uniform locations
        gImImplPrivateParams.uniLocTexture = glGetUniformLocation(gImImplPrivateParams.program,"Texture");
        gImImplPrivateParams.uniLocOrthoMatrix = glGetUniformLocation(gImImplPrivateParams.program,"ortho");

        //Get Attribute locations
        gImImplPrivateParams.attrLocPosition  = glGetAttribLocation(gImImplPrivateParams.program,"Position");
        gImImplPrivateParams.attrLocUV  = glGetAttribLocation(gImImplPrivateParams.program,"UV");
        gImImplPrivateParams.attrLocColour  = glGetAttribLocation(gImImplPrivateParams.program,"Colour");

        // Debug
        /*
        printf("gUniLocTexture = %d\n",gImImplPrivateParams.gUniLocTexture);
        printf("gUniLocLayers = %d\n",gImImplPrivateParams.gUniLocOrthoMatrix);
        printf("gAttrLocPosition = %d\n",gImImplPrivateParams.gAttrLocPosition);
        printf("gAttrLocUV = %d\n",gImImplPrivateParams.gAttrLocUV);
        printf("gAttrLocColour = %d\n",gImImplPrivateParams.gAttrLocColour);
        */
    }
#endif //IMIMPL_SHADER_NONE
}

void DestroyImGuiProgram()	{
#ifndef IMIMPL_SHADER_NONE
    if (gImImplPrivateParams.program)	{
        glDeleteProgram(gImImplPrivateParams.program);
        gImImplPrivateParams.program = 0;
    }
#endif //IMIMPL_SHADER_NONE
}
void InitImGuiBuffer()	{
#ifndef IMIMPL_SHADER_NONE
    if (gImImplPrivateParams.vertexBuffers[0]==0) glGenBuffers(IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS, &gImImplPrivateParams.vertexBuffers[0]);
    if (gImImplPrivateParams.indexBuffers[0]==0) glGenBuffers(IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS, &gImImplPrivateParams.indexBuffers[0]);
#endif //IMIMPL_SHADER_NONE
}
void DestroyImGuiBuffer()	{
#ifndef IMIMPL_SHADER_NONE
    if (gImImplPrivateParams.vertexBuffers[0]) {
        glDeleteBuffers( IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS, &gImImplPrivateParams.vertexBuffers[0] );
        gImImplPrivateParams.vertexBuffers[0] = 0;
    }
    if (gImImplPrivateParams.indexBuffers[0]) {
        glDeleteBuffers( IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS, &gImImplPrivateParams.indexBuffers[0] );
        gImImplPrivateParams.indexBuffers[0] = 0;
    }
#endif //IMIMPL_SHADER_NONE
}

#ifdef IMIMPL_FORCE_DEBUG_CONTEXT
extern "C" void GLDebugMessageCallback(GLenum source, GLenum type,
    GLuint id, GLenum severity,GLsizei length, const GLchar *msg,const void *userParam)
{
    char sourceStr[32];
    const char *sourceFmt = "UNDEFINED(0x%04X)";
    switch(source)

    {
    case GL_DEBUG_SOURCE_API_ARB:             sourceFmt = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:   sourceFmt = "WINDOW_SYSTEM"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: sourceFmt = "SHADER_COMPILER"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:     sourceFmt = "THIRD_PARTY"; break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB:     sourceFmt = "APPLICATION"; break;
    case GL_DEBUG_SOURCE_OTHER_ARB:           sourceFmt = "OTHER"; break;
    }

    snprintf(sourceStr, 32, sourceFmt, source);

    char typeStr[32];
    const char *typeFmt = "UNDEFINED(0x%04X)";
    switch(type)
    {

    case GL_DEBUG_TYPE_ERROR_ARB:               typeFmt = "ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: typeFmt = "DEPRECATED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  typeFmt = "UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:         typeFmt = "PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:         typeFmt = "PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER_ARB:               typeFmt = "OTHER"; break;
    }
    snprintf(typeStr, 32, typeFmt, type);


    char severityStr[32];
    const char *severityFmt = "UNDEFINED";
    switch(severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_ARB:   severityFmt = "HIGH";   break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB: severityFmt = "MEDIUM"; break;
    case GL_DEBUG_SEVERITY_LOW_ARB:    severityFmt = "LOW"; break;
    }

    snprintf(severityStr, 32, severityFmt, severity);

    fprintf(stderr,"OpenGL: %s [source=%s type=%s severity=%s(%d) id=%d]\n",msg, sourceStr, typeStr, severityStr,severity, id);

    if (strcmp(severityFmt,"UNDEFINED")!=0) {
        fprintf(stderr,"BREAKPOINT TRIGGERED\n");
    }
}
#endif //IMIMPL_FORCE_DEBUG_CONTEXT



void ImImpl_RenderDrawLists(ImDrawData* draw_data)
{
#ifndef IMIMPL_SHADER_NONE
    // Setup render state: alpha-blending enabled, no face culling (or GL_FRONT face culling), no depth testing, scissor enabled
    //glEnable(GL_ALPHA_TEST);glAlphaFunc(GL_GREATER,0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_FRONT);       // with this I can leave GL_CULL_FACE as it is
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    // Setup texture
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, gImImplPrivateParams.fontTex);
    // Setup program and uniforms
    glUseProgram(gImImplPrivateParams.program);
    glUniform1i(gImImplPrivateParams.uniLocTexture, 0);
    // Setup orthographic projection matrix
    ImGuiIO& io = ImGui::GetIO();
    const float width = io.DisplaySize.x;
    const float height = io.DisplaySize.y;
    const float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;   // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    const float ortho[4][4] = {
        { 2.0f/width,   0.0f,           0.0f, 0.0f },
        { 0.0f,         2.0f/-height,   0.0f, 0.0f },
        { 0.0f,         0.0f,          -1.0f, 0.0f },
        {-1.0f,         1.0f,           0.0f, 1.0f },
    };
    glUniformMatrix4fv(gImImplPrivateParams.uniLocOrthoMatrix, 1, GL_FALSE, &ortho[0][0]);

    static int bufferNum = 0;
#   if IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS > 1
    if (++bufferNum == IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS) bufferNum = 0;
    //fprintf(stderr,"Using buffer: %d\n",bufferNum);
#   endif //IMIMPL_NUM_ROUND_ROBIN_VERTEX_BUFFERS

    IM_ASSERT(gImImplPrivateParams.vertexBuffers[bufferNum]!=0);
    IM_ASSERT(gImImplPrivateParams.indexBuffers[bufferNum]!=0);

    glBindBuffer(GL_ARRAY_BUFFER, gImImplPrivateParams.vertexBuffers[bufferNum]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gImImplPrivateParams.indexBuffers[bufferNum]);

    glEnableVertexAttribArray(gImImplPrivateParams.attrLocPosition);
    glEnableVertexAttribArray(gImImplPrivateParams.attrLocUV);
    glEnableVertexAttribArray(gImImplPrivateParams.attrLocColour);

    glVertexAttribPointer(gImImplPrivateParams.attrLocPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const void*)(0));
    glVertexAttribPointer(gImImplPrivateParams.attrLocUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const void*)(0 + 8));
    glVertexAttribPointer(gImImplPrivateParams.attrLocColour, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (const void*)(0 + 16));



    GLuint lastTex = 0,tex=0;
    glBindTexture(GL_TEXTURE_2D, lastTex);
    for (int n = 0; n < draw_data->CmdListsCount; n++)  {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

        //fprintf(stderr,"%d/%d) cmd_list->VtxBuffer.size() = %d",n+1,draw_data->CmdListsCount,cmd_list->VtxBuffer.size());
        for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)    {
            if (pcmd->UserCallback) pcmd->UserCallback(cmd_list, pcmd);
            else    {
                tex = (GLuint)(intptr_t)pcmd->TextureId;
                if (tex!=lastTex)   {
                    glBindTexture(GL_TEXTURE_2D, tex);
                    lastTex = tex;
                }
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                //fprintf(stderr,"    pcmd->ElemCount = %d    idx_buffer_offset = %d\n",pcmd->ElemCount,idx_buffer_offset);
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, GL_UNSIGNED_SHORT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    glDisableVertexAttribArray(gImImplPrivateParams.attrLocPosition);
    glDisableVertexAttribArray(gImImplPrivateParams.attrLocUV);
    glDisableVertexAttribArray(gImImplPrivateParams.attrLocColour);
    glUseProgram(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
#else //IMIMPL_SHADER_NONE
    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    ImGuiIO& io = ImGui::GetIO();
    float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup orthographic projection matrix
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render command lists
    #define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
        const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, GL_UNSIGNED_SHORT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
    #undef OFFSETOF

    // Restore modified state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
#endif //IMIMPL_SHADER_NONE
}


