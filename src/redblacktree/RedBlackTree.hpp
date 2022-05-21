/* @file RedBlackTree.hpp */

#pragma once

#include <array>
#include <cassert>

namespace xo {
  namespace tree {
    template<typename Key, typename Value>
    class RedBlackTree;

    namespace detail {
      enum Color { C_Invalid = -1, C_Black, C_Red, N_Color };

      enum Direction { D_Invalid = -1, D_Left, D_Right, N_Direction };

      inline Direction other(Direction d) {
	return static_cast<Direction>(1 - d);
      } /*other*/

      template <typename Key, typename Vaue>
      class RbTreeUtil;

      template <typename Key, typename Value>
      class Node {
      public:
	Node() = default;
	Node(Key const & k, Value const & v)
	  : color_(C_Red), size_(1), key_(k), value_(v) {}
	Node(Key && k, Value && v)
	  : color_(C_Red), size_(1), key_(std::move(k)), value_(std::move(v)) {}

	/* return #of key/vaue pairs in tree rooted at x. */
	static size_t tree_size(Node * x) {
	  if(x)
	    return x->size();
	  else
	    return 0;
	} /*tree_size*/

	static bool is_black(Node * x) {
	  if(x)
	    return x->is_black();
	  else
	    return true;
	} /*is_black*/

	static bool is_red(Node * x) {
	  if(x)
	    return x->is_red();
	  else
	    return false;
	} /*is_red*/

        /* replace root pointer *pp_root with x;
         * set x parent pointer to nil
	 */
	static void replace_root_reparent(Node * x, Node ** pp_root)
	{
	  *pp_root = x;
	  if(x)
	    x->parent_ = nullptr;
	} /*replace_root_reparent*/
	  

        size_t size() const { return size_; }
	Node * parent() const { return parent_; }
	Node * child(Direction d) const { return child_v_[d]; }
	Node * left_child() const { return child_v_[0]; }
	Node * right_child() const { return child_v_[1]; }

	/* true if this node has 0 children */
	bool is_leaf() const { return ((child_v_[0] == nullptr) && (child_v_[1] == nullptr)); }

        /* identify which child x represents
         * Require:
	 * - x != nullptr
	 * - x is either this->left_child() or this->right_child()
	 */
	Direction child_direction(Node * x) {
	  if(x == this->left_child())
	    return D_Left;
	  else if(x == this->right_child())
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
	  if(this->color_ == C_Red) {
	    Node * left = this->left_child();
	    Node * right = this->right_child();

	    if(left && left->is_red())
	      return true;

	    if(right && right->is_red())
	      return true;
	  }

	  return false;
	} /*is_red_violation*/

	Color color() const { return color_; }
	Key const & key() const { return key_; }
	Value const & value() const { return value_; }

	/* recalculate size from immediate childrens' sizes */
	void local_recalc_size() {
	  this->size_ = (1
			 + Node::tree_size(this->left_child())
			 + Node::tree_size(this->right_child()));
	} /*local_recalc_size*/

      private:
	void assign_color(Color x) { this->color_ = x; }
	void assign_size(size_t z) { this->size_ = z; }

	void assign_child_reparent(Direction d, Node * x) {
	  Node * old_x = this->child_v_[d];

	  if(old_x)
	    old_x->parent_ = nullptr;
	  
	  this->child_v_[d] = x;

	  if(x) {
	    x->parent_ = this;
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
	Direction replace_child_reparent(Node * x, Node * x_new) {
	  Direction d = this->child_direction(x);

	  if(d == D_Left || d == D_Right) {
	    this->assign_child_reparent(d, x_new);
	    return d;
	  } else {
	    return D_Invalid;
	  }
	} /*replace_child_reparent*/

	friend class RbTreeUtil<Key, Value>;
	friend class xo::tree::RedBlackTree<Key, Value>;
	
      private:
	/* red | black */
	Color color_ = C_Red;
	/* size of subtree (#of key/value pairs) rooted at this node */
	size_t size_ = 0;
	/* key associated with this node */
	Key key_;
	/* value associated with this node */
	Value value_;
        /* pointer to parent node,  nullptr iff this is the root node */
        Node * parent_ = nullptr;
        /*
         * .child_v[0] = left child
         * .child_v[1] = right child
         *
         * invariants:
	 * - if .child_v[x] non-null,  then .child_v[0]->parent = this
	 * - a red node may not have red children
	 */
	std::array<Node *, 2> child_v_ = { nullptr, nullptr };
      }; /*Node*/

      template <typename Key, typename Value>
      class RbTreeUtil {
      public:
	using RbNode = Node<Key, Value>;

      public:
	/* return #of key/vaue pairs in tree rooted at x. */
	static size_t tree_size(RbNode * x) {
	  if(x)
	    return x->size();
	  else
	    return 0;
	} /*tree_size*/

	static bool is_black(RbNode * x) {
	  if(x)
	    return x->is_black();
	  else
	    return true;
	} /*is_black*/

	static bool is_red(RbNode * x) {
	  if(x)
	    return x->is_red();
	  else
	    return false;
	} /*is_red*/

        /* starting from x,  traverse only right children
         * to find node with a nil right child
         *
         * Require:
         * - N non-nil
	 */
	static RbNode * find_rightmost(RbNode * N) {
	  for(;;) {
	    RbNode * S = N->right_child();

	    if(!S)
	      break;

	    N = S;
	  }

	  return N;
	} /*find_rightmost*/

	/* find node in x with key k */
	static RbNode * find(RbNode * x, Key const & k)
	{
	  for(;;) {
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
	static RbNode * find_glb_aux(RbNode * x, RbNode * h, Key const & k, bool is_closed)
	{
	  for(;;) {
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
	} /*find_glb_aux*/

        /* find greatest lower bound node for a key,  in this subtree
         *
         * is_open.  if true,  allow result with N->key = k exactly
	 *           if false,  require N->key < k
	 */ 
	static RbNode * find_glb(RbNode * x, Key const & k, bool is_closed) {
	  return find_glb_aux(x, nullptr, k, is_closed);
	} /*find_glb*/

#ifdef NOT_IN_USE
        /* find least upper bound node for a key,  in this subtree*
         *
         * is_open.  if true,  allow result with N->key = k exactly
         *           if false,  require N->key > k
	 */
	static RbNode * find_lub(RbNode * x, Key const & k, bool is_closed) {
	  if(x->key() > k) {
	    /* x.key is an upper bound for k */
	    if(x->left_child() == nullptr) {
	      /* no tigher upper bound present in subtree rooted at x */
	      return x;
	    }

	    RbNode * y = find_lub(x->left_child(), k, is_closed);

	    if(y) {
	      /* found better upper bound in left subtree */
	      return y;
	    } else {
	      return x;
	    }
	  } else if(is_closed && (x->key() == k)) {
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
         * this-> A                 B  <- retval
         *       / \               / \
         *      R   B      ==>    A   T
         *         / \           / \
         *        S   T         R   S
         *
         * if direction=D_Right:
         *
	 *        G                  G
	 *        |                  |
	 * this-> A                  B <- retval     
	 *       / \                / \           
         *      B   R        ==>   T   A
         *     / \                    / \
	 *    T   S                  S   R
	 */
	static RbNode * rotate(Direction d,
			       RbNode * A,
			       RbNode ** pp_root)
	{
	  Direction other_d = other(d);

	  RbNode * G = A->parent();
	  RbNode * B = A->child(other_d);
	  RbNode * R = A->child(d);
	  RbNode * S = B->child(d);
	  RbNode * T = B->child(other_d);

	  if(G) {
	    G->replace_child_reparent(A, B);
	  } else {
	    RbNode::replace_root_reparent(B, pp_root);
	  }

	  A->assign_child_reparent(other_d, S);
	  A->local_recalc_size();

	  B->assign_child_reparent(d, A);
	  B->local_recalc_size();

	  return B;
	} /*rotate*/

        /* assign x as new child (on side=d) and rebalance.
         * in diagrams below, G is 'this'.
         *
         * 1. Note that P is new child of G after recursive descent into
         * that subtree of G;   it may differ from current child of G
         * because of rotations etc.
         *
         * 2. diagrams are for d=D_Left;
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
	 */
	static void rebalance_child(Direction d,
				    RbNode * G,
				    RbNode ** pp_root)
	{
	  RbNode * P = G->child(d);

	  for(;;) {
            if (G && G->is_red_violation()) {
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
	      G = G->parent();
	      P = G;
	      d = G->child_direction(P);

	      continue;
            }

	    if (!P->is_red_violation()) {
	      /* RB-shape restored */
	      return;
	    }

	    if (!G) {
              /* special case:  P is root of tree.
               * can fix red violation by making P black
	       */
	      P->assign_color(C_Black);
	      return;
	    }

	    Direction other_d = other(d);

            RbNode * R = P->child(d);
            RbNode * S = P->child(other_d);
            RbNode * U = G->child(other_d);

	    assert(is_black(G));
	    assert(is_red(P));
	    assert(is_red(R) || is_red(S));

	    if(RbNode::is_red(U)) {
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
               * (*) exactly one of R or S is red (since we have a red-violation at P)
               *
               * Note: this transformation preserves #of black nodes along path
               * from root to each of {T, R, S},  so it preserves the "equal
               * black-node path" property
               */
              G->assign_color(C_Red);
              P->assign_color(C_Black);
              U->assign_color(C_Black);

	      /* still need to check for red-violation at G's parent */
	      G = G->parent();
	      P = G;
	      d = G->child_direction(P);

	      continue;
	    }

	    assert(RbNode::is_black(U));

            if (RbNode::is_red(S)) {
              /* preparatory step: rotate P in d direction if "inner child" (S)
               * is red inner-child = right-child of left-parent or vice versa
               *
               *        G                      G
               *       / \                    / \
               *      P*  U     ==>    (P'=) S*  U
               *     / \                    / \
               *    R   S*           (R'=) P*
               *                          / \
               *                         R
               */
              RbTreeUtil::rotate(d, P, pp_root);

              /* (relabel S->P etc. for merged control flow below) */
              R = P;
              P = S;
            }

	    /*
	     *    this->  G                P
	     *           / \              / \
	     *          P*  U     ==>    R*  G*
	     *         / \                  / \
	     *        R*  S                S   U
	     *
	     * ok since every path that went through previously-black G
	     * now goes through newly-black P
	     */
	    P->assign_color(C_Black);
	    G->assign_color(C_Red);

	    RbTreeUtil::rotate(other_d, G, pp_root);
	    return;
	  } /*walk toward root until red violation fixed*/
	} /*rebalance_child*/

        /* insert key-value pair (key, value) into this subtree;
	 * return (rebalanced) subtree root
	 */
	static void insert(Key const & k,
			   Value const & v,
			   RbNode * N,
			   RbNode ** pp_root)
	{
	  Direction d = D_Invalid;

	  while(N) {
	    if(k == N->key_) {
	      /* match on this key already present in tree -> just update assoc'd value */
	      N->value_ = v;
	      return;
	    }

	    d = ((k < N->key_) ? D_Left : D_Right);

	    /* insert into left subtree somewhere */
	    RbNode * C = N->child(d);

	    if(!C)
	      break;

	    N = C;
	  }

	  /* invariant: N->child(d) is nil */

	  if(N) {
	    N->assign_child_reparent(d, new Node<Key, Value>(k, v));
	    N->local_recalc_size();

	    assert(is_red(N->child(d)));

	    /* after adding a node,  must rebalance to restore RB-shape */

	    RbTreeUtil::rebalance_child(d, N, pp_root);
	  } else {
	    *pp_root = new Node<Key, Value>(k, v);

	    /* tree with a single node might as well be black */
	    (*pp_root)->assign_color(C_Black);
	  }
	} /*insert*/

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
        static void remove_black_leaf(RbNode *N, RbNode **pp_root) {
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
            assert(is_black(N));

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

            if (is_black(P) && is_black(S) && is_black(C) && is_black(D)) {
              /* Case(1) */

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
              if (P)
                d = P->child_direction(N);

              continue;
            } else {
              break;
            }
          } /*loop looking for a red node*/

          if (is_red(S)) {
            /* Case(3) */

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

            RbTreeUtil::rotate(d, P, pp_root);

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
          }

          assert(is_black(S));

          if (is_red(P) && is_black(C) && is_black(D)) {
            /* Case(4) */

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

            RbTreeUtil::rotate(other_d, S, pp_root);

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

            RbTreeUtil::rotate(d, P, pp_root);

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

        /* returns subtree with node k removed.
         *
         * Require:
         * - this is an immediate child of P
         * - if P is nil,  then this is the root of RB tree
	 *
	 * TODO: return success/fail flag also
	 */
	static bool remove_aux(Key const & k, RbNode * N, RbNode ** pp_root) {
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

	  if(!N || (N->key() != k)) {
	    /* no node with .key = k present,  so cannot remove it */
	    return false;
	  }

          /* first step is to simplify problem so that we're removing
           * a node with 0 or 1 children.
	   */

	  RbNode * X = N->left_child();

	  if(X == nullptr) {
	    /* N has 0 or 1 children */
	    ;
	  } else {
	    /* R will be 'replacement node' for N */
	    RbNode * R = RbTreeUtil::find_rightmost(X);

            /* R->right_child() is nil by definition
             *
             * copy R's (key + value) into N;
             * N now serves as container for information previously
	     * represented by R.
	     */

	    N->key_ = R->key_;
	    N->value_ = R->value_;
	    /* (preserving N->parent_, N->child_v_[]) */

            /* now relabel N as new R (R'),
             * and relabel R as new N (N').
             * Then got to work on reduced problem of deleting N'.
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

	  RbNode * P = N->parent();

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

	  if(N->is_red()) {
	    if(N->is_leaf()) {
              /* replace pointer to N with nil in N's parent. */
	      
	      if(P) {
		P->replace_child_reparent(N, nullptr);
	      } else {
		/* N was sole root node;  tree will be empty after removing it */
		*pp_root = nullptr;
	      }

	      delete N;
	    } else {
	      assert(false);

              /* control can't come here for RB-tree,
	       * because a red node can't have red children,  or just one black child.
	       */
	    }
	  } else /*N->is_black()*/ {
	    RbNode * R = N->left_child();

	    if(!R)
	      R = N->right_child();

	    if(R) {
	      /* if a black node has one child,  that child cannot be black */
	      assert(R->is_red());

              /* replace N with R in N's parent,
	       * + make R black to preserve black-height
	       */
	      R->assign_color(C_Black);

	      if(P) {
		P->replace_child_reparent(N, R);
	      } else {
		/* N was root node */
		*pp_root = R;
	      }
	      
	      delete N;
	    } else {
              /* N is black with no children,
               * may need rebalance here
	       */

	      if(!P) {
		/* N was root node */
		*pp_root = nullptr;

		delete N;
	      }
	      
	      RbTreeUtil::remove_black_leaf(N, pp_root);
	    }
	  }

	  return true;
	} /*remove_aux*/
      }; /*RbTreeUtil*/
    } /*namespace detail*/

    template<typename Key, typename Value>
    class RedBlackTree {
      using RbTreeUtil = detail::RbTreeUtil<Key, Value>;
      using RbNode = detail::Node<Key, Value>;
      using Direction = detail::Direction;

    public:
      RedBlackTree() = default;

      size_t size() const { return size_; }

      void insert(Key const & k, Value const & v) {
	RbTreeUtil::insert(k, v, this->root_, &(this->root_));
      } /*insert*/
      
      void insert(Key && k, Value && v) {
	RbTreeUtil::insert(k, v, this->root_, &(this->root_));
      } /*insert*/

      bool remove(Key const & k) {
	return RbTreeUtil::remove_aux(k, this->root_, &(this->root_));
      } /*remove*/

      /* verify class invariants
       * 1. if root node is non-nil,  then root->parent() is nil.
       * 2. if N = P->child(d),  then N->parent() == P
       * 3. all paths to leaves have the same black height
       * 4. no red node has a red parent
       * 5. inorder traversal visits keys in monotonically increasing order
       * 6. Node::size reports the size of the subtree reachable from that node
       *    via child pointers
       * 7. RedBlackTree.size() equals the #of nodes in tree
       */
      bool verify_ok() const {
      } /*verify_ok*/

    private:
    private:
      /* #of key/value pairs in this tree */
      size_t size_ = 0;
      /* root of red/black tree */
      RbNode * root_ = nullptr;
    }; /*RedBlackTree*/
  } /*namespace tree*/
} /*namespace xo*/

/* end RedBlackTree.hpp */
