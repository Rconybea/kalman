/* @file KalmanFilterSvc.hpp */

#include "queue/Sink.hpp"
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
    class KalmanFilterSvc : public xo::reactor::Sink, public xo::reactor::Source {
    public:
      using Source = xo::reactor::Source;
      
    public:
      /* named ctor idiom */
      static ref::rp<KalmanFilterSvc> make(KalmanFilterSpec spec);

      KalmanFilter const & filter() const { return filter_; }

      /* provide input observation stream for a kalman filter */
      void attach_input(ref::rp<KalmanFilterInputSource> input_src);

      /* add callback to receive filter output */
      void add_filter_callback(ref::rp<KalmanFilterOutputCallback> const & cb) {
	this->pub_.add_callback(cb);
	//this->pub_->add_callback(cb);
      }

      /* notify incoming observations;  will trigger kalman filter step */
      void notify_input(KalmanFilterInput const & input_kp1);

      // ----- inherited from reactor::Sink -----

      /* provide source of kalman filter input events.
       * src.get() must dynamic cast to KalmanFilterInputSource
       */
      virtual void attach_source(ref::rp<Source> src) override;

    private:
      KalmanFilterSvc(KalmanFilterSpec spec);

    private:
      /* passive kalman filter */
      KalmanFilter filter_;
      /* receive filter input from this source; see .attach_input() */
      ref::rp<KalmanFilterInputSource> input_src_;
      /* publish filter state updates to these callbacks */
      fn::CallbackSet<ref::rp<KalmanFilterOutputCallback>> pub_;
    }; /*KalmanFilterSvc*/
  } /*namespace kalman*/
} /*namespace xo*/

/* KalmanFilterSvc.hpp */
