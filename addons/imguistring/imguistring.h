#include <imgui.h>
#include <string.h> //strlen

#include <stdio.h> // printf (to remove)

//#define ImString std::string<char>    //that's what most users want...
#ifndef ImString
// Warning: USE IT AT YOUR OWN RISK!
class ImString : public ImVector<char> {
typedef ImVector<char> base;

public:
inline ImString() : base() {base::resize(1);operator[](0)='\0';}
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
    if (other.size()==0) operator=('\0');
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




