/* @file OptionPy.cpp */

#include "refcnt/Refcounted.hpp"
#include "option/OptionStrikeSet.hpp"
#include "option/OptionId.hpp"
#include "option_util/Pxtick.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/operators.h>

/* xo::ref::intrusive_ptr<T> is an intrusively-reference-counted pointer.
 * always safe to create one from a T* p
 * (since refcount is directly accessible from p)
 */
PYBIND11_DECLARE_HOLDER_TYPE(T, xo::ref::intrusive_ptr<T>, true);

namespace xo {
  using xo::option::VanillaOption;
  using xo::option::OptionStrikeSet;
  using xo::option::OptionId;
  using xo::option::Pxtick;
  using xo::ref::rp;
  namespace py = pybind11;

  namespace process {
    PYBIND11_MODULE(option_py, m) {
      m.doc() = "pybind11 plugin for xo.option";

      // ----- xo::option::Pxtick -----

      py::enum_<Pxtick>(m, "Pxtick")
	.value("all_penny", Pxtick::all_penny)
	.value("penny_nickel", Pxtick::penny_nickel)
	.value("nickel_dime", Pxtick::nickel_dime);
	//.export_values(); // only need this for pre-c++11-style enum inside a class

      // ----- xo::option::OptionId -----

      py::class_<OptionId>(m, "OptionId")
        .def(py::init<uint32_t>())
	.def("is_valid", &OptionId::is_valid)
	.def("is_invalid", &OptionId::is_invalid)
	.def_property_readonly("num", &OptionId::num)
	.def("strike_ix", &OptionId::strike_ix)
	.def("strike_pair_ix", &OptionId::strike_pair_ix)
	.def(py::self == py::self)
	.def(py::self != py::self)
	.def(py::self <  py::self)
	.def(py::self <= py::self)
	.def(py::self >= py::self)
	.def(py::self >  py::self)
	.def("__repr__", &OptionId::display_string);

      // ----- xo::option::VanillaOption -----

      py::class_<VanillaOption,
		 rp<VanillaOption>>(m, "VanillaOption")
	.def_property_readonly("id", &VanillaOption::id)
	.def_property_readonly("callput", &VanillaOption::callput)
	.def_property_readonly("stated_strike", &VanillaOption::stated_strike)
	.def_property_readonly("expiry", &VanillaOption::expiry)
	.def_property_readonly("pxtick", &VanillaOption::pxtick)
	.def_property_readonly("pxmult", &VanillaOption::pxmult)
	.def_property_readonly("delivmult", &VanillaOption::delivmult)
	.def_property_readonly("effective_strike", &VanillaOption::effective_strike)
	.def("__repr__", &VanillaOption::display_string);
      
      // ----- xo::option::OptionStrikeSet -----

      py::class_<OptionStrikeSet,
		 rp<OptionStrikeSet>>(m, "OptionStrikeSet")
	.def("n_strike", &OptionStrikeSet::n_strike)
	.def("n_option", &OptionStrikeSet::n_option)
	.def("get_options",
	     [](OptionStrikeSet const & x) {
	       std::vector<rp<VanillaOption>> v;
	       x.append_options(&v);
	       return v;
	     })
	.def("__repr__", &OptionStrikeSet::display_string);

      // OptionStrikeSet::regular(n, start_id, lo_strike, d_strike, expiry, pxtick);
      m.def("make_option_strike_set", &OptionStrikeSet::regular,
	    py::arg("n"), py::arg("start_id"), py::arg("lo_strike"), py::arg("hi_strike"),
	    py::arg("expiry"), py::arg("pxtick"));

    } /*option_py*/
  } /*namespace process*/
} /*namespace xo*/

/* end OptionPy.cpp */
