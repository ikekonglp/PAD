#include "optionparser.h"
#include "model.hpp"
#include "parse_features.hpp"
#include "sentence.hpp"
#include "pruning.hpp"
#include "inference.hpp"
#include "oracle.hpp"


struct FullModel {
    FullModel(string grammar_file, bool simple_features)
            : grammar(*read_rule_set(grammar_file)),
              lexicon(),
              feature_gen(simple_features),
              scorer(simple_features),
              pruner(&lexicon, &grammar) {}

    void init() {
        feature_gen.init(&lexicon, &grammar);
        scorer.set(&feature_gen);
    }

    FullModel() {}

    template <class Archive>
    void save(Archive &ar) const {
        ar(grammar, lexicon, scorer, pruner, feature_gen);
    }

    template <class Archive>
    void load(Archive &ar) {
        ar(grammar, lexicon, scorer, pruner, feature_gen);
        feature_gen.init(&lexicon, &grammar);
        scorer.set(&feature_gen);
    }

    Grammar grammar;
    Lexicon lexicon;
    FeatureGenBackoff feature_gen;
    Model scorer;
    Pruning pruner;
};


struct Arg: public option::Arg {
    static option::ArgStatus Required(const option::Option& option, bool msg) {
        if (option.arg != 0)
            return option::ARG_OK;

        if (msg) cerr << "Option '" << option
                      << "' requires an argument\n";
        return option::ARG_ILLEGAL;
    }
    static option::ArgStatus Numeric(const option::Option& option, bool msg) {
        char* endptr = 0;
        if (option.arg != 0 && strtol(option.arg, &endptr, 10)) {}
        if (endptr != option.arg && *endptr == 0)
            return option::ARG_OK;

        if (msg) {
            cerr << "Option '" << option
                 << "' requires a numeric argument" << endl;
        }
        return option::ARG_ILLEGAL;
    }
};
