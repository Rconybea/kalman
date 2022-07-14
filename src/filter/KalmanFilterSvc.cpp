/* @file KalmanFilterSvc.cpp */

#include "KalmanFilterSvc.hpp"

namespace xo {
  namespace kalman {
    KalmanFilterSvc::KalmanFilterSvc(KalmanFilterSpec spec)
      : filter_{std::move(spec)}
    {}

    namespace {
      class XferInputCb : public KalmanFilterInputCallback {
      public:
	XferInputCb(KalmanFilterSvc * svc) : svc_{svc} {}

	virtual void notify_input(KalmanFilterInput const & input_kp1) {
	  KalmanFilterSvc * svc = this->svc_;

	  if (svc) {
	    svc->notify_input(input_kp1);
	  }
	} /*notify_input*/

      private:
	/* destination for input events.
	 * naked pointer to avoid refcount cycle
	 */
	KalmanFilterSvc * svc_ = nullptr;
      }; /*XferInputCb*/
    } /*namespace*/

    void
    KalmanFilterSvc::attach_input(ref::rp<KalmanFilterInputSource> input_src)
    {
      /* only allowed to call .attach_input() once per KalmanFilterSvc  */
      assert(!this->input_src_.get());

      this->input_src_ = input_src;

      input_src->add_callback(new XferInputCb(this));

    } /*attach_input*/

    void
    KalmanFilterSvc::notify_input(KalmanFilterInput const & input_kp1)
    {
      this->filter_.notify_input(input_kp1);

      this->pub_.invoke(&KalmanFilterOutputCallback::notify_filter,
			this->filter_.state_ext());
    } /*notify_input*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterSvc.cpp */
