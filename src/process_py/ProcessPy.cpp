/* @file ProcessPy.cpp */

#include "process/StochasticProcess.hpp"
#include "process/BrownianMotion.hpp"
#include "process/ExpProcess.hpp"
#include "process/RealizationSource.hpp"
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
  using xo::reactor::AbstractSink;
  using xo::reactor::SinkToConsole;
  using xo::time::utc_nanos;
  using xo::random::Seed;
  using xo::random::xoshiro256ss;
  using xo::ref::rp;
  namespace py = pybind11;

  namespace process {
    PYBIND11_MODULE(process_py, m) {
      /* e.g. for xo::reactor::ReactorSource */
      py::module_::import("reactor_py");

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

      m.def("make_tracer",
	    &RealizationTracer<double>::make);

      py::class_<RealizationTracer<double>,
		 xo::ref::rp<RealizationTracer<double>>>(m, "RealizationTracer");

      /* e.g.
       *   import datetime as dt
       *   t0=dt.datetime.now()
       *   ebm=process_py.make_exponential_brownian_motion(t0, 0.5)
       *   s=process_py.make_realization_source(ebm, dt.timedelta(seconds=1))
       */
      m.def("make_realization_source",
	    [](xo::ref::rp<StochasticProcess<double>> p,
	       xo::time::nanos sample_dt)
	    {
	      return RealizationSource<double>::make(RealizationTracer<double>::make(p),
						     sample_dt);
	    });

      py::class_<RealizationSource<double>,
		 reactor::ReactorSource,
		 xo::ref::rp<RealizationSource<double>>>(m, "RealizationSource");
										    
      /* ----------------------------------------------------------------
       * trying code below here instead of in reactor_py/,
       * to see if it resolves a typeinfo bug
       */

      py::class_<SinkToConsole<std::pair<utc_nanos, double>>,
		 AbstractSink,
		 rp<SinkToConsole<std::pair<utc_nanos, double>>>>
	(m, "SinkToConsole");
	
      /* prints
       *   std::pair<utc_nanos, double>
       * pairs
       */
      m.def("make_realization_printer",
	    []
	    {
	      return new SinkToConsole<std::pair<utc_nanos, double>>();
	    });

      /* this implementation fails -- looks like .so libraries
       * have separate typeinfo for std::pair<utc_nanos, double>
       * and don't find each other.
       */
      m.def("make_realization_printer2",
	    []
	    {
	      return reactor::TemporaryTest::realization_printer();
	    });

    } /*process_py*/
  } /*namespace process*/
} /*namespace xo*/

/* end ProcessPy.cpp */
