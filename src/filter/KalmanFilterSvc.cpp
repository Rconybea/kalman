/* @file KalmanFilterSvc.cpp */

#include "KalmanFilterSvc.hpp"

namespace xo {
  using xo::ref::rp;

  namespace kalman {
    rp<KalmanFilterSvc>
    KalmanFilterSvc::make(KalmanFilterSpec spec)
    {
      return new KalmanFilterSvc(std::move(spec));
    } /*make*/

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

    void
    KalmanFilterSvc::attach_source(rp<Source> src)
    {
      /* Source must actually be a KalmanFilterInputSource */
      KalmanFilterInputSource * kf_src
	= dynamic_cast<KalmanFilterInputSource *>(src.get());

      if (!kf_src) {
	throw std::runtime_error("KalmanFilterSvc::attach_source: "
				 " expected src to be a KalmanFilterInputSource");
      }

      this->attach_input(kf_src);
    } /*attach_source*/
  } /*namespace kalman*/
} /*namespace xo*/

/* end KalmanFilterSvc.cpp */
