/* @file CallbackSet.hpp */

#pragma once

#include <vector>

namespace xo {
  namespace fn {
      
    /*   queue add/remove callback instructions encountered during callback
     *   execution, to avoid invalidating vector iterator.
     *
     */
    template<typename Fn>
    struct ReentrantCbsetCmd {
      enum CbsetCmdEnum { AddCallback, RemoveCallback };

      ReentrantCbsetCmd() = default;
      ReentrantCbsetCmd(CbsetCmdEnum cmd, Fn const & fn) : cmd_(cmd), fn_(fn) {}

      static ReentrantCbsetCmd add(Fn const & fn) {
	return ReentrantCbsetCmd{AddCallback, fn};
      } /*add*/

      static ReentrantCbsetCmd remove(Fn const & fn) {
	return ReentrantCbsetCmd{RemoveCallback, fn};
      } /*remove*/

      bool is_add() const { return cmd_ == AddCallback; }
      bool is_remove() const { return cmd_ == RemoveCallback; }
      Fn const & fn() const { return fn_; }

    private:
      /* AddCallback:    deferred CallbackSet<Fn>::add_callback(.fn) 
       * RemoveCallback: deferred CallbackSet<Fn>::remove_callback(.fn)
       */
      CbsetCmdEnum cmd_ = AddCallback;
      Fn fn_;
    }; /*ReentrantCbsetCmd*/
    
    /* If Fnptr is a type such that this works:
     *   Fnptr fn = ...;
     *   using Fn = Fnptr::destination_type;
     *   Fn * native_fn = fn.get();
     *   (native_fn->*member_fn)(args ...);
     * 
     * then
     *   CallbackSet<Fnptr> cbset = ...;
     *   cbset.invoke(&Fn::member_fn, args...)
     *
     * calls
     *   (cb->*member_fn)(args...)
     *
     * for each callback cb in this set.
     *
     * In addition,  calls hook methods:
     *   cb->notify_add_callback()
     *   cb->notify_remove_callback()
     * when adding/removing callback.
     *
     * Require:
     * - can invoke (Fnptr->*member_fn)(...)
     *
     * implementation is reentrant: running callbacks can safely make
     * add/remove calls on the cbset that invoked them.
     *
     * not threadsafe.
     */
    template<typename Fn>
    class CallbackSetImpl {
    public:
      using callback_type = typename Fn::destination_type;

    public:
      CallbackSetImpl() = default;

      /* invoke callbacks registered with this callback set */
      template<typename ... Tn, typename ... Sn>
      void invoke(void (callback_type::* member_fn)(Sn... args), Tn&&... args) {
	this->cb_running_ = true;

	try {
	  for(Fn const & cb : this->cb_v_) {
	    callback_type * native_cb = cb.get();

	    /* clang11 doesn't like
	     *   cb->*member_fn
	     * when cb-> is overloaded
	     */
	    (native_cb->*member_fn)(args...);
	  }

	  this->make_deferred_changes();
	} catch(...) {
	  this->make_deferred_changes();
	  throw;
	}
      } /*operator()*/

      /* add callback target_fn to this callback set.
       * reentrant
       */
      void add_callback(Fn const & target_fn) {
	if(this->cb_running_) {
	  /* defer until callback execution completes */
	  this->reentrant_cmd_v_.push_back(ReentrantCbsetCmd<Fn>::add(target_fn));
	} else {
	  this->cb_v_.push_back(target_fn);
	}
      } /*add_callback*/
      
      /* remove callback target_fn from this callback set.
       * noop if callback is not present
       */
      void remove_callback(Fn const & target_fn) {
	if(this->cb_running_) {
	  /* defer until callback execution completes */
	  this->reentrant_cmd_v_.push_back(ReentrantCbsetCmd<Fn>::remove(target_fn));
	} else {
	  this->remove_callback_impl(target_fn);
	}
      } /*remove_callback*/

    private:
      /* apply deferred changes to .cb_v[] */
      void make_deferred_changes() {
	this->cb_running_ = false;

	std::vector<ReentrantCbsetCmd<Fn>> cmd_v;
	std::swap(cmd_v, this->reentrant_cmd_v_);

	for(ReentrantCbsetCmd<Fn> const & cmd : cmd_v) {
	  if(cmd.is_add()) {
	    this->cb_v_.push_back(cmd.fn());

	    cmd.fn()->notify_add_callback();
	  } else if(cmd.is_remove()) {
	    this->remove_callback_impl(cmd.fn());
	  }
	} 
      } /*make_deferred_changes*/

      void remove_callback_impl(Fn const & target_fn) {
	auto ix = std::find(this->cb_v_.begin(), this->cb_v_.end(), target_fn);

	if(ix != this->cb_v_.end())
	  this->cb_v_.erase(ix);

	target_fn->notify_remove_callback();
      } /*remove_callback_impl*/

    private:
      bool cb_running_ = false;
      /* collection of callback functions */
      std::vector<Fn> cb_v_;
      /* when a callback registered with *this, while running,
       * attempts to add/remove a callback to/from this set
       * (including removing itself),
       * must defer until all callbacks have executed.
       * remember deferred instructions here.
       */
      std::vector<ReentrantCbsetCmd<Fn>> reentrant_cmd_v_;
    }; /*CallbackSetImpl*/

    template<typename NativeFn>
    using RpCallbackSet = CallbackSetImpl<xo::ref::rp<NativeFn>>;

    /* like RpCallbackSet<NativeFn>,
     * but also provides overload(s) for operator()(..)
     *
     * FOR NOW: only implemented for callback methods that take
     *          0 or 1 arguments
     */
    template<typename NativeFn, typename MemberFn>
    class NotifyCallbackSet : public RpCallbackSet<NativeFn> {
    public:
      NotifyCallbackSet(MemberFn fn)
	: privileged_member_fn_{fn} {}

      template<typename ... Tn>
      void operator()(Tn&&... args) {
	this->invoke(this->privileged_member_fn_, args...);
      } /*operator()*/

    private:
      /* implements operator()(...) */
      MemberFn privileged_member_fn_;
    }; /*NotifyCallbackSet*/

    template<typename NativeFn, typename Sret, typename ... Sn>
    inline NotifyCallbackSet<NativeFn, Sret (NativeFn::*)(Sn...)>
    make_notify_cbset(Sret (NativeFn::* member_fn)(Sn...)) {
      return NotifyCallbackSet<NativeFn, Sret (NativeFn::*)(Sn...)>(member_fn);
    } /*make_notify_cbset*/
  } /*namespace fn*/
} /*namespace xo*/

/* end CallbackSet.hpp */
