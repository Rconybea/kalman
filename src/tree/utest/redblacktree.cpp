/* @file redblacktree.cpp */

#include "tree/RedBlackTree.hpp"
#include "random/xoshiro256.hpp"
#include "catch2/catch.hpp"
#include <map>

namespace {
  using xo::tree::RedBlackTree;
  using xo::tree::OrdinalReduce;
  using xo::tree::NullReduce;
  using xo::random::xoshiro256;

  using logutil::scope;
  using logutil::xtag;

  using RbTree = RedBlackTree<int, double, OrdinalReduce<double>>;

/* do n random inserts (taken from *p_rgen) into *p_rbtree.
 * inserted keys will be distinct values in [0, .., n-1]
 */
void
random_inserts(uint32_t n,
	       xo::random::xoshiro256 * p_rgen,
	       RbTree * p_rbtree)
{
  REQUIRE(p_rbtree->verify_ok());

  /* n keys 0..n-1 */
  std::vector<uint32_t> u(n);
  for(uint32_t i=0; i<n; ++i)
    u[i] = i;

  /* shuffle to get unpredictable insert order */
  std::shuffle(u.begin(), u.end(), *p_rgen);

  /* insert keys according to permutation u */
  uint32_t i = 1;
  for(uint32_t x : u) {
    //lscope.log(c_self, ": ", i, "/", n, ": insert key", xtag("x", x));
    p_rbtree->insert(x, 10 * x);
    //rbtree.display();
    REQUIRE(p_rbtree->verify_ok());
    ++i;
  }

  REQUIRE(p_rbtree->size() == n);
} /*random_inserts*/

  /* Require:
   * - rbtree has keys [0..n-1], where n=rbtree.size()
   * - rbtree values at key k is dvalue+10*k
   */
  void
  check_bidirectional_iterator(uint32_t dvalue,
			       RbTree const & rbtree)
  {
    size_t const n = rbtree.size();
    size_t i = 0;
    int last_key = -1;

    constexpr char const * c_self = "check_forward_iterator";
    constexpr bool c_logging_enabled = false;
    scope lscope(c_self, c_logging_enabled);

    if(c_logging_enabled)
      lscope.log("tree with size n", xtag("n", n));

    auto end_ix = rbtree.end();

    if(c_logging_enabled)
      lscope.log(xtag("end_ix.locn", end_ix.location()),
		 xtag("end_ix.node", end_ix.node()));

    auto begin_ix = rbtree.begin();
    auto ix = begin_ix;

    while(ix != end_ix) {
      if(c_logging_enabled)
	lscope.log("loop top",
		   xtag("i", i),
		   xtag("locn", ix.location()),
		   xtag("node", ix.node()));

      REQUIRE(ix->first == i);
      REQUIRE(ix->second == dvalue + 10*i);
      if(i > 0) {
	REQUIRE(ix->first > last_key);
      }
      last_key = ix->first;
      ++i;
      ++ix;

      if(c_logging_enabled)
	lscope.log("loop bottom",
		   xtag("last_key", last_key),
		   xtag("next locn", ix.location()),
		   xtag("next node", ix.node()));
    }

    /* should have visited exactly n locations */
    REQUIRE(i == n);

    /* now run iterator backwards,
     * starting from "one past the end"
     */

    if(ix != begin_ix) {
      do {
	--i;
	--ix;

	REQUIRE((*ix).first == i);
      } while(ix != begin_ix);
    }

    /* should have visited exactly n locations in reverse */
    REQUIRE(i == 0);
  } /*check_bidirectional_iterator*/

  /* generate vector with integers [0.. n-1] */
  std::vector<uint32_t>
  vector_upto(uint32_t n)
  {
    std::vector<uint32_t> u(n);
    for(uint32_t i=0; i<n; ++i)
      u[i] = i;

    return u;
  } /*vector_upto*/

  std::map<uint32_t, uint32_t>
  map_upto(uint32_t n)
  {
    std::map<uint32_t, uint32_t> m;
    for(uint32_t i=0; i<n; ++i) {
      m[i] = i;
    }

    return m;
  } /*map_upto*/

  /* generate random permutation of integers [0.. n-1] */
  std::vector<uint32_t>
  random_permutation(uint32_t n,
		     xoshiro256 * p_rgen)
  {
    /* vector [0 .. n-1] */
    std::vector<uint32_t> u = vector_upto(n);

    /* shuffle to get unpredictable permutation */
    std::shuffle(u.begin(), u.end(), *p_rgen);

    return u;
  } /*random_permutation*/

  /* Require:
   * - *p_rbtree has keys [0..n-1],  where n=rbtree.size()
   * - for each key k,  associated value is 10*k
   */
  void
  random_lookups(RbTree const & rbtree,
		 xoshiro256 * p_rgen)
  {
    REQUIRE(rbtree.verify_ok());

    size_t n = rbtree.size();
    std::vector<uint32_t> u
      = random_permutation(n, p_rgen);

    /* lookup keys in permutation order */
    uint32_t i = 1;
    for (uint32_t x : u) {
      INFO(tostr(xtag("i", i), xtag("n", n), xtag("x", x)));

      REQUIRE(rbtree[x] == x*10);
      REQUIRE(rbtree.verify_ok());
      REQUIRE(rbtree.size() == n);
      ++i;
    }

    REQUIRE(rbtree.size() == n);
  } /*random_lookups*/

  /* Require:
   * - *p_rbtree has keys [0..n-1],  where n=rbtree.size()
   * - for each key k,  associated value is 10*k
   *
   * Promise:
   * - for each key k,  associated value is dvalue + 10*k
   */
  void
  random_updates(uint32_t dvalue,
		 RbTree * p_rbtree,
		 xoshiro256 * p_rgen)
  {
    REQUIRE(p_rbtree->verify_ok());

    std::size_t n = p_rbtree->size();
    std::vector<uint32_t> u
      = random_permutation(n, p_rgen);

    /* update key/value pairs in permutation order */
    uint32_t i = 1;
    for (uint32_t x : u) {
      REQUIRE((*p_rbtree)[x] == x*10);

      (*p_rbtree)[x] = dvalue + 10*x;

      REQUIRE((*p_rbtree)[x] == dvalue + 10*x);
      REQUIRE(p_rbtree->verify_ok());
      /* assignment to existing key does not change tree size */
      REQUIRE(p_rbtree->size() == n);
      ++i;
    }

    REQUIRE(p_rbtree->size() == n);
  } /*random_updates_1*/
  
  /* Require:
   * - *p_rbtree has keys [0..n-1], where n=p_rbtree->size()
   */
  void
  random_removes(xoshiro256 * p_rgen,
		 RbTree * p_rbtree)
  {
    REQUIRE(p_rbtree->verify_ok());

    uint32_t n = p_rbtree->size();

    /* random permutation of keys in *p_rbtree */
    std::vector<uint32_t> u
      = random_permutation(n, p_rgen);

    /* will keep track of which keys remain as we move them */
    std::map<uint32_t, uint32_t> m = map_upto(n);
    
    /* remove keys in permutation order */
    uint32_t i = 1;
    for (uint32_t x : u) {
      INFO(tostr("iter i: removing x from n-node tree",
		 xtag("i", i), xtag("x", x), xtag("n", n)));

      /* remove x from tracking map m also */
      m.erase(x);

      // lscope.log(c_self, ": ", i, "/", n, ": remove key", xtag("x", x));
      p_rbtree->remove(x);
      // rbtree.display();
      REQUIRE(p_rbtree->size() == n-i);
      /* amongst other things,  this guarantees that keys in *p_rbtree
       * appear in increasing order
       */
      REQUIRE(p_rbtree->verify_ok());
      /* 1. rbtree should now contain all the keys in [0..n-1],
       *    with u[0]..u[i-1] excluded;  this is the same as the
       *    contents of m.  
       */
      auto m_ix = m.begin();
      auto m_end_ix = m.end();
      auto visitor_fn =
	([&m_ix, m_end_ix]
	 (std::pair<int, double> const & contents)
	{
	  REQUIRE(m_ix != m_end_ix);
	  REQUIRE(contents.first == m_ix->second);
	  ++m_ix;
	});
      p_rbtree->visit_inorder(visitor_fn);
      ++i;
    }

    REQUIRE(m.empty());
    REQUIRE(p_rbtree->size() == 0);
  } /*random_removes*/

TEST_CASE("rbtree", "[redblacktree]") {
  RbTree rbtree;

  uint64_t seed = 14950349842636922572UL;
  /* can reseed from /dev/urandom with: */
  //arc4random_buf(&seed, sizeof(seed));
  
  auto rgen = xo::random::xoshiro256(seed);

  /* check iteration on empty tree */
  check_bidirectional_iterator(0, rbtree);

  for(uint32_t n=1; n<=1024; n*=2) {
    /* insert [0..n-1] in random order */
    random_inserts(n, &rgen, &rbtree);
    /* check iterator traverses [0..n-1] in both directions */
    check_bidirectional_iterator(0, rbtree);
    /* verify behavior of read-only variant of operator[] */
    random_lookups(rbtree, &rgen);
    /* verify that lookups didn't disturb tree contents */
    check_bidirectional_iterator(0, rbtree);
    /* verify update via read/write operator[] */
    random_updates(10000, &rbtree, &rgen);
    /* verify that updates changed tree contents in expected way */
    check_bidirectional_iterator(10000, rbtree);
    /* verify behavior of read/write variant of operator[] */
    random_removes(&rgen, &rbtree);
  }
} /*TEST_CASE(rbtree)*/
} /*namespace*/

/* end redblacktree.cpp */
