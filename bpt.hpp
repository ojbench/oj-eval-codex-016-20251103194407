#ifndef BPT_HPP
#define BPT_HPP

#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int M = 100;
const int MAX_KEY = 65;

struct Key {
    char str[MAX_KEY];
    Key() { memset(str, 0, sizeof(str)); }
    Key(const string& s) {
        memset(str, 0, sizeof(str));
        strncpy(str, s.c_str(), MAX_KEY - 1);
    }
    int cmp(const Key& o) const { return strcmp(str, o.str); }
    bool operator<(const Key& o) const { return cmp(o) < 0; }
    bool operator>(const Key& o) const { return cmp(o) > 0; }
    bool operator<=(const Key& o) const { return cmp(o) <= 0; }
    bool operator==(const Key& o) const { return cmp(o) == 0; }
};

struct Record {
    Key key;
    int val;
    Record() : val(0) {}
    Record(const Key& k, int v) : key(k), val(v) {}
    bool operator<(const Record& o) const {
        int c = key.cmp(o.key);
        return c < 0 || (c == 0 && val < o.val);
    }
    bool operator==(const Record& o) const {
        return key == o.key && val == o.val;
    }
};

struct Node {
    bool leaf;
    int n;
    int next;
    Record keys[M];
    int ch[M + 1];
    
    Node() : leaf(true), n(0), next(-1) {
        for (int i = 0; i <= M; i++) ch[i] = -1;
    }
};

class BPTree {
private:
    string fname;
    fstream f;
    int root;
    int nodeCnt;
    
    void readNode(int pos, Node& nd) {
        f.seekg(8 + pos * sizeof(Node));
        f.read((char*)&nd, sizeof(Node));
    }
    
    void writeNode(int pos, const Node& nd) {
        f.seekp(8 + pos * sizeof(Node));
        f.write((const char*)&nd, sizeof(Node));
        f.flush();
    }
    
    int newNode() { return nodeCnt++; }
    
    void saveMeta() {
        f.seekp(0);
        f.write((const char*)&root, 4);
        f.write((const char*)&nodeCnt, 4);
        f.flush();
    }
    
    void split(int par, int idx) {
        Node p, c;
        readNode(par, p);
        readNode(p.ch[idx], c);
        
        Node nw;
        nw.leaf = c.leaf;
        int mid = M / 2;
        nw.n = c.n - mid;
        
        for (int i = 0; i < nw.n; i++) {
            nw.keys[i] = c.keys[mid + i];
            if (!c.leaf) nw.ch[i] = c.ch[mid + i];
        }
        if (!c.leaf) nw.ch[nw.n] = c.ch[c.n];
        
        int nwPos = newNode();
        if (c.leaf) {
            nw.next = c.next;
            c.next = nwPos;
        }
        c.n = mid;
        writeNode(nwPos, nw);
        
        for (int i = p.n; i > idx; i--) {
            p.keys[i] = p.keys[i - 1];
            p.ch[i + 1] = p.ch[i];
        }
        
        p.keys[idx] = nw.keys[0];
        p.ch[idx + 1] = nwPos;
        p.n++;
        
        writeNode(p.ch[idx], c);
        writeNode(par, p);
    }
    
    void insertNF(int pos, const Record& rec) {
        Node nd;
        readNode(pos, nd);
        
        if (nd.leaf) {
            for (int i = 0; i < nd.n; i++) {
                if (nd.keys[i] == rec) return;
            }
            
            int i = nd.n - 1;
            while (i >= 0 && rec < nd.keys[i]) {
                nd.keys[i + 1] = nd.keys[i];
                i--;
            }
            nd.keys[i + 1] = rec;
            nd.n++;
            writeNode(pos, nd);
        } else {
            int i = 0;
            while (i < nd.n && !(rec.key < nd.keys[i].key)) i++;
            
            int cpos = nd.ch[i];
            Node c;
            readNode(cpos, c);
            
            if (c.n == M) {
                split(pos, i);
                readNode(pos, nd);
                i = 0;
                while (i < nd.n && !(rec.key < nd.keys[i].key)) i++;
                cpos = nd.ch[i];
            }
            insertNF(cpos, rec);
        }
    }
    
    bool delHelper(int pos, const Record& rec) {
        Node nd;
        readNode(pos, nd);
        
        if (nd.leaf) {
            for (int i = 0; i < nd.n; i++) {
                if (nd.keys[i] == rec) {
                    for (int j = i; j < nd.n - 1; j++) {
                        nd.keys[j] = nd.keys[j + 1];
                    }
                    nd.n--;
                    writeNode(pos, nd);
                    return true;
                }
            }
            return false;
        }
        
        int i = 0;
        while (i < nd.n && !(rec.key < nd.keys[i].key)) i++;
        return delHelper(nd.ch[i], rec);
    }
    
public:
    BPTree(const string& fn) : fname(fn) {
        ifstream test(fname);
        bool ex = test.good();
        test.close();
        
        if (ex) {
            f.open(fname, ios::in | ios::out | ios::binary);
            f.read((char*)&root, 4);
            f.read((char*)&nodeCnt, 4);
        } else {
            f.open(fname, ios::in | ios::out | ios::binary | ios::trunc);
            root = 0;
            nodeCnt = 1;
            Node r;
            writeNode(0, r);
            saveMeta();
        }
    }
    
    ~BPTree() {
        if (f.is_open()) {
            saveMeta();
            f.close();
        }
    }
    
    void insert(const string& k, int v) {
        Record rec(Key(k), v);
        Node r;
        readNode(root, r);
        
        if (r.n == M) {
            Node nr;
            nr.leaf = false;
            nr.n = 0;
            nr.ch[0] = root;
            int nrPos = newNode();
            writeNode(nrPos, nr);
            
            split(nrPos, 0);
            root = nrPos;
            saveMeta();
            
            insertNF(root, rec);
        } else {
            insertNF(root, rec);
        }
    }
    
    vector<int> find(const string& k) {
        vector<int> res;
        Key sk(k);
        
        int pos = root;
        Node nd;
        readNode(pos, nd);
        
        // Navigate to LEFTMOST leaf that could contain key
        while (!nd.leaf) {
            int i = 0;
            while (i < nd.n && sk > nd.keys[i].key) i++;
            pos = nd.ch[i];
            readNode(pos, nd);
        }
        
        // Scan through leaves - limit iterations for safety
        int maxLeaves = 10000; // Safety limit
        while (maxLeaves-- > 0) {
            for (int i = 0; i < nd.n; i++) {
                if (nd.keys[i].key == sk) {
                    res.push_back(nd.keys[i].val);
                } else if (nd.keys[i].key > sk && !res.empty()) {
                    // Found all matches
                    goto done;
                }
            }
            
            // Move to next leaf if exists
            if (nd.next >= 0) {
                pos = nd.next;
                readNode(pos, nd);
            } else {
                break;
            }
        }
        
        done:
        sort(res.begin(), res.end());
        return res;
    }
    
    void del(const string& k, int v) {
        Record rec(Key(k), v);
        delHelper(root, rec);
    }
};

#endif
