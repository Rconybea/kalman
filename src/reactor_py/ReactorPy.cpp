/* @file ReactorPy.cpp */

#include "reactor/ReactorSource.hpp"
#include "reactor/Sink.hpp"
#include "time/Time.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

/* xo::ref::intrusive_ptr<T> is an intrusively-reference-counted pointer.
 * always safe to create one from a T* p
 * (since refcount is directly accessible from p)
 */
PYBIND11_DECLARE_HOLDER_TYPE(T, xo::ref::intrusive_ptr<T>, true);

namespace xo {
  using xo::time::utc_nanos;
  namespace py = pybind11;

  namespace reactor {
    PYBIND11_MODULE(reactor_py, m) {
      /* module docstring */
      m.doc() = "pybind11 plugin for xo.reactor";

      py::class_<AbstractSource,
		 xo::ref::rp<AbstractSource>>(m, "AbstractSource")
	.def("__repr__", &AbstractSource::display_string)
	.def("attach_sink", &AbstractSource::attach_sink)
	.def("detach_sink", &AbstractSource::detach_sink)
	.def("deliver_one", &AbstractSource::deliver_one);

      py::class_<AbstractSink,
		 xo::ref::rp<AbstractSink>>(m, "AbstractSink")
	//.cdef("__repr__", &AbstractSink::display_string)
	.def("item_type", &AbstractSink::item_type)
	.def("attach_source", &AbstractSink::attach_source);

      py::class_<ReactorSource,
		 AbstractSource,
		 xo::ref::rp<ReactorSource>>
	(m, "ReactorSource");

#ifdef NOT_IN_USE  // trying removed code in ProcessPy.cpp instead for now
      /* prints
       *   std::pair<utc_nanos, double>
       * pairs
       */
      m.def("make_realization_printer",
	    []
	    {
	      return new SinkToConsole<std::pair<utc_nanos, double>>();
	    });

      py::class_<SinkToConsole<std::pair<utc_nanos, double>>,
		 AbstractSink,
		 xo::ref::rp<SinkToConsole<std::pair<utc_nanos, double>>>>
	(m, "SinkToConsole");
#endif	
    } /*reactor_py*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end ReactorPy.cpp */
