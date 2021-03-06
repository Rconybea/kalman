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

	virtual void notify_ev(KalmanFilterInput const & input_kp1) override {
	  KalmanFilterSvc * svc = this->svc_;

	  if (svc) {
	    svc->notify_ev(input_kp1);
	  }
	} /*notify_input*/

	virtual std::string_view self_typename() const override {
	  return reflect::type_name<XferInputCb>();
	}

	virtual std::string_view parent_typename() const override {
	  return reflect::type_name<KalmanFilterInputCallback>();
	}

	virtual std::type_info const * parent_typeinfo() const override {
	  return &typeid(KalmanFilterInputCallback);
	}

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
    KalmanFilterSvc::notify_ev(KalmanFilterInput const & input_kp1)
    {
      this->filter_.notify_input(input_kp1);

      this->pub_.invoke(&KalmanFilterOutputCallback::notify_ev,
			this->filter_.state_ext());
    } /*notify_input*/

    void
    KalmanFilterSvc::attach_source(rp<AbstractSource> const & src)
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
