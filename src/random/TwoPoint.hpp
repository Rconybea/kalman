/* @file TwoPoint.hpp */

#pragma once

#include <random/Bernoulli.hpp>

namespace xo {
  namespace random {
    // generates either x1 or x2:
    //   x1 with probability p
    //   x2 with probability 1-p
    // can be used as generator
    //
    template<typename ValueType>
    class TwoPointDistribution {
    public:
      using result_type = ValueType;
      
      TwoPointDistribution(double prob, ValueType const & x1, ValueType const & x2)
	: bernoulli_dist_{prob}, x1_{x1}, x2_{x2} {}

      template<class Engine>
      result_type operator()(Engine & e) {
	bool flag = bernoulli_dist_(e);

	return (flag ? x1_ : x2_);
      } /*operator()*/

    private:
      /* generates true|false */
      std::bernoulli_distribution bernoulli_dist_;
      ValueType x1_;
      ValueType x2_;
    }; /*TwoPointDistribution */

    class TwoPointGen {
    public:
      template<typename ValueType>
      using generator_type = Generator<std::mt19937,
				       TwoPointDistribution<ValueType>>;

      template<typename ValueType>
      static generator_type<ValueType> make(uint64_t seed,
					    double prob,
					    ValueType const & x1,
					    ValueType const & x2)
      {
	return generator_type<ValueType>::make(std::mt19937(seed),
					       TwoPointDistribution<ValueType>(prob, x1, x2));
      } /*make*/
    }; /*TwoPointGen*/
  } /*namespace random*/
} /*namespace xo*/

/* end TwoPoint.hpp */
