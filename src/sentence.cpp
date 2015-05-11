#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "sentence.hpp"

using namespace std;

void annotate_gold(string file, vector<Sentence> *sentences) {
    ifstream in_file;
    in_file.open(file.c_str());
    for (int j = 0; j < sentences->size(); ++j) {
        int n_rules;
        in_file >> n_rules;
        for (int i = 0; i < n_rules; ++i) {
            AppliedRule rule;
            in_file >> rule.i >> rule.j >> rule.k
                    >> rule.h >> rule.m >> rule.rule;
            (*sentences)[j].gold_rules.push_back(rule);
        }
    }
}

vector<Sentence> *read_sentences(istream &in_file,
                                 Lexicon *lexicon, Grammar *grammar) {
    vector<Sentence> *sentences = new vector<Sentence>();

    while (in_file.good()) {
        Sentence sentence;
        read_sentence(in_file, lexicon, grammar, &sentence);
        sentences->push_back(sentence);
    }

    for (auto &sentence : *sentences) {
        lexicon->process_sentence(&sentence, grammar);
    }
    return sentences;
}

void read_sentence(istream &in_file, const Lexicon *lexicon, const Grammar *grammar,
                   Sentence *sentence) {
    string word, tag, deplabel, lemma, coarse_tag, blank;
    int position, dep;

    string s;
    int root_dep = -1;
    while (getline(in_file, s)) {
        if (s.empty()) {
            return;
        } else {
            istringstream tmp(s);
            tmp >> position >> word >> lemma
                    >> coarse_tag >> tag
                    >> blank >> dep >> deplabel;

            sentence->words.push_back(word);
            sentence->tags.push_back(tag);


            dep -= 1;
            if (dep == -1) {
                if (root_dep == -1) {
                    root_dep = position - 1;
                } else {
                    dep = root_dep;
                }
            }
            sentence->deps.push_back(dep);
            sentence->deplabels.push_back(deplabel);
        }
    }
}


void Lexicon::process_sentence(Sentence *sentence, const Grammar *grammar) {
    for (unsigned j = 0; j < sentence->tags.size(); ++j) {
        sentence->int_tags.push_back(
            tag_index.get_or_add(sentence->tags[j]));
        sentence->preterms.push_back(
            grammar->to_nonterm(sentence->tags[j]));
        sentence->int_words.push_back(
            word_index.get_or_add(sentence->words[j]));
        sentence->int_deplabels.push_back(
            deplabel_index.get_or_add(sentence->deplabels[j]));
    }
}

void Lexicon::process_test_sentence(Sentence *sentence, const Grammar *grammar) const {
    for (unsigned j = 0; j < sentence->tags.size(); ++j) {
        sentence->int_tags.push_back(
            tag_index.index(sentence->tags[j]));
        sentence->preterms.push_back(
            grammar->nonterm_index.index(sentence->tags[j]));
        sentence->int_words.push_back(
            word_index.index(sentence->words[j]));
        sentence->int_deplabels.push_back(
            deplabel_index.index(sentence->deplabels[j]));
    }
}
