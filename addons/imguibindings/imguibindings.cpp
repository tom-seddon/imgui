#include "imguibindings.h"


// These variables can be declared extern and set at runtime-----------------------------------------------------
bool gImGuiPaused = false;
bool gImGuiDynamicFPSInsideImGui = false;                      // Well, almost...
float gImGuiInverseFPSClampInsideImGui = -1.0f;    // CAN'T BE 0. < 0 = No clamping.
float gImGuiInverseFPSClampOutsideImGui = -1.0f;   // CAN'T BE 0. < 0 = No clamping.
bool gImGuiCapturesInput = false;
bool gImGuiWereOutsideImGui = true;
bool gImGuiBindingMouseDblClicked[5]={false,false,false,false,false};
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



void InitImGuiFontTexture(const ImImpl_InitParams* pOptionalInitParams) {
    ImGuiIO& io = ImGui::GetIO();
    DestroyImGuiFontTexture();	// reentrant

    if (pOptionalInitParams)    {
        const ImImpl_InitParams& P = *pOptionalInitParams;
        if (P.forceAddDefaultFontAsFirstFont) io.Fonts->AddFontDefault();

        for (int i=0,isz=(int)P.fonts.size();i<isz;i++)   {
            const ImImpl_InitParams::FontData& fd = P.fonts[i];
            ImFont* my_font = NULL;
            const bool hasValidPath = strlen(fd.filePath)>0;
            const bool hasValidMemory = fd.pMemoryData && fd.memoryDataSize>0;
            //if (i==0 && !P.forceAddDefaultFontAsFirstFont && (hasValidPath || hasValidMemory) && fd.useFontConfig && fd.fontConfig.MergeMode) io.Fonts->AddFontDefault();
            if (hasValidPath)  {
#if         (!defined(NO_IMGUIHELPER) && defined(IMGUI_USE_ZLIB))
                bool isTtfGz = false;
                char* ttfGzExt = strrchr((char*) fd.filePath,'.');
                if (ttfGzExt && (strcmp(ttfGzExt,".gz")==0 || strcmp(ttfGzExt,".GZ")==0))   {
                    ImVector<char> buffVec;
                    if (ImGui::GzDecompressFromFile((const char*)fd.filePath,buffVec) && buffVec.size()>0)  {
                        // Actually the only thing I can do with a vector is to allocate, copy and let ImGui delete it:
                        char* tempBuffer = NULL;void* bufferToFeedImGui = NULL;
                        tempBuffer = (char*)ImGui::MemAlloc(buffVec.size());
                        memcpy(tempBuffer,(void*)&buffVec[0],buffVec.size());
                        bufferToFeedImGui = tempBuffer;
                        ImImpl_InitParams::FontData fd2 = fd;fd2.fontConfig.FontDataOwnedByAtlas=true;
                        my_font = io.Fonts->AddFontFromMemoryTTF(bufferToFeedImGui,buffVec.size(),fd.sizeInPixels,fd.useFontConfig?&fd2.fontConfig:NULL,fd.pGlyphRanges);
                        if (!my_font) {ImGui::MemFree(tempBuffer);tempBuffer=NULL;bufferToFeedImGui=NULL;}
                    }
                    isTtfGz = my_font!=NULL;
                }
                if (!isTtfGz) my_font = io.Fonts->AddFontFromFileTTF(fd.filePath,fd.sizeInPixels,fd.useFontConfig?&fd.fontConfig:NULL,fd.pGlyphRanges);
#           else   //IMGUI_USE_ZLIB
                my_font = io.Fonts->AddFontFromFileTTF(fd.filePath,fd.sizeInPixels,fd.useFontConfig?&fd.fontConfig:NULL,fd.pGlyphRanges);
#           endif   //IMGUI_USE_ZLIB
            }
            else if (hasValidMemory)  {
                // Sometimes ImGui tries to delete the memory we have passed it: should this happen, we should try something like:
                bool mustCloneMemoryBufferBecauseImGuiDeletesIt = (fd.memoryDataCompression==ImImpl_InitParams::FontData::COMP_NONE && fd.useFontConfig) ? fd.fontConfig.FontDataOwnedByAtlas : true;//false;//true;
#if             (!defined(NO_IMGUIHELPER) && defined(IMGUI_USE_ZLIB))
                if (fd.memoryDataCompression==ImImpl_InitParams::FontData::COMP_GZ) mustCloneMemoryBufferBecauseImGuiDeletesIt = false;
#               endif
                char* tempBuffer = NULL;void* bufferToFeedImGui = NULL;
                if (!mustCloneMemoryBufferBecauseImGuiDeletesIt) bufferToFeedImGui = (void*) fd.pMemoryData;
                else {
                    tempBuffer = (char*)ImGui::MemAlloc(fd.memoryDataSize);
                    memcpy(tempBuffer,(void*)fd.pMemoryData,fd.memoryDataSize);
                    bufferToFeedImGui = tempBuffer;
                }
                switch (fd.memoryDataCompression)   {
                case ImImpl_InitParams::FontData::COMP_NONE:
                    my_font = io.Fonts->AddFontFromMemoryTTF(bufferToFeedImGui,fd.memoryDataSize,fd.sizeInPixels,fd.useFontConfig?&fd.fontConfig:NULL,fd.pGlyphRanges);
                    break;
                case ImImpl_InitParams::FontData::COMP_STB:
                    my_font = io.Fonts->AddFontFromMemoryCompressedTTF(bufferToFeedImGui,fd.memoryDataSize,fd.sizeInPixels,fd.useFontConfig?&fd.fontConfig:NULL,fd.pGlyphRanges);
                    break;
                case ImImpl_InitParams::FontData::COMP_STBBASE85:
                    my_font = io.Fonts->AddFontFromMemoryCompressedBase85TTF((const char*)bufferToFeedImGui,fd.sizeInPixels,fd.useFontConfig?&fd.fontConfig:NULL,fd.pGlyphRanges);
                    break;
#if             (!defined(NO_IMGUIHELPER) && defined(IMGUI_USE_ZLIB))
                case ImImpl_InitParams::FontData::COMP_GZ:  {
                    ImVector<char> buffVec;
                    if (ImGui::GzDecompressFromMemory((const char*)fd.pMemoryData,fd.memoryDataSize,buffVec) && buffVec.size()>0)  {
                        // Actually the only thing I can do with a vector is to allocate, copy and let ImGui delete it:
                        tempBuffer = (char*)ImGui::MemAlloc(buffVec.size());
                        memcpy(tempBuffer,(void*)&buffVec[0],buffVec.size());
                        bufferToFeedImGui = tempBuffer;
                        ImImpl_InitParams::FontData fd2 = fd;fd2.fontConfig.FontDataOwnedByAtlas=true;
                        my_font = io.Fonts->AddFontFromMemoryTTF(bufferToFeedImGui,buffVec.size(),fd.sizeInPixels,fd.useFontConfig?&fd2.fontConfig:NULL,fd.pGlyphRanges);
                        if (!my_font) {ImGui::MemFree(tempBuffer);tempBuffer=NULL;bufferToFeedImGui=NULL;}
                    }
                    }
                    break;
#               endif   //IMGUI_USE_ZLIB
                default:
                    IM_ASSERT(true);    //Unsupported font compression
                    break;
                }
            }

            if (!my_font)   fprintf(stderr,"An error occurred while trying to load font %d\n",i);
        }

        if (!P.forceAddDefaultFontAsFirstFont && P.fonts.size()==0) io.Fonts->AddFontDefault();
    }
    else io.Fonts->AddFontDefault();

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

#   ifdef IMGUIBINDINGS_CLEAR_INPUT_DATA_SOON
    // Cleanup (don't clear the input data if you want to append new fonts later)    
    io.Fonts->ClearInputData();
    io.Fonts->ClearTexData();
#   endif //IMGUIBINDINGS_CLEAR_INPUT_DATA_SOON

    //fprintf(stderr,"Loaded font texture\n");
#   if (!defined(NO_IMGUIHELPER) && !defined(NO_IMGUIHELPER_FONT_METHODS))
    ImGui::InitPushFontOverload();  // Allows us to use ImGui::PushFont(fontIndex). Can be called in InitGL() instead and the system is self-inited on the first call to ImGui::PushFont(), but better stay on the safe side.
#   endif //NO_IMGUIHELPER

}

void DestroyImGuiFontTexture()	{
    if (gImImplPrivateParams.fontTex)	{
#       ifndef IMGUIBINDINGS_CLEAR_INPUT_DATA_SOON
        ImGuiIO& io = ImGui::GetIO();
        // Cleanup (don't clear the input data if you want to append new fonts later)
        if (io.Fonts) io.Fonts->ClearInputData();
        if (io.Fonts) io.Fonts->ClearTexData();
#       endif //IMGUIBINDINGS_CLEAR_INPUT_DATA_SOON
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

static const GLchar* gFragmentShaderSource[] = {
#ifdef IMIMPL_SHADER_GLES
      "#version 100\n"
      "precision mediump float;\n"
      "uniform lowp sampler2D Texture;\n"
#else //IMIMPL_SHADER_GLES
#   ifdef __EMSCRIPTEN__
    "precision mediump float;\n"
#   endif //__EMSCRIPTEN__
      "uniform sampler2D Texture;\n"
#endif //IMIMPL_SHADER_GLES
      "varying vec2 Frag_UV;\n"
      "varying vec4 Frag_Colour;\n"
      "void main()\n"
      "{\n"
      " gl_FragColor = Frag_Colour * texture2D( Texture, Frag_UV.st);\n"
      "}\n"
    };
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
    GLint last_texture=0;
#   ifdef IMGUIBINDINGS_RESTORE_GL_STATE
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
#   endif //IMGUIBINDINGS_RESTORE_GL_STATE
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
#   ifdef IMGUIBINDINGS_RESTORE_GL_STATE
    glPopAttrib();
    glBindTexture(GL_TEXTURE_2D,last_texture);
#   endif //IMGUIBINDINGS_RESTORE_GL_STATE
#else //IMIMPL_SHADER_NONE
    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    GLint last_texture=0;
#   ifdef IMGUIBINDINGS_RESTORE_GL_STATE
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
#   endif //IMGUIBINDINGS_RESTORE_GL_STATE
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
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
#   ifdef IMGUIBINDINGS_RESTORE_GL_STATE
    glPopAttrib();
    glBindTexture(GL_TEXTURE_2D, last_texture);
#   endif //IMGUIBINDINGS_RESTORE_GL_STATE
#endif //IMIMPL_SHADER_NONE
}


