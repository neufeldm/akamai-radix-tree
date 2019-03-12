#ifndef AKAMAI_MAPPER_RADIX_TREE_CURSOR_TRAVERSAL_H_
#define AKAMAI_MAPPER_RADIX_TREE_CURSOR_TRAVERSAL_H_

/*
Copyright (c) 2019 Akamai Technologies, Inc

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "CompoundCursor.h"
#include "MetaUtils.h"
#include "CursorMetaUtils.h"

/**
 * \file CursorTraversal.h
 * Generic radix tree traversal functions - should be able to run these on anything
 * that supports the cursor interface. The "ReverseChildren" template argument
 * allows you to make the traversal "right to left" instead of "left to right".
 * That is, if ReverseChildren is true instead of starting at child 0 and incrementing
 * to child R - 1 the traversal will start at child R - 1 and decrement through child 0.
 * 
 * We implement four basic traversal types: pre-order, post-order, in-order, and pre/post-order.
 * Pre/Post/In are as canonically defined. In-order traversals may only be performed on a tree
 * with an even radix. The pre/post order traversal combines pre and post order traversals by
 * calling a separate callback at both pre and post order positions for values in the tree.
 * When multiple cursors are provided a "value" is defined as a position at which any
 * of the individual cursors has a value, i.e. a union of the trees traversed. Child branches
 * are explored if any one of the cursors has a value, the callback is invoked if any of
 * the cursors has a value.
 * 
 * In addition to a standard tree "walk" operation we also implement two additional traversals:
 * "follow" and "follow over". Both of these traversals perform a standard walk over a set of
 * cursors, but add an extra "follower" cursor. The follower follows the same path as the "leading"
 * cursor walk, but the follower isn't used when determining whether or not there's a value at a
 * cursor position or in any child positions. The follower is included in the callbacks, so may be
 * used as part of any required processing. The canonical expected use for a "follower" cursor is
 * for operations that take values from multiple leading trees and produce a result follower tree
 * that is a summary of the contents of the leading trees. The "follow over" operation is the same
 * as the "follow" operation with one important difference. When deciding whether or not to invoke
 * a provided callback the follower is taken into account, i.e. the callback is invoked wherever
 * any of the leading trees or the follower has a value. Note that this doesn't apply to traversing
 * children: a particular child branch of the tree is only traversed if any of the leading cursors
 * have a value, the follower is ignored. The "follow over" operation is useful when accumulating
 * leader tree values "over" the values in another tree.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Traverse tree cursors pre-order, execute callback cb wherever any cursor has a value.
 * 
 * The callback will get passed all of the cursor values as arguments.
 */
template <bool ReverseChildren = false,typename Callback,typename... Cursors>
inline void preOrderWalk(Callback&& cb,Cursors&&... c);

/**
 * \brief Traverse tree cursors post-order, execute callback cb wherever any cursor has a value.
 * 
 * The callback will get passed all of the cursor values as arguments.
 */
template <bool ReverseChildren = false,typename Callback,typename... Cursors>
inline void postOrderWalk(Callback&& cb,Cursors&&... c);

/**
 * \brief Traverse tree cursors in-order, execute callback cb wherever any cursor has a value.
 * Only valid for cursors with an even radix value.
 * The callback will get passed all of the cursor values as arguments.
 */
template <bool ReverseChildren = false,typename Callback,typename... Cursors>
inline void inOrderWalk(Callback&& cb,Cursors&&... c);

/**
 * \brief Traverse tree cursors pre and post order, execute callbacks wherever any cursor has a value.
 * Combination of pre and post order walks. Useful if you need to push state "down" the tree during pre-order
 * to be used on the way "up" during the post-order traversal.
 * The callback will get passed all of the cursor values as arguments.
 */
template <bool ReverseChildren = false,typename PreCallback,typename PostCallback,typename... Cursors>
inline void prePostOrderWalk(PreCallback&& precb,PostCallback&& postcb,Cursors&&... c);

/**
 * @see preOrderWalk, definitions of "follow"
 */
template <bool ReverseChildren = false,typename Callback,typename Follower,typename... Leaders>
inline void preOrderFollow(Callback&& cb,Follower&& f,Leaders&&... l);

/**
 * @see postOrderWalk, definitions of "follow"
 */
template <bool ReverseChildren = false,typename Callback,typename Follower,typename... Leaders>
inline void postOrderFollow(Callback&& cb,Follower&& f,Leaders&&... l);

/**
 * @see inOrderWalk, definitions of "follow"
 */
template <bool ReverseChildren = false,typename Callback,typename Follower,typename... Leaders>
inline void inOrderFollow(Callback&& cb,Follower&& f,Leaders&&... l);

/**
 * @see prePostOrderWalk, definitions of "follow"
 */
template <bool ReverseChildren = false,typename PreCallback,typename PostCallback,typename Follower,typename... Leaders>
inline void prePostOrderFollow(PreCallback&& precb,PostCallback&& postcb,Follower&& f,Leaders&&... l);

/**
 * @see preOrderWalk, definitions of "follow over"
 */
template <bool ReverseChildren = false,typename Callback,typename Follower,typename... Leaders>
inline void preOrderFollowOver(Callback&& cb,Follower&& f,Leaders&&... l);

/**
 * @see postOrderWalk, definitions of "follow over"
 */
template <bool ReverseChildren = false,typename Callback,typename Follower,typename... Leaders>
inline void postOrderFollowOver(Callback&& cb,Follower&& f,Leaders&&... l);

/**
 * @see inOrderWalk, definitions of "follow over"
 */
template <bool ReverseChildren = false,typename Callback,typename Follower,typename... Leaders>
inline void inOrderFollowOver(Callback&& cb,Follower&& f,Leaders&&... l);

/**
 * @see prePostOrderWalk, definitions of "follow over"
 */
template <bool ReverseChildren = false,typename PreCallback,typename PostCallback,typename Follower,typename... Leaders>
inline void prePostOrderFollowOver(PreCallback&& precb,PostCallback&& postcb,Follower&& f,Leaders&&... l);

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <bool ReverseChildren,typename Callback,typename Cursor>
void preOrderWalk(Callback&& cb,Cursor&& c) {
  constexpr std::size_t Radix = std::decay<Cursor>::type::Radix;
  if (c.atValue()) { cb(std::forward<Cursor>(c)); }
  if (ReverseChildren) {
    for (std::size_t i = Radix; i != 0; --i) {
      if (c.canGoChildNode(i-1)) {
        c.goChild(i-1);
        preOrderWalk<ReverseChildren>(std::forward<Callback>(cb),std::forward<Cursor>(c));
        c.goParent();
      }
    }
  } else {
    for (std::size_t i = 0; i < Radix; ++i) {
      if (c.canGoChildNode(i)) {
        c.goChild(i);
        preOrderWalk<ReverseChildren>(std::forward<Callback>(cb),std::forward<Cursor>(c));
        c.goParent(); 
      }
    }
  }
}

template <bool ReverseChildren,typename Callback,typename... Cursors>
void preOrderWalk(Callback&& cb,Cursors&&... c) {
  auto cc = make_compound_cursor_ro(std::forward<Cursors>(c)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  preOrderWalk<ReverseChildren>(newCB,cc);
}


template <bool ReverseChildren,typename Callback,typename Cursor>
void postOrderWalk(Callback&& cb,Cursor&& c) {
  constexpr std::size_t Radix = std::decay<Cursor>::type::Radix;
  if (ReverseChildren) {
    for (std::size_t i = Radix; i != 0; --i) {
      if (c.canGoChildNode(i-1)) {
        c.goChild(i-1);
        postOrderWalk<ReverseChildren>(std::forward<Callback>(cb),std::forward<Cursor>(c));
        c.goParent();
      }
    }
  } else {
    for (std::size_t i = 0; i<Radix; ++i) {
      if (c.canGoChildNode(i)) {
        c.goChild(i);
        postOrderWalk<ReverseChildren>(std::forward<Callback>(cb),std::forward<Cursor>(c));
        c.goParent();
      }
    }
  }
  if (c.atValue()) { cb(std::forward<Cursor>(c)); }
}
template <bool ReverseChildren, typename Callback,typename... Cursors>
void postOrderWalk(Callback&& cb,Cursors&&... c) {
  auto cc = make_compound_cursor_ro(std::forward<Cursors>(c)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  postOrderWalk<ReverseChildren>(newCB,cc);
}


template <bool ReverseChildren=false, typename Callback,typename Cursor>
void inOrderWalk(Callback&& cb,Cursor&& c) {
  constexpr std::size_t Radix = std::decay<Cursor>::type::Radix;
  static_assert((Radix % 2) == 0,"Tree degree must be even for in-order traversal.");
  if (ReverseChildren) {
    for (std::size_t i = Radix; i != 0; --i) {
      if (c.canGoChildNode(i-1)) {
        c.goChild(i-1);
        inOrderWalk<ReverseChildren>(std::forward<Callback>(cb),std::forward<Cursor>(c));
        c.goParent();
      }
      if ((i == ((Radix/2) + 1)) && c.atValue()) { cb(std::forward<Cursor>(c)); }
    }
  } else {
    for (std::size_t i = 0; i<Radix; ++i) {
      if (c.canGoChildNode(i)) {
        c.goChild(i);
        inOrderWalk<ReverseChildren>(std::forward<Callback>(cb),std::forward<Cursor>(c));
        c.goParent();
      }
      if ((i == ((Radix/2) - 1)) && c.atValue()) { cb(std::forward<Cursor>(c)); }
    }
  }
}
template <bool ReverseChildren,typename Callback,typename... Cursors>
void inOrderWalk(Callback&& cb,Cursors&&... c) {
  auto cc = make_compound_cursor_ro(std::forward<Cursors>(c)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  inOrderWalk<ReverseChildren>(newCB,cc);
}

template <bool ReverseChildren,typename PreCallback,typename PostCallback,typename Cursor>
void prePostOrderWalk(PreCallback&& precb,PostCallback&& postcb,Cursor&& c) {
  constexpr std::size_t Radix = std::decay<Cursor>::type::Radix;
  if (c.atValue()) { precb(std::forward<Cursor>(c)); }
  if (ReverseChildren) {
    for (std::size_t i = Radix; i != 0; --i) {
      if (c.canGoChildNode(i-1)) {
        c.goChild(i-1);
        prePostOrderWalk<ReverseChildren>(std::forward<PreCallback>(precb),std::forward<PostCallback>(postcb),std::forward<Cursor>(c));
        c.goParent();
      }
    }
  } else {
    for (std::size_t i = 0; i<Radix; ++i) {
      if (c.canGoChildNode(i)) {
        c.goChild(i);
        prePostOrderWalk<ReverseChildren>(std::forward<PreCallback>(precb), std::forward<PostCallback>(postcb), std::forward<Cursor>(c));
        c.goParent();
      }
    }
  }
  if (c.atValue()) { postcb(std::forward<Cursor>(c)); }
}

template <bool ReverseChildren, typename PreCallback,typename PostCallback,typename... Cursors>
void prePostOrderWalk(PreCallback&& precb,PostCallback&& postcb,Cursors&&... c) {
  auto cc = make_compound_cursor_ro(std::forward<Cursors>(c)...);
  auto preCBPtr = &precb;
  auto newPreCB =
    [preCBPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<PreCallback>(*preCBPtr),cbc.allCursors());
    };
  auto postCBPtr = &postcb;
  auto newPostCB =
    [postCBPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<PostCallback>(*postCBPtr),cbc.allCursors());
    };
  prePostOrderWalk<ReverseChildren>(newPreCB,newPostCB,cc);
}


// traverse leader pre-order, follower traces the same path, callback whenever leader is at a value
template <bool ReverseChildren,typename Callback,typename Follower,typename... Leaders>
void preOrderFollow(Callback&& cb,Follower&& f,Leaders&&... l) {
  auto cc = make_compound_follow_cursor_ro(std::forward<Follower>(f),std::forward<Leaders>(l)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  preOrderWalk<ReverseChildren>(newCB,cc);
}

template <bool ReverseChildren,typename Callback,typename Follower,typename... Leaders>
void postOrderFollow(Callback&& cb,Follower&& f,Leaders&&... l) {
  auto cc = make_compound_follow_cursor_ro(std::forward<Follower>(f),std::forward<Leaders>(l)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  postOrderWalk<ReverseChildren>(newCB,cc);
}
template <bool ReverseChildren,typename Callback,typename Follower,typename... Leaders>
void inOrderFollow(Callback&& cb,Follower&& f,Leaders&&... l) {
  auto cc = make_compound_follow_cursor_ro(std::forward<Follower>(f),std::forward<Leaders>(l)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  inOrderWalk<ReverseChildren>(newCB,cc);
}
template <bool ReverseChildren,typename PreCallback,typename PostCallback,typename Follower,typename... Leaders>
void prePostOrderFollow(PreCallback&& precb,PostCallback&& postcb,Follower&& f,Leaders&&... l) {
  auto cc = make_compound_follow_cursor_ro(std::forward<Follower>(f),std::forward<Leaders>(l)...);
  auto preCBPtr = &precb;
  auto newPreCB =
    [preCBPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<PreCallback>(*preCBPtr),cbc.allCursors());
    };
  auto postCBPtr = &postcb;
  auto newPostCB =
    [postCBPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<PostCallback>(*postCBPtr),cbc.allCursors());
    };
  prePostOrderWalk<ReverseChildren>(newPreCB,newPostCB,cc);
}

  // traverse leader pre-order, follower traces the same path, callback whenever either leader or follower is at a value
template <bool ReverseChildren, typename Callback,typename Follower,typename... Leaders>
void preOrderFollowOver(Callback&& cb,Follower&& f,Leaders&&... l) {
  auto cc = make_compound_follow_over_cursor_ro(std::forward<Follower>(f),std::forward<Leaders>(l)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  preOrderWalk<ReverseChildren>(newCB,cc);
}

template <bool ReverseChildren,typename Callback,typename Follower,typename... Leaders>
void postOrderFollowOver(Callback&& cb,Follower&& f,Leaders&&... l) {
  auto cc = make_compound_follow_over_cursor_ro(std::forward<Follower>(f),std::forward<Leaders>(l)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  postOrderWalk<ReverseChildren>(newCB,cc);
}
template <bool ReverseChildren,typename Callback,typename Follower,typename... Leaders>
void inOrderFollowOver(Callback&& cb,Follower&& f,Leaders&&... l) {
  auto cc = make_compound_follow_over_cursor_ro(std::forward<Follower>(f),std::forward<Leaders>(l)...);
  auto cbPtr = &cb;
  auto newCB =
    [cbPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<Callback>(*cbPtr),cbc.allCursors());
    };
  inOrderWalk<ReverseChildren>(newCB,cc);
}
template <bool ReverseChildren,typename PreCallback,typename PostCallback,typename Follower,typename... Leaders>
void prePostOrderFollowOver(PreCallback&& precb,PostCallback&& postcb,Follower&& f,Leaders&&... l) {
  auto cc = make_compound_follow_over_cursor_ro(std::forward<Follower>(f),std::forward<Leaders>(l)...);
  auto preCBPtr = &precb;
  auto newPreCB =
    [preCBPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<PreCallback>(*preCBPtr),cbc.allCursors());
    };
  auto postCBPtr = &postcb;
  auto newPostCB =
    [postCBPtr](decltype(cc)& cbc) {
      callOnAllTuple(std::forward<PostCallback>(*postCBPtr),cbc.allCursors());
    };
  prePostOrderWalk<ReverseChildren>(newPreCB,newPostCB,cc);
}

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif

