/* @file PollingReactor.cpp */

#include "queue/PollingReactor.hpp"

namespace xo {
  using std::size_t;
  using std::uint64_t;
  using std::int64_t;

  namespace reactor {
    void
    PollingReactor::add_source(Source * src)
    {
      /* make sure src does not already appear in .source_v[] */
      for(Source * x : this->source_v_) {
	if(x == src) {
	  throw std::runtime_error("PollingReactor::add_source; source already present");
	  return;
	}
      }

      src->notify_reactor_add(this);

      this->source_v_.push_back(src);
    } /*add_source*/

    void
    PollingReactor::remove_source(Source * src)
    {
      auto ix = std::find(this->source_v_.begin(),
			  this->source_v_.end(),
			  src);

      if(ix != this->source_v_.end()) {
	src->notify_reactor_remove(this);

	this->source_v_.erase(ix);
      }
    } /*remove_source*/

    int64_t
    PollingReactor::find_nonempty_source(size_t start_ix)
    {
      size_t z = this->source_v_.size();

      /* search sources [ix .. z) */
      for(size_t ix = start_ix; ix < z; ++ix) {
	Source * src = this->source_v_[ix];

	if(src->is_nonempty())
	  return ix;
      }

      /* search source [0 .. ix) */
      for(size_t ix = 0, n = std::min(start_ix, z); ix < n; ++ix) {
	Source * src = this->source_v_[ix];

	if(src->is_nonempty())
	  return ix;
      } 

      return -1;
    } /*find_nonempty_source*/

    uint64_t
    PollingReactor::run_one()
    {
      int64_t ix = this->find_nonempty_source(this->next_ix_);

      if(ix >= 0) {
	Source * src = this->source_v_[ix];

	return src->deliver_one();
      } else {
	return 0;
      }
    } /*run_one*/
  } /*namespace reactor*/
} /*namespace xo*/

/* end PollingReactor.cpp */
