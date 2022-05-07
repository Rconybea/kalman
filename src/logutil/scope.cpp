/* @file scope.cpp */

#include "scope.hpp"
#include <memory>

namespace logutil {
  using std::stringstream;

  // track per-thread state associated with nesting logger
  //
  class state_impl {
  public:
    state_impl() = default;

    void incr_nesting() { ++nesting_level_; }
    void decr_nesting() { --nesting_level_; }

    stringstream & ss() { return ss_; }

  private:
    /* current nesting level for this thread */
    uint32_t nesting_level_ = 0;
    /* buffer space for logging;
     * reused across tos() and scope::log() calls
     */ 
    stringstream ss_;
  }; /*state_impl*/

  thread_local std::unique_ptr<state_impl> s_state;

  stringstream &
  scope::require_thread_local_stream()
  {
    return require_thread_local_state()->ss();
  } /*require_thread_local_straem*/

  state_impl *
  scope::require_thread_local_state()
  {
    if(!s_state) {
      s_state.reset(new state_impl);
    }

    return s_state.get();
  } /*require_thread_local_state*/

  scope::scope() {
    require_thread_local_state()->incr_nesting();
  } /*ctor*/

  scope::~scope() {
    require_thread_local_state()->decr_nesting();
  } /*dtor*/

} /*namespace logutil*/

/* end scope.cpp */
