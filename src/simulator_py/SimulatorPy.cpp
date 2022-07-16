/* @file SimulatorPy.cpp */

#include "simulator/Simulator.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

/* xo::ref::intrusive_ptr<T> is an intrusively-reference-counted pointer.
 * always safe to create one from a T* p
 * (since refcount is directly accessible from p)
 */
PYBIND11_DECLARE_HOLDER_TYPE(T, xo::ref::intrusive_ptr<T>, true);

namespace xo {
  namespace py = pybind11;

  namespace sim {

    int
    xoadd(int i, int j) {
      return i + j;
    }

    PYBIND11_MODULE(simulator_py, m) {
      m.doc() = "pybind11 plugin for xo.simulator"; // optional module docstring

      // python: simulator_py.xoadd(x,y)
      m.def("xoadd",
	    &xoadd,
	    "A function that adds two numbers",
	    py::arg("x"),
	    py::arg("y"));

      m.def("make_simulator",
	    []() {
	      return xo::sim::Simulator::make(xo::time::Time::epoch());
	    },
	    "create new Simulator instance");

      py::class_<Simulator, xo::ref::intrusive_ptr<Simulator>>(m, "Simulator")
	.def("start_tm", &Simulator::t0)
	.def("is_exhausted", &Simulator::is_exhausted)
	.def("__repr__", &Simulator::display_string);
    } /*simulator_py*/
  } /*namespace sim*/
} /*namespace xo*/

/* end SimulatorPy.cpp */

