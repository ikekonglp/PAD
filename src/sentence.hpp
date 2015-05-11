//
// Classes for handling the lexicon and sentences.
//

#ifndef SENTENCE_H_
#define SENTENCE_H_

#include "index.hpp"
#include "grammar.hpp"
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

using namespace std;

class AppliedRule;
class Grammar;

struct Sentence {
  public:
    vector<string> tags;
    vector<int> int_tags;

    vector<string> words;
    vector<int> int_words;

    vector<string> deplabels;
    vector<int> int_deplabels;

    vector<int> deps;
    vector<AppliedRule> gold_rules;
    vector<int> preterms;
};

class Lexicon {
  public:
    Lexicon() {
        word_index.get_or_add("#START#");
        word_index.get_or_add("#END#");
	
        tag_index.get_or_add("#START#");
        tag_index.get_or_add("#END#");
    }


    template <class Archive>
    void serialize(Archive &ar) {
        ar(tag_index, word_index, deplabel_index);
    }

    void process_sentence(Sentence *sentence,
                          const Grammar *grammar);

    void process_test_sentence(Sentence *sentence,
                               const Grammar *grammar) const;

    Index tag_index;
    Index word_index;
    Index deplabel_index;
};

vector<Sentence> *read_sentences(istream &in_file, Lexicon *lexicon, Grammar *grammar);
void read_sentence(istream &in_file, const Lexicon *lexicon, const Grammar *grammar, Sentence *sentence);
void annotate_gold(string file, vector<Sentence> *sentences);

#endif  // SENTENCE_H_
