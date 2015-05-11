#ifndef INFERENCE_H_
#define INFERENCE_H_


#include "grammar.hpp"
#include "pruning.hpp"
#include "sentence.hpp"
#include "features.hpp"
#include <vector>
using namespace std;

struct Item {
    Item() : used(false) {}
    Item(int i_, int k_, int h_, int nt_, int layer_)
            : i(i_), k(k_), h(h_), nt(nt_), layer(layer_), used(true) {}
    int i;
    int k;
    int h;
    int nt;
    int layer;
    bool used;
};


struct BackPointer {
    BackPointer() : terminal(false), single(false), promotion(false) {}
    bool terminal;
    bool single;
    bool promotion;
    Item item1;
    Item item2;
    AppliedRule rule;
};

const int kLayers = 3;
const int kLayer0 = 0;
const int kLayer1 = 1;
const int kLayer2 = 2;


class Chart {
  public:
    Chart(int n, int N, const vector<string> *words, const Scorer *scorer);
    void promote(const Item &item, const Item &item1);
    void update(const Item &item, const Item &item1, const Item &item2,
                const AppliedRule &rule, double score1, double score2);

    void update(const Item &item, const Item &item1,
                const AppliedRule &rule, double score1);


    inline bool has_item(const Item &item)  {
        int index = index_item(item);
        return item_score_[index].find(item.nt) != item_score_[index].end();
    }

    inline double score(const Item &item) {
        int index = index_item(item);
        return item_score_[index][item.nt];
    }

    void init(const Item &item) {
        span_init(item);
        int index = index_item(item);
        BackPointer &bp = bps_[index][item.nt];
        bp.terminal = true;
        item_score_[index][item.nt] = 0.0;
    }

    inline void span_init(const Item &item) {
        span_nts_[index_item(item)].push_back(item.nt);
    }

    const vector<int> & span_nts(const Item &item) {
        return span_nts_[index_item(item)];
    }

    BackPointer &bps(const Item &item) {
        int index = index_item(item);
        BackPointer &bp = bps_[index][item.nt];
        return bp;
    }

    map<int, vector<int> > span_nts_;
  private:

    int index_item(const Item &item) const {
        // 3 * item.nt
        return kLayers * n_ * n_ *  item.i +  kLayers * n_ * item.k +
                kLayers * item.h +  item.layer;
    }

    int n_;
    int N_;
    map<int, map<int, double> > item_score_;
    map<int, map<int, BackPointer> > bps_;
    const vector<string> *words_;
    const Scorer *scorer_;
};

class Parser {
  public:

    Parser(const Sentence *sentence,
           const Grammar *grammar,
           const Scorer *scorer,
           const Pruning *pruner);


    ~Parser() {
        delete chart_;
    }

    double cky(bool output, bool no_prune);

    vector<AppliedRule> best_rules;
    bool success;

  private:
    bool complete(int i, int k, int h, bool no_prune);
    void add_right(int i, int j, int k, int h, int m, bool no_prune);
    void add_left(int i, int j, int k, int h, int m,  bool no_prune);
    void find_spans(int h);

    bool to_tree(const Item &item,
                 vector<AppliedRule> *best_rules, bool output,
                 stringstream &out);


    const Sentence *sentence_;
    const Grammar *grammar_;
    const Scorer *scorer_;
    const Pruning *pruner_;

    Chart *chart_;


    // Preprocessed Deps.
    vector<int> left_, right_;
    vector<vector<int> > children_;
    vector<int> ordered_;
};

#endif  // INFERENCE_H_
