#ifndef AKAMAI_MAPPER_RADIX_TREE_CURSOR_ITERATOR_H_
#define AKAMAI_MAPPER_RADIX_TREE_CURSOR_ITERATOR_H_

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

#include <limits>
#include <vector>
#include <stdint.h>

/**
 * \file CursorIterator.h
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Performs a pre-order iteration with a cursor.
 * Takes a cursor, on each call to next() moves to next place in the
 * underlying tree in pre-order that has a value.
 */
template <typename CursorT,bool ReverseChildren = false,bool StopAtAllNodes = false>
class CursorIteratorPre {
public:
  using CursorType = CursorT;

  CursorIteratorPre() = default;
  CursorIteratorPre(const CursorT& c) : cursor_(c) { reset(); }

  /**
   * \brief Access underlying cursor object.
   */
  const CursorType& cursor() const { return cursor_; }
  CursorType& cursor() { return cursor_; }

  /**
   * \brief Clear current status, return cursor to start of iteration.
   */
  CursorType& reset();

  /**
   * \brief Take new cursor, go to start of iteration.
   */
  CursorType& reset(const CursorType& c) { cursor_ = c; return reset(); }
  CursorType& reset(CursorType&& c) { cursor_ = std::move(c); return reset(); }

  /**
   * \brief Go to next position in iterator.
   */
  CursorType& next();
  CursorIteratorPre<CursorT,ReverseChildren,StopAtAllNodes>& operator++() { next(); return *this; }
  CursorIteratorPre<CursorT,ReverseChildren,StopAtAllNodes> operator++(int) {
    CursorIteratorPre<CursorT,ReverseChildren,StopAtAllNodes> tmp(*this);
    next();
    return tmp;
  }

  /**
   * \brief Return true if we're at the end of our iteration sequence.
   */
  bool finished() const { return lastChildDoneStack_.empty(); }

  /**
   * \brief Access underlying cursor object.
   */
  CursorType* operator->() { return &cursor_; }
  const CursorType* operator->() const { return &cursor_; }

  /**
   * \brief Access underlying cursor object.
   */
  CursorType& operator*() { return cursor_; }
  const CursorType& operator*() const { return cursor_; }

private:
  static std::size_t NoChild() { return std::numeric_limits<std::size_t>::max(); }
  std::vector<std::size_t> lastChildDoneStack_{};
  CursorT cursor_{};
  bool atStopNode() const { return StopAtAllNodes ? cursor_.atNode() : cursor_.atValue(); }
};

/**
 * \brief Convenience function for creating a pre-order iterator.
 */
template <bool ReverseChildren = false,bool StopAtAllNodes = false,typename CursorT>
CursorIteratorPre<typename std::decay<CursorT>::type,ReverseChildren,StopAtAllNodes>
make_preorder_iterator(CursorT&& c) { return CursorIteratorPre<typename std::decay<CursorT>::type,ReverseChildren,StopAtAllNodes>(std::forward<CursorT>(c)); }

/**
 * \brief Performs a post-order iteration with a cursor.
 * Takes a cursor, on each call to next() moves to next place in the
 * underlying tree in post-order that has a value.
 */
template <typename CursorT,bool ReverseChildren = false,bool StopAtAllNodes = false>
class CursorIteratorPost {
public:
  using CursorType = CursorT;

  CursorIteratorPost() = default;
  CursorIteratorPost(const CursorT& c) : cursor_(c) { reset(); }

  /**
   * \brief Access underlying cursor object.
   */
  const CursorType& cursor() const { return cursor_; }
  CursorType& cursor() { return cursor_; }

  /**
   * \brief Clear current status, return cursor to start of iteration.
   */
  CursorType& reset();

  /**
   * \brief Take new cursor, go to start of iteration.
   */
  CursorType& reset(const CursorType& c) { cursor_ = c; return reset(); }
  CursorType& reset(CursorType&& c) { cursor_ = std::move(c); return reset(); }

  /**
   * \brief Go to next position in iterator.
   */
  CursorType& next();
  CursorIteratorPost<CursorT,ReverseChildren,StopAtAllNodes>& operator++() { next(); return *this; }
  CursorIteratorPost<CursorT,ReverseChildren,StopAtAllNodes> operator++(int) {
    CursorIteratorPost<CursorT,ReverseChildren,StopAtAllNodes> tmp(*this);
    next();
    return tmp;
  }

  /**
   * \brief Return true if we're at the end of our iteration sequence.
   */
  bool finished() const { return lastChildDoneStack_.empty(); }

  /**
   * \brief Access underlying cursor object.
   */
  CursorType* operator->() { return &cursor_; }
  const CursorType* operator->() const { return &cursor_; }

  /**
   * \brief Access underlying cursor object.
   */
  CursorType& operator*() { return cursor_; }
  const CursorType& operator*() const { return cursor_; }

private:
  static std::size_t NoChild() { return std::numeric_limits<std::size_t>::max(); }
  std::vector<std::size_t> lastChildDoneStack_{};
  CursorT cursor_{};
  bool atStopNode() const { return StopAtAllNodes ? cursor_.atNode() : cursor_.atValue(); }
};

/**
 * \brief Convenience function for creating a post-order iterator.
 */
template <bool ReverseChildren = false,bool StopAtAllNodes = false,typename CursorT>
CursorIteratorPost<typename std::decay<CursorT>::type,ReverseChildren,StopAtAllNodes>
make_postorder_iterator(CursorT&& c) { return CursorIteratorPost<typename std::decay<CursorT>::type,ReverseChildren,StopAtAllNodes>(std::forward<CursorT>(c)); }

/**
 * \brief Performs a in-order iteration with a cursor.
 * Takes a cursor, on each call to next() moves to next place in the
 * underlying tree in in-order that has a value. Only valid for
 * trees that have radix == 0 mod 2.
 */
template <typename CursorT,bool ReverseChildren = false, bool StopAtAllNodes = false>
class CursorIteratorIn {
public:
  static_assert((CursorT::Radix % 2) == 0,"in-order iterator only available if radix is even");
  using CursorType = CursorT;

  CursorIteratorIn() = default;
  CursorIteratorIn(const CursorT& c) : cursor_(c) { reset(); }
  
  /**
   * \brief Access underlying cursor object.
   */
  const CursorType& cursor() const { return cursor_; }
  CursorType& cursor() { return cursor_; }

  /**
   * \brief Clear current status, return cursor to start of iteration.
   */
  CursorType& reset();

  /**
   * \brief Take new cursor, go to start of iteration.
   */
  CursorType& reset(const CursorType& c) { cursor_ = c; return reset(); }
  CursorType& reset(CursorType&& c) { cursor_ = std::move(c); return reset(); }

  /**
   * \brief Go to next position in iterator.
   */
  CursorType& next();
  CursorIteratorIn<CursorT,ReverseChildren,StopAtAllNodes>& operator++() { next(); return *this; }
  CursorIteratorIn<CursorT,ReverseChildren,StopAtAllNodes> operator++(int) {
    CursorIteratorIn<CursorT,ReverseChildren,StopAtAllNodes> tmp(*this);
    next();
    return tmp;
  }
  /**
   * \brief Return true if we're at the end of our iteration sequence.
   */
  bool finished() const { return iterStack_.empty(); }

  /**
   * \brief Access underlying cursor object.
   */
  CursorType* operator->() { return &cursor_; }
  const CursorType* operator->() const { return &cursor_; }

  /**
   * \brief Access underlying cursor object.
   */
  CursorType& operator*() { return cursor_; }
  const CursorType& operator*() const { return cursor_; }

private:
  static constexpr std::size_t MidPoint = (CursorT::Radix/2);
  static std::size_t NoChild() { return std::numeric_limits<std::size_t>::max(); }
  struct IterPos {
    std::size_t lastChildDone{NoChild()};
    bool finishedMidPoint{false};
    IterPos() = default;
    IterPos(std::size_t lcd) : lastChildDone(lcd) {}
  };
  std::vector<IterPos> iterStack_{};
  CursorT cursor_{};
  bool atStopNode() const { return (cursor_.atValue() || (StopAtAllNodes && cursor_.atNode())); }
};

/**
 * \brief Convenience function for creating an in-order iterator.
 */
template <bool ReverseChildren = false,bool StopAtAllNodes = false,typename CursorT>
CursorIteratorIn<typename std::decay<CursorT>::type,ReverseChildren,StopAtAllNodes>
make_inorder_iterator(CursorT&& c) { return CursorIteratorIn<typename std::decay<CursorT>::type,ReverseChildren,StopAtAllNodes>(std::forward<CursorT>(c)); }

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

//////////////// CursorIteratorPre

template <typename CursorT,bool ReverseChildren,bool StopAtAllNodes>
CursorT& CursorIteratorPre<CursorT,ReverseChildren,StopAtAllNodes>::reset() {
  // If we've already got a stack, then blow it away and reset our
  // cursor to its root.
  std::size_t cursorAt = lastChildDoneStack_.size();
  lastChildDoneStack_.clear();
  // Single entry for the root, so retreat until we get
  // that far (if we need to go anywhere at all).
  while (cursorAt > 1) { cursor_.goParent(); --cursorAt; }

  lastChildDoneStack_.push_back(NoChild());
  // Pre-order, so if we're at a value for the initial cursor
  // position then this is where we need to stay.
  return (atStopNode() ? cursor_ : next());
}

template <typename CursorT,bool ReverseChildren,bool StopAtAllNodes>
CursorT& CursorIteratorPre<CursorT,ReverseChildren,StopAtAllNodes>::next() {
  bool atStop(false);
  while (!lastChildDoneStack_.empty() && !atStop) {
    std::size_t& lastChildDone = lastChildDoneStack_.back();
    std::size_t c{};
    for (c=(lastChildDone+1);c < CursorT::Radix;++c) {
      if (cursor_.canGoChildNode(ReverseChildren ? (CursorT::Radix - c - 1) : c)) { break; }
    }
    if (c == CursorT::Radix) {
      // We've finished all of the children - time to go back
      // up the stack.
      cursor_.goParent();
      lastChildDoneStack_.pop_back();
    } else {
      lastChildDone = c;
      cursor_.goChild(ReverseChildren ? (CursorT::Radix - c - 1) : c);
      lastChildDoneStack_.push_back(NoChild());
      atStop = atStopNode();
    }
  }
  return cursor_;
}

//////////////// CursorIteratorPost

template <typename CursorT,bool ReverseChildren,bool StopAtAllNodes>
CursorT& CursorIteratorPost<CursorT,ReverseChildren,StopAtAllNodes>::reset() {
  // If we've already got a stack, then blow it away and reset our
  // cursor to its root.
  std::size_t cursorAt = lastChildDoneStack_.size();
  lastChildDoneStack_.clear();
  // Single entry in our stack for the root, so retreat until we get
  // that far (if we need to go anywhere at all).
  while (cursorAt > 1) { cursor_.goParent(); --cursorAt; }

  lastChildDoneStack_.push_back(NoChild());
  // For a post-order traversal we'll need to find our actual
  // starting place.
  return next();
}

template <typename CursorT,bool ReverseChildren,bool StopAtAllNodes>
CursorT& CursorIteratorPost<CursorT,ReverseChildren,StopAtAllNodes>::next() {
  bool atStop(false);
  while (!lastChildDoneStack_.empty() && !atStop) {
    std::size_t& lastChildDone = lastChildDoneStack_.back();
    if (lastChildDone == CursorT::Radix) {
      // Pop the place we just finished visiting
      cursor_.goParent();
      lastChildDoneStack_.pop_back();
    } else {
      std::size_t c{};
      for (c=(lastChildDone+1);c < CursorT::Radix;++c) {
        if (cursor_.canGoChildNode(ReverseChildren ? (CursorT::Radix - c - 1) : c)) { break; }
      }
      if (c == CursorT::Radix) {
        // We've finished all of the children - time to visit the node/value if applicable.
        atStop = atStopNode();
        lastChildDone = CursorT::Radix;
      } else {
        cursor_.goChild(ReverseChildren ? (CursorT::Radix - c - 1) : c);
        lastChildDone = c;
        lastChildDoneStack_.push_back(NoChild());
      }
    }
  }
  return cursor_;
}

//////////////// CursorIteratorIn

template <typename CursorT,bool ReverseChildren,bool StopAtAllNodes>
CursorT& CursorIteratorIn<CursorT,ReverseChildren,StopAtAllNodes>::reset() {
  // If we've already got a stack, then blow it away and reset our
  // cursor to its root.
  std::size_t cursorAt = iterStack_.size();
  iterStack_.clear();
  // Single entry for the root, so retreat until we get
  // that far (if we need to go anywhere at all).
  while (cursorAt > 1) { cursor_.goParent(); --cursorAt; }

  iterStack_.emplace_back();
  // For an in-order traversal we need to run an iteration to
  // get to our actual starting point.
  return next();
}

template <typename CursorT,bool ReverseChildren,bool StopAtAllNodes>
CursorT& CursorIteratorIn<CursorT,ReverseChildren,StopAtAllNodes>::next() {
  bool atStop(false);
  while (!iterStack_.empty() && !atStop) {
    IterPos& pos = iterStack_.back();
    std::size_t c{};
    for (c=(pos.lastChildDone+1);c < CursorT::Radix;++c) {
      if (cursor_.canGoChildNode(ReverseChildren ? (CursorT::Radix - c - 1) : c)) { break; }
    }

    if (pos.finishedMidPoint) {
      if (c == CursorT::Radix) {
        // Finished all children and here, pop back up the stack.
        cursor_.goParent();
        iterStack_.pop_back();
      } else {
        pos.lastChildDone = c;
        cursor_.goChild(ReverseChildren ? (CursorT::Radix - c - 1) : c);
        iterStack_.emplace_back();
      }
    } else {
      if (((pos.lastChildDone < MidPoint) || (pos.lastChildDone == NoChild())) &&
          (c >= MidPoint))
      {
        pos.finishedMidPoint = true;
        atStop = atStopNode();
      } else {
        pos.lastChildDone = c;
        cursor_.goChild(ReverseChildren ? (CursorT::Radix - c - 1) : c);
        iterStack_.emplace_back();
      }
    }
  }
  return cursor_;
}

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif