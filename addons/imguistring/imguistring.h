#ifndef IMGUISTRING_H_
#define IMGUISTRING_H_

#include <imgui.h>
#include <string.h> //strlen


//#define IMGUISTRING_STL_FALLBACK  // Never tested
#ifdef IMGUISTRING_STL_FALLBACK
#include <string>
#include <vector>
#define ImString std::string
#define ImVectorEx std::vector
#endif //IMGUISTRING_STL_FALLBACK


#ifndef ImString
// A string implementation based on ImVector<char>
class ImString : public ImVector<char> {
typedef ImVector<char> base;

public:
inline ImString(int reservedChars=-1) : base() {if (reservedChars>0) base::reserve(reservedChars+1);base::resize(1);operator[](0)='\0';}
inline ImString(const ImString& other) : base() {base::resize(1);operator[](0)='\0';*this = other;}
inline ImString(const char* str) : base() {
    base::resize(1);operator[](0)='\0';
    *this = str;
}
inline int size() const {
    if (base::size()<1) return 0;
    return base::size()-1;
}
inline int length() const {
    return size();
}
inline bool empty() const {
    return size()==0;
}

inline const char* c_str() const {
    return (internalSize()>0 ? &operator[](0) : NULL);
}
inline int compare(const ImString& other) const  {
    return (empty() ? -1 : other.empty() ? 1 : strcmp(c_str(),other.c_str()));
}
inline bool operator==(const ImString& other) const {
    if (internalSize()!=other.internalSize()) return false;
    //for (int i=0,sz=internalSize();i<sz;i++) {if (operator[](i)!=other[i]) return false;}
    //return true;
    return (compare(other)==0);
}
inline bool operator!=(const ImString& other) const {
    return !operator==(other);
}
inline bool operator<(const ImString& other) const  {
    return compare(other)<0;
}


inline const ImString& operator=(const char* other) {
    //printf("operator=(const char* other) START: \"%s\" \"%s\"",this->c_str(),other);
    if (!other) return *this;
    const int len = strlen(other);
    base::resize(len+1);
    for (int i=0;i<len;i++) operator[](i) = other[i];   // strcpy ?
    operator[](len) = '\0';
    //printf("operator=(const char* other) END: \"%s\"\n",this->c_str());
    return *this;
}
inline const ImString& operator=(const ImString& other) {
    //printf("operator=(const ImString& other) START: \"%s\" \"%s\"",this->c_str(),other.c_str());
    //base::operator=(other); // Warning: ImVector has NO DEFAULT ASSIGNMENT ATM
    // -----------------------
    if (other.size()==0) *this="";
    else operator=(other.c_str());
    // -----------------------
    //printf("operator=(const ImString& other) END: \"%s\"\n",this->c_str());
    return *this;
}

inline const ImString operator+(const ImString& other) const {
    ImString rv(*this);
    return rv+=other;
}
inline const ImString& operator+=(const ImString& other) {
    const int curSize = size();
    if (curSize==0) return operator=(other);
    const int len = other.size();
    base::resize(curSize + len+1);
    for (int i=curSize;i<curSize+len;i++) operator[](i) = other[i-curSize];
    operator[](curSize+len) = '\0';
    return *this;

}
inline const ImString& operator+=(const char* other) {
    if (!other) return *this;
    const int curSize = size();
    if (curSize==0) return operator=(other);
    const int len = strlen(other);
    base::resize(curSize + len+1);
    for (int i=curSize;i<curSize+len;i++) operator[](i) = other[i-curSize];
    operator[](curSize+len) = '\0';
    return *this;
}
inline const ImString& operator+=(const char c) {
    const int curSize = internalSize();
    if (curSize==0) {
        resize(2);
        operator[](0) = c;
        operator[](1) = '\0';
    }
    else    {
        base::resize(curSize + 1);
        operator[](curSize-1) = c;
        operator[](curSize) = '\0';
    }
    return *this;
}

static const int npos = -1;


inline int find(const char c,int beg = 0) const  {
    for (int i=beg,sz = size();i<sz;i++)    {
        if (operator[](i) == c) return i;
    }
    return npos;
}
inline int find_first_of(const char c,int beg = 0) const {
    return find(c,beg);
}
inline int find_last_of(const char c,int beg = 0) const  {
    for (int i=size()-1;i>=beg;i--)    {
        if (operator[](i) == c) return i;
    }
    return npos;
}
inline int find(const ImString& s,int beg = 0) const  {
    int i,j,sz;
    const int ssize = s.size();
    if (ssize == 0 || beg+ssize>=size()) return -1;
    for (i=beg,sz = size()-beg;i<sz;i++)    {
        for (j=0;j<ssize;j++)   {
            if (operator[](i+j) != s.operator [](j)) break;
            if (j==ssize-1) return i;
        }
    }
    return npos;
}
inline int find_first_of(const ImString& s,int beg = 0) const {
    return find(s,beg);
}
// not tested:
inline int find_last_of(const ImString& s,int beg = 0) const  {
    int i,j;
    const int ssize = s.size();
    if (ssize == 0 || beg+ssize>=size()) return -1;
    for (i=size()-ssize-1;i>=beg;i--)    {
        for (j=0;j<ssize;j++)   {
            if (operator[](i+j) != s.operator [](j)) break;
            if (j==ssize-1) return i;
        }
    }
    return npos;
}

inline const ImString substr(int beg,int cnt=-1) const {
    const int sz = size();
    if (beg>=sz) return ImString("");
    if (cnt==-1) cnt = sz - beg;
    ImString rv;rv.resize(cnt+1);
    for (int i=0;i<cnt;i++) {
        rv.operator [](i) = this->operator [](beg+i);
    }
    rv.operator [](cnt) = '\0';
    return rv;
}

protected:

private:
inline int internalSize() const {
    return base::size();
}
inline void reserve(int i) {
    return base::reserve(i);
}
inline void resize(int i) {base::resize(i);}
inline void clear() {base::clear();}
inline int findLinearSearch(const char c) {
    for (int i=0,sz=size();i<sz;i++)    {
        if (c == operator[](i)) return i;
    }
    return npos;
}
inline void push_back(const char c) {
    return base::push_back(c);
}

//TODO: redefine all the other methods we want to hide here...

};
#endif //ImString


#ifndef ImVectorEx
#include <new>
// Attempt to make ImVector call ctr/dstr on elements and to make them proper copy (without using memcpy/memmove):
template<typename T>
class ImVectorEx
{
public:
    int                         Size;
    int                         Capacity;
    T*                          Data;

    typedef T                   value_type;
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    ImVectorEx(int size=0)      { Size = Capacity = 0; Data = NULL;if (size>0) resize(size);}
    ~ImVectorEx()               { clear(); }

    inline bool                 empty() const                   { return Size == 0; }
    inline int                  size() const                    { return Size; }
    inline int                  capacity() const                { return Capacity; }

    inline value_type&          operator[](int i)               { IM_ASSERT(i < Size); return Data[i]; }
    inline const value_type&    operator[](int i) const         { IM_ASSERT(i < Size); return Data[i]; }

    void                        clear()                         {
        if (Data) {
            for (int i=0,isz=Size;i<isz;i++) Data[i].~T();
            ImGui::MemFree(Data);
            Data = NULL;
            Size = Capacity = 0;
        }
    }
    inline iterator             begin()                         { return Data; }
    inline const_iterator       begin() const                   { return Data; }
    inline iterator             end()                           { return Data + Size; }
    inline const_iterator       end() const                     { return Data + Size; }
    inline value_type&          front()                         { IM_ASSERT(Size > 0); return Data[0]; }
    inline const value_type&    front() const                   { IM_ASSERT(Size > 0); return Data[0]; }
    inline value_type&          back()                          { IM_ASSERT(Size > 0); return Data[Size-1]; }
    inline const value_type&    back() const                    { IM_ASSERT(Size > 0); return Data[Size-1]; }

    inline int                  _grow_capacity(int new_size) const   { int new_capacity = Capacity ? (Capacity + Capacity/2) : 8; return new_capacity > new_size ? new_capacity : new_size; }

    void                        resize(int new_size)            {
        if (new_size > Capacity) {
            reserve(_grow_capacity(new_size));
            for (int i=Size;i<new_size;i++) {
                new(&Data[i]) T();
            }
        }
        if (new_size < Size)   {
            for (int i=new_size;i<Size;i++) {
                Data[i].~T();
            }
        }
        Size = new_size;
    }
    void                        reserve(int new_capacity)
    {
        if (new_capacity <= Capacity) return;
        T* new_data = (value_type*)ImGui::MemAlloc((size_t)new_capacity * sizeof(value_type));
        for (int i=0;i<Size;i++) {
            new(&new_data[i]) T();       // Is this dstr/ctr pair really needed or can I just copy...?
            new_data[i] = Data[i];
            Data[i].~T();
        }
        //memcpy(new_data, Data, (size_t)Size * sizeof(value_type));
        ImGui::MemFree(Data);
        Data = new_data;
        Capacity = new_capacity;
    }

    inline void                 push_back(const value_type& v)  {
        if (Size == Capacity) reserve(_grow_capacity(Size+1));
        new(&Data[Size]) T();
        Data[Size++] = v;
    }
    inline void                 pop_back()                      {
        IM_ASSERT(Size > 0);
        if (Size>0) {
            Size--;
            Data[Size].~T();
        }
    }

    const ImVectorEx<T>& operator=(const ImVectorEx<T>& o)  {
        resize(o.Size);
        for (int i=0;i<o.Size;i++) (*this)[i]=o[i];
        return *this;
    }

    // Not too sure about this
    inline void                 swap(ImVectorEx<T>& rhs)          { int rhs_size = rhs.Size; rhs.Size = Size; Size = rhs_size; int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; value_type* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }


private:

    // These 2 does not work: should invoke the dstr and cstr, and probably they should not use memmove
    inline iterator             erase(const_iterator it)        {
        IM_ASSERT(it >= begin() && it < end());
        const ptrdiff_t off = it - begin();
        memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(value_type));
        Size--;
        return Data + off;
    }
    inline iterator             insert(const_iterator it, const value_type& v)  {
        IM_ASSERT(it >= begin() && it <= end());
        const ptrdiff_t off = it - begin();
        if (Size == Capacity) reserve(Capacity ? Capacity * 2 : 4);
        if (off < (int)Size) memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(value_type));
        Data[off] = v;
        Size++;
        return Data + off;
    }
};
#endif //ImVectorEx


// ImHashMap moved to imguicodeeditor/imhashmap.h (it was not API-compatible with STL)



#endif //IMGUISTRING_H_



