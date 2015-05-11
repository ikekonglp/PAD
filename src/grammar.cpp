#include "grammar.hpp"
#include <iostream>
#include <fstream>
using namespace std;

int Grammar::to_nonterm(string nonterm) const {
    return nonterm_index.index(nonterm);
}

int Grammar::to_nonterm(string nonterm) {
    return nonterm_index.get_or_add(nonterm);
}

unsigned Grammar::n_nonterms() const {
    return nonterm_index.size();
}

void Grammar::add_rule(BinaryRule rule) {
    if (rules_by_first.size() <= n_nonterms()) {
        rules_by_first.resize(n_nonterms() + 1);
    }
    if (rules_by_second.size() <= n_nonterms()) {
        rules_by_second.resize(n_nonterms() + 1);
    }

    head_symbol.push_back(rule.nt_X);
    left_symbol.push_back(rule.nt_Y);
    right_symbol.push_back(rule.nt_Z);
    is_unary.push_back(0);

    if (rule.dir == 0) {
        rules_by_first[rule.nt_Y].push_back(rule);
    } else {
        rules_by_second[rule.nt_Z].push_back(rule);
    }

    n_rules_++;
}

void Grammar::add_unary_rule(UnaryRule rule) {
    assert(rule.nt_X < n_nonterms());
    assert(rule.nt_Y < n_nonterms());
    if (unary_rules_by_first.size() <= n_nonterms()) {
        unary_rules_by_first.resize(n_nonterms() + 1);
    }
    head_symbol.push_back(rule.nt_X);
    left_symbol.push_back(rule.nt_Y);
    right_symbol.push_back(0);
    is_unary.push_back(1);
    unary_rules_by_first[rule.nt_Y].push_back(rule);

    n_rules_++;
}

void Grammar::finish(int roots_) {
    roots = roots_;
    if (rules_by_first.size() <= n_nonterms()) {
        rules_by_first.resize(n_nonterms() + 1);
    }

    if (rules_by_second.size() <= n_nonterms()) {
        rules_by_second.resize(n_nonterms() + 1);
    }

    if (unary_rules_by_first.size() <= n_nonterms()) {
        unary_rules_by_first.resize(n_nonterms() + 1);
    }
}

Grammar *read_rule_set(string file) {
    Grammar *grammar = new Grammar();

    ifstream in_file;
    in_file.open(file.c_str());

    if (in_file.is_open()) {
        int rule_num;
        while (in_file >> rule_num) {
            string X, blank, Y, Z;
            bool is_unary;
            int dir;
            in_file >>  is_unary;
            if (!is_unary) {
                in_file >> X >> Y >> Z >> dir;
                int nt_X = grammar->to_nonterm(X);
                int nt_Y = grammar->to_nonterm(Y);
                int nt_Z = grammar->to_nonterm(Z);
                BinaryRule rule(rule_num, nt_X, nt_Y, nt_Z, dir);
                grammar->add_rule(rule);
            } else {
                in_file >> X >> Y;
                int nt_X = grammar->to_nonterm(X);
                int nt_Y = grammar->to_nonterm(Y);
                UnaryRule rule(rule_num, nt_X, nt_Y);
                grammar->add_unary_rule(rule);
            }
        }
    }

    grammar->finish(grammar->to_nonterm("TOP"));
    return grammar;
}
