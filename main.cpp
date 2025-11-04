#include <iostream>
#include <sstream>
#include <string>
#include "bpt.hpp"

using namespace std;

int main() {
    BPTree bpt("data.db");
    
    int n;
    cin >> n;
    cin.ignore();
    
    for (int i = 0; i < n; i++) {
        string line;
        getline(cin, line);
        istringstream iss(line);
        
        string cmd;
        iss >> cmd;
        
        if (cmd == "insert") {
            string idx;
            int val;
            iss >> idx >> val;
            bpt.insert(idx, val);
        } else if (cmd == "delete") {
            string idx;
            int val;
            iss >> idx >> val;
            bpt.del(idx, val);
        } else if (cmd == "find") {
            string idx;
            iss >> idx;
            vector<int> res = bpt.find(idx);
            
            if (res.empty()) {
                cout << "null" << endl;
            } else {
                for (size_t j = 0; j < res.size(); j++) {
                    if (j > 0) cout << " ";
                    cout << res[j];
                }
                cout << endl;
            }
        }
    }
    
    return 0;
}
