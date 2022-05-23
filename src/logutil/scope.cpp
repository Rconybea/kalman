/* @file scope.cpp */

#include "scope.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <cassert>

namespace logutil {
  using std::ostream;
  using std::stringstream;
  using std::streambuf;
  using std::streamsize;
  using std::ios_base;

  /* recycling buffer for logging.
   * write to self-extending storage array;
   */
  class log_streambuf : public streambuf {
  public:
    log_streambuf(uint32_t buf_z) {
      this->buf_v_.resize(buf_z);
      
      this->reset_stream();
    } /*ctor*/

    streamsize capacity() const { return this->buf_v_.size(); }
    char const * lo() const { return this->pbase(); }
    uint32_t pos() const { return this->pptr() - this->pbase(); }

    void reset_stream() {
      char * p_lo = &(this->buf_v_[0]);
      char * p_hi = p_lo + this->capacity();

      this->setp(p_lo, p_hi);
    } /*reset_stream*/

  protected:
    virtual std::streamsize
    xsputn(char const * s, streamsize n) override
    {
      //std::cout << "xsputn: s=" << s << ", n=" << n << std::endl;

      std::memcpy(this->pptr(), s, n);
      this->pbump(n);
      return n;
    } /*xsputn*/

    virtual int_type
    overflow(int_type ch) override
    {
      // logging stream buffer doesn't handle overflow for now.
      // to handle overflow:
      // - increase size of .buf_v[]
      //
      throw std::runtime_error("log_stream overflow");
    } /*overflow*/

    /* off.   offset, relative to starting point dir.
     * dir.
     * which. in|out|both
     */
    virtual pos_type seekoff(off_type off,
		     std::ios_base::seekdir dir,
                     std::ios_base::openmode which) override {
      //std::cout << "seekoff: off=" << off << ", dir=" << dir << ", which=" << which << std::endl;

      // Only output stream is supported
      if (which != std::ios_base::out)
        throw std::runtime_error("log_streambuf: only output mode supported");

      if (dir == std::ios_base::cur) {
        this->pbump(off);
      } else if (dir == std::ios_base::end) {
        /* .setp(): using for side effect: sets .pptr to .pbase */
        this->setp(this->pbase(), this->epptr());
        this->pbump(off);
      } else if (dir == std::ios_base::beg) {
        /* .setp(): using for side effect: sets .pptr to .pbase */
        this->setp(this->pbase(), this->epptr());
        this->pbump(this->capacity() + off);
      }

      return this->pptr() - this->pbase();
    } /*seekoff*/

  private:
    /* buffered output stored here */
    std::vector<char> buf_v_;
  }; /*log_streambuf*/

  // track per-thread state associated with nesting logger
  //
  class state_impl {
  public:
    state_impl();

    void incr_nesting() { ++nesting_level_; }
    void decr_nesting() { --nesting_level_; }

    ostream & ss() { return ss_; }

    /* call on entry to new scope */
    void preamble(char const * name);
    /* call before each new log entry */
    void indent(char pad_char);
    /* call on exit from scope */
    void postamble(char const * name);

    /* write collected output to std::clog */
    void flush2clog();

    /* discard output, reset write pointer to beginning of buffer */
    void reset_stream() {
      p_sbuf_phase1_->reset_stream();
      p_sbuf_phase2_->reset_stream();
    }

  private:
    /* common implementation for .preamble(), .postamble() */
    void entryexit_aux(char const * name, char label_char);

  private:
    /* current nesting level for this thread */
    uint32_t nesting_level_ = 0;

    /* buffer space for logging
     * (before pretty-printing for scope::log() calls that span multiple lines)
     * reused across tos() and scope::log() calls
     */ 
    std::unique_ptr<log_streambuf> p_sbuf_phase1_;

    /* buffer space for handling scope::log() calls that span multiple lines;
     * inserts extra characters in effort to indent gracefully
     */
    std::unique_ptr<log_streambuf> p_sbuf_phase2_;

    /* output stream -- always attached to .p_sbuf_phase1
     * stream inserters for application datatypes will target this stream
     */
    ostream ss_;
  }; /*state_impl*/

  constexpr uint32_t c_default_buf_size = 1024;

  state_impl::state_impl()
    : p_sbuf_phase1_(new log_streambuf(c_default_buf_size)),
      p_sbuf_phase2_(new log_streambuf(c_default_buf_size)),
      ss_(p_sbuf_phase1_.get())
  {
    assert(p_sbuf_phase1_.get() == ss_.rdbuf());
  } /*ctor*/

  void
  state_impl::indent(char pad_char)
  {
    log_streambuf * sbuf = this->p_sbuf_phase1_.get();

#ifdef NOT_IN_USE
    {
      char buf[80];
      ::snprintf(buf, sizeof(buf), "[%02d] ", this->nesting_level_);

      this->ss_ << buf;
      //this->p_sbuf_->sputn(buf, strlen(buf));
    }
#endif

    /* indent to nesting level */
    for(uint32_t i = 0, n = this->nesting_level_; i<n; ++i) {
      this->ss_ << pad_char;
    }
  } /*indent*/

  void
  state_impl::entryexit_aux(char const * name,
			    char label_char)
  {
    log_streambuf * sbuf = this->p_sbuf_phase1_.get();

    sbuf->reset_stream();
    this->indent(label_char);

    /* mnemonic for scope entry/exit */
    this->ss_ << label_char;

    /* scope name */
    this->ss_ << name << "\n";
  } /*entryexit_aux*/

  void
  state_impl::preamble(char const * name)
  {
    this->entryexit_aux(name, '+' /*label_char*/);
  } /*preamble*/

  void
  state_impl::postamble(char const * name)
  {
    this->entryexit_aux(name, '-' /*label_char*/);
  }  /*postamble*/

  void
  state_impl::flush2clog()
  {
    log_streambuf * sbuf1 = this->p_sbuf_phase1_.get();
    log_streambuf * sbuf2 = this->p_sbuf_phase2_.get();

    /* expecting sbuf to contain one line of output.
     * if it contains multiple newlines,  need to indent
     * after each one.
     *
     * will scan output in *sbuf1,  post-process to *sbuf2,
     * then write *sbuf2 to clog
     */
    char const * s = sbuf1->lo();
    char const * e = s + sbuf1->pos();

    char const * p = s;

    /* point to first space following a non-space character.
     * will indent to just after this space
     */
    char const * space_after_nonspace = nullptr;

    while(true) {
      bool have_nonspace = false;

      /* invariant: s<=p<=e */

      /* for indenting,  looking for first 'space following non-space, on first line', if any */

      while(p < e) {
	if(space_after_nonspace) {
	  ;
	} else {
	  if(*p != ' ')
	    have_nonspace = true;

	  if(have_nonspace && (*p == ' ')) {
	    space_after_nonspace = p;
	  }
	}

	if(*p == '\n') {
	  ++p;
	  break;
	} else {
	  ++p;
	}
      }

      /* p=e or *p=\n */

      /* charseq [s,p) does not contain any newlines,  print it */
      sbuf2->sputn(s, p - s);

      if(p == e) {
	break;
      }

      // {
      //   char buf[80];
      //   snprintf(buf, sizeof(buf), "*** indent=[%d] next=[%c]", this->nesting_level_, *(p+1));
      //
      //   std::clog.rdbuf()->sputn(buf, strlen(buf));
      //}

      /* at least 1 char following newline,  need to indent for it
       * - minimum indent = nesting level;
       * - however if space_after_nonspace defined, indent to that
       */
      uint32_t n_indent = this->nesting_level_;

      if(space_after_nonspace)
	n_indent += (space_after_nonspace - s);

      for(uint32_t i = 0; i < n_indent; ++i)
	sbuf2->sputc(' ');
	
      s = p;
    }

    /* now write entire contents of *sbuf2 to clog */
    std::clog.rdbuf()->sputn(sbuf2->lo(), sbuf2->pos());

    /* reset streams for next message */
    this->reset_stream();
  } /*flush2clog*/

  /* keep logging state separately for each thread */
  thread_local
  std::unique_ptr<state_impl> s_state;

  state_impl *
  scope::require_indent_thread_local_state()
  {
    state_impl * local_state = require_thread_local_state();

    local_state->reset_stream();
    local_state->indent(' ' /*pad_char*/);

    return local_state;
  } /*require_thread_local_stream*/

  state_impl *
  scope::require_thread_local_state()
  {
    if(!s_state) {
      s_state.reset(new state_impl());
    }

    return s_state.get();
  } /*require_thread_local_state*/

  std::ostream &
  scope::logstate2stream(state_impl * logstate)
  {
    return logstate->ss();
  } /*logstate2stream*/

  void
  scope::flush2clog(state_impl * logstate) {
    logstate->flush2clog(); 
  } /*flush2clog*/

  scope::scope(char const * fn, bool enabled_flag)
    : name_(fn),
      finalized_(!enabled_flag)
  {
    if(enabled_flag) {
      state_impl * logstate = scope::require_thread_local_state();

      logstate->preamble(this->name_);
      logstate->flush2clog();

      ///* next call to scope::log() can reset to beginning of buffer space */
      //logstate->ss().seekp(0);

      logstate->incr_nesting();
    }
  } /*ctor*/

  scope::scope(char const * fn) : scope(fn, true /*enabled_flag*/) {}

  void
  scope::end_scope()
  {
    if(!this->finalized_) {
      this->finalized_ = true;

      state_impl * logstate
	= scope::require_thread_local_state();

      logstate->decr_nesting();

      logstate->postamble(this->name_);
      logstate->flush2clog();
    }
  } /*end_scope*/

  scope::~scope() {
    if(!this->finalized_)
      this->end_scope();
  } /*dtor*/

} /*namespace logutil*/

/* end scope.cpp */
