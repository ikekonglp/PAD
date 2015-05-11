//
// A class for reading lexicalized binarized CFG.
//


#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include <string>
#include <assert.h>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <iostream>

#include "index.hpp"
#include "sentence.hpp"

using namespace std;

struct UnaryRule {
    UnaryRule() {}
    UnaryRule(int rule_num_, int nt_X_, int nt_Y_)
                : rule_num(rule_num_), nt_X(nt_X_), nt_Y(nt_Y_) {}
    int rule_num;
    int nt_X;
    int nt_Y;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(rule_num, nt_X, nt_Y);
    }
};

struct BinaryRule {
    BinaryRule() {}
    BinaryRule(int rule_num_, int nt_X_,
               int nt_Y_, int nt_Z_, int dir_)
            : rule_num(rule_num_), nt_X(nt_X_),
              nt_Y(nt_Y_), nt_Z(nt_Z_), dir(dir_)
      {}
    int rule_num;
    int nt_X;
    int nt_Y;
    int nt_Z;
    int dir;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(rule_num, nt_X, nt_Y, nt_Z, dir);
    }
};

struct AppliedRule {
    AppliedRule() {}
    AppliedRule(int i_, int j_, int k_,
                int h_, int m_, int rule_)
            : i(i_), j(j_), k(k_),
              h(h_), m(m_), rule(rule_) {}

    bool same_span(const AppliedRule &other) const{
        return other.i == i &&
                other.j == j &&
                other.k == k;
    }

    bool same_top(const AppliedRule &other) const{
        return other.i == i &&
                other.k == k;
    }

    bool same_non_head(const AppliedRule &other) const{
        return other.i == i &&
                other.j == j &&
                other.k == k &&
                other.rule == rule;
    }


    bool same(const AppliedRule &other) const{
        return other.i == i &&
                other.j == j &&
                other.k == k &&
                other.h == h &&
                other.m == m &&
                other.rule == rule;
    }

    int i, j, k, h, m, rule;
};

class Scorer {
  public:
    virtual double score(const AppliedRule &rule) const = 0;
};


class Grammar {
  public:
    Grammar() {}

    unsigned n_rules() const {
        return n_rules_;
    }

    unsigned n_nonterms() const;

    int to_nonterm(string nonterm);
    int to_nonterm(string nonterm) const;
    void add_rule(BinaryRule rule);

    void add_unary_rule(UnaryRule rule);
    void finish(int roots_);

    template <class Archive>
    void serialize(Archive &ar) {
        ar(unary_rules_by_first,
           rules_by_first,
           rules_by_second,
           roots,
           n_rules_,
           nonterm_index,
           head_symbol,
           left_symbol,
           right_symbol,
           is_unary);
    }

    vector<vector<UnaryRule> > unary_rules_by_first;
    vector<vector<BinaryRule> > rules_by_first;
    vector<vector<BinaryRule> > rules_by_second;

    int roots;
    unsigned n_rules_;

    Index nonterm_index;

    vector<int> head_symbol;
    vector<int> left_symbol;
    vector<int> right_symbol;
    vector<int> is_unary;
};

Grammar *read_rule_set(string file);

#endif  // GRAMMAR_H_
