/* @file FilterPy.cpp */

#include "refcnt/Refcounted.hpp"
#include "filter/KalmanFilterState.hpp"
#include "filter/KalmanFilterTransition.hpp"
#include "filter/KalmanFilterObservable.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
//#include <pybind11/stl.h>
#include <pybind11/chrono.h>
//#include <pybind11/operators.h>

/* xo::ref::intrusive_ptr<T> is an intrusively-reference-counted pointer.
 * always safe to create one from a T* p
 * (since refcount is directly accessible from p)
 */
PYBIND11_DECLARE_HOLDER_TYPE(T, xo::ref::intrusive_ptr<T>, true);

namespace xo {
  using xo::kalman::KalmanFilterState;
  using xo::kalman::KalmanFilterTransition;
  using xo::kalman::KalmanFilterObservable;
  //  using xo::ref::rp;
  using xo::time::utc_nanos;
  using Eigen::VectorXd;
  using Eigen::MatrixXd;
  namespace py = pybind11;

  namespace filter {
    PYBIND11_MODULE(filter_py, m) {
      m.doc() = "pybind11 plugin for xo.filter";

      m.def("print_matrix",
	    [](MatrixXd const & m) {
	      std::cout << m << std::endl;
	    });

      m.def("print_vector",
	    [](VectorXd const & v) {
	      std::cout << v << std::endl;
	    });

      // ----- xo::kalman::KalmanFilterState -----

      py::class_<KalmanFilterState>(m, "KalmanFilterState")
	.def(py::init<uint32_t, utc_nanos, VectorXd, MatrixXd>())
	.def("step_no", &KalmanFilterState::step_no)
	.def("tm", &KalmanFilterState::tm)
	.def("n_state", &KalmanFilterState::n_state)
	.def("state_v", &KalmanFilterState::state_v)
	.def("state_cov", &KalmanFilterState::state_cov)
	.def_property_readonly("k", &KalmanFilterState::step_no)
	.def_property_readonly("tk", &KalmanFilterState::tm)
	.def_property_readonly("x", &KalmanFilterState::state_v)
	.def_property_readonly("P", &KalmanFilterState::state_cov)
	.def("__repr__", &KalmanFilterState::display_string);
      
      // ----- xo::kalman::KalmanFilterTransition -----

      py::class_<KalmanFilterTransition>(m, "KalmanFilterTransition")
	.def(py::init<MatrixXd, MatrixXd>())
	.def("n_state", &KalmanFilterTransition::n_state)
	.def("transition_mat", &KalmanFilterTransition::transition_mat)
	.def("transition_cov", &KalmanFilterTransition::transition_cov)
	.def("check_ok", &KalmanFilterTransition::check_ok)
	.def_property_readonly("F", &KalmanFilterTransition::transition_mat)
	.def_property_readonly("Q", &KalmanFilterTransition::transition_cov)
	.def("__repr__", &KalmanFilterTransition::display_string);

      // ----- xo::kalman::KalmanFilterObservable -----

      py::class_<KalmanFilterObservable>(m, "KalmanFilterObservable")
	.def(py::init<MatrixXd, MatrixXd>())
	.def("n_state", &KalmanFilterObservable::n_state)
	.def("n_observable", &KalmanFilterObservable::n_observable)
	.def("observable_mat", &KalmanFilterObservable::observable)
	.def("observable_cov", &KalmanFilterObservable::observable_cov)
	.def_property_readonly("H", &KalmanFilterObservable::observable)
	.def_property_readonly("R", &KalmanFilterObservable::observable_cov)
	.def("__repr__", &KalmanFilterObservable::display_string);

#ifdef OBSOLETE
      // ----- xo::option::Pxtick -----

      py::enum_<Pxtick>(m, "Pxtick")
	.value("all_penny", Pxtick::all_penny)
	.value("penny_nickel", Pxtick::penny_nickel)
	.value("nickel_dime", Pxtick::nickel_dime);
	//.export_values(); // only need this for pre-c++11-style enum inside a class

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

#endif
    } /*filter_py*/
  } /*namespace filter*/
} /*namespace xo*/

/* end FilterPy.cpp */
