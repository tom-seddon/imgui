#ifndef IMGUIHELPER_H_
#define IMGUIHELPER_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API


namespace ImGui {

// Experimental: tested on Ubuntu only. Should work with urls, folders and files.
bool OpenWithDefaultApplication(const char* url,bool exploreModeForWindowsOS=false);

}   // ImGui



#ifndef NO_IMGUIHELPER_SERIALIZATION
// These classed are supposed to be used internally
#include <stdio.h> // FILE*. Why must I include this if only a FILE* appears in this header ?
namespace ImGuiHelper {

// IMPORTANT: the serialization/deserialization supports only 5 types: FT_INT,FT_UNSIGNED,FT_FLOAT,FT_DOUBLE and FT_STRING.
// The other types are just "hints" to the user that must convert the values.
enum FieldType {
    FT_INT=0,
    FT_UNSIGNED,
    FT_FLOAT,
    FT_DOUBLE,
    //--------------- End types that support 1 to 4 array components ----------
    FT_STRING,
    FT_ENUM,        // serialized/deserialized as FT_INT
    FT_BOOL,        // serialized/deserialized as FT_INT
    FT_COLOR,       // serialized/deserialized as FT_FLOAT
    FT_COUNT
};

#ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
class Deserializer {
    char* f_data;
    size_t f_size;
    void clear();
    bool loadFromFile(const char* filename);
    bool allocate(size_t sizeToAllocate,const char* optionalTextToCopy=NULL,size_t optionalTextToCopySize=0);
    public:
    Deserializer() : f_data(NULL),f_size(0) {}
    Deserializer(const char* filename);
    Deserializer(const char* text,size_t textSizeInBytes);
    ~Deserializer() {clear();}
    bool isValid() const {return (f_data && f_size>0);}

    // returns whether to stop parsing or not
    typedef bool (*ParseCallback)(FieldType ft,int numArrayElements,void* pValue,const char* name,void* userPtr);
    // returns a pointer to "next_line" if the callback has stopped parsing or NULL.
    // returned value can be refeed as optionalBufferStart
    const char *parse(ParseCallback cb,void* userPtr,const char* optionalBufferStart=NULL) const;

protected:
    void operator=(const Deserializer&) {}
    Deserializer(const Deserializer&) {}
};
#endif //NO_IMGUIHELPER_SERIALIZATION_LOAD

#ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
class Serializer {
    FILE* f;        // stdio.h
    void clear();
    bool saveToFile(const char* filename);
    public:
    Serializer(const char* filename=NULL);
    ~Serializer() {clear();}
    bool isValid() const {return (f);}

    bool save(FieldType ft, const float* pValue, const char* name, int numArrayElements=1);
    bool save(const double* pValue, const char* name, int numArrayElements=1);
    bool save(FieldType ft, const int* pValue, const char* name, int numArrayElements=1);
    bool save(const unsigned* pValue, const char* name, int numArrayElements=1);
    bool save(const float* pValue,const char* name,int numArrayElements=1)    {
        return save(FT_FLOAT,pValue,name,numArrayElements);
    }
    bool save(const int* pValue,const char* name,int numArrayElements)  {
        return save(FT_INT,pValue,name,numArrayElements);
    }
    bool save(const char* pValue,const char* name,int pValueSize=-1);

protected:
    void operator=(const Serializer&) {}
    Serializer(const Serializer&) {}

};
#endif //NO_IMGUIHELPER_SERIALIZATION_SAVE

} // ImGuiHelper
#endif //NO_IMGUIHELPER_SERIALIZATION


#endif //IMGUIHELPER_H_

