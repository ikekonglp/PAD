#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <stdio.h>
#include <time.h>

#include "optionparser.h"
#include "model.hpp"
#include "parse_features.hpp"
#include "sentence.hpp"
#include "pruning.hpp"
#include "inference.hpp"
#include "oracle.hpp"

#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include "util.hpp"

enum optionIndex {
    UNKNOWN, HELP, GRAMMAR, SENTENCES, EPOCHS, LAMBDA,
    ANNOTATIONS, MODEL, SENTENCE_TEST, PRUNING,
    ORACLE, LABEL_PRUNING, SIMPLE_FEATURES, DIR_PRUNING};

const option::Descriptor usage[] = {
    {UNKNOWN, 0,"" , ""    ,option::Arg::None,
     "\nPADt: Phrases After Dependencies trainer\n"
     "USAGE: padt [options]\n\n"
     "Options:" },

    {HELP,    0, "" , "help", option::Arg::None,
     "--help:             \tPrint this message and exit." },

    {GRAMMAR, 0, "g", "grammar", Arg::Required,
     "--grammar, -g:      \t(Required) Grammar file." },

    {SENTENCES, 0, "s", "conll", Arg::Required,
     "--conll, -c:        \t(Required) CoNLL sentence file." },

    {MODEL, 0, "m", "model", Arg::Required,
     "--model, -m:        \t(Required) Model file to output." },


    {ANNOTATIONS, 0, "s", "annotations", Arg::Required,
     "--annotations, -a   \t(Required) Gold phrase structure file." },

    {EPOCHS, 0, "e", "epochs", Arg::Numeric,
     "--epochs[=10], -e:  \tNumber of epochs." },

    {LAMBDA, 0, "l", "lambda", Arg::Required,
     "--lambda[=0.0001]:  \tL1 Regularization constant. " },


    {SIMPLE_FEATURES, 0, "", "simple_features", option::Arg::None,
     "--simple_features   \tUse simple set of features (debugging)." },

    {0, 0, 0, 0, 0, 0}
};

int main(int argc, char* argv[]) {
    // For arg parsing.
    argc -= (argc>0);
    argv += (argc>0);
    char *temp = 0;

    option::Stats  stats(usage, argc, argv);
    option::Option options[stats.options_max], buffer[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (parse.error())
        return 1;

    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        return 0;
    }

    FullModel model(string(options[GRAMMAR].arg),
                    options[SIMPLE_FEATURES]);

    ifstream in_file(options[SENTENCES].arg);
    vector<Sentence> *sentences = read_sentences(in_file,
                                                 &model.lexicon, &model.grammar);
    model.init();
    model.scorer.is_cost_augmented_ = true;
    model.pruner.build_pruning(*sentences, &model.grammar);
    model.pruner.build_dir_pruning(*sentences);

    assert(options[ANNOTATIONS]);
    annotate_gold(options[ANNOTATIONS].arg, sentences);

    // Read in epochs and lambda.
    double lambda = 0.0001;
    if (options[LAMBDA]) {
        lambda = strtod(options[LAMBDA].arg, &temp);
    }

    int epochs = 10;
    if (options[EPOCHS]) {
        epochs = strtol(options[EPOCHS].arg, &temp, 10);
    }
    model.scorer.adagrad_.set_lambda(lambda);

    cerr << "[Begining training]" << endl;
    Pruning *pruner = new Pruning(&model.lexicon,
                                  &model.grammar);

    for (int epoch = 0; epoch < epochs; ++epoch) {
        cout << "[Begining epoch " << epoch << "]" << endl;
        double total_score = 0.0;
        int total = 0;
        for (unsigned i = 0; i < (*sentences).size(); i++) {
            const Sentence sentence = (*sentences)[i];
            if (sentence.words.size() <= 5) continue;
            model.scorer.set_sentence(&sentence);
            Parser parser(&sentence, &model.grammar, &model.scorer, pruner);
            parser.cky(false, true);

            // Compute difference.
            int correct2 = model.scorer.update_full(sentence.gold_rules,
                                                    parser.best_rules);
            total_score += correct2 /
                    static_cast<double>(sentence.gold_rules.size());
            total += 1;
            model.scorer.adagrad_.next_round();

        }
        cout << "[ EPOCH: " << epoch << " "
             << total_score / (float)total
             << " ] " << endl;

        //ofstream os(string(options[MODEL].arg), std::ios::binary);
        //cereal::BinaryOutputArchive archive(os);
        //archive(model);
    }

    //Output the final model.
    ofstream os(string(options[MODEL].arg),
                std::ios::binary);
    cereal::BinaryOutputArchive archive(os);
    archive(model);
}
