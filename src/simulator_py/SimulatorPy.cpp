/* @file SimulatorPy.cpp */

#include <pybind11/pybind11.h>

namespace xo {
  namespace sim {
    namespace py = pybind11;

    int
    xoadd(int i, int j) {
      return i + j;
    }

    PYBIND11_MODULE(simulator_py, m) {
      m.doc() = "pybind11 example plugin for xo"; // optional module docstring

      // python: simulator_py.xoadd(x,y)
      m.def("xoadd",
	    &xoadd,
	    "A function that adds two numbers",
	    py::arg("x"),
	    py::arg("y"));
    }
  } /*namespace sim*/
} /*namespace xo*/

/* end SimulatorPy.cpp */
