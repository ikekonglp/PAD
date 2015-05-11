//
// Helper code for generating sparse features with a hash kernel.
//



#ifndef FEATURES_H_
#define FEATURES_H_

#include "sentence.hpp"
#include <cmath>

const int n_size = 100000000;

class FeatureGen {
  public:
    virtual double generate(const Sentence &sentence,
                            const AppliedRule &rule,
                            vector<long> *base,
                            const vector<double> *weights) const = 0;
};

// Simple int tuple.
class Triple {
  public:
    Triple() {}
    Triple(const Triple &o) :
            _size_a(o._size_a),
            _size_b(o._size_b),
            _size_c(o._size_c),
            _total_size(o._total_size) {}

    Triple(long size_a, long size_b, long size_c)
            : _size_a(size_a), _size_b(size_b), _size_c(size_c),
              _total_size(size_a * size_b * size_c) {}

    long _size_a;
    long _size_b;
    long _size_c;
    long _total_size;
};


struct FeatureState {
    FeatureState(vector<long> *base_,
                 const vector<double> *weights_)
            : base(base_), weights(weights_) {
        tally = 0;
        score = 0.0;
        feature_num = 0;
    }

    vector<long> *base;
    const vector<double> *weights;
    long tally;
    double score;
    int feature_num;
    const vector<Triple> *features;

    inline void inc(long a, long b, long c=1) {
        const Triple &t = (*features)[feature_num];
        long app = (a * t._size_b * t._size_c + b * t._size_c + c);
        long index = tally + app;

        if (weights != NULL) {
            score += (*weights)[((long)abs(index)) % n_size];
        } else {
            base->push_back(index);
        }
        (tally) += t._total_size;
        feature_num += 1;
    }
};


#endif  // FEATURES_H_
