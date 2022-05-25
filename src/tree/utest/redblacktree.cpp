/* @file redblacktree.cpp */

#include "tree/RedBlackTree.hpp"
#include "random/xoshiro256.hpp"
#include "catch2/catch.hpp"

using xo::tree::RedBlackTree;
using xo::tree::NullReduce;

namespace {
  using logutil::scope;
  using logutil::xtag;

/* do n random inserts (taken from *p_rgen) into *p_rbtree.
 * inserted keys will be distinct values in [0, .., n-1]
 */
void
random_inserts(uint32_t n,
	       xo::random::xoshiro256 * p_rgen,
	       RedBlackTree<int, double, NullReduce<int>> * p_rbtree)
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
   */
  void
  check_bidirectional_iterator(RedBlackTree<int, double, NullReduce<int>> const & rbtree)
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

      REQUIRE((*ix).first == i);
      if(i > 0) {
	REQUIRE((*ix).first > last_key);
      }
      last_key = (*ix).first;
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

/* Require:
 * - *p_rbtree has keys [0..n-1], where n=p_rbtree->size()
 */
void
random_removes(xo::random::xoshiro256 * p_rgen,
	       RedBlackTree<int, double, NullReduce<int>> * p_rbtree)
{
  REQUIRE(p_rbtree->verify_ok());

  /* n keys 0..n-1 */
  uint32_t n = p_rbtree->size();
  std::vector<uint32_t> u(n);
  for(uint32_t i=0; i<n; ++i)
    u[i] = i;

  /* shuffle to get unpredictable insert order */
  std::shuffle(u.begin(), u.end(), *p_rgen);

  /* remove keys according to permutation u */
  uint32_t i = 1;
  for (uint32_t x : u) {
    //lscope.log(c_self, ": ", i, "/", n, ": remove key", xtag("x", x));
    p_rbtree->remove(x);
    //rbtree.display();
    REQUIRE(p_rbtree->verify_ok());
    ++i;
  }

  REQUIRE(p_rbtree->size() == 0);
} /*random_removes*/

} /*namespace*/

TEST_CASE("rbtree", "[redblacktree]") {
  RedBlackTree<int, double, NullReduce<int>> rbtree;

  uint64_t seed = 14950349842636922572UL;
  /* can reseed from /dev/urandom with: */
  //arc4random_buf(&seed, sizeof(seed));
  
  auto rgen = xo::random::xoshiro256(seed);

  /* check iteration on empty tree */
  check_bidirectional_iterator(rbtree);

  for(uint32_t n=1; n<=1024; n*=2) {
    random_inserts(n, &rgen, &rbtree);
    check_bidirectional_iterator(rbtree);
    random_removes(&rgen, &rbtree);
  }
} /*TEST_CASE(rbtree)*/

/* end redblacktree.cpp */
