#ifndef IMHASHMAP_H_
#define IMHASHMAP_H_

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


#endif //IMHASHMAP_H_

