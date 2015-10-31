#include "imguihelper.h"

#ifdef _WIN32
#include <shellapi.h>	// ShellExecuteA(...) - Shell32.lib
#include <objbase.h>    // CoInitializeEx(...)  - ole32.lib
#else //_WIN32
#include <unistd.h>
#endif //_WIN32



namespace ImGui {

bool OpenWithDefaultApplication(const char* url,bool exploreModeForWindowsOS)	{
#       ifdef _WIN32
            //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);  // Needed ??? Well, let's suppose the user initializes it himself for now"
            return ((size_t)ShellExecuteA( NULL, exploreModeForWindowsOS ? "explore" : "open", url, "", ".", SW_SHOWNORMAL ))>32;
#       else //_WIN32
            if (exploreModeForWindowsOS) exploreModeForWindowsOS = false;   // No warnings
            char tmp[4096];
            const char* openPrograms[]={"xdg-open","gnome-open"};	// Not sure what can I append here for MacOS

            static int openProgramIndex=-2;
            if (openProgramIndex==-2)   {
                openProgramIndex=-1;
                for (size_t i=0,sz=sizeof(openPrograms)/sizeof(openPrograms[0]);i<sz;i++) {
                    strcpy(tmp,"/usr/bin/");	// Well, we should check all the folders inside $PATH... and we ASSUME that /usr/bin IS inside $PATH (see below)
                    strcat(tmp,openPrograms[i]);
                    FILE* fd = fopen(tmp,"r");
                    if (fd) {
                        fclose(fd);
                        openProgramIndex = (int)i;
                        //printf(stderr,"found %s\n",tmp);
                        break;
                    }
                }
            }

            // Note that here we strip the prefix "/usr/bin" and just use openPrograms[openProgramsIndex].
            // Also note that if nothing has been found we use "xdg-open" (that might still work if it exists in $PATH, but not in /usr/bin).
            strcpy(tmp,openPrograms[openProgramIndex<0?0:openProgramIndex]);

            strcat(tmp," \"");
            strcat(tmp,url);
            strcat(tmp,"\"");
            return system(tmp)==0;
#       endif //_WIN32
}

} // namespace Imgui


#ifndef NO_IMGUIHELPER_SERIALIZATION
#include <stdio.h>  // FILE
namespace ImGuiHelper   {

static const char* FieldTypeNames[FT_COUNT+1] = {"INT","UNSIGNED","FLOAT","DOUBLE","STRING","ENUM","BOOL","COLOR","COUNT"};
static const char* FieldTypeFormats[FT_COUNT]={"%d","%u","%f","%f","%s","%d","%d","%f"};
static const char* FieldTypeFormatsWithCustomPrecision[FT_COUNT]={"%.*d","%*u","%.*f","%.*f","%*s","%*d","%*d","%.*f"};

#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
void Deserializer::clear() {
    if (f_data) ImGui::MemFree(f_data);
    f_data = NULL;f_size=0;
}
bool Deserializer::loadFromFile(const char *filename) {
    clear();
    if (!filename) return false;
    FILE* f;
    if ((f = fopen(filename, "rt")) == NULL) return false;
    if (fseek(f, 0, SEEK_END))  {
        fclose(f);
        return false;
    }
    const long f_size_signed = ftell(f);
    if (f_size_signed == -1)    {
        fclose(f);
        return false;
    }
    f_size = (size_t)f_size_signed;
    if (fseek(f, 0, SEEK_SET))  {
        fclose(f);
        return false;
    }
    f_data = (char*)ImGui::MemAlloc(f_size+1);
    f_size = fread(f_data, 1, f_size, f); // Text conversion alter read size so let's not be fussy about return value
    fclose(f);
    if (f_size == 0)    {
        clear();
        return false;
    }
    f_data[f_size] = 0;
    ++f_size;
    return true;
}
bool Deserializer::allocate(size_t sizeToAllocate, const char *optionalTextToCopy, size_t optionalTextToCopySize)    {
    clear();
    if (sizeToAllocate==0) return false;
    f_size = sizeToAllocate;
    f_data = (char*)ImGui::MemAlloc(f_size);
    if (!f_data) {clear();return false;}
    if (optionalTextToCopy && optionalTextToCopySize>0) memcpy(f_data,optionalTextToCopy,optionalTextToCopySize>f_size ? f_size:optionalTextToCopySize);
    return true;
}
Deserializer::Deserializer(const char *filename) : f_data(NULL),f_size(0) {
    if (filename) loadFromFile(filename);
}
Deserializer::Deserializer(const char *text, size_t textSizeInBytes) : f_data(NULL),f_size(0) {
    allocate(textSizeInBytes,text,textSizeInBytes);
}

const char* Deserializer::parse(Deserializer::ParseCallback cb, void *userPtr, const char *optionalBufferStart) const {
    if (!cb || !f_data || f_size==0) return NULL;
    //------------------------------------------------
    // Parse file in memory
    char name[128];name[0]='\0';
    char typeName[32];char format[32]="";bool quitParsing = false;
    char charBuffer[sizeof(double)*10];void* voidBuffer = (void*) &charBuffer[0];
    const char* varName = NULL;int numArrayElements = 0;FieldType ft = FT_COUNT;
    const char* buf_end = f_data + f_size-1;
    for (const char* line_start = optionalBufferStart ? optionalBufferStart : f_data; line_start < buf_end; )
    {
        const char* line_end = line_start;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r') line_end++;

        if (name[0]=='\0' && line_start[0] == '[' && line_end > line_start && line_end[-1] == ']')
        {
            ImFormatString(name, IM_ARRAYSIZE(name), "%.*s", (int)(line_end-line_start-2), line_start+1);
            //fprintf(stderr,"name: %s\n",name);  // dbg

            // Here we have something like: FLOAT-4:VariableName
            // We have to split into FLOAT 4 VariableName
            varName = NULL;numArrayElements = 0;ft = FT_COUNT;format[0]='\0';
            const char* colonCh = strchr(name,':');
            const char* minusCh = strchr(name,'-');
            if (!colonCh) {
                fprintf(stderr,"MemoryFile::parse(...) warning (skipping line with no semicolon). name: %s\n",name);  // dbg
                name[0]='\0';
            }
            else {
                ptrdiff_t diff = 0,diff2 = 0;
                if (!minusCh || (minusCh-colonCh)>0)  {diff = (colonCh-name);numArrayElements=1;}
                else {
                    diff = (minusCh-name);
                    diff2 = colonCh-minusCh;
                    if (diff2>1 && diff2<7)    {
                        static char buff[8];
                        strncpy(&buff[0],(const char*) &minusCh[1],diff2);buff[diff2-1]='\0';
                        sscanf(buff,"%d",&numArrayElements);
                        //fprintf(stderr,"WARN: %s\n",buff);
                    }
                    else if (diff>0) numArrayElements = ((char)name[diff+1]-(char)'0');  // TODO: FT_STRING needs multibytes -> rewrite!
                }
                if (diff>0) {
                    const size_t len = (size_t)(diff>31?31:diff);
                    strncpy(typeName,name,len);typeName[len]='\0';

                    for (int t=0;t<=FT_COUNT;t++) {
                        if (strcmp(typeName,FieldTypeNames[t])==0)  {
                            ft = (FieldType) t;break;
                        }
                    }
                    varName = ++colonCh;

                    if (ft==FT_COUNT || numArrayElements<1 || (numArrayElements>4 && ft!=FT_STRING))   {
                        fprintf(stderr,"MemoryFile::parse(...) Error (wrong type detected): line:%s type:%d numArrayElements:%d varName:%s typeName:%s\n",name,(int)ft,numArrayElements,varName,typeName);
                        varName=NULL;
                    }
                    else {

                        if (ft==FT_STRING && varName && varName[0]!='\0')  {
                            //Process soon here, as the string can be multiline
                            line_start = line_end+1;
                            //--------------------------------------------------------
                            for (int sp=0;sp<numArrayElements && line_end < buf_end;sp++) line_end++;
                            while (line_end < buf_end && *line_end != '\n' && *line_end != '\r') line_end++;
                            static char textBuffer[2050];
                            textBuffer[0]=textBuffer[2049]='\0';
                            strcpy(format,"%");
                            sprintf(&format[1],"%d",numArrayElements>2049?2049:numArrayElements);
                            strcat(format,"s");
                            //if (
                            sscanf(line_start,format,textBuffer)
                            //==numArrayElements)
                            ;quitParsing = cb(ft,numArrayElements,(void*)textBuffer,varName,userPtr);
                            //else fprintf(stderr,"MemoryFile::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
                            //--------------------------------------------------------
                            ft = FT_COUNT;name[0]='\0';varName=NULL; // mandatory
                        }
                        else {
                            format[0]='\0';
                            for (int t=0;t<numArrayElements;t++) {
                                if (t>0) strcat(format," ");
                                strcat(format,FieldTypeFormats[ft]);
                            }
                            // DBG:
                            //fprintf(stderr,"MemoryFile::parse(...) DBG: line:%s type:%d numArrayElements:%d varName:%s format:%s\n",name,(int)ft,numArrayElements,varName,format);  // dbg
                        }
                    }
                }
            }
        }
        else if (varName && varName[0]!='\0')
        {
            switch (ft) {
            case FT_FLOAT:
            case FT_COLOR:
            {
                float* p = (float*) voidBuffer;
                if ( (numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"MemoryFile::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case FT_DOUBLE:  {
                double* p = (double*) voidBuffer;
                if ( (numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"MemoryFile::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case FT_INT:
            case FT_BOOL:
            case FT_ENUM:
            {
                int* p = (int*) voidBuffer;
                if ( (numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"MemoryFile::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            case FT_UNSIGNED:  {
                unsigned* p = (unsigned*) voidBuffer;
                if ( (numArrayElements==1 && sscanf(line_start, format, p)==numArrayElements) ||
                     (numArrayElements==2 && sscanf(line_start, format, &p[0],&p[1])==numArrayElements) ||
                     (numArrayElements==3 && sscanf(line_start, format, &p[0],&p[1],&p[2])==numArrayElements) ||
                     (numArrayElements==4 && sscanf(line_start, format, &p[0],&p[1],&p[2],&p[3])==numArrayElements))
                     quitParsing = cb(ft,numArrayElements,voidBuffer,varName,userPtr);
                else fprintf(stderr,"MemoryFile::parse(...) Error converting value:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            }
            break;
            default:
            fprintf(stderr,"MemoryFile::parse(...) Warning missing value type:\"%s\" to type:%d numArrayElements:%d varName:%s\n",line_start,(int)ft,numArrayElements,varName);  // dbg
            break;
            }
            //---------------------------------------------------------------------------------
            name[0]='\0';varName=NULL; // mandatory
        }

        line_start = line_end+1;

        if (quitParsing) return line_start;
    }

    //------------------------------------------------
    return buf_end;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
void Serializer::clear() {if (f) {fclose(f);f=NULL;}}
bool Serializer::saveToFile(const char *filename)   {
    clear();
    f = fopen(filename,"wt");
    return (f);
}
Serializer::Serializer(const char *filename) {
    f=NULL;
    if (filename) saveToFile(filename);
}

template <typename T> inline static bool SaveTemplate(FILE* f,FieldType ft, const T* pValue, const char* name, int numArrayElements=1, int prec=-1)   {
    if (!f || ft==FT_COUNT || numArrayElements<0 || numArrayElements>4 || !pValue || !name || name[0]=='\0') return false;
    // name
    fprintf(f, "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) fprintf(f, "-%d",numArrayElements);
    fprintf(f, ":%s]\n",name);
    // value
    const char* precision = FieldTypeFormatsWithCustomPrecision[ft];
    for (int t=0;t<numArrayElements;t++) {
        if (t>0) fprintf(f," ");
        fprintf(f,precision,prec,pValue[t]);
    }
    fprintf(f,"\n\n");
    return true;
}
bool Serializer::save(FieldType ft, const float* pValue, const char* name, int numArrayElements,  int prec)   {
    IM_ASSERT(ft==FT_FLOAT || ft==FT_COLOR);
    return SaveTemplate<float>(f,ft,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const double* pValue,const char* name,int numArrayElements, int prec)   {
    return SaveTemplate<double>(f,FT_DOUBLE,pValue,name,numArrayElements,prec);
}
bool Serializer::save(FieldType ft,const int* pValue,const char* name,int numArrayElements, int prec) {
    IM_ASSERT(ft==FT_INT || ft==FT_BOOL || ft==FT_ENUM);
    if (prec==0) prec=-1;
    return SaveTemplate<int>(f,ft,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const unsigned* pValue,const char* name,int numArrayElements, int prec) {
    if (prec==0) prec=-1;
    return SaveTemplate<unsigned>(f,FT_UNSIGNED,pValue,name,numArrayElements,prec);
}
bool Serializer::save(const char* pValue,const char* name,int pValueSize)    {
    FieldType ft = FT_STRING;
    int numArrayElements = pValueSize;
    if (!f || ft==FT_COUNT || !pValue || !name || name[0]=='\0') return false;
    numArrayElements = pValueSize;
    pValueSize=(int)strlen(pValue);if (numArrayElements<pValueSize) numArrayElements=pValueSize;
    if (numArrayElements<0) numArrayElements=0;

    // name
    fprintf(f, "[%s",FieldTypeNames[ft]);
    if (numArrayElements==0) numArrayElements=1;
    if (numArrayElements>1) fprintf(f, "-%d",numArrayElements);
    fprintf(f, ":%s]\n",name);
    // value
    fprintf(f,"%s\n\n",pValue);
    return true;
}
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE

} //namespace ImGuiHelper
#endif //NO_IMGUIHELPER_SERIALIZATION




