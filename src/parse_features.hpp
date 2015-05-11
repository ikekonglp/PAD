//
// Specific features for the PAD parser as described in (Kong et. al. 2015).
//

#ifndef PARSE_FEATURES_H_
#define PARSE_FEATURES_H_

#include "features.hpp"

class FeatureGenBackoff : public FeatureGen {
  public:
    FeatureGenBackoff() {}
    FeatureGenBackoff(bool simple);

    void init(const Lexicon *lexicon,
              const Grammar *grammar);

    double generate(const Sentence &sentence,
                    const AppliedRule &rule,
                    vector<long> *base,
                    const vector<double> *weights) const;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(simple_);
    }

  private:
    void backed_off_features(const Sentence &sentence,
                             const AppliedRule &rule,
                             int index,
                             int extra,
                             FeatureState *state) const;

    void add_template(int a, int b, int c=1);
    void add_backed_off_template(int plus=1);

    vector <Triple> features_;
    const Lexicon *lexicon_;
    const Grammar *grammar_;
    bool simple_;
};

#endif  // PARSE_FEATURES_H_
