/* @file RedBlackTree.hpp */

/* provides red-black tree with order statistics.
 */

#pragma once

#include "logutil/scope.hpp"
#include "logutil/pad.hpp"
#include <concepts>
#include <iterator>
#include <array>
#include <cmath>
#include <cassert>

namespace xo {
namespace tree {

  /* concept for the 'Reduce' argument to RedBlackTree<...>
   * 
   * e.g.
   *   struct ReduceCountAndSum {
   *     using value_type = std::pair<uint32_t, int64_t>:
   *
   *     value_type nil() { return value_type(0, 0); }
   *     value_type operator()(value_type const & acc, int64_t val)
   *       { return value_type(acc.first + val.first, acc.second + val.second); }
   *     value_type operator()(value_type const & a1, value_type const & a2)
   *       { return value_type(a1.first + a2.first, a1.second + a2.second); }
   *   };
   */
  template <class T, typename Value>
  concept ReduceConcept = requires(T r, Value v, typename T::value_type a) { 
    typename T::value_type;
    { r.nil() } -> std::same_as<typename T::value_type>;
    { r.leaf(v) } -> std::same_as<std::pair<typename T::value_type, typename T::value_type>>;
    { r(a, v) } -> std::same_as<typename T::value_type>;
    { r.combine(a, a) } -> std::same_as<typename T::value_type>;
  };

  /* reduce function that disappears at compile time */
  template<typename NodeValue>
  struct NullReduce;

  /* red-black tree with order statistics
   *
   * require:
   * - Key is equality comparable
   * - Key, Value, Reduce are copyable and null-constructible
   * - Reduce.value_type = Accumulator
   * - Reduce.operator() :: (Accumulator x Key) -> Accumulator
   * - Reduce.operator() :: (Accumulator x Accumulator) -> Accumulator
   */
  template <typename Key, typename Value, typename Reduce = NullReduce<Key>>
  class RedBlackTree;

  namespace detail {
    enum Color { C_Invalid = -1, C_Black, C_Red, N_Color };

    enum Direction { D_Invalid = -1, D_Left, D_Right, N_Direction };

    inline Direction other(Direction d) {
      return static_cast<Direction>(1 - d);
    } /*other*/

    template <typename Key, typename Value, typename Reduce>
    class RbTreeUtil;

    /* xo::tree::detail::Node
     *
     * Require:
     * - Key.operator<
     * - Key.operator==
     *
     */
    template <typename Key,
	      typename Value,
	      typename Reduce>
    class Node {
    public:
      using ReducedValue = typename Reduce::value_type;
      using ContentsType = std::pair<Key, Value>;

    public:
      Node() = default;
      Node(Key const & k, Value const &v,
	   std::pair<ReducedValue, ReducedValue> const & r)
	: color_(C_Red), size_(1), contents_(k, v), reduced_(r) {}
      Node(Key && k, Value && v, std::pair<ReducedValue, ReducedValue> && r)
	: color_(C_Red), size_(1), contents_{std::move(k), std::move(v)},
	  reduced_{std::move(r)} {}

      /* return #of key/vaue pairs in tree rooted at x. */
      static size_t tree_size(Node *x) {
        if (x)
          return x->size();
        else
          return 0;
      } /*tree_size*/

      static bool is_black(Node *x) {
        if (x)
          return x->is_black();
        else
          return true;
      } /*is_black*/

      static bool is_red(Node *x) {
        if (x)
          return x->is_red();
        else
          return false;
      } /*is_red*/

      static Direction child_direction(Node *p, Node *n) {
        if (p) {
          return p->child_direction(n);
        } else {
          return D_Invalid;
        }
      } /*child_direction*/

      static ReducedValue reduce_aux(Reduce reduce, Node *x)
      {
	if(x)
	  return x->reduced2();
	else
	  return reduce.nil();
      } /*reduce_aux*/

      /* calculate reduced values for node x.
       * does not used x.reduced
       */
      static std::pair<ReducedValue,
		       ReducedValue> reduced_pair(Reduce r, Node const * x)
      {
	if(!x)
	  assert(false);

	ReducedValue r1 = r(reduce_aux(r, x->left_child()),
			    x->value());
	ReducedValue r2 = r.combine(r1,
				    reduce_aux(r, x->right_child()));
	return std::pair<ReducedValue, ReducedValue>(r1, r2);
      } /*reduced_pair*/

      /* replace root pointer *pp_root with x;
       * set x parent pointer to nil
       */
      static void replace_root_reparent(Node *x, Node **pp_root) {
        *pp_root = x;
        if (x)
          x->parent_ = nullptr;
      } /*replace_root_reparent*/

      size_t size() const { return size_; }
      /* const access */
      ContentsType const & contents() const { return contents_; }
      /* non-const value access.   
       *
       * editorial: would prefer to return
       *   std::pair<Key const, Value> &
       * here,  so that tree[k].first = newk
       * prohibited,  but std::pair<Key const, Value>
       * is considered unrelated to std::pair<Key, Value>,
       * so l-value conversion not allowed
       */
      ContentsType & contents() { return contents_; }

      Node *parent() const { return parent_; }
      Node *child(Direction d) const { return child_v_[d]; }
      Node *left_child() const { return child_v_[0]; }
      Node *right_child() const { return child_v_[1]; }
      ReducedValue const & reduced1() const { return reduced_.first; }
      ReducedValue const & reduced2() const { return reduced_.second; }

      /* true if this node has 0 children */
      bool is_leaf() const {
        return ((child_v_[0] == nullptr) && (child_v_[1] == nullptr));
      }

      /* identify which child x represents
       * Require:
       * - x != nullptr
       * - x is either this->left_child() or this->right_child()
       */
      Direction child_direction(Node *x) {
        if (x == this->left_child())
          return D_Left;
        else if (x == this->right_child())
          return D_Right;
        else
          return D_Invalid;
      } /*child_direction*/

      bool is_black() const { return this->color_ == C_Black; }
      bool is_red() const { return this->color_ == C_Red; }

      bool is_red_left() const { is_red(this->left_child()); }
      bool is_red_right() const { is_red(this->right_child()); }

      /* true if this node is red,  and either child is red */
      bool is_red_violation() const {
        if (this->color_ == C_Red) {
          Node *left = this->left_child();
          Node *right = this->right_child();

          if (left && left->is_red())
            return true;

          if (right && right->is_red())
            return true;
        }

        return false;
      } /*is_red_violation*/

      Color color() const { return color_; }
      Key const & key() const { return contents_.first; }
      Value const & value() const { return contents_.second; }

      /* recalculate size from immediate childrens' sizes
       * editor bait: recalc_local_size()
       */
      void local_recalc_size(Reduce const & reduce_fn) {
	using logutil::scope;
	using logutil::xtag;

	constexpr char const * c_self = "Node::local_recalc_size";
	constexpr bool c_logging_enabled = false;
	scope lscope(c_self, c_logging_enabled);

        this->size_ = (1
		       + Node::tree_size(this->left_child())
		       + Node::tree_size(this->right_child()));

	/* (note: want reduce applied to all of left subtree) */
	this->reduced_ = Node::reduced_pair(reduce_fn, this);

        if (c_logging_enabled) {
          lscope.log(c_self, ": done recalc for key k, value v, reduced r",
                     xtag("k", this->key()), xtag("v", this->value()),
                     xtag("r1", this->reduced1()),
		     xtag("r2", this->reduced2()));
        }
      } /*local_recalc_size*/

    private:
      void assign_color(Color x) { this->color_ = x; }
      void assign_size(size_t z) { this->size_ = z; }

      void assign_child_reparent(Direction d, Node *new_x) {
        Node *old_x = this->child_v_[d];

        // trying to fix old_x can be counterproductive,
        // since old_x->parent_ may already have been corrected,
        //
        if (old_x && (old_x->parent_ == this))
          old_x->parent_ = nullptr;

        this->child_v_[d] = new_x;

        if (new_x) {
          new_x->parent_ = this;
        }
      } /*assign_child_reparent*/

      /* replace child that points to x,  with child that points to x_new
       * and return direction of the child that was replaced
       *
       * Require:
       * - x is a child of *this
       * - x_new is not a child of *this
       *
       * promise:
       * - x is nullptr or x.parent is nullptr
       * - x_new is nullptr or x_new.parent is this
       */
      Direction replace_child_reparent(Node *x, Node *x_new) {
        Direction d = this->child_direction(x);

        if (d == D_Left || d == D_Right) {
          this->assign_child_reparent(d, x_new);
          return d;
        } else {
          return D_Invalid;
        }
      } /*replace_child_reparent*/

      friend class RbTreeUtil<Key, Value, Reduce>;
      friend class xo::tree::RedBlackTree<Key, Value, Reduce>;

    private:
      /* red | black */
      Color color_ = C_Red;
      /* size of subtree (#of key/value pairs) rooted at this node */
      size_t size_ = 0;
      /* .first        = key   associated with this node
       * .second       = value associated with this node
       * .third       = reduced value
       */
      ContentsType contents_;
      /* accumulator for some binary function of Values.
       * must be associative,  since value will be produced
       * by any testing of calls to Reduce::combine().
       *
       * e.g. {a, b, c, d} could be reduced by:
       *   r(r(a,b), r(c,d))
       * or
       *   r(a, r(r(b, c), d))
       * etc.
       *
       * examples:
       *  - count #of keys
       *  - sum key values
       *
       * .reduced.first:  reduce applied to all values with keys <= .contents.first
       * .reduced.second: reduce applied to all values in this subtree.
       */
      std::pair<ReducedValue, ReducedValue> reduced_;
      /* pointer to parent node,  nullptr iff this is the root node */
      Node *parent_ = nullptr;
      /*
       * .child_v[0] = left child
       * .child_v[1] = right child
       *
       * invariants:
       * - if .child_v[x] non-null,  then .child_v[0]->parent = this
       * - a red node may not have red children
       */
      std::array<Node *, 2> child_v_ = {nullptr, nullptr};
    }; /*Node*/

    /* specify iterator location relative to Iterator::node.
     * using this to make it possible to correctly decrement an
     * iterator at RedBlackTree::end().
     *
     * IL_BeforeBegin.    if non-empty tree, .node is the first node
     *                    in the tree (the one with smallest key),
     *                    and iterator refers to the location
     *                    "one before" that first node.
     * IL_Regular.        iterator refers to member of the tree
     *                    given by Iterator::node
     * IL_AfterEnd.       if non-empty tree, .node is the last node
     *                    in the tree (the one with largest key),
     *                    and iterator refers the the location
     *                    "one after" that last node.
     */
    enum IteratorLocation {
      IL_BeforeBegin,
      IL_Regular,
      IL_AfterEnd,
    }; /*IteratorLocation*/

    /* require:
     * - Reduce::value_type
     */
    template <typename Key,
	      typename Value,
              typename Reduce>
    class RbTreeUtil {
    public:
      using RbNode = Node<Key, Value, Reduce>;
      using ReducedValue = typename Reduce::value_type;

    public:
      /* return #of key/vaue pairs in tree rooted at x. */
      static size_t tree_size(RbNode *x) {
        if (x)
          return x->size();
        else
          return 0;
      } /*tree_size*/

      static bool is_black(RbNode *x) {
        if (x)
          return x->is_black();
        else
          return true;
      } /*is_black*/

      static bool is_red(RbNode *x) {
        if (x)
          return x->is_red();
        else
          return false;
      } /*is_red*/

      /* for every node n in tree, call fn(n, d').
       * d' is the depth of the node n relative to starting point x,
       * not counting red nodes.
       * make calls in increasing key order (i.e. inorder traversal)
       * argument d is the black-height of tree above x
       *
       * Require:
       * - fn(x, d)
       */
      template <typename Fn>
      static void inorder_node_visitor(RbNode const *x, uint32_t d, Fn &&fn) {
        if (x) {
          /* dd: black depth of child subtrees*/
          uint32_t dd = (x->is_black() ? d + 1 : d);

          inorder_node_visitor(x->left_child(), dd, fn);
          /* black-depth should count this node x */
          fn(x, dd);
          inorder_node_visitor(x->right_child(), dd, fn);
        }
      } /*inorder_node_visitor*/

      /* return the i'th inorder node (counting from 0)
       * belonging to the subtree rooted at N.
       *
       * behavior not defined if subtree at N contains less than
       * (i + 1) nodes
       */
      static RbNode * find_ith(RbNode * N, uint32_t i) {
	if(!N)
	  return nullptr;

	RbNode * L = N->left_child();
	uint32_t n_left = tree_size(L);

	if(i < n_left)
	  return find_ith(L, i);
	else if(i == n_left)
	  return N;
	else if(i < N->size_)
	  return find_ith(N->right_child(), i - (n_left + 1));
	else
	  return nullptr;
      } /*find_ith*/

      /* starting from x,  traverse only left children
       * to find node with a nil left child.
       *
       * This node has the smallest key in subtree N
       */
      static RbNode * find_leftmost(RbNode * N) {
	while(N) {
	  RbNode * S = N->left_child();

	  if(!S)
	    break;

	  N = S;
	}

	return N;
      } /*find_leftmost*/

      /* return node containing the next key after N->key_ in the tree
       * containing N.  This will be either a descendant of N, 
       * or an ancestor of N.
       * returns nil if x.key is the largest key in tree containing x.
       */
      static RbNode * next_inorder_node(RbNode * N) {
	if(!N)
	  return nullptr;

	if(N->right_child())
	  return find_leftmost(N->right_child());

        /* N has no right child -->
         * successor is the nearest ancestor with a left child
	 * on path to N
	 */

	RbNode * x = N;

	while(x) {
	  RbNode * P = x->parent();

	  if(P && P->left_child() == x) {
	    return P;
	  }

          /* path P..N traverses only right-child pointers) */
	  x = P;
	}

        /* no ancestor of N with a left child,  so N has the largest key
	 * in the tree
	 */
	return nullptr;
      } /*next_inorder_node*/

      /* return node containing the key before N->key_ in the tree containing N.
       * This will be either a descendant of N, or an ancestor of N
       */
      static RbNode * prev_inorder_node(RbNode * N) {
	if(!N)
	  return nullptr;

	if(N->left_child())
	  return find_rightmost(N->left_child());

        /* N has no left child -->
         * predecessor is the nearest ancestor with a right child
         * on path to N
	 */

	RbNode * x = N;

	while(x) {
	  RbNode * P = x->parent();

	  if(P && (P->right_child() == x)) {
	    return P;
	  }

	  /* path P..N traverses only left-child pointers */
	  x = P;
	}

        /* no ancestor of N with a right child,  so N has the smallest key
         * in tree that containing it.
	 */
	return nullptr;
      } /*prev_inorder_node*/

      /* compute value of reduce applied to the set K of all keys k[j] in subtree N
       * with:
       *   k[j] <= lub_key  if is_closed = true
       *   k[j] <  lub_key  if is_closed = false
       * return reduce_fn.nil() if K is empty
       */
      static ReducedValue reduce_lub(Key const & lub_key,
				     Reduce const & reduce_fn,
				     bool is_closed,
				     RbNode * N)
      {
        ReducedValue retval = reduce_fn.nil();

        for (;;) {
          if (!N)
            return retval;

          if ((N->key() < lub_key) || (is_closed && (N->key() == lub_key))) {
            /* all keys k[i] in left subtree of N satisfy k[i] < lub_key
             * apply reduce to:
             * - left subtree of N
             * - N->key() depending on comparison with lub_key
             * - any members of right subtree of N, with key < lub_key;
             */
            retval = reduce_fn.combine(retval, N->reduced1());
            N = N->right_child();
          } else {
            /* all keys k[j] in right subtree of N do NOT satisfy k[j] <
             * lub_key, exclude these.   also exclude N->key()
             */
            N = N->left_child();
          }
        }
      } /*reduce_lub*/

      /* find largest key k such that
       *   reduce({node j in subtree(N)) | j.key <= k}) < p
       *
       *   ^
       * 1 |           xxxx
       *   |         xx
       * p |....... x
       *   |       x
       *   |     xx .
       *   | xxxx   .
       * 0 +----------------> 
       *            ^
       *            find_cum_glb(p)
       *
       * here Key is a sample value,
       * Value counts #of samples with that key.
       *
       * find_cum_glb() computes inverse for a monotonically increasing function,
       * if reduce(S) = sum {j.value | j in S};
       *
       * if rbtree stores values for a discrete function f: IR -> IR+,
       * then x = find_sum_glb(p)->key() inverts the integral of f,  i.e.
       * computes:
       *             x
       *            /
       *            |
       *   sup { x: |  f(z) dz < y }
       *            |
       *            /
       *             -oo
       * 
       * Require:
       * - Reduce behaves like sum:
       *   must deliver monotonically increasing values
       *   with increasing key-values.
       * 
       *   (for example: if Value is non-negative and Reduce is SumReduce<Value>)
       */
      static RbNode * find_sum_glb(Reduce const & reduce_fn,
				   RbNode * N,
				   typename Reduce::value_type y) {
	using logutil::scope;
	using logutil::xtag;
	
	constexpr char const * c_self = "RbTreeUtil::find_sum_glb";
	constexpr bool c_logging_enabled = false;
	scope lscope(c_self, c_logging_enabled);

	if(!N) {
	  if(c_logging_enabled)
	    lscope.log(c_self, ": return nullptr");
	  return nullptr;
	}

	typename Reduce::value_type left_sum
	  = RbNode::reduce_aux(reduce_fn, N->left_child());
	typename Reduce::value_type right_sum
	  = RbNode::reduce_aux(reduce_fn, N->right_child());

	if (c_logging_enabled) {
	  lscope.log(c_self, ": with",
		     xtag("y", y),
		     xtag("N.key", N->key()),
		     xtag("N.value", N->value()),
		     xtag("N.reduced1", N->reduced1()),
		     xtag("left_sum", left_sum),
		     xtag("right_sum", right_sum));
	}

	if (y <= left_sum) {
	  return find_sum_glb(reduce_fn, N->left_child(), y);
	} else if (y <= N->reduced1() || !N->right_child()) {
	  if (c_logging_enabled)
	    lscope.log(c_self, ": return N");
	  /* since N.reduced = reduce(left_sum, N.value, right_sum) */
	  return N;
	} else {
	  /* find bound in non-null right subtree */
	  return find_sum_glb(reduce_fn, N->right_child(), y - N->reduced1());
	}
      } /*find_sum_glb*/

      /* starting from x,  traverse only right children
       * to find node with a nil right child
       *
       * This node has the largest key in subtree N
       */
      static RbNode * find_rightmost(RbNode *N) {
        while(N) {
          RbNode *S = N->right_child();

          if (!S)
            break;

          N = S;
        }

        return N;
      } /*find_rightmost*/

      /* find node in x with key k
       * return nullptr iff no such node exists.
       */
      static RbNode * find(RbNode * x, Key const & k) {
        for (;;) {
          if (!x)
            return nullptr;

          if (k < x->key()) {
            /* search in left subtree */
            x = x->left_child();
          } else if (k == x->key()) {
            return x;
          } else /* k > x->key() */ {
            x = x->right_child();
          }
        }
      } /*find*/

      /* find greatest lower bound for key k in tree x,
       * provided it's tighter than candidate h.
       *
       * require:
       * if h is provided,  then x belongs to right subtree of h
       * (so any key k' in x satisfies k' > h->key)
       *
       */
      static RbNode *find_glb_aux(RbNode *x, RbNode *h, Key const &k,
                                  bool is_closed) {
        for (;;) {
          if (!x)
            return h;

          if (x->key() < k) {
            /* x.key is a lower bound for k */

            if (x->right_child() == nullptr) {
              /* no tighter lower bounds present in subtree rooted at x */

              /* x must be better lower bound than h,
               * since when h is non-nil we are searching right subtree of h
               */
              return x;
            }

            /* look for better lower bound in right child */
            h = x;
            x = x->right_child();
            continue;
          } else if (is_closed && (x->key() == k)) {
            /* x.key is exact match */
            return x;
          } else {
            /* x.key is an upper bound for k.  If there's a lower bound,
             * it must be in left subtree of x
             */

            /* preserving h */
            x = x->left_child();
            continue;
          }
        } /*looping over tree nodes*/
      }   /*find_glb_aux*/

      /* find greatest lower bound node for a key,  in this subtree
       *
       * is_open.  if true,  allow result with N->key = k exactly
       *           if false,  require N->key < k
       */
      static RbNode *find_glb(RbNode *x, Key const &k, bool is_closed) {
        return find_glb_aux(x, nullptr, k, is_closed);
      } /*find_glb*/

#ifdef NOT_IN_USE
      /* find least upper bound node for a key,  in this subtree*
       *
       * is_open.  if true,  allow result with N->key = k exactly
       *           if false,  require N->key > k
       */
      static RbNode *find_lub(RbNode *x, Key const &k, bool is_closed) {
        if (x->key() > k) {
          /* x.key is an upper bound for k */
          if (x->left_child() == nullptr) {
            /* no tigher upper bound present in subtree rooted at x */
            return x;
          }

          RbNode *y = find_lub(x->left_child(), k, is_closed);

          if (y) {
            /* found better upper bound in left subtree */
            return y;
          } else {
            return x;
          }
        } else if (is_closed && (x->key() == k)) {
          return x;
        } else {
          /* x.key is not an upper bound for k */
          return find_lub(x->right_child(), k, is_closed);
        }
      } /*find_lub*/
#endif

      /* perform a tree rotation in direction d at node A.
       *
       * Require:
       * - A is non-nil
       * - A->child(other(d)) is non-nil
       *
       * if direction=D_Left:
       *
       *        G                 G
       *        |                 |
       *        A                 B  <- retval
       *       / \               / \
       *      R   B      ==>    A   T
       *         / \           / \
       *        S   T         R   S
       *
       * if direction=D_Right:
       *
       *        G                  G
       *        |                  |
       *        A                  B <- retval
       *       / \                / \
       *      B   R        ==>   T   A
       *     / \                    / \
       *    T   S                  S   R
       */
      static RbNode *rotate(Direction d, RbNode *A,
			    Reduce const & reduce_fn,
			    RbNode **pp_root) {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const *c_self = "RbTreeUtil::rotate";
        constexpr bool c_logging_enabled = false;
        scope lscope(c_self, c_logging_enabled);

        Direction other_d = other(d);

        RbNode *G = A->parent();
        RbNode *B = A->child(other_d);
        RbNode *R = A->child(d);
        RbNode *S = B->child(d);
        RbNode *T = B->child(other_d);

        if (c_logging_enabled) {
          lscope.log(c_self, ": rotate-", (d == D_Left) ? "left" : "right",
                     " at", xtag("A", A), xtag("A.key", A->key()), xtag("B", B),
                     xtag("B.key", B->key()));

          if (G) {
            lscope.log(c_self, ": with G", xtag("G", G),
                       xtag("G.key", G->key()));
            // display_aux(D_Invalid /*side*/, G, 0, &lscope);
          } else {
            lscope.log(c_self, ": with A at root");
            // display_aux(D_Invalid /*side*/, A, 0, &lscope);
          }
        }

        /* note: this will set A's old child B to have null parent ptr */
        A->assign_child_reparent(other_d, S);
        A->local_recalc_size(reduce_fn);

        B->assign_child_reparent(d, A);
        B->local_recalc_size(reduce_fn);

        if (G) {
          G->replace_child_reparent(A, B);
          assert(B->parent() == G);

          /* note: G.size not affected by rotation */
        } else {
          RbNode::replace_root_reparent(B, pp_root);
        }

        return B;
      } /*rotate*/

      /* fixup size in N and all ancestors of N,
       * after insert/remove affecting N
       */
      static void fixup_ancestor_size(Reduce const & reduce_fn, RbNode *N) {
        while (N) {
          N->local_recalc_size(reduce_fn);
          N = N->parent();
        }
      } /*fixup_ancestor_size*/

      /* rebalance to fix possible red-red violation at node G or G->child(d).
       *
       * diagrams are for d=D_Left;
       * mirror left-to-right to get diagram for d=D_Right
       *
       *             G
       *        d-> / \ <-other_d
       *           P   U
       *          / \
       *         R   S
       *
       * relative to prevailing black-height h:
       * - P at h
       * - U at h
       * - may have red-red violation between G and P
       *
       * Require:
       * - tree is in RB-shape,  except for possible red-red violation
       *   between {G,P} or {P,R|S}
       * Promise:
       * - tree is in RB-shape
       */
      static void fixup_red_shape(Direction d, RbNode *G,
				  Reduce const & reduce_fn,
				  RbNode **pp_root) {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const *c_self = "RbTreeUtil::fixup_red_shape";
        constexpr bool c_logging_enabled = false;
        constexpr bool c_excessive_verify_enabled = false;

        scope lscope(c_self, c_logging_enabled);

        RbNode *P = G->child(d);

        for (uint32_t iter = 0;; ++iter) {
          if (c_excessive_verify_enabled)
            RbTreeUtil::verify_subtree_ok(reduce_fn, G, nullptr /*&black_height*/);

          if (c_logging_enabled) {
            if (G) {
              lscope.log(c_self, ": consider node G with d-child P",
                         xtag("iter", iter), xtag("G", G),
                         xtag("G.col", ((G->color() == C_Red) ? "r" : "B")),
                         xtag("G.key", G->key()),
                         xtag("d", (d == D_Left) ? "L" : "R"), xtag("P", P),
                         xtag("P.col", ((P->color() == C_Red) ? "r" : "B")),
                         xtag("P.key", P->key()));
            } else {
              lscope.log(c_self, ": consider root P", xtag("iter", iter),
                         xtag("P", P),
                         xtag("P.col", ((P->color() == C_Red) ? "r" : "B")),
                         xtag("P.key", P->key()));
            }
            RbTreeUtil::display_aux(D_Invalid /*side*/, G ? G : P, 0 /*d*/,
                                    &lscope);
          } /*if logging enabled*/

          if (G && G->is_red_violation()) {
            if (c_logging_enabled)
              lscope.log(c_self, ": red-red violation at G - defer");

            /* need to fix red-red violation at next level up
             *
             *       .  (=G')
             *       |  (=d')
             *       G* (=P')
             *  d-> / \ <-other-d
             *     P*  U
             *    / \
             *   R   S
             */
            P = G;
            G = G->parent();
            d = RbNode::child_direction(G, P);

            continue;
          }

          if (c_logging_enabled)
            lscope.log(c_self, ": check for red violation at P");

          if (!P->is_red_violation()) {
            if (c_logging_enabled)
              lscope.log(c_self, ": red-shape ok at {G,P}");

            /* RB-shape restored */
            return;
          }

          if (!G) {
            if (c_logging_enabled)
              lscope.log(c_self, ": make P black to fix red-shape at root");

            /* special case:  P is root of tree.
             * can fix red violation by making P black
             */
            P->assign_color(C_Black);
            return;
          }

          Direction other_d = other(d);

          RbNode *R = P->child(d);
          RbNode *S = P->child(other_d);
          RbNode *U = G->child(other_d);

          if (c_logging_enabled) {
            lscope.log(c_self, ": got R,S,U", xtag("R", R), xtag("S", S),
                       xtag("U", U));
            if (R) {
              lscope.log(c_self, ": with",
                         xtag("R.col", (R->color_ == C_Black ? "B" : "r")),
                         xtag("R.key", R->key()));
            }
            if (S) {
              lscope.log(c_self, ": with",
                         xtag("S.col", (S->color_ == C_Black ? "B" : "r")),
                         xtag("S.key", S->key()));
            }
            if (U) {
              lscope.log(c_self, ": with",
                         xtag("U.col", (U->color_ == C_Black ? "B" : "r")),
                         xtag("U.key", U->key()));
            }
          }

          assert(is_black(G));
          assert(is_red(P));
          assert(is_red(R) || is_red(S));

          if (RbNode::is_red(U)) {
            /* if d=D_Left:
             *
             *   *=red node
             *
             *           .                 .  (=G')
             *           |                 |  (=d')
             *           G                 G* (=P')
             *      d-> / \               / \
             *         P*  U*   ==>      P   U
             *        / \               / \
             *    (*)R   S(*)       (*)R   S(*)
             *
             * (*) exactly one of R or S is red (since we have a red-violation
             * at P)
             *
             * Note: this transformation preserves #of black nodes along path
             * from root to each of {T, R, S},  so it preserves the "equal
             * black-node path" property
             */
            G->assign_color(C_Red);
            P->assign_color(C_Black);
            U->assign_color(C_Black);

            if (c_logging_enabled)
              lscope.log(c_self,
                         ": fixed red violation at P, retry 1 level higher");

            /* still need to check for red-violation at G's parent */
            P = G;
            G = G->parent();
            d = RbNode::child_direction(G, P);

            continue;
          }

          assert(RbNode::is_black(U));

          if (RbNode::is_red(S)) {
            if (c_logging_enabled) {
              lscope.log(c_self, ": rotate-", (d == D_Left) ? "left" : "right",
                         " at P", xtag("P", P), xtag("P.key", P->key()),
                         xtag("S", S), xtag("S.key", S->key()));
            }

            /* preparatory step: rotate P in d direction if "inner child"
             * (S) is red inner-child = right-child of left-parent or vice
             * versa
             *
             *        G                      G
             *       / \                    / \
             *      P*  U     ==>    (P'=) S*  U
             *     / \                    / \
             *    R   S*           (R'=) P*
             *                          / \
             *                         R
             */
            RbTreeUtil::rotate(d, P, reduce_fn, pp_root);

            if (c_excessive_verify_enabled)
              RbTreeUtil::verify_subtree_ok(reduce_fn, S, nullptr /*&black_height*/);

            /* (relabel S->P etc. for merged control flow below) */
            R = P;
            P = S;
          }

          /*
           *        G                P
           *       / \              / \
           *      P*  U     ==>    R*  G*
           *     / \                  / \
           *    R*  S                S   U
           *
           * ok since every path that went through previously-black G
           * now goes through newly-black P
           */
          P->assign_color(C_Black);
          G->assign_color(C_Red);

          if (c_logging_enabled) {
            lscope.log(c_self, ": rotate-",
                       (other_d == D_Left) ? "left" : "right", " at G",
                       xtag("G", G), xtag("G.key", G->key()));
          }

          RbTreeUtil::rotate(other_d, G, reduce_fn, pp_root);

          if (c_excessive_verify_enabled) {
            RbNode *GG = G ? G->parent() : G;
            if (!GG)
              GG = P;

            if (c_logging_enabled) {
              lscope.log(c_self, ": verify subtree at GG", xtag("GG", GG),
                         xtag("GG.key", GG->key()));

              RbTreeUtil::verify_subtree_ok(reduce_fn, GG, nullptr /*&black_height*/);
              RbTreeUtil::display_aux(D_Invalid, GG, 0 /*depth*/, &lscope);

              lscope.log(c_self, ": fixup complete");
            }
          }

          return;
        } /*walk toward root until red violation fixed*/
      }   /*fixup_red_shape*/

      /* insert key-value pair (key, value) into *pp_root.
       * on exit *pp_root contains new tree with (key, value) inserted.
       * returns true if node was inserted,  false if instead an existing node
       * with the same key was replaced.
       *
       * Require:
       * - pp_root is non-nil  (*pp_root may be nullptr -> empty tree)
       * - *pp_root is in RB-shape
       *
       * allow_replace_flag.   if true,  v will replace an existing value
       *                       associated with key k.
       *                       if false,  preserve existing value.
       *                       when k already exists in *pp_root.
       *
       * return pair<f,n> with:
       * - f=true for new node (k did not exist in tree before this call)
       * - f=false for existing node (k already in tree before this call)
       * - n=node containing key k
       */
      static std::pair<bool, RbNode *>
      insert_aux(Key const & k,
		 Value const & v,
		 bool allow_replace_flag,
		 Reduce const & reduce_fn,
		 RbNode ** pp_root)
      {
        RbNode *N = *pp_root;

        Direction d = D_Invalid;

        while (N) {
          if (k == N->key()) {
	    if(allow_replace_flag) {
	      /* match on this key already present in tree
	       *  -> just update assoc'd value
	       */
	      N->contents_.second = v;
	    }

	    /* after modifying a node n,  must recalculate reductions
	     * along path [root .. n]
	     */
	    RbTreeUtil::fixup_ancestor_size(reduce_fn, N);

	    /* since we didn't change the set of nodes,
	     * tree is still in RB-shape,  don't need to call fixup_red_shape()
	     */
            return std::make_pair(false, N);
          }

          d = ((k < N->key()) ? D_Left : D_Right);

          /* insert into left subtree somewhere */
          RbNode *C = N->child(d);

          if (!C)
            break;

          N = C;
        }

        /* invariant: N->child(d) is nil */

        if (N) {
	  RbNode * new_node = new RbNode(k, v,
					 reduce_fn.leaf(v));

          N->assign_child_reparent(d, new_node);

          assert(is_red(N->child(d)));

          /* recalculate Node sizes on path [root .. N] */
          RbTreeUtil::fixup_ancestor_size(reduce_fn, N);
          /* after adding a node,  must rebalance to restore RB-shape */
          RbTreeUtil::fixup_red_shape(d, N, reduce_fn, pp_root);

	  return std::make_pair(true, N->child(d));
        } else {
          *pp_root = new RbNode(k, v,
				reduce_fn.leaf(v));

          /* tree with a single node might as well be black */
          (*pp_root)->assign_color(C_Black);

	  //(*pp_root)->local_recalc_size(reduce_fn);

          /* Node.size will be correct for tree,  since
           * new node is only node in the tree
           */

	  return std::make_pair(true, *pp_root);
        }

      } /*insert_aux*/

      /* remove a black node N with no children.
       * this will reduce black-height along path to N
       * by 1,   so will need to rebalance tree
       *
       * pp_root.  pointer to location of tree root;
       *           may update with new root
       *
       * Require:
       * - N != nullptr
       * - N has no child nodes
       * - N->parent() != nullptr
       */
      static void remove_black_leaf(RbNode *N,
				    Reduce const & reduce_fn,
				    RbNode **pp_root)
      {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const *c_self = "RbTreeUtil::remove_black_leaf";
        constexpr bool c_logging_enabled = false;

        scope lscope(c_self, c_logging_enabled);

        assert(pp_root);

        RbNode *P = N->parent();

        if (!P) {
          /* N was the root node,  tree now empty */
          *pp_root = nullptr;
          delete N;
          return;
        }

        /* d: direction in P to immediate child N;
         * also sets N.parent to nil
         */
        Direction d = P->replace_child_reparent(N, nullptr);

        delete N;

        /* need to delay this assignment until
         * we've determined d
         */
        N = nullptr;

        /* fixup sizes on path root..P
         * subsequent rebalancing rotations will preserve correct .size values
         */
        RbTreeUtil::fixup_ancestor_size(reduce_fn, P);

        /* other_d, S, C, D will be assigned by loop below
         *
         * diagram shown with d=D_Left;  mirror left-to-right for d=D_Right
         *
         *       P
         *  d-> / \ <-other_d
         *     N   S
         *        / \
         *       C   D
         */
        Direction other_d;
        RbNode *S = nullptr;
        RbNode *C = nullptr;
        RbNode *D = nullptr;

        /* table of outcomes as a function of node color
         *
         * .=black
         * *=red
         * x=don't care
         *
         * #=#of combinations (/16) for P,S,C,D color explained by this row
         *
         *  P  S  C  D  case     #
         * -----------------------
         *  .  .  .  .  Case(1)  1
         *  x  *  x  x  Case(3)  8  P,C,D black is forced by RB rules
         *  *  .  .  .  Case(4)  1
         *  x  .  *  .  Case(5)  2
         *  x  .  x  *  Case(6)  4
         *                      --
         *                      16
         *
         */

        while (true) {
          assert(is_black(N)); /* reminder: nil is black too */

          /* Invariant:
           * - either:
           *   - N is nil (first iteration only), and
           *     P->child(d) = nil, or:
           *   - P is nil and non-nil N is tree root, or:
           *   - N is an immediate child of P,
           *     and P->child(d) = N
           * - N is black
           * - all paths that don't go thru N have prevailing black-height h.
           * - paths through N have black-height h-1
           */

          if (!P) {
            /* N is the root node, in which case all paths go through N,
             * so black-height is h-1
             */
            *pp_root = N;
            return;
          }

          other_d = other(d);
          S = P->child(other_d);

          /* S can't be nil:  since N is non-nil and black,
           * it must have a non-nil sibling
           */
          assert(S);

          C = S->child(d);
          D = S->child(other_d);

          if (c_logging_enabled) {
            lscope.log(c_self, ": rebalance at parent P of curtailed subtree N",
                       xtag("P", P),
                       xtag("P.col", P->color() == C_Black ? "B" : "r"),
                       xtag("P.key", P->key()));
            lscope.log(c_self, ": with sibling S, nephews C,D", xtag("S", S),
                       xtag("S.col", S->color() == C_Black ? "B" : "r"),
                       xtag("C", C), xtag("D", D));
          }

          if (is_black(P) && is_black(S) && is_black(C) && is_black(D)) {
            /* Case(1) */

            if (c_logging_enabled) {
              lscope.log(c_self,
                         "P,S,C,D all black: mark S red + go up 1 level");
            }

            /* diagram with d=D_Left: flip left-to-right for d=D_Right
             *    =black
             *   *=red
             *   _=red or black
             *
             *     P
             *    / \
             *   N   S
             *      / \
             *     C   D
             *
             * relative to prevailing black-height h:
             * - N at h-1
             * - C at h
             * - D at h
             */

            S->assign_color(C_Red);

            /* now have:
             *
             *    G (=P')
             *    |
             *    P (=N')
             *   / \
             *  N   S*
             *     / \
             *    C   D
             *
             * relative to prevailing black-height h:
             * - N at h-1
             * - C at h-1
             * - D at h-1
             *
             * relabel to one level higher in tree
             */
            N = P;
            P = P->parent();
            d = RbNode::child_direction(P, N);

            continue;
          } else {
            break;
          }
        } /*loop looking for a red node*/

        if (is_red(S)) {
          /* Case(3) */

          if (c_logging_enabled) {
            lscope.log(
                "case 3: S red, P,C,D black -> rotate at P to promote S");
            lscope.log("case 3: + make P red instead of S");
            lscope.log("case 3: with", xtag("P", P),
                       xtag("P.col", P->color() == C_Black ? "B" : "r"),
                       xtag("P.key", P->key()), xtag("S", S),
                       xtag("S.col", S->color() == C_Black ? "B" : "r"),
                       xtag("S.key", S->key()));
          }

          /* since S is red, {P,C,D} are all black
           *
           * diagram with d=D_Left: flip left-to-right for d=D_Right
           *    =black
           *   *=red
           *   _=red or black
           *
           *     P
           *    / \
           *   N   S*
           *      / \
           *     C   D
           *
           * relative to prevailing black-height h:
           * - N at h-1
           * - C at h
           * - D at h
           */

          assert(is_black(C));
          assert(is_black(D));
          assert(is_black(P));
          assert(is_black(N));

          RbTreeUtil::rotate(d, P, reduce_fn, pp_root);

          /* after rotation d at P:
           *
           *       S*
           *      / \
           *     P   D
           *    / \
           *   N   C
           *
           * relative to prevailing black-height h:
           * - N at h-1  (now goes thru red S)
           * - C at H    (still goes through black P, red S)
           * - D at h-1  (no longer goes thru black P)
           */

          P->assign_color(C_Red);
          S->assign_color(C_Black);

          /* after reversing colors of {P,S}:
           *
           *       S
           *      / \
           *     P*  D
           *    / \
           *   N   C (=S')
           *
           * relative to prevailing black-height h:
           * - N at h-1 (now thru black S, red P instead of red S, black P)
           * - C at h   (now thru black S, red P instead of red S, black P)
           * - D at h   (now through black S instead of red S, black P)
           */

          /* now relabel for subsequent cases */
          S = C;
          C = S ? S->child(d) : nullptr;
          D = S ? S->child(other_d) : nullptr;
        }

        assert(is_black(S));

        if (is_red(P) && is_black(C) && is_black(D)) {
          /* Case(4) */

          if (c_logging_enabled) {
            lscope.log("case 4: P red, N,S,C,D black -> recolor and finish");
            lscope.log("case 4: with", xtag("P", P),
                       xtag("P.col", P->color() == C_Black ? "B" : "r"),
                       xtag("P.key", P->key()), xtag("S", S),
                       xtag("S.col", S->color() == C_Black ? "B" : "r"),
                       xtag("S.key", S->key()));
          }

          assert(is_black(N));

          /* diagram with d=D_Left: flip left-to-right for d=D_Right*
           *    =black
           *   *=red
           *   _=red or black
           *
           *     P*
           *    / \
           *   N   S
           *      / \
           *     C   D
           *
           * relative to prevailing black-height h:
           * - N at h-1
           * - C at h
           * - D at h
           */

          P->assign_color(C_Black);
          S->assign_color(C_Red);

          /* after making P black, and S red (swapping colors of P,S):
           *
           *     P
           *    / \
           *   N   S*
           *      / \
           *     C   D
           *
           * relative to prevailing black-height h:
           * - N at h
           * - C at h
           * - D at h
           *
           * and RB-shape is restored
           */
          return;
        }

        assert(is_black(S) && (is_black(P) || is_red(C) || is_red(D)));

        if (is_red(C) && is_black(D)) {
          if (c_logging_enabled) {
            lscope.log("case 5: C red, S,D black -> rotate at S");
          }

          /* diagram with d=D_Left;  flip left-to-right for d=D_Right
           *
           *    =black
           *   *=red
           *   _=red or black
           *
           *     P_
           *    / \
           *   N   S
           *      / \
           *     C*  D
           *
           * relative to prevailing black-height h:
           * - N at h-1
           * - C at h
           * - D at h
           */

          RbTreeUtil::rotate(other_d, S, reduce_fn, pp_root);

          assert(P->child(other_d) == C);

          /* after other(d) rotation at S:
           *
           *     P_
           *    / \
           *   N   C*
           *        \
           *         S
           *          \
           *           D
           *
           * relative to prevailing black-height h:
           * - N at h-1
           * - C at h-1  (no longer goes thru black S)
           * - S at h    (now goes thru red C)
           * - D at h    (now goes thru red C)
           */

          C->assign_color(C_Black);
          S->assign_color(C_Red);

          /* after exchanging colors of C,S:
           *
           *     P_
           *    / \
           *   N   C (=S')
           *        \
           *         S* (=D')
           *          \
           *           D
           *
           * relative to prevailing black-height h:
           * - N at h-1
           * - C at h    (no longer goes thru black S, but now C black)
           * - S at h    (no longer red, but now goes thru black C)
           * - D at h    (now goes thru black C, red S instead of black S)
           */

          /* now relabel to match next and final case */
          D = S;
          S = C;
          C = nullptr; /* won't be using C past this point */

          assert(D);
          assert(D->is_red());

          /* fall through to next case */
        }

        if (is_red(D)) {
          if (c_logging_enabled) {
            lscope.log("case 6: S black, D red -> rotate at P and finish");
          }

          /* diagram with d=D_Left;  flip left-to-right for d=D_Right
           *
           * Sibling is black,  and distant child is red
           *
           * if N=P->left_child():
           *
           *   *=red
           *   _=red or black
           *
           *      P_
           *     / \
           *    N   S
           *       / \
           *      C_  D*
           *
           * relative to prevailing black-height h:
           * - N   at h-1
           * - S   (+also C,D) at h
           */

          RbTreeUtil::rotate(d, P, reduce_fn, pp_root);

          /* after rotate at P toward d: *
           *
           *      S
           *     / \
           *    P_  D*
           *   / \
           *  N   C_
           *
           * Now,  relative to prevailing black-height h:
           * - N at h+1  (paths to N now visit black S)
           * - C at h    (paths to C still visit P,S)
           * - D at: h   if P red,
           *         h-1 if P black
           *   (paths to D now skip P)
           */

          S->assign_color(P->color());
          P->assign_color(C_Black);
          D->assign_color(C_Black);

          /* after recolor: S to old P color, P to black, D to black.
           *
           *       S_
           *      / \
           *     P   D
           *    / \
           *   N   C_
           *
           * Now, relative to prevailing black-height h:
           * - N at h+1   (swapped P, S colors)
           * - C at h     (paths to C still visit P,S,  swapped P,S colors)
           * - D at: h    if S red  (was P red, S black, D red; now S red, D
           * black) h    if S black (was P black, S black, D red; now S
           * black, D black)
           *
           * RB-shape has been restored
           */
          return;
        }
      } /*remove_black_leaf*/

      /* remove node with key k from tree rooted at *pp_root.
       * on exit *pp_root contains new tree root.
       *
       * Require:
       * - pp_root is non-null.  (*pp_root can be null -> tree is empty)
       * - *pp_root is in RB-shape
       *
       * return true if a node was removed;  false otherwise.
       */
      static bool remove_aux(Key const &k,
			     Reduce const & reduce_fn,
			     RbNode **pp_root) {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const *c_self = "RbTreeUtil::remove_aux";
        constexpr bool c_logging_enabled = false;

        scope lscope(c_self, c_logging_enabled);

        RbNode *N = *pp_root;

        if (c_logging_enabled)
          lscope.log(c_self, ": enter", xtag("N", N));

        /*
         * here the triangle ascii art indicates a tree structure,
         * of arbitrary size
         *
         *       o <- this
         *      / \
         *     o-N-o
         *      / \
         *     X
         *    / \
         *   o---R
         */

        N = RbTreeUtil::find_glb(N, k, true /*is_closed*/);

        if (!N || (N->key() != k)) {
          /* no node with .key = k present,  so cannot remove it */
          return false;
        }

        if (c_logging_enabled)
          lscope.log(c_self, ": got lower bound", xtag("N", N),
                     xtag("N.key", N->key()));

        /* first step is to simplify problem so that we're removing
         * a node with 0 or 1 children.
         */

        RbNode *X = N->left_child();

        if (X == nullptr) {
          /* N has 0 or 1 children */
          ;
        } else {
          /* R will be 'replacement node' for N */
          RbNode *R = RbTreeUtil::find_rightmost(X);

          /* R->right_child() is nil by definition
           *
           * copy R's (key + value) into N;
           * N now serves as container for information previously
           * represented by R.
           */

          N->contents_ = R->contents_;
          /* (preserving N->parent_, N->child_v_[]) */

          /* now relabel N as new R (R'),
           * and relabel R as new N (N').
           * Then go to work on reduced problem of deleting N'.
           * Problem is redueced since now  N' has 0 or 1 child.
           *
           * (Doesn't matter that N' contains key,values of R,
           *  since we're going to delete it anyway)
           */
          N = R;
          /* (preserving R->parent_, R->child_v_[]) */

          /*       o
           *      / \
           *     o-R'o
           *      /
           *     X
           *    / \
           *   o---N'
           */
        }

        RbNode *P = N->parent();

        /* N has 0 or 1 children
         *
         * Implications:
         * 1. if N is red, it cannot have red children (by RB rules),
         *    and it cannot have just 1 black child.
         *    Therefore red N must have 0 children
         *     -> can delete N without disturbing RB properties
         * 2. if N is black:
         *    2.1 if N has 1 child S,  then S must be red
         *        (if S were black,  that would require N to have a 2nd child
         *         to preserve equal black-height for all paths)
         *     -> replace N with S, repainting S black,  in place of
         *        to-be-reclaimed N
         *    1.2 if N is black with 0 children,  need to rebalance
         */

        if (N->is_red()) {
          if (N->is_leaf()) {
            /* replace pointer to N with nil in N's parent. */

            if (P) {
              P->replace_child_reparent(N, nullptr);
              RbTreeUtil::fixup_ancestor_size(reduce_fn, P);
            } else {
              /* N was sole root node;  tree will be empty after removing it */
              *pp_root = nullptr;
            }

            if (c_logging_enabled)
              lscope.log(c_self, ": delete node", xtag("addr", N));
            delete N;
          } else {
            assert(false);

            /* control can't come here for RB-tree,
             * because a red node can't have red children,  or just one black
             * child.
             */
          }
        } else /*N->is_black()*/ {
          RbNode *R = N->left_child();

          if (!R)
            R = N->right_child();

          if (R) {
            /* if a black node has one child,  that child cannot be black */
            assert(R->is_red());

            /* replace N with R in N's parent,
             * + make R black to preserve black-height
             */
            R->assign_color(C_Black);

            if (P) {
              P->replace_child_reparent(N, R);
              RbTreeUtil::fixup_ancestor_size(reduce_fn, P);
            } else {
              /* N was root node */
              RbNode::replace_root_reparent(R, pp_root);
            }

            if (c_logging_enabled)
              lscope.log(c_self, ": delete node", xtag("addr", N));
            delete N;
          } else {
            /* N is black with no children,
             * may need rebalance here
             */

            if (P) {
              RbTreeUtil::remove_black_leaf(N, reduce_fn, pp_root);
            } else {
              /* N was root node */
              *pp_root = nullptr;

              if (c_logging_enabled)
                lscope.log(c_self, ": delete node", xtag("addr", N));
              delete N;
            }
          }
        }

        return true;
      } /*remove_aux*/

      /* verify that subtree at N is in RB-shape.
       * will cover subset of RedBlackTree class invariants:
       * 
       * RB2. if N = P->child(d),  then N->parent()=P
       * RB3. all paths to leaves have the same black height
       * RB4. no red node has a red parent
       * RB5. inorder traversal visits keys in monotonically increasing order
       * RB6. Node::size reports the size of the subtree reachable from that node
       *      via child pointers
       * RB7. Node::reduced reports the value of
       *       f(f(L, Node::value), R)
       *      where: L is reduced-value for left child,
       *             R is reduced-value for right child
       *
       * returns the #of nodes in subtree rooted at N.
       */
      static size_t verify_subtree_ok(Reduce const & reduce_fn,
				      RbNode const * N,
				      int32_t * p_black_height)
      {
        using logutil::scope;
        using logutil::xtag;

        constexpr char const *c_self = "RbTreeUtil::verify_subtree_ok";

        // scope lscope(c_self);

	/* counts #of nodes in subtree rooted at N */
        size_t i_node = 0;
        Key const *last_key = nullptr;
        /* inorder node index when establishing black_height */
        size_t i_black_height = 0;
        /* establish on first leaf node encountered */
        int32_t black_height = 0;

        auto verify_fn = [c_self,
			  &reduce_fn,
			  &i_node,
			  &last_key,
			  &i_black_height,
                          &black_height] (RbNode const *x,
					  uint32_t bd)
	{
          /* RB2. if c=x->child(d), then c->parent()=x */

          if (x->left_child()) {
            XO_EXPECT(x == x->left_child()->parent(),
                      tostr(c_self, ": expect symmetric child/parent pointers",
                            xtag("i", i_node), xtag("node[i]", x),
                            xtag("key[i]", x->key()),
                            xtag("child", x->left_child()),
                            xtag("child.key", x->left_child()->key()),
                            xtag("child.parent", x->left_child()->parent_)));
          }

          if (x->right_child()) {
            XO_EXPECT(x == x->right_child()->parent(),
                      tostr(c_self, ": expect symmetric child/parent pointers",
                            xtag("i", i_node), xtag("node[i]", x),
                            xtag("key[i]", x->key()),
                            xtag("child", x->right_child()),
                            xtag("child.key", x->right_child()->key()),
                            xtag("child.parent", x->right_child()->parent_)));
          }

          /* RB3. all nodes have the same black-height */

          if (x->is_leaf()) {
            if (black_height == 0) {
              black_height = bd;
            } else {
              XO_EXPECT(black_height == bd,
                        tostr(c_self,
                              ": expect all RB-tree nodes to have the same "
                              "black-height",
                              xtag("i1", i_black_height), xtag("i2", i_node),
                              xtag("blackheight(i1)", black_height),
                              xtag("blackheight(i2)", bd)));
            }
          }

          /* RB4. a red node may not have a red parent
           *      (conversely,  a red node may not have a red child)
           */

          RbNode *red_child =
              ((x->left_child() && x->left_child()->is_red())
                   ? x->left_child()
                   : ((x->right_child() && x->right_child()->is_red())
                          ? x->right_child()
                          : nullptr));

          XO_EXPECT(
              x->is_red_violation() == false,
              tostr(c_self,
                    ": expect RB-shape tree to have no red violations but "
                    "red y is child of red x",
                    xtag("i", i_node), xtag("x.addr", x),
                    xtag("x.col", (x->color_ == C_Black) ? "B" : "r"),
                    xtag("x.key", x->key()), xtag("y.addr", red_child),
                    xtag("y.col", (red_child->color_ == C_Black) ? "B" : "r"),
                    xtag("y.key", red_child->key())));

	  /* RB5.  inorder traversal visits nodes in strictly increasing key order */

          if (last_key) {
            XO_EXPECT((*last_key) < x->key(),
                      tostr(c_self,
                            ": expect inorder traversal to visit keys"
                            " in strictly increasing order",
                            xtag("i", i_node), xtag("key[i-1]", *last_key),
                            xtag("key[i]", x->key())));
          }

          last_key = &(x->key());

          /* RB6. Node::size reports the size of the subtree reachable from that
           *      node by child pointers.
	   */
	  XO_EXPECT(x->size() == (tree_size(x->left_child())
				  + 1
				  + tree_size(x->right_child())),
		    tostr(c_self,
			  ": expect Node::size to be 1 + sum of childrens' size",
			  xtag("i", i_node),
			  xtag("key[i]", x->key()),
			  xtag("left.size", tree_size(x->left_child())),
			  xtag("right.size", tree_size(x->right_child()))));

	  /* RB7. Node::reduced reports the value of
	   *       f(f(L, Node::value), R)
	   *      where: L is reduced-value for left child,
	   *             R is reduced-value for right child
	   */
	  auto reduced_pair
	    = RbNode::reduced_pair(reduce_fn, x);

	  XO_EXPECT(reduce_fn.is_equal
		    (x->reduced1(), reduced_pair.first),
		    tostr(c_self,
			  ": expect Node::reduced to be reduce_fn"
			  " applied to (.L, .value)",
			  xtag("node.reduced1", x->reduced1()),
			  xtag("reduced_pair.first", reduced_pair.first)));

	  XO_EXPECT(reduce_fn.is_equal
		    (x->reduced2(), reduced_pair.second),
		    tostr(c_self,
			  ": expect Node::reduced to be reduce_fn"
			  " applied to (.L, .value, .R)",
			  xtag("node.reduced2", x->reduced2()),
			  xtag("reduce2_expr", reduced_pair.second)));

	  ++i_node;
        };

        RbTreeUtil::inorder_node_visitor(N, 0 /*d*/, verify_fn);

        if (p_black_height)
          *p_black_height = black_height;

	return i_node;
      } /*verify_subtree_ok*/

      /* display tree structure,  1 line per node.
       * indent by node depth + d
       */
      static void display_aux(Direction side, RbNode const *N, uint32_t d,
                              logutil::scope *p_scope) {
        using logutil::pad;
        using logutil::xtag;

        if (N) {
          p_scope->log(pad(d), xtag("addr", N), xtag("par", N->parent()),
                       xtag("side", ((side == D_Left)    ? "L"
                                     : (side == D_Right) ? "R"
                                                         : "root")),
                       xtag("col", N->is_black() ? "B" : "r"),
                       xtag("key", N->key()), xtag("value", N->value()),
                       xtag("wt", N->size()),
		       xtag("reduced1", N->reduced1()),
		       xtag("reduced2", N->reduced2()));
          display_aux(D_Left, N->left_child(), d + 1, p_scope);
          display_aux(D_Right, N->right_child(), d + 1, p_scope);
        }
      } /*display_aux*/

      static void display(RbNode const *N, uint32_t d) {
        using logutil::scope;

        constexpr const char *c_self = "RbTreeUtil::display";

        scope lscope(c_self);

        display_aux(D_Invalid, N, d, &lscope);
      } /*display*/
    }; /*RbTreeUtil*/

    /* xo::tree::detail::RedBlackTreeLhsBase
     *
     * use for const version of RedBlackTree::operator[].
     *
     * Require: RbNode is either
     *   RedBlackTree::RbNode
     * or
     *   RedBlackTree::RbNode const
     */
    template <class RedBlackTree, class RbNode>
    class RedBlackTreeLhsBase {
    public:
      using value_type = typename RedBlackTree::value_type;
      using RbUtil = typename RedBlackTree::RbUtil;

    public:
      RedBlackTreeLhsBase() = default;
      RedBlackTreeLhsBase(RedBlackTree * tree, RbNode * node)
	: p_tree_(tree), node_(node)
      {}

      operator value_type const & () const {
	using logutil::tostr;

	if(!this->node_) {
	  throw std::runtime_error
	    (tostr("rbtree: attempt to use empty lhs object as rvalue"));
	}

	return this->node_->contents().second;
      } /*operator value_type const &*/

    protected:
      RedBlackTree * p_tree_ = nullptr;
      /* invariant: if non-nil, .node belongs to .*p_tree */
      RbNode * node_ = nullptr;
    }; /*RedBlackTreeLhsBase*/

    template<class RedBlackTree>
    class RedBlackTreeConstLhs : public RedBlackTreeLhsBase<RedBlackTree const,
							    typename RedBlackTree::RbNode const>
    {
    public:
      RedBlackTreeConstLhs() = default;
      RedBlackTreeConstLhs(RedBlackTree const * tree,
			   typename RedBlackTree::RbNode const * node)
	: RedBlackTreeLhsBase<RedBlackTree const,
			      typename RedBlackTree::RbNode const>(tree, node) {}
    }; /*RedBlackTreeConstLhs*/

    /* xo::tree::detail::RedBlackTreeLhs
     *
     * use for RedBlackTree::operator[].
     * can't return a regular lvalue,
     * because assignment within a Node N invalidates partial sums along
     * the path from tree root to N.
     *
     * instead interpolate instance of this class,   that can intercept
     * asasignments.
     */
    template <class RedBlackTree>
    class RedBlackTreeLhs : public RedBlackTreeLhsBase<RedBlackTree,
						       typename RedBlackTree::RbNode>
    {
    public:
      using value_type = typename RedBlackTree::value_type;
      using RbUtil = typename RedBlackTree::RbUtil;
      using RbNode = typename RedBlackTree::RbNode;

    public:
      RedBlackTreeLhs() = default;
      RedBlackTreeLhs(RedBlackTree * tree, typename RedBlackTree::RbNode * node)
	: RedBlackTreeLhsBase<RedBlackTree, RbNode>(tree, node) {}
      
      RedBlackTreeLhs & operator=(value_type const & v) {
	using logutil::tostr;
	
	if(this->p_tree_ && this->node_) {
	  this->node_->contents().second = v;

	  /* after modifying a node n,
	   * must recalculate reductions along path [root .. n]
	   */
	  RbUtil::fixup_ancestor_size(this->p_tree_->reduce_fn(),
				      this->node_);
	} else {
	  throw std::runtime_error
	    (tostr("rbtree: attempt to assign through empty lhs object"));
	}

	return *this;
      } /*operator=*/
    }; /*RedBlackTreeLhs*/

    /* tragically, we can't partially specialize an alias template.
     * however we /can/ partially specialize a struct that nests a typealias.
     */
    template <typename Key,
	      typename Value,
	      typename Reduce,
	      bool IsConst>
    struct NodeTypeTraits { using NodeType = void; };

    template <typename Key,
	      typename Value,
	      typename Reduce>
    struct NodeTypeTraits<Key, Value, Reduce, false> {
      using NativeNodeType = Node<Key, Value, Reduce>;
      using NodeType = NativeNodeType;
      using ContentsType = typename NodeType::ContentsType;
      using NodePtrType = NodeType *;
    };

    template <typename Key,
	      typename Value,
	      typename Reduce>
    struct NodeTypeTraits<Key, Value, Reduce, true> {
      using NativeNodeType = Node<Key, Value, Reduce>;
      using NodeType = NativeNodeType const;
      using ContentsType = typename NodeType::ContentsType const;
      using NodePtrType = NodeType const *;
    };

    /* xo::tree::detail::IteratorBase
     * 
     * shared between const & and non-const red-black-tree iterators.
     *
     * editor bait: BaseIterator
     */
    template <typename Key,
	      typename Value,
	      typename Reduce,
	      bool IsConst>
    class IteratorBase {
    public:
      using RbUtil = RbTreeUtil<Key, Value, Reduce>;
      using RbNode = Node<Key, Value, Reduce>;
      using Traits = NodeTypeTraits<Key, Value, Reduce, IsConst>;
      using ReducedValue = typename Reduce::value_type;
      using RbNativeNodeType = typename Traits::NativeNodeType;
      using RbNodePtrType = typename Traits::NodePtrType;
      using RbContentsType = typename Traits::ContentsType;

    protected:
      IteratorBase() = default;
      IteratorBase(IteratorLocation loc, RbNodePtrType node)
	: location_(loc), node_(node) {}
      IteratorBase(IteratorBase const & x) = default;
      
      static IteratorBase begin_aux(RbNodePtrType node) {
	return IteratorBase(node ? IL_Regular : IL_AfterEnd, node);
      } /*begin_aux*/

      static IteratorBase end_aux(RbNodePtrType node) {
	return IteratorBase(IL_AfterEnd, node);
      } /*end_aux*/

    public:
      IteratorLocation location() const { return location_; }
      RbNodePtrType node() const { return node_; }

      ReducedValue const & reduced() const { return node_->reduced(); }

      RbContentsType & operator*() const {
	this->check_regular();
	return this->node_->contents();
      } /*operator**/

      RbContentsType * operator->() const {
	return &(this->operator*());
      }

      /* true for "just before beginning" and "just after the end" states.
       * false otherwise
       */
      bool is_sentinel() const { return (this->location_ != IL_Regular); }
      /* true unless iterator is in a sentinel state */
      bool is_dereferenceable() const { return !this->is_sentinel(); }

      /* deferenceable iterators are truth-y;
       * sentinel iterators are false-y
       */
      operator bool() const { return this->is_dereferenceable(); }

      bool operator==(IteratorBase const & x) const {
	return (this->location_ == x.location_) && (this->node_ == x.node_);
      } /*operator==*/

      bool operator!=(IteratorBase const & x) const {
	return (this->location_ != x.location_) || (this->node_ != x.node_);
      } /*operator!=*/

      /* pre-increment */
      IteratorBase & operator++() {
	switch(this->location_) {
	case IL_BeforeBegin:
	  /* .node is first node in tree */
	  this->location_ = IL_Regular;
	  break;
	case IL_Regular:
	  {
	    RbNodePtrType next_node
	      = RbUtil::next_inorder_node(const_cast<RbNativeNodeType *>(this->node_));

	    if(next_node) {
	      this->node_ = next_node;
	    } else {
	      this->location_ = IL_AfterEnd;
	    }
	  }
	  break;
	case IL_AfterEnd:
	  break;
	} /*operator++*/

	return *this;
      } /*operator++*/

      /* pre-decrement */
      IteratorBase & operator--() {
	switch(this->location_) {
	case IL_BeforeBegin:
	  break;
	case IL_Regular:
	  {
	    RbNode * prev_node = RbUtil::prev_inorder_node(const_cast<RbNativeNodeType *>(this->node_));

	    if(prev_node) {
	      this->node_ = prev_node;
	    } else {
	      this->location_ = IL_BeforeBegin;
	    }
	  }
	  break;
	case IL_AfterEnd:
	  /* .node is already last node in tree */
	  this->location_ = IL_Regular;
	  break;
	}

	return *this;
      } /*operator--*/

    protected:
      void check_regular() const {
	using logutil::tostr;

	if(this->location_ != IL_Regular)
	  throw std::runtime_error(tostr("rbtree iterator: cannot deref iterator"
					 " in non-regular state"));
      } /*check_regular*/

    protected:
      /* IL_BeforeBegin, IL_Regular, IL_AfterEnd */
      IteratorLocation location_ = IL_AfterEnd;
      /* location = IL_BeforeBegin: .node is leftmost node in tree
       * location = IL_Regular:     .node is some node in tree,
       *                            iterator refers to that node.
       * location = IL_AfterEnd:    .node is rightmost node in tree
       */
      RbNodePtrType node_ = nullptr;
    }; /*IteratorBase*/

    /* xo::tree::detail::Iterator
     *
     * inorder iterator over nodes in a red-black tree.
     * invalidated on insert or remove operations on the parent tree.
     *
     * satisfies the std::bidirectional_iterator concept
     */
    template <typename Key,
	      typename Value,
	      typename Reduce>
    class Iterator : public IteratorBase<Key, Value, Reduce, false /*!IsConst*/> {
    public:
      using iterator_concept = std::bidirectional_iterator_tag;
      
      using RbIteratorBase = IteratorBase<Key, Value, Reduce, false /*!IsConst*/>;
      using RbNode = typename RbIteratorBase::RbNode;
      using RbUtil = typename RbIteratorBase::RbUtil;
      using ReducedValue = typename Reduce::value_type;
      
    public:
      Iterator() = default;
      Iterator(IteratorLocation loc, RbNode * n)
	: RbIteratorBase(loc, n) {}
      Iterator(Iterator const & x) = default;
      Iterator(RbIteratorBase const & x) : RbIteratorBase(x) {}
      Iterator(RbIteratorBase && x) : RbIteratorBase(std::move(x)) {}

      static Iterator begin_aux(RbNode const * n) { return RbIteratorBase::begin_aux(n); }
      static Iterator end_aux(RbNode const * n) { return RbIteratorBase::end_aux(n); }

      /* pre-increment */
      Iterator & operator++() {
	RbIteratorBase::operator++();
	return *this;
      } /*operator++*/

      /* post-increment */
      Iterator operator++(int) {
	Iterator retval = *this;

	++(*this);

	return retval;
      } /*operator++(int)*/

      /* pre-decrement */
      Iterator & operator--() {
	RbIteratorBase::operator--();
	return *this;
      } /*operator--*/

      /* post-decrement */
      Iterator operator--(int) {
	Iterator retval = *this;

	--(*this);

	return retval;
      } /*operator--(int)*/
    }; /*Iterator*/

    /* xo::tree::detail::ConstIterator
     *
     * inorder iterator over nodes in a red-black tree.
     * invalidated on insert or remove operations on the parent tree.
     *
     * satisfies the std::bidirectional_iterator concept
     */
    template <typename Key,
	      typename Value,
	      typename Reduce>
    class ConstIterator : public IteratorBase<Key, Value, Reduce, true /*IsConst*/> {
    public:
      using iterator_concept = std::bidirectional_iterator_tag;

      using RbIteratorBase = IteratorBase<Key, Value, Reduce, true /*IsConst*/>;
      using RbNode = typename RbIteratorBase::RbNode;
      using RbUtil = typename RbIteratorBase::RbUtil;
      using ReducedValue = typename Reduce::value_type;
      
    public:
      ConstIterator() = default;
      ConstIterator(IteratorLocation loc, RbNode const * node)
	: RbIteratorBase(loc, node) {}
      ConstIterator(ConstIterator const & x) = default;
      ConstIterator(RbIteratorBase const & x) : RbIteratorBase(x) {}
      ConstIterator(RbIteratorBase && x) : RbIteratorBase(std::move(x)) {}

      static ConstIterator begin_aux(RbNode const * n) { return RbIteratorBase::begin_aux(n); }
      static ConstIterator end_aux(RbNode const * n) { return RbIteratorBase::end_aux(n); }

      /* pre-increment */
      ConstIterator & operator++() {
	RbIteratorBase::operator++();
	return *this;
      } /*operator++*/

      /* post-increment */
      ConstIterator operator++(int) {
	ConstIterator retval = *this;

	++(*this);

	return retval;
      } /*operator++(int)*/

      /* pre-decrement */
      ConstIterator & operator--() {
	RbIteratorBase::operator--();
	return *this;
      } /*operator--*/

      /* post-decrement */
      ConstIterator operator--(int) {
	ConstIterator retval = *this;

	--(*this);

	return retval;
      } /*operator--(int)*/
    }; /*ConstIterator*/
  } /*namespace detail*/

  struct null_reduce_value {};

  /* for null reduce,  just have it return 0;
   * otherwise breaks verification (e.g. verify_subtree_ok() below)
   */
  template<typename NodeValue>
  struct NullReduce {
    static constexpr bool is_null_reduce() { return true; }

    /* data type for reduced values */
    using value_type = null_reduce_value;

    value_type nil() const { return value_type(); }
    std::pair<value_type, value_type> leaf(NodeValue const & x) const {
      return std::pair<value_type, value_type>(nil(), nil());
    }
    value_type operator()(value_type x,
			  NodeValue const & value) const { return nil(); }
    value_type combine(value_type x,
		       value_type y) const { return nil(); }
    bool is_equal(value_type x, value_type y) const { return true; }
  }; /*NullReduce*/

  inline std::ostream & operator<<(std::ostream & os,
				   null_reduce_value x)
  {
    os << "{}";
    return os;
  } /*operator<<*/

  /* just counts #of distinct values;
   * redundant,  same as detail::Node<>::size_.
   * providing for completeness' sake
   */
  template <typename Value>
  class OrdinalReduce {
  public:
    using value_type = std::size_t;

  public:
    static constexpr bool is_monotonic() { return true; }

    value_type nil() const { return 0; }

    std::pair<value_type, value_type> leaf(Value const & x) const {
      return std::pair<value_type, value_type>(1, 1);
    } /*leaf*/

    value_type operator()(value_type acc,
			  Value const & x) const {
      /* counts #of values */
      return acc + 1;
    }

    value_type combine(value_type x, value_type y) const { return x + y; }
    bool is_equal(value_type x, value_type y) const { return x == y; }
  }; /*OrdinalReduce*/

  /* reduction for inverting the integral of a non-negative discrete function
   * computes sum of values for each subtree
   */
  template<typename Value>
  struct SumReduce {
    using value_type = Value;

    static constexpr bool is_monotonic() { return true; }

    value_type nil() const { return -std::numeric_limits<value_type>::infinity(); }
    std::pair<value_type, value_type> leaf(Value const & x) const {
      return std::pair<value_type, value_type>(x, x);
    } /*leaf*/
    
    value_type operator()(value_type reduced,
			  Value const & x) const {
      /* sums tree values */
      if(std::isfinite(reduced)) {
	return reduced + x;
      } else {
	/* omit -oo reduced value from .nil() */
	return x;
      }
    } /*operator()*/

    value_type combine(value_type const & x,
		       value_type const & y) const {
      /* omit -oo reduced value from .nil() */
      if(!std::isfinite(x))
	return y;
      if(!std::isfinite(y))
	return x;

      return x + y;
    } /*combine*/

    bool is_equal(value_type const & x, value_type const & y) const { return x == y; }
  }; /*SumReduce*/

  /* red-black tree with order statistics
   */
  template <typename Key, typename Value, typename Reduce>
  class RedBlackTree {
    static_assert(ReduceConcept<Reduce, Value>);
    //static_assert(requires(Reduce r) { r.nil(); }, "missing .nil() method");

  public:
    using key_type = Key;
    using value_type = Value;
    using ReducedValue = typename Reduce::value_type;
    using RbTreeLhs = detail::RedBlackTreeLhs<RedBlackTree<Key, Value, Reduce>>;
    using RbTreeConstLhs = detail::RedBlackTreeConstLhs<RedBlackTree<Key, Value, Reduce>>;
    using RbUtil = detail::RbTreeUtil<Key, Value, Reduce>;
    using RbNode = detail::Node<Key, Value, Reduce>;
    using Direction = detail::Direction;
    using iterator = detail::Iterator<Key, Value, Reduce>;
    using const_iterator = detail::ConstIterator<Key, Value, Reduce>;

  public:
    RedBlackTree() = default;

    std::size_t size() const { return size_; }
    Reduce const & reduce_fn() const { return reduce_fn_; }

    const_iterator cbegin() const {
      return const_iterator::begin_aux(RbUtil::find_leftmost(this->root_));
    } /*begin*/

    const_iterator cend() const {
      return const_iterator::end_aux(RbUtil::find_rightmost(this->root_));
    } /*end*/

    const_iterator begin() const { return this->cbegin(); }
    const_iterator end() const { return this->cend(); }

    iterator begin() {
      return iterator::begin_aux(RbUtil::find_leftmost(this->root_));
    } /*begin*/

    iterator end() {
      return iterator::end_aux(RbUtil::find_rightmost(this->root_));
    } /*end*/

    /* visit tree contents in increasing key order
     *
     * Require:
     * - Fn(std::pair<Key, Value> const &)
     */
    template<typename Fn>
    void visit_inorder(Fn && fn) {
      auto visitor_fn = [&fn](RbNode const * x, uint32_t d) { fn(x->contents()); };

      RbUtil::inorder_node_visitor(this->root_,
				   0 /*depth -- will be ignored*/,
				   visitor_fn);
    } /*visit_inorder*/

    /* if i in [0 .. .size], return iterator referring to ith inorder node in tree
     * otherwise return this->end()
     */
    const_iterator find_ith(uint32_t i) const {
      RbNode * node = RbUtil::find_ith(this->root_, i);

      if(node) {
	return const_iterator(detail::IL_Regular, node);
      } else {
	return this->end();
      }
    } /*find_ith*/

    iterator find_ith(uint32_t i) {
      RbNode * node = RbUtil::find_ith(this->root_, i);

      if(node) {
	return iterator(detail::IL_Regular, node);
      } else {
	return this->end();
      }
    } /*find_ith*/

    /* find node with key equal to x in this tree.
     * on success,  return iterator ix with ix->first = x.
     * on failure,  return this->end()
     */
    const_iterator find(Key const & x) const {
      RbNode * node = RbUtil::find(this->root_, x);
	
      if(node) {
	return const_iterator(detail::IL_Regular, node);
      } else {
	return this->end();
      }
    } /*find*/

    iterator find(Key const & x) {
      RbNode * node = RbUtil::find(this->root_, x);

      if(node) {
	return const_iterator(detail::IL_Regular, node);
      } else {
	return this->end();
      }
    } /*find*/

    /* RbTreeConstLhs provides rvalue-substitute for lookup-only in const RedBlackTree
     * instances
     */
    RbTreeConstLhs operator[](Key const & k) const
    {
      RbNode const * node = RbUtil::find(this->root_, k);

      return RbTreeConstLhs(this, node);
    } /*operator[]*/

    /* RbTreeLhs defers assignment,  so that rbtree can update values of
     * Node::reduce along path from root to Node n with n.key = k
     *
     * 
     * Note:
     * 1. return value remains valid across subsequent inserts and assignments,
     *    so this is legal:
     *      RbTree rbtree = ...;
     *      auto v = rbtree[key1];
     *
     *      rbtree[key2] = ...;
     *      rbtree.insert(key3, value3);
     *
     *      v = ...;
     *
     * 2. return value is not valid across removes,  even of distinct keys,
     *    so this is ILLEGAL:
     *      RbTree rbtree = ...;
     *      auto v = rbtree[key1];
     *
     *      assert(key1 != key2);
     *
     *      rbtree.remove(key2);
     *
     *      v = ...;   // undefined behavior,
     *                 // v.node contents may have been copied and v.node deleted
     */
    RbTreeLhs operator[](Key const & k) {
      std::pair<bool, RbNode *> insert_result
	= RbUtil::insert_aux(k,
			     Value() /*used iff creating new node */,
			     false /*allow_replace_flag*/,
			     this->reduce_fn_,
			     &(this->root_));

      return RbTreeLhs(this, insert_result.second);
    } /*operator[]*/

    /* compute value of reduce applied to the set K of all keys k[j] in subtree
     * N with:
     * - k[j] <= lub_key  if is_closed = true
     * - k[j] <  lub_key  if is_closed = false
     * return reduce_fn.nil() if K is empty
     */
    ReducedValue reduce_lub(Key const &lub_key, bool is_closed) const {
      return RbUtil::reduce_lub(lub_key,
				this->reduce_fn_,
				is_closed,
				this->root_);
    } /*reduce_lub*/

    /* Provided Reduce computes sum,  and we call this rbtree f
     * with keys k[i] and values v[i]:
     *
     * returns iterator pointing to i'th key-value pair {k[i],v[i]} in this tree,
     * with reduced value r(i) (i.e. RbNode::reduced1);
     * where r(i) is the result of reducing all values v[j] with j<=i
     *
     * editor bait: invert_integral
     */
    const_iterator cfind_sum_glb(ReducedValue const & y) const {
      using logutil::tostr;
      using logutil::xtag;

      char const * c_self = "RedBlackTree::find_sum_glb";

      RbNode * N = RbUtil::find_sum_glb(this->reduce_fn_,
					this->root_,
					y);

      if(!N) {
	/* for no-lower-bound edge cases,  return iterator ix
	 * pointing to 'before the beginning' of this tree.
	 *
	 * will have
	 *   ix.is_deferenceable() == false
	 *   (bool)ix == false
	 */
	return const_iterator(detail::IL_BeforeBegin,
			      RbUtil::find_leftmost(this->root_));
      }

      return const_iterator(detail::IL_Regular, N);
    } /*cfind_sum_glb*/

    const_iterator find_sum_glb(ReducedValue const & y) const {
      return this->cfind_sum_glb(y);
    } /*find_sum_glb*/

    /* non-const version of .cfind_sum_glb() */
    iterator find_sum_glb(ReducedValue const & y) {
      const_iterator ix = this->cfind_sum_glb(y);

      return iterator(ix.location(),
		      const_cast<RbNode *>(ix.node()));
    } /*find_sum_glb*/

    bool insert(Key const & k, Value const & v) {
      std::pair<bool, RbNode *> insert_result
	= RbUtil::insert_aux(k,
			     v,
			     true /*allow_replace_flag*/,
			     this->reduce_fn_,
			     &(this->root_));

      if (insert_result.first)
        ++(this->size_);

      return insert_result.first;
    } /*insert*/

    bool insert(Key &&k, Value &&v) {
      using logutil::scope;
      using logutil::xtag;

      constexpr const char *c_self = "RedBlackTree::insert";

      constexpr bool c_logging_enabled = false;
      scope lscope(c_self, c_logging_enabled);

      std::pair<bool, RbNode *> insert_result
	= RbUtil::insert_aux(k,
			     v,
			     true /*allow_replace_flag*/,
			     this->reduce_fn_,
			     &(this->root_));

      if (insert_result.first)
        ++(this->size_);

      if (c_logging_enabled) {
        lscope.log(c_self, ": after insert", xtag("key", k), xtag("value", v),
                   xtag("tree.size", root_->size()), xtag("retval", insert_result.first));
      }

      return insert_result.first;
    } /*insert*/

    bool remove(Key const &k) {
      bool retval = RbUtil::remove_aux(k,
				       this->reduce_fn_,
				       &(this->root_));

      if (retval)
        --(this->size_);

      return retval;
    } /*remove*/

    /* verify class invariants.
     * unless implementation is broken,  or client manages
     * to violate api rules,   this will always return true.
     *
     * RB0. if root node is nil then .size is 0
     * RB1. if root node is non-nil,  then root->parent() is nil,
     *      and .size = root->size
     * RB2. if N = P->child(d),  then N->parent()=P
     * RB3. all paths to leaves have the same black height
     * RB4. no red node has a red parent
     * RB5. inorder traversal visits keys in monotonically increasing order
     * RB6. Node::size reports the size of the subtree reachable from that node
     *      via child pointers
     * RB7. Node::reduced reports the value of
     *       f(f(L, Node::value), R)
     *      where: L is reduced-value for left child,
     *             R is reduced-value for right child
     * RB8. RedBlackTree.size() equals the #of nodes in tree
     */
    bool verify_ok() const {
      using logutil::scope;
      using logutil::tostr;
      using logutil::xtag;

      constexpr const char *c_self = "RedBlackTree::verify_ok";
      constexpr bool c_logging_enabled = false;

      scope lscope(c_self, c_logging_enabled);

      /* RB0. */
      if (root_ == nullptr) {
        XO_EXPECT(size_ == 0, tostr(c_self, ": expect .size=0 with null root",
                                    xtag("size", size_)));
      }

      /* RB1. */
      if (root_ != nullptr) {
        XO_EXPECT(root_->parent_ == nullptr,
                  tostr(c_self, ": expect root->parent=nullptr",
                        xtag("parent", root_->parent_)));
        XO_EXPECT(root_->size_ == this->size_,
                  tostr(c_self, ": expect self.size=root.size",
                        xtag("self.size", size_),
                        xtag("root.size", root_->size_)));
      }

      /* height (counting only black nodes) of tree */
      int32_t black_height = 0;

      /* n_node: #of nodes in this->root_ */
      size_t n_node = RbUtil::verify_subtree_ok(this->reduce_fn_,
						this->root_,
						&black_height);

      /* RB8. RedBlackTree.size() equals #of nodes in tree */
      XO_EXPECT(n_node == this->size_,
		tostr(c_self, ": expect self.size={#of nodes n in tree}",
		      xtag("self.size", size_),
		      xtag("n", n_node)));

      if (c_logging_enabled)
        lscope.log(xtag("size", this->size_),
                   xtag("blackheight", black_height));

      return true;
    } /*verify_ok*/

    void display() const { RbUtil::display(this->root_, 0); } /*display*/

  private:
    /* #of key/value pairs in this tree */
    size_t size_ = 0;
    /* root of red/black tree */
    RbNode *root_ = nullptr;
    /* .reduce_fn :: (Accumulator x Key) -> Accumulator */
    Reduce reduce_fn_;
  }; /*RedBlackTree*/

  template <typename Key,
	    typename Value,
	    typename Reduce>
  std::ostream &operator<<(std::ostream &os,
			   RedBlackTree<Key, Value, Reduce> const &tree)
  {
    tree.display();
    return os;
  } /*operator<<*/

} /*namespace tree*/
} /*namespace xo*/

/* end RedBlackTree.hpp */
