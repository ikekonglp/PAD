//
// Simple AdaGrad implementation.
//


#ifndef ADAGRAD_H_
#define ADAGRAD_H_

#include <cereal/types/vector.hpp>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include <iostream>
#include <cereal/types/memory.hpp>


using namespace std;

class AdaGrad {
  public:
    AdaGrad(int n_dims) : weights(n_dims, 0.0),
                          unnormalized_averaged_subgradient(n_dims, 0.0),
                          g_square_over_time(n_dims, 0.0){
        round_ = 1;
        lambda_ = 0.2;
    }

    void set_lambda(double lambda){
        lambda_ = lambda;
    }

    double get_lambda(){
        return lambda_;
    }

    void next_round() {
        round_ += 1;
    }

    void update(long feature, int direction) {
        // lpk: using ada grad l1 here
        unnormalized_averaged_subgradient[feature] += direction;
        g_square_over_time[feature] += pow(direction, 2);

        double abs_normalized_averaged_subgradient =
                ((unnormalized_averaged_subgradient[feature])) / ((double)round_);
        abs_normalized_averaged_subgradient =
                abs_normalized_averaged_subgradient >= 0 ?
                abs_normalized_averaged_subgradient :
                (-abs_normalized_averaged_subgradient);

        if (abs_normalized_averaged_subgradient <= lambda_) {
            weights[feature] = 0;
        } else {
            weights[feature] = (1.0) * sgn(unnormalized_averaged_subgradient[feature]) *
                    (((double)round_) / sqrt(g_square_over_time[feature])) *
                    (abs_normalized_averaged_subgradient - lambda_);
        }
    }

    template <typename T> int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }


    template <class Archive>
    void serialize(Archive &ar) {
        ar(weights);
    }

    vector<double> weights;

  private:
    long round_;
    double lambda_;
    vector<double> unnormalized_averaged_subgradient;
    vector<double> g_square_over_time;
};

#endif  // ADAGRAD_H_
