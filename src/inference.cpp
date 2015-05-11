#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <bitset>
#include <map>

#include "grammar.hpp"
#include "features.hpp"
#include "inference.hpp"

using namespace std;


bitset<50000> have_nt;

Chart::Chart(int n, int N, const vector<string> *words, const Scorer *scorer) {
    n_ = n;
    N_ = N;
    words_ = words;
    scorer_ = scorer;
}

void Chart::promote(const Item &item, const Item &item1) {
    assert(has_item(item1));

    int index = index_item(item);
    int index1 = index_item(item1);

    double new_score = item_score_[index1][item1.nt];
    map<int, double>::iterator iter = item_score_[index].find(item.nt);
    if (iter == item_score_[index].end() || new_score > iter->second) {
        BackPointer &bp = bps_[index][item.nt];
        bp.item1 = item1;
        bp.promotion = true;
        if (iter == item_score_[index].end()) {
            item_score_[index][item.nt] = new_score;
            span_init(item);
        } else {
            iter->second = new_score;
        }
    }
}

void Chart::update(const Item &item,
                   const Item &item1, const Item &item2,
                   const AppliedRule &rule, double score1, double score2) {

    int index = index_item(item);
    double score = scorer_->score(rule);
    double new_score = score + score1 + score2;

    assert(has_item(item1) && has_item(item2));

    map<int, double>::iterator iter = item_score_[index].find(item.nt);
    if (iter == item_score_[index].end() || new_score > iter->second) {
        BackPointer &bp = bps_[index][item.nt];
        bp.item1 = item1;
        bp.item2 = item2;
        bp.rule = rule;

        if (iter == item_score_[index].end()) {
            item_score_[index][item.nt] = new_score;
            span_init(item);
        } else {
            iter->second = new_score;
        }
    }
}

void Chart::update(const Item &item,
                   const Item &item1, const AppliedRule &rule,
                   double score1) {

    assert(has_item(item1));
    double score = scorer_->score(rule);
    int index = index_item(item);
    double new_score = score + score1;
    map<int, double>::iterator iter = item_score_[index].find(item.nt);
    if (iter == item_score_[index].end() || new_score > iter->second) {
        BackPointer &bp = bps_[index][item.nt];
        bp.item1 = item1;
        bp.single = true;
        bp.rule = rule;
        if (iter == item_score_[index].end()) {
            item_score_[index][item.nt] = new_score;
            span_init(item);
        } else {
            iter->second = new_score;
        }
    }
}


Parser::Parser(const Sentence *sentence,
               const Grammar *grammar,
               const Scorer *scorer,
               const Pruning *pruner) : sentence_(sentence),
                                        grammar_(grammar),
                                        scorer_(scorer),
                                        pruner_(pruner) {

    assert(sentence->preterms.size() == sentence->words.size());
    assert(sentence->preterms.size() == sentence->deps.size());
    int n = sentence_->preterms.size();
    chart_ = new Chart(n, grammar_->n_nonterms(), &sentence_->words, scorer);

    left_.resize(n, -2);
    right_.resize(n, -2);
    children_.resize(n);
}


bool Parser::to_tree(const Item &item,
                     vector<AppliedRule> *best_rules, bool output,
                     stringstream &out) {

    BackPointer &bp = chart_->bps(item);
    bool success = true;
    if (bp.terminal) {
        assert(item.i == item.k);
        if (item.i != item.k) {
            cerr << "FAIL" << endl;
            return false;
        }

        if (output)
            out << " (" << grammar_->nonterm_index.get_string(item.nt)
                << " " << sentence_->words[item.i] << ") ";

    } else if (bp.single) {
        best_rules->push_back(bp.rule);
        string nt_string = grammar_->nonterm_index.get_string(item.nt);
        bool removable = nt_string.back() == '|';

        if (output && !removable) {
            out << " (" << grammar_->nonterm_index.get_string(item.nt) << " ";
        }
        success &= to_tree(bp.item1, best_rules, output, out);
        if (output && !removable) {
            out << ") ";
        }
    } else if (bp.promotion) {
        success &= to_tree(bp.item1, best_rules, output, out);
    } else {
        string nt_string = grammar_->nonterm_index.get_string(item.nt);
        if (!item.used || !bp.item1.used) {
            cerr << "FAIL" << endl;
            return false;
        }


        best_rules->push_back(bp.rule);

        bool removable = nt_string.back() == '|';
        if (output && !removable) {
            out << " (" <<  nt_string << " ";
        }

        success &= to_tree(bp.item1, best_rules, output, out);
        if (output)
            out << " ";
        success &= to_tree(bp.item2, best_rules, output, out);
        if (output && !removable) {
            out << ") ";
        }
    }

    return success;
}

void Parser::add_right(int i, int j, int k, int h, int m,
                      bool no_prune) {
    
    have_nt.reset();
    for (int Z : chart_->span_nts(Item(j+1, k, m, 0, kLayer2))) {
        have_nt[Z] = 1;
    }
    for (int Y : chart_->span_nts(Item(i, j, h, 0, kLayer2))) {
        Item item(i, j, h, Y, kLayer2);
        double item_score = chart_->score(item);
        for (const BinaryRule &b_rule : grammar_->rules_by_first[Y]) {
            int Z = b_rule.nt_Z;
            if (!no_prune && pruner_->prune(b_rule.rule_num,
                                            sentence_->preterms[h]))
                continue;
            if (have_nt[Z]) {
                Item other_item(j+1, k, m, Z, kLayer2);
                double other_item_score = chart_->score(other_item);
                Item new_item(i, k, h, b_rule.nt_X, kLayer0);
                AppliedRule rule(i, j, k, h, m, b_rule.rule_num);
                chart_->update(new_item, item, other_item, rule,
                              item_score, other_item_score);
            }
        }
    }
    
}

void Parser::add_left(int i, int j, int k, int h, int m,  bool no_prune) {
    
    have_nt.reset();
    for (int Y : chart_->span_nts(Item(i, j, m, 0, kLayer2))) {
        have_nt[Y] = 1;
    }
    for (int Z : chart_->span_nts(Item(j+1, k, h, 0, kLayer2))) {
        Item item(j+1, k, h, Z, kLayer2);
        double item_score = chart_->score(item);
        for (const BinaryRule &b_rule : grammar_->rules_by_second[Z]) {
            if (!no_prune && pruner_->prune(b_rule.rule_num,
                                            sentence_->preterms[h]))
                continue;
            if (have_nt[b_rule.nt_Y]) {
                Item other_item(i, j, m, b_rule.nt_Y, kLayer2);
                double other_item_score = chart_->score(other_item);
                Item new_item(i, k, h, b_rule.nt_X, kLayer0);
                AppliedRule rule(i, j, k, h, m, b_rule.rule_num);
                chart_->update(new_item, other_item, item,
                              rule, item_score, other_item_score);
            }
        }
    }
    
}

bool Parser::complete(int i, int k, int h,  bool no_prune) {
    bool any = false;

    // Fill in the unary rules.
    for (int Y : chart_->span_nts(Item(i, k, h, 0, kLayer0))) {
        any = true;
        Item item(i, k, h, Y, kLayer0);
        Item item_prom(i, k, h, Y, kLayer2);
        assert(chart_->has_item(item));
        chart_->promote(item_prom, item);

        double item_score = chart_->score(item);

        for (const UnaryRule &u_rule : grammar_->unary_rules_by_first[Y]) {
            if (!no_prune && pruner_->prune(u_rule.rule_num, sentence_->preterms[h]))
                continue;

            AppliedRule rule(i, k, k, h, h, u_rule.rule_num);
            Item item2(i, k, h, u_rule.nt_X, kLayer1);
            chart_->update(item2, item, rule, item_score);

            assert(chart_->has_item(item2));
            Item item2_prom(i, k, h, u_rule.nt_X, kLayer2);
            chart_->promote(item2_prom, item2);
        }
    }

    for (int Y : chart_->span_nts(Item(i, k, h, 0, kLayer1))) {
        Item item(i, k, h, Y, kLayer1);
        double item_score = chart_->score(item);

        for (const UnaryRule &u_rule : grammar_->unary_rules_by_first[Y]) {
            if (!no_prune && pruner_->prune(u_rule.rule_num, sentence_->preterms[h]))
                continue;

            AppliedRule rule(i, k, k, h, h, u_rule.rule_num);
            Item item2(i, k, h, u_rule.nt_X, kLayer2);
            chart_->update(item2, item, rule, item_score);
        }
    }
    return any;
}

void Parser::find_spans(int h) {
    (left_)[h] = h;
    (right_)[h] = h;

    for (unsigned i = 0; i < sentence_->deps.size(); ++i) {
        if (sentence_->deps[i] == h) {
            find_spans(i);
            children_[h].push_back(i);
            if (i < (unsigned)h) {
                left_[h] = min(left_[h], left_[i]);
            } else {
                right_[h] = max(right_[h], right_[i]);
            }
        }
    }
    ordered_.push_back(h);
}


double Parser::cky(bool output, bool no_prune) {
    int n = sentence_->preterms.size();

    int root_word = -1;
    for (unsigned i = 0; i < sentence_->deps.size(); ++i) {
        if (sentence_->deps[i] == -1) {
            root_word = i;
            find_spans(i);
        }
    }
    if (root_word == -1) {
        cerr << "no root!" << endl;
        exit(1);
    }

    // Initialize the chart.
    for (int i = 0; i < n; ++i) {
        int Y = sentence_->preterms[i];
        Item item(i, i, i, Y, kLayer0);
        chart_->init(item);
        complete(i, i, i, no_prune);
    }

    // Main loop.
    for (int h : ordered_) {
        vector<int> &deps = children_[h];
        vector<int> left_children;
        vector<int> right_children;

        for (int dep : deps) {
            if (dep < h) {
                left_children.insert(left_children.begin(), dep);
            } else {
                right_children.push_back(dep);
            }
        }

        // Unsigned weirdness.
        int L = left_children.size();
        int R = right_children.size();
        int head_tag = sentence_->preterms[h];
        for (int l = -1; l < L; ++l) {
            for (int r = -1; r < R; ++r) {

                int left_cur = (l != -1) ? left_[left_children[l]] : h;
                int right_cur = (r != -1) ? right_[right_children[r]] : h;
                bool any = complete(left_cur, right_cur, h, no_prune);
                if (!any) continue;

                int i, j, m, k;

                // Add a right child.
                bool try_right = r < R - 1;
                bool try_left = l < L - 1;

                if (try_right && try_left && !no_prune && pruner_->dir_pruning) {
                    // Both possible, but one may be better.
                    int right_tag = sentence_->preterms[right_children[r+1]];
                    int left_tag = sentence_->preterms[left_children[l+1]];
                    int left_pre = 0, right_pre = 0;

                    // Pruning.
                    if (l == -1) {
                        left_pre = 1;
                    } else if (sentence_->preterms[left_children[l]] ==
                               grammar_->nonterm_index.index(",")) {
                        left_pre = 2;
                    }
                    if (r == -1) {
                        right_pre = 1;
                    }  else if (sentence_->preterms[right_children[r]] ==
                                grammar_->nonterm_index.index(",")) {
                        right_pre = 2;
                    }
                    DirPrune prune(head_tag, left_tag, right_tag, left_pre, right_pre);
                    pruner_->dir_pick(prune, &try_left, &try_right);
                }

                if (try_right) {
                    i = left_cur;
                    m = right_children[r+1];
                    j = left_[m] - 1;
                    k = right_[m];
                    add_right(i, j, k, h, m, no_prune);
                }

                // Add a left child.
                if (try_left) {
                    k = right_cur;
                    m = left_children[l+1];
                    i = left_[m];
                    j = right_[m];
                    add_left(i, j, k, h, m, no_prune);
                }
            }
        }
    }


    int root = grammar_->roots;
    Item item(0, n-1, root_word, root, 2);

    stringstream out;
    success = to_tree(item, &best_rules, output, out);

    if (success && out) {
        if (output) {
            cout << out.str() << endl;
        }
    } else if (!no_prune) {
        cky(output, true);
    } else {
        // Corner case. Length 1.
        if (output && sentence_->words.size() == 1) {
            cout << "(TOP (X "<< sentence_->words[0] << ") )" << endl;
        } else if (output) {
            cout << endl;
        }
    }

    return chart_->score(item);
}
