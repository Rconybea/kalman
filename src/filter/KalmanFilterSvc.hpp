/* @file KalmanFilterSvc.hpp */

#include "reactor/Sink.hpp"
#include "filter/KalmanFilter.hpp"
#include "filter/KalmanFilterInputSource.hpp"
#include "filter/KalmanFilterOutputCallback.hpp"
#include "callback/CallbackSet.hpp"

namespace xo {
  namespace kalman {
    /* encapsulate a passive KalmanFilter
     * instance as an active event consumer+producer
     *
     * sinks that want to consume KalmanFilterSvc events will use
     * .add_filter_callback()
     */
    class KalmanFilterSvc : public xo::reactor::Sink1<KalmanFilterInput>,
			    public xo::reactor::AbstractSource {
    public:
      using AbstractSource = xo::reactor::AbstractSource;
      
    public:
      /* named ctor idiom */
      static ref::rp<KalmanFilterSvc> make(KalmanFilterSpec spec);

      KalmanFilter const & filter() const { return filter_; }

      /* provide input observation stream for a kalman filter */
      void attach_input(ref::rp<KalmanFilterInputSource> input_src);

      /* add callback to receive filter output */
      void add_filter_callback(ref::rp<KalmanFilterOutputCallback> const & cb) {
	this->pub_.add_callback(cb);
      }

      /* reverse the effect of .add_callback(cb) */
      void remove_filter_callback(ref::rp<KalmanFilterOutputCallback> const & cb) {
	this->pub_.remove_callback(cb);
      }

      /* notify incoming observations;  will trigger kalman filter step */
      void notify_ev(KalmanFilterInput const & input_kp1) override;

      // ----- inherited from reactor::AbstractSink -----

      /* provide source of kalman filter input events.
       * src.get() must dynamic cast to KalmanFilterInputSource
       */
      virtual void attach_source(ref::rp<AbstractSource> const & src) override;

      // ----- inherited from reactor::AbstractSource -----

      virtual void attach_sink(ref::rp<AbstractSink> const & sink) override {
	constexpr std::string_view c_self_name = "KalmanFilterSvc::attach_sink";

	this->add_filter_callback
	  (Sink1<KalmanFilterStateExt>::require_native(c_self_name, sink));
      } /*attach_sink*/

      virtual void detach_sink(ref::rp<AbstractSink> const & sink) override {
	constexpr std::string_view c_self_name = "KalmanFilterSvc::detach_sink";

	this->remove_filter_callback
	  (Sink1<KalmanFilterStateExt>::require_native(c_self_name, sink));
      } /*detach_sink*/

    private:
      KalmanFilterSvc(KalmanFilterSpec spec);

    private:
      /* passive kalman filter */
      KalmanFilter filter_;
      /* receive filter input from this source; see .attach_input() */
      ref::rp<KalmanFilterInputSource> input_src_;
      /* publish filter state updates to these callbacks */
      fn::RpCallbackSet<KalmanFilterOutputCallback> pub_;
    }; /*KalmanFilterSvc*/
  } /*namespace kalman*/
} /*namespace xo*/

/* KalmanFilterSvc.hpp */
