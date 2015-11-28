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
// We can't define ImHashMap here... not compatible with STL
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


#ifndef ImHashMap
typedef unsigned char ImHashInt;  // optimized for MAX_HASH_INT = 256 (this was we can skip clamping the hash: hash%=MAX_HASH_INT)

// Default hash function class (optional, just to ease ImHashMap usage)
template <typename K> struct ImHashFunctionDefault {
    ImHashInt operator()(const K& key) const {
        return reinterpret_cast<ImHashInt>(key);
    }
};
// Hash map class template
template <typename K, typename V, typename F = ImHashFunctionDefault<K>,int MAX_HASH_INT = 256 > class ImHashMap
{
protected:
struct HashNode {
    K key;
    V value;
    HashNode *next; // next HashNode with the same key
    friend class HashMap;
};
typedef HashNode HashNode;
public:
    ImHashMap() {
        hashNodes.resize(MAX_HASH_INT);
        for (int i=0,isz=hashNodes.size();i<isz;i++) hashNodes[i]=NULL;
        mustClampHash = (sizeof(ImHashInt)!=8 || MAX_HASH_INT<256);
        IM_ASSERT(!(sizeof(ImHashInt)==8 && MAX_HASH_INT>256));    // Tip: It's useless to have MAX_HASH_INT>256, if you don't change the type ImHashInt.
    }
    ~ImHashMap() {
        clear();
        hashNodes.clear();
    }

    inline void clear() {
         for (int i = 0,isz=hashNodes.size(); i < isz; ++i) {
            HashNode*& node = hashNodes[i];
            while (node) {
                HashNode*& prev = node;
                node = node->next;
                //prev->~NodeType();          // ImVector does not call it
                ImGui::MemFree(prev);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
            }
            node = NULL;
        }
    }

    inline bool get(const K &key, V &value) const {
        ImHashInt hash = hashFunc(key);
        if (mustClampHash) hash%=MAX_HASH_INT;
        //fprintf(stderr,"ImHashMap::get(...): hash = %d\n",(int)hash);   // Dbg
        IM_ASSERT(hash>=0 && hash<hashNodes.size());    // MAX_HASH_INT too low
        HashNode* node = hashNodes[hash];
        while (node) {
            if (node->key == key) {value = node->value;return true;}
            node = node->next;
        }
        return false;
    }

    inline void put(const K &key, const V &value) {
        ImHashInt hash = hashFunc(key);
        if (mustClampHash) hash%=MAX_HASH_INT;
        //fprintf(stderr,"ImHashMap::put(...): hash = %d\n",(int)hash);   // Dbg
        IM_ASSERT(hash>=0 && hash<hashNodes.size());    // MAX_HASH_INT too low
        HashNode* node = hashNodes[hash];
        HashNode* prev = NULL;
        while (node && node->key!=key) {prev = node;node = node->next;}
        if (!node) {
            // key not found
            node = (HashNode*) ImGui::MemAlloc(sizeof(HashNode));   // items MUST be free by the user using ImGui::MemFree(...)
            //new (node) HashNode(key,value);                         // ImVector does not call it
            node->key=key;node->value=value;node->next=NULL;
            if (!prev) hashNodes[hash] = node;                 // first bucket slot
            else prev->next = node;
        }
        else node->value = value;                           // key in the hashMap already
    }

    inline void remove(const K &key) {
        ImHashInt hash = hashFunc(key);
        if (mustClampHash) hash%=MAX_HASH_INT;
        //fprintf(stderr,"ImHashMap::remove(...): hash = %d\n",(int)hash);   // Dbg
        IM_ASSERT(hash>=0 && hash<hashNodes.size());    // MAX_HASH_INT too low
        HashNode *node = hashNodes[hash];
        HashNode *prev = NULL;
        while (node && node->key != key) {prev = node; node = node->next;}
        if (!node) return; // key not found
        else {
            if (!prev) hashNodes[hash] = node->next;  // node in the first bucket slot -> replace the entry with node->next
            else prev->next = node->next;             // bypass node
            // delete node
            //node->~NodeType();          // ImVector does not call it
            ImGui::MemFree(node);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
        }
    }

protected:
    ImVector<HashNode* > hashNodes; // Since MAX_HASH_INT is constant, we could have just used an array...
    F hashFunc;
    bool mustClampHash;             // This could be static...
};

// Default hash function class specialized for strings (actually char*)
struct ImHashFunctionString {
    ImHashInt operator()(const char* s) const {
        ImHashInt h = 0;
        for (const char* t=s;*t!='\0';t++) h+=*t;   // We just sum up the chars (naive hash)
        return h;
    }
};

// ImHashMap specialized for strings (actually char*)
// Important the keys must be persistent somewhere (they are not owned, because they are not copied for performance reasons).
template <int MAX_HASH_INT = 256> class ImHashMapStringBase : protected ImHashMap<const char*,int,ImHashFunctionString,MAX_HASH_INT> {
    protected:
    typedef ImHashMap<const char*,int,ImHashFunctionString,MAX_HASH_INT> Base;
    typedef typename Base::HashNode HashNode;
    typedef const char* K;
    typedef int V;

    public:
    inline ImHashMapStringBase() : Base() {}
    inline bool get(const K &key, V &value) const {
        ImHashInt hash = Base::hashFunc(key);
        if (Base::mustClampHash) hash%=MAX_HASH_INT;
        //fprintf(stderr,"ImHashMap::get(...): hash = %d\n",(int)hash);   // Dbg
        IM_ASSERT(hash<Base::hashNodes.size());    // MAX_HASH_INT too low
        HashNode* node = Base::hashNodes[hash];
        while (node) {
            if (strcmp(node->key,key)==0) {value = node->value;return true;}
            node = node->next;
        }
        return false;
    }
    inline void put(const K &key, const V &value) {
        ImHashInt hash = Base::hashFunc(key);
        if (Base::mustClampHash) hash%=MAX_HASH_INT;
        //fprintf(stderr,"ImHashMap::put(...): hash = %d\n",(int)hash);   // Dbg
        IM_ASSERT(hash<Base::hashNodes.size());    // MAX_HASH_INT too low
        HashNode* node = Base::hashNodes[hash];
        HashNode* prev = NULL;
        while (node && strcmp(node->key,key)!=0) {prev = node;node = node->next;}
        if (!node) {
            // key not found
            node = (HashNode*) ImGui::MemAlloc(sizeof(HashNode));   // items MUST be free by the user using ImGui::MemFree(...)
            //new (node) HashNode(key,value);                         // ImVector does not call it
            node->key=key;node->value=value;node->next=NULL;        // node->key=key; => The string is NOT copied and must survive
            if (!prev) Base::hashNodes[hash] = node;                 // first bucket slot
            else prev->next = node;
        }
        else node->value = value;                           // key in the hashMap already
    }
    inline void remove(const K &key) {
        ImHashInt hash = Base::hashFunc(key);
        if (Base::mustClampHash) hash%=MAX_HASH_INT;
        //fprintf(stderr,"ImHashMap::remove(...): hash = %d\n",(int)hash);   // Dbg
        IM_ASSERT(hash<Base::hashNodes.size());    // MAX_HASH_INT too low
        HashNode *node = Base::hashNodes[hash];
        HashNode *prev = NULL;
        while (node && strcmp(node->key,key)!=0) {prev = node; node = node->next;}
        if (!node) return; // key not found
        else {
            if (!prev) Base::hashNodes[hash] = node->next;  // node in the first bucket slot -> replace the entry with node->next
            else prev->next = node->next;             // bypass node
            // delete node
            //node->~NodeType();          // ImVector does not call it
            ImGui::MemFree(node);       // items MUST be allocated by the user using ImGui::MemAlloc(...)
        }
    }
    inline void clear() {Base::clear();}
};

struct ImHashFunctionChar {
    inline ImHashInt operator()(char key) const {
        return (ImHashInt) (key);
    }
};

// Finally, here are the two types that are actually used inside imguicodeeditor:
typedef ImHashMapStringBase<256>            ImHashMapString;    // map <const char* (not owned),int>.   We can change the int to comsume less memory if needed
typedef ImHashMap<char,int,ImHashFunctionChar,256>       ImHashMapChar;      // map <char,int>.                We can change the int to comsume less memory if needed
#endif //ImHashMap

#endif //IMGUISTRING_H_



