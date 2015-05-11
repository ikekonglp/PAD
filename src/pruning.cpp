#include "pruning.hpp"
#include "grammar.hpp"

#include <fstream>

// void Pruning::read_label_pruning(string file) {
//     ifstream in_file;
//     label_pruning = true;
//     in_file.open(file.c_str());

//     if (in_file.is_open()) {
//         string label;
//         while (in_file >> label) {
//             int label_id = lexicon_->deplabel_index.index(label);
//             int allowed_rules;
//             in_file >> allowed_rules;
//             for (int i = 0; i < allowed_rules; ++i) {
//                 int rule_num;
//                 in_file >> rule_num;
//                 label_rule_allowed[label_id][rule_num] = 1;
//             }
//         }
//     }
// }

void Pruning::build_pruning(const vector<Sentence> &sentences,
                            const Grammar *grammar) {
    rule_head_tags.resize(grammar->n_rules() + 1);
    for (unsigned i = 0; i < grammar->n_rules(); ++i) {
        rule_head_tags[i].resize(grammar->n_nonterms(), 0);
    }

    for (auto sentence : sentences) {
        for (auto rule : sentence.gold_rules) {
            int head = sentence.preterms[rule.h];
            rule_head_tags[rule.rule][head] = 1;
        }
    }
}


void Pruning::build_dir_pruning(const vector<Sentence> &sentences) {

    for (auto sentence : sentences) {
        int n = sentence.words.size();
        vector<vector<int>> right_deps(n), left_deps(n);
        vector<unsigned> seen_left(n, 0);
        vector<unsigned> seen_right(n, 0);
        for (int i = 0; i < n; ++i) {
            int d = sentence.deps[i];
            if (d == -1) continue;
            if (i < d)
                left_deps[d].insert(left_deps[d].begin(), i);
            else
                right_deps[d].push_back(i);
        }

        for (auto rule : sentence.gold_rules) {
            int head = sentence.preterms[rule.h];
            // Skip unary.
            if (rule.h == rule.m) continue;

            int mod = sentence.preterms[rule.m];

            int left_prev=0, right_prev=0;
            if (seen_left[rule.h] == 0) {
                left_prev = 1;
            } else if (sentence.preterms[left_deps[rule.h][seen_left[rule.h]-1]] ==
                       grammar_->to_nonterm(",")) {
                left_prev = 2;
            }

            if (seen_right[rule.h] == 0) {
                right_prev = 1;
            } else if (sentence.preterms[right_deps[rule.h][seen_right[rule.h]-1]] ==
                       grammar_->to_nonterm(",")) {
                right_prev = 2;
            }

            if (rule.m < rule.h) {
                if (seen_right[rule.h] == right_deps[rule.h].size())
                    continue;
                int right_mod = sentence.preterms[right_deps[rule.h][seen_right[rule.h]]];
                DirPrune prune(head, mod, right_mod,
                               (seen_left[rule.h] == 0),
                               (seen_right[rule.h] == 0));
                dir_pruner[prune].first += 1;
                seen_left[rule.h] += 1;
            } else {
                if (seen_left[rule.h] == left_deps[rule.h].size())
                    continue;
                int left_mod = sentence.preterms[left_deps[rule.h][seen_left[rule.h]]];
                DirPrune prune(head, left_mod, mod,
                               (seen_left[rule.h] == 0),
                               (seen_right[rule.h] == 0));
                dir_pruner[prune].second += 1;
                seen_right[rule.h] += 1;
            }
        }
    }
}


double Pruning::dir_pick(const DirPrune &prune,
                         bool *try_left, bool *try_right) const {
    (*try_left) = true;
    (*try_right) = true;

    map<DirPrune, Pair>::const_iterator test = dir_pruner.find(prune);
    if (test == dir_pruner.end()) {
        // (*try_left) = false;
        return 0.001;
    }
    Pair pair = test->second;
    int total = pair.first + pair.second;

    if (total == 0) {
        return 0.0;
    }

    // cerr << "counts: " << (pair.first / (float) total) << endl;
    if ((pair.first / (float) total) > 0.95) {
        *try_right = false;
        return pair.first / (float) total;
    }

    if ((pair.second / (float) total) > 0.95) {
        *try_left = false;
        return pair.second / (float) total;
    }
    return max(pair.first, pair.second) / (float) total;
}
