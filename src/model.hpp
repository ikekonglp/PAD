//
// A sparse-linear model for structured prediction.
//


#ifndef MODEL_H_
#define MODEL_H_

#include "sentence.hpp"
#include "features.hpp"
#include "adagrad.hpp"


class Model : public Scorer {
  public:
    Model(bool simple=false)
            : adagrad_(n_size), simple_(simple) {}

    void set(const FeatureGen *feature_gen) {
        feature_gen_ = feature_gen;
    }

    void set_sentence(const Sentence *sentence) {
        sentence_ = sentence;
    }

    template <class Archive>
    void serialize(Archive &ar) {
        ar(adagrad_);
    }

    double score(const AppliedRule &rule) const;

    int update_full(const vector<AppliedRule> &gold_rules,
                    const vector<AppliedRule> &best_rules);

    void update(const vector<AppliedRule> &good,
                const vector<AppliedRule> &bad);

    long hashed_feature(long val) const {
        return ((long)abs(val)) % n_size;
    }


    bool is_cost_augmented_;

    int full_feature_count_ = 1;

    AdaGrad adagrad_;
  private:


    const FeatureGen *feature_gen_;
    const Sentence *sentence_;

    bool use_positive_features_;
    map<long, int> positive_features_;

    mutable map<long, int> all_features_;
    bool simple_;
};

#endif  // MODEL_H_
