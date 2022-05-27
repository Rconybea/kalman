/* @file junk.cpp */

#include "KalmanConfig.h"
#include "tree/RedBlackTree.hpp"
#include "statistics/Histogram.hpp"
#include "statistics/SampleStatistics.hpp"
#include "process/BrownianMotion.hpp"
#include "distribution/Normal.hpp"
#include "random/Uniform.hpp"
#include "random/TwoPoint.hpp"
#include "random/Exponential.hpp"
#include "random/Normal.hpp"
#include "random/xoshiro256.hpp"
#include "logutil/scope.hpp"
#include "logutil/tag.hpp"
#include "reflect/demangle.hpp"
#include <Eigen/Dense>
#include <algorithm>
#include <iostream>
#include <stdlib.h>

namespace xo {
  using xo::process::BrownianMotion;
  using xo::time::utc_nanos;
  using xo::time::days;

  /* (*p_v)[i_lo] and (*p_v)[i_hi]
   * already populated
   */
  template <uint32_t N>
  void
  fill_interior_samples(utc_nanos t0,
			uint32_t i_lo,
			uint32_t i_hi,
			BrownianMotion *p_bm,
			std::array<double, N> *p_v)
  {
    if (i_lo == i_hi)
      return;

    if (i_lo + 1 == i_hi)
      return;

    utc_nanos t_lo = t0 + days(i_lo);
    utc_nanos t_hi = t0 + days(i_hi);

    uint32_t i_mid = i_lo + (i_hi - i_lo) / 2;
    utc_nanos t_mid = t0 + days(i_mid);

    /* sample B(t_mid) for v[i_mid];
     * this will be interior sample using B(t_lo), B(t_hi);
     */
    (*p_v)[i_mid] = p_bm->interior_sample(t_mid,
					  BrownianMotion::event_type(t_lo, (*p_v)[i_lo]),
					  BrownianMotion::event_type(t_hi, (*p_v)[i_hi]));

    fill_interior_samples<N>(t0, i_lo, i_mid, p_bm, p_v);
    fill_interior_samples<N>(t0, i_mid, i_hi, p_bm, p_v);
  } /*fill_interior_samples*/
} /*namespace xo*/

int
main(int argc, char **argv)
{
  // see:
  //   https: // eigen.tuxfamily.org/dox/GettingStarted.html

  using xo::tree::RedBlackTree;
  using xo::tree::OrdinalReduce;
  using xo::statistics::Histogram;
  using xo::statistics::Bucket;
  using xo::statistics::SampleStatistics;
  using xo::process::BrownianMotion;
  using xo::random::UnitIntervalGen;
  using xo::random::TwoPointGen;
  using xo::random::TwoPointDistribution;
  using xo::random::ExponentialGen;
  using xo::random::NormalGen;
  using xo::random::xoshiro256;
  using xo::distribution::Normal;
  using xo::time::utc_nanos;
  using xo::time::days;
  using xo::time::hours;
  using logutil::scope;
  using logutil::tostr;
  using logutil::xtag;
  using Eigen::MatrixXd;
  using Eigen::DiagonalMatrix;
  using Eigen::VectorXd;
  using std::cout;
  using std::endl;

  constexpr char const * c_self = "main";

  scope lscope(c_self);
#ifdef NOT_IN_USE
  lscope.log("Hello world");
  lscope.log("Hi");

  {
    scope lscope2("main2");
    lscope2.log("nesting!");

    {
      scope lscope3("main3");

      lscope3.log("nesting squared");
      lscope3.end_scope();
    }

    lscope2.end_scope();
  }
  
  //cout << "Hello world" << endl;

  MatrixXd m(2, 2);
  MatrixXd c = MatrixXd::Constant(2, 2, 1.1);
  VectorXd v(2);

  m(0,0) = 3;
  m(1,0) = 2.5;
  m(0,1) = -1;
  m(1,1) = m(1, 0) + m(0, 1);

  lscope.log(TAG(m));

  v(0) = 1;
  v(1) = -1;

  lscope.log(TAG(v));

  lscope.log(TAG2("vT", v.transpose()));

  lscope.log(TAG2("v.vT", v * v.transpose()));

  lscope.log(TAG(c));

  lscope.log(TAG2("mc", m*c));

  lscope.log(TAG2("mcv", m*c*v));

  lscope.log(TAG2("ones", VectorXd::Constant(2, 1.0)));

  lscope.log(TAG2("u1", VectorXd::Unit(2, 0)));
  lscope.log(TAG2("u2", VectorXd::Unit(2, 1)));

  /* 2-norm */
  lscope.log(TAG2("v.norm", v.norm()));

  /* vT.v */
  lscope.log(TAG2("v.v", v.dot(v)));

  /* diagonal matrix */
  MatrixXd d = DiagonalMatrix<double, 2>(3.3, -0.25);

  MatrixXd id = MatrixXd::Identity(2, 2);

  lscope.log(TAG2("d", d));

  lscope.log(TAG2("id", id));

  lscope.log(TAG2("id.inverse", id.inverse()));

  lscope.log(TAG2("id.trace", id.trace()));

  lscope.log(TAG2("id.norm", id.norm()));
#endif

  enum Cmd {
    C_NormalDistribution,
    C_UnitIntRandom,
    C_TwoPoint,
    C_Exponential,
    C_Normal,
    C_BrownianMotion,
    C_Histogram,
    C_RedBlackTree,
  };

  Cmd cmd = C_RedBlackTree;

  if(cmd == C_NormalDistribution) {
    constexpr size_t c_n = 500;

    for(size_t i=0; i<=c_n; ++i) {
      constexpr double c_lo = -4.0;
      constexpr double c_hi = +4.0;

      double xi = c_lo + i * (c_hi - c_lo) / c_n;
      double yi = Normal::density(xi);

      std::cout << xi << " " << yi << std::endl;
    }
  } else if(cmd == C_UnitIntRandom) {
    auto rgen = UnitIntervalGen<xo::random::xoshiro256>::make(time(nullptr) /*seed*/);

    for(size_t i=0; i<10; ++i) {
      double xi = rgen();

      std::cout << xi << std::endl;
    }
  } else if(cmd == C_TwoPoint) {
    auto rgen = TwoPointGen::make(time(nullptr) /*seed*/,
				  0.5 /*prob*/,
				  -1.0 /*x1*/,
				  +1.0 /*x2*/);

    //std::cout << "type(rgen)=" << typeid(decltype(rgen)).name() << std::endl;
    //std::cout << "type(rgen)=" << xo::reflect::type_name<decltype(rgen)>();

    for(size_t i=0; i<50; ++i) {
      double xi = rgen();

      std::cout << xi << std::endl;
    }
  } else if(cmd == C_Exponential) {
    auto rgen = ExponentialGen::make(time(nullptr) /*seed*/,
				     10.1 /*lambda*/);

    for(size_t i=0; i<50; ++i) {
      double xi = rgen();

      std::cout << xi << std::endl;
    }
  } else if(cmd == C_Normal) {
    auto rgen = NormalGen<xo::random::xoshiro256>::make(time(nullptr) /*seed*/,
							0.0 /*mean*/,
							100.0 /*sdev*/);

    for(size_t i=0; i<50; ++i) {
      double xi = rgen();

      std::cout << xi << std::endl;
    }
  } else if(cmd == C_BrownianMotion) {
    /* note this is arithmetic brownian motion;
     * -ve values allowed!
     */

    utc_nanos t0 = std::chrono::system_clock::now();

    BrownianMotion bm(t0,
		      0.5 /*50% annual volatility - sdev ~ .025/day*/,
		      time(nullptr) /*seed*/);

    {
      double var_1day = bm.variance_dt(xo::time::days(1));
      lscope.log(TAG2("var(1day)", var_1day));
      lscope.log(TAG2("sdev(1day)", ::sqrt(var_1day)));
    }
    {
      double var_30day = bm.variance_dt(xo::time::days(30));
      lscope.log(TAG2("var(30day)", var_30day));
      lscope.log(TAG2("sdev(30day)", ::sqrt(var_30day)));
    }

    lscope.log("using exterior_sample (30 days)");
    constexpr uint32_t n = 30;
    std::array<double, n> v;

    v[0] = 0.0;
    for(uint32_t i=1; i<n; ++i) {
      utc_nanos ti_prev = t0 + days(i-1);
      utc_nanos ti = t0 + days(i);

      v[i] = bm.exterior_sample(ti, BrownianMotion::event_type(ti_prev, v[i-1]));

      lscope.log(TAG(i), " ", TAG(v[i]));
    }

    lscope.log("using interior_sample");

    xo::fill_interior_samples<n>(t0, 0, n-1, &bm, &v);

    for(uint32_t i=1; i<n; ++i) {
      lscope.log(TAG(i), " ", TAG(v[i]));
    }
  } else if(cmd == C_Histogram) {
    Histogram hist(100 /*n_interior_bucket*/,
		   0.0 /*lo_bucket*/,
		   1.0 /*hi_bucket*/);

    SampleStatistics sample;

    auto rgen = UnitIntervalGen<xo::random::xoshiro256>::make(time(nullptr) /*seed*/);

    /* generate samples */
    for(uint32_t i=0; i<1000; ++i) {
      /* generate U(0,1) random value */
      double xi = rgen();

      hist.include_sample(xi);
      sample.include_sample(xi);
    } 

    lscope.log("histogram of U(0,1) psuedorandom vars");
    lscope.log(TAG2("sample-mean", sample.mean()));
    lscope.log(TAG2("sample-variance", sample.sample_variance()));

    lscope.log("bucket, n, mean");
    for(uint32_t i=0; i<hist.n_bucket(); ++i) {
      Bucket const & bucket = hist.lookup(i);

      lscope.log(hist.bucket_hi_edge(i),
		 " ", bucket.n_sample(),
		 " ", bucket.mean());
    }
  } else if(cmd == C_RedBlackTree) {
    RedBlackTree<int, double, OrdinalReduce<int>> rbtree;

    uint64_t seed = 14950349842636922572UL;
    //uint64_t seed = static_cast<uint64_t>(time(nullptr));
    //arc4random_buf(&seed, sizeof(seed));

    lscope.log(c_self, ": using rng seed from /dev/urandom", xtag("seed", seed));

    auto rgen = xo::random::xoshiro256(seed);

    /* generate a random series of inserts and removes,  with increasing scale */

    for(uint32_t n=1; n<64; n *= 2) {
      /* n random inserts */
      rbtree.verify_ok();
      
      /* n keys 0..n-1 */
      std::vector<uint32_t> u(n);

      for(uint32_t i=0; i<n; ++i)
	u[i] = i;
      /* generate random permutation of u[0]..u[n-1] */
      std::shuffle(u.begin(), u.end(), rgen);

      {
        uint32_t i = 1;
        for (uint32_t x : u) {
          lscope.log(c_self, ": ", i, "/", n, ": insert key", xtag("x", x));
          rbtree.insert(x, 10 * x);
          rbtree.display();
          rbtree.verify_ok();
          ++i;
        }
      }

      XO_EXPECT(rbtree.size() == n,
		tostr(c_self,
		      ": expect tree.size=n after n distinct inserts",
		      xtag("n", n)));

      /* n random removals */
      /* generate new random permutation of u[0]..u[n-1] */
      std::shuffle(u.begin(), u.end(), rgen);

      {
	uint32_t i = 1;
        for (uint32_t x : u) {
          lscope.log(c_self, ": ", i, "/", n, ": remove key", xtag("x", x));
          rbtree.remove(x);
          rbtree.display();
          rbtree.verify_ok();
	  ++i;
        }
      }

      XO_EXPECT(rbtree.size() == 0,
		tostr(c_self,
		      ": expect tree.size=0 after n distinct removes"));
    }

  }

#ifdef NOT_IN_USE
  if (argc < 2) {
    // report version
    cout << argv[0]
	 << " version "
	 << Kalman_VERSION_MAJOR
	 << "."
	 << Kalman_VERSION_MINOR
	 << endl;

    //cout << "usage: " << argv[0] << " ..." << endl;

    return 1;
  }
#endif
}

/* junk.cpp */

  
