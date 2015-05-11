//
// Helper class for constructing indexed sets.
//


#ifndef INDEX_H_
#define INDEX_H_

#include <cereal/types/memory.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>

using namespace std;

struct Index {
    Index() : cur_index(0), p_cur_index(&cur_index) {
        p_fmap = &fmap;
        p_rmap = &rmap;
    }

    int get_or_add(string item) {
        if (fmap.find(item) != fmap.end()) {
            return fmap[item];
        } else {
            fmap[item] = cur_index;
            rmap[cur_index] = item;
            cur_index++;
            return cur_index - 1;
        }
    }

    int index(string item) const {
        if (fmap.find(item) != fmap.end()) {
            return fmap.at(item);
        } else {
            (*p_fmap)[item] = cur_index;
            (*p_rmap)[cur_index] = item;
            (*p_cur_index)++;
            return (*p_cur_index) - 1;
        }

        // if (fmap.find(item) != fmap.end()) {
        //     return fmap.at(item);
        // } else {
        //     // Debug only, should never come here
        //     cerr << "NOT HERE!" << endl;
        //     exit(1);
        //     // Debug only, should never come here
        //     return -1;
        // }
    }

    string get_string(int index) const {
        return rmap.at(index);
    }

    int size() const {
        return cur_index;
    }

    template <class Archive>
    void serialize(Archive &ar) {
        ar(fmap, rmap, cur_index);
    }

  private:
    int cur_index;
    int *p_cur_index;
    unordered_map<string, int> fmap;
    unordered_map<string, int> *p_fmap;
    unordered_map<int, string> rmap;
    unordered_map<int, string> *p_rmap;
};

#endif  // INDEX_H_
