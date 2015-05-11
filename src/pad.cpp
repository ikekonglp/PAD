#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <fstream>
#include "util.hpp"

enum optionIndex {
    UNKNOWN, HELP, MODEL, PRUNING, ORACLE,
    LABEL_PRUNING, DIR_PRUNING, SENTENCES};

const option::Descriptor usage[] = {
    {UNKNOWN, 0,"" , ""    ,option::Arg::None,
     "\nPAD: Phrases After Dependencies\n"
     "USAGE: pad [options]\n\n"
     "Options:" },

    {HELP,    0, "" , "help", option::Arg::None,
     "--help:              \tPrint this message and exit." },

    {MODEL, 0, "m", "model", Arg::Required,
     "--model, -m:         \t(Required) Model file." },

    {SENTENCES, 0, "s", "sentences", Arg::Required,
     "--sentences, -s:     \tCoNLL sentence file." },


    {ORACLE, 0, "o", "oracle", option::Arg::None,
     "--oracle, -o:        \tRun in oracle mode." },

    {PRUNING, 0, "p", "pruning", option::Arg::None,
     "--pruning, -p:       \tUse pruning based on head POS tags." },

    {DIR_PRUNING, 0, "", "dir_pruning", option::Arg::None,
     "--dir_pruning:       \tUse pruning based on dependency topology." },

    {0, 0, 0, 0, 0, 0}
};

int main(int argc, char* argv[]) {
    // For arg parsing.
    argc -= (argc>0);
    argv += (argc>0);

    option::Stats  stats(usage, argc, argv);
    option::Option options[stats.options_max], buffer[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (parse.error())
        return 1;

    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        return 0;
    }


    FullModel model;
    // Read model. Make function.
    ifstream is(string(options[MODEL].arg),
                std::ios::binary);
    cereal::BinaryInputArchive iarchive(is);
    iarchive(model);

    Pruning *pruner = &model.pruner;
    const FullModel *cmodel = &model;

    pruner->pruning = options[PRUNING];
    pruner->dir_pruning =  options[DIR_PRUNING];

    // Make scorer.
    istream *sentence_handle = &cin;
    ifstream input;
    if (options[SENTENCES]) {
        input.open(options[SENTENCES].arg);
        sentence_handle = &input;
    }

    model.scorer.is_cost_augmented_ = false;
    if (!options[ORACLE]) {
        while (cin.good()) {
            Sentence sentence;
            read_sentence(*sentence_handle, &cmodel->lexicon, &cmodel->grammar, &sentence);
            cmodel->lexicon.process_test_sentence(&sentence, &model.grammar);
            model.scorer.set_sentence(&sentence);
            Parser parser(&sentence, &cmodel->grammar, &cmodel->scorer, pruner);
            double score = parser.cky(true, false);
        }
    } else {
        cerr << "ORACLE mode";
        while (cin.good()) {
            Sentence sentence;
            read_sentence(*sentence_handle, &cmodel->lexicon, &cmodel->grammar, &sentence);
            model.lexicon.process_test_sentence(&sentence, &cmodel->grammar);
            OracleScorer oracle(&sentence.gold_rules, &model.grammar);
            Parser parser(&sentence, &model.grammar, &oracle, pruner);
            parser.cky(true, false);
        }
    }
}
