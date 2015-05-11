#include <cmath>
#include <iostream>
#include <map>
#include <vector>

#include "model.hpp"

// const int n_size = 100000000;

double Model::score(const AppliedRule &rule) const {
    vector<long> features;
    double score = feature_gen_->generate(*sentence_, rule, &features,
                                          &adagrad_.weights);

    if (!is_cost_augmented_) {
        return score;
    } else {
        double y = 0;
        for (unsigned j = 0; j < sentence_->gold_rules.size(); ++j) {
            if (rule.same(sentence_->gold_rules[j])) {
                y = 1;
                break;
            }
        }
        return score + (1 - 2 * y);
    }
}

void Model::update(const vector<AppliedRule> &good,
                   const vector<AppliedRule> &bad) {
    map<long, int> count_map;
    for (unsigned i = 0; i < good.size(); ++i) {
        vector<long> good_features;
        feature_gen_->generate(*sentence_, good[i], &good_features, NULL);
        for (long good_feature : good_features) {
            long index = hashed_feature(good_feature);
            double orignal_value = 0.0;
            if (count_map.find(index) != count_map.end()) {
                orignal_value = count_map[index];
            }
            count_map[index] = orignal_value + 1;
        }
    }
    for (unsigned i = 0; i < bad.size(); ++i) {
        vector<long> bad_features;
        feature_gen_->generate(*sentence_, bad[i], &bad_features, NULL);
        for (long bad_feature : bad_features) {
            long index = hashed_feature(bad_feature);
            double orignal_value = 0.0;
            if (count_map.find(index) != count_map.end()) {
                orignal_value = count_map[index];
            }
            count_map[index] = orignal_value - 1;
        }
    }

    for (auto& iter : count_map) {
        adagrad_.update((long)iter.first, (int)iter.second);
    }
}


int Model::update_full(const vector<AppliedRule> &gold_rules,
                       const vector<AppliedRule> &best_rules) {

    int correct = 0, correct2 = 0;
    vector<AppliedRule> bad, good;
    double best_score = 0.0;
    for (auto best_rule : best_rules) {
        bool have = false;
        best_score += score(best_rule);
        for (auto gold_rule : gold_rules) {
            if (best_rule.same(gold_rule)) {
                correct += 1;
                have = true;
                break;
            }
        }
        if (!have) bad.push_back(best_rule);
    }

    double gold_score = 0.0;
    for (auto gold_rule : gold_rules) {
        bool have = false;
        gold_score += score(gold_rule);
        for (auto best_rule : best_rules) {
            if (best_rule.same(gold_rule)) {
                correct2 += 1;
                have = true;
                break;
            }
        }
        if (!have) good.push_back(gold_rule);
    }
    update(good, bad);
    return correct2;
}
