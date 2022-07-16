/* @file ProcessPy.cpp */

#include "process/StochasticProcess.hpp"
#include "process/BrownianMotion.hpp"
#include "process/ExpProcess.hpp"
#include "random/random_seed.hpp"
#include "random/xoshiro256.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

/* xo::ref::intrusive_ptr<T> is an intrusively-reference-counted pointer.
 * always safe to create one from a T* p
 * (since refcount is directly accessible from p)
 */
PYBIND11_DECLARE_HOLDER_TYPE(T, xo::ref::intrusive_ptr<T>, true);

namespace xo {
  using xo::time::utc_nanos;
  using xo::random::Seed;
  using xo::random::xoshiro256ss;
  namespace py = pybind11;

  namespace process {
    PYBIND11_MODULE(process_py, m) {
      m.doc() = "pybind11 plugin for xo.process";

      m.def("make_brownian_motion",
	    [](utc_nanos start_tm,
	       double annual_volatility) {
	      Seed<xoshiro256ss> seed;

	      return BrownianMotion<xoshiro256ss>::make(start_tm,
							annual_volatility,
							seed);
	    },
	    "create new BrownianMotion instance");

      m.def("make_exponential_brownian_motion",
	    [](utc_nanos start_tm,
	       double annual_volatility) {
	      Seed<xoshiro256ss> seed;

	      return ExpProcess::make(1.0 /*scale*/,
				      BrownianMotion<xoshiro256ss>::make(start_tm,
									 annual_volatility,
									 seed));
	    });

      py::class_<StochasticProcess<double>,
                 xo::ref::rp<StochasticProcess<double>>>(m, "StochasticProcess")
	.def("exterior_sample", &StochasticProcess<double>::exterior_sample)
	.def("__repr__", &StochasticProcess<double>::display_string);

      py::class_<BrownianMotion<xoshiro256ss>,
		 StochasticProcess<double>,
		 xo::ref::rp<BrownianMotion<xoshiro256ss>>>(m, "BrownianMotion");
      //.def("exterior_sample", &BrownianMotion<xoshiro256ss>::exterior_sample)
      //.def("__repr__", &BrownianMotion<xoshiro256ss>::display_string);

      py::class_<ExpProcess, StochasticProcess<double>,
		 xo::ref::rp<ExpProcess>>(m, "ExpProcess");

    } /*process_py*/
  } /*namespace process*/
} /*namespace xo*/

/* end ProcessPy.cpp */
