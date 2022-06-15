/* @file DistributionPy.cpp */

#include <pybind11/pybind11.h>
#include "distribution/Normal.hpp"

namespace xo {
  using xo::distribution::Normal;

  namespace sim {
    namespace py = pybind11;

    int
    xoadd(int i, int j) {
      return i + j;
    }

    PYBIND11_MODULE(distribution_py, m) {
      m.doc() = "pybind11 distribution plugin"; // optional module docstring

      // python: simulator_py.xoadd(x,y)
      m.def("normalcdf",
	    &Normal::cdf_impl,
	    "cumulative normal distribution",
	    py::arg("x"));
    }
  } /*namespace sim*/
} /*namespace xo*/

/* end DistributionPy.cpp */
