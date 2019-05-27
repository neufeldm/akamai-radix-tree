#ifndef AKAMAI_MAPPER_RADIX_TREE_COMPOUND_CURSOR_H_
#define AKAMAI_MAPPER_RADIX_TREE_COMPOUND_CURSOR_H_

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

#include <stdint.h>
#include <limits>
#include <tuple>
#include <stack>

#include "NodeValue.h"
#include "MetaUtils.h"
#include "CursorMetaUtils.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief A cursor that contains multiple other cursors, effectively presenting the union of them as a single cursor.
 * 
 * The node values and current cursor paths are all presented as tuples of the individual
 * values for each contained cursor.
 */
template <typename... CursorTypes>
class CompoundCursorRO
{
public:
  typedef CompoundCursorRO<CursorTypes...> CursorROType;
  typedef CompoundCursorRO<CursorTypes...> CursorType;

  using AllCursors = std::tuple<typename std::decay<CursorTypes>::type...>;
  using AllCursorsRef = std::tuple<typename std::add_lvalue_reference<typename std::decay<CursorTypes>::type>::type...>;
  using AllCursorsConstRef = std::tuple<typename std::add_const<typename std::add_lvalue_reference<typename std::decay<CursorTypes>::type>::type>::type...>;

  static_assert(CheckCursorRadixMatch<AllCursors>::value,"all cursors must have same radix");
  static_assert(CheckCursorPathSizeMatch<AllCursors>::value,"all cursors must have same path size");
  
  static constexpr std::size_t Radix = std::tuple_element<0,AllCursors>::type::Radix;
  static constexpr std::size_t MaxDepth = std::tuple_element<0,AllCursors>::type::MaxDepth;


  using NodeValueRO = std::tuple<typename std::decay<CursorTypes>::type::NodeValueRO...>;
  using NodeValue = NodeValueRO;
  using ValueType = std::tuple<typename std::decay<CursorTypes>::type::ValueType...>;
  using PathType = std::tuple<typename std::decay<CursorTypes>::type::PathType...>;

  CompoundCursorRO() = delete;
  CompoundCursorRO(const CursorTypes&... cursors) : allCursors_(cursors...) {}
  CompoundCursorRO(const CompoundCursorRO& other) = default;
  CompoundCursorRO(CompoundCursorRO&& other) = default;
  CompoundCursorRO& operator=(const CompoundCursorRO& other) = default;
  CompoundCursorRO& operator=(CompoundCursorRO&& other) = default;

  // Interface methods start here
  PathType getPath() const { return callOnAllTuple(callOnEachTupleResult(CursorGetPath{}, allCursors_)); }
  bool atNode() const { return callOnAllTuple(CheckIfAny<CursorAtNode>{},allCursors_,CursorAtNode{}); }
  bool atLeafNode() const { return allAtNode() && callOnAllTuple(CheckIfAll<CursorAtLeafNode>{},allCursors_,CursorAtLeafNode{}); }
  bool allAtNode() const { return callOnAllTuple(CheckIfAll<CursorAtNode>{},allCursors_,CursorAtNode{}); }
  bool atValue() const { return callOnAllTuple(CheckIfAny<CursorAtValue>{},allCursors_,CursorAtValue{}); }
  bool allAtValue() const { return callOnAllTuple(CheckIfAll<CursorAtValue>{},allCursors_,CursorAtValue{}); }
  bool goChild(std::size_t child) {
    if (!canGoChild(child)) { return false; }
    callOnEachTuple(CursorGoChild{child},allCursors_);
    return true;
  }
  bool canGoChild(std::size_t child) const { return callOnAllTuple(CheckIfAll<CursorCanGoChild>{},allCursors_,CursorCanGoChild{child}); }
  bool canGoChildNode(std::size_t child) const { return callOnAllTuple(CheckIfAny<CursorCanGoChildNode>{},allCursors_,CursorCanGoChildNode{child}); }
  bool goParent() {
    if (!canGoParent()) { return false; }
    callOnEachTuple(CursorGoParent{},allCursors_);
    return true;
  }
  bool canGoParent() const { return callOnAllTuple(CheckIfAll<CursorCanGoParent>{},allCursors_,CursorCanGoParent{}); }

  NodeValueRO nodeValueRO() const { return callOnEachTupleResult(CursorGetNodeValueRO{},allCursors_); }
  template <std::size_t I>
  typename std::tuple_element<I,NodeValueRO>::type
  nodeValueRO() const { return std::get<I>(allCursors_).nodeValueRO(); }
  
  NodeValue nodeValue() const { return nodeValueRO(); }
  template <std::size_t I>
  typename std::tuple_element<I,NodeValue>::type
  nodeValue() const { return nodeValueRO<I>(); }
  
  NodeValueRO coveringNodeValueRO() const { return callOnEachTupleResult(CursorGetCoveringNodeValueRO{},allCursors_); }
  template <std::size_t I>
  typename std::tuple_element<I,NodeValueRO>::type
  coveringNodeValueRO() const { return std::get<I>(allCursors_).coveringNodeValueRO(); }

  NodeValueRO coveringNodeValue() const { return coveringNodeValueRO(); }
  template <std::size_t I>
  typename std::tuple_element<I,NodeValueRO>::type
  coveringNodeValue() const { return coveringNodeValueRO<I>(); }

  const AllCursors& allCursors() const { return allCursors_; }
  AllCursors& allCursors() { return allCursors_; }

  /**
   * \brief Access cursor number I in the compound cursor.
   */
  template <std::size_t I>
  typename std::tuple_element<I,AllCursorsRef>::type
  cursor() { return std::get<I>(allCursors_); }

  /**
   * \brief Const access cursor number I in the compound cursor.
   */
  template <std::size_t I>
  typename std::tuple_element<I,AllCursorsConstRef>::type
  cursor() const { return std::get<I>(allCursors_); }

protected:
  AllCursors allCursors_;
};

/**
 * \brief Read/write version of CompoundCursorRO
 */
template <typename... CursorTypes>
class CompoundCursor
  : public CompoundCursorRO<CursorTypes...>
{
public:
  using NodeValue = std::tuple<typename std::decay<CursorTypes>::type::NodeValue...>;
  static constexpr std::size_t Radix = CompoundCursorRO<CursorTypes...>::Radix;
  static constexpr std::size_t MaxDepth = CompoundCursorRO<CursorTypes...>::MaxDepth;

  CompoundCursor() = delete;
  CompoundCursor(const CursorTypes&... cursors) : CompoundCursorRO<CursorTypes...>(cursors...) {}
  CompoundCursor(const CompoundCursor& other) = default;
  CompoundCursor(CompoundCursor&& other) = default;
  CompoundCursor& operator=(const CompoundCursor& other) = default;
  CompoundCursor& operator=(CompoundCursor&& other) = default;

  NodeValue addNode() {
    callOnEachTuple(CursorAddNode{},this->allCursors_);
    return nodeValue();
  }

  bool canRemoveNode() const { return callOnAllTuple(CheckIfAll<CursorCanRemoveNode>{},this->allCursors_,CursorCanRemoveNode{}); }
  // only remove if we can remove on all
  bool removeNode() {
    if (!canRemoveNode()) { return false; }
    callOnEachTuple(CursorRemoveNode{},this->allCursors_);
    return true;
  }
  NodeValue nodeValue() { return callOnEachTupleResult(CursorGetNodeValue{},this->allCursors_); }
  template <std::size_t I>
  typename std::tuple_element<I,NodeValue>::type
  nodeValue() { return std::get<I>(this->allCursors_).nodeValue(); }
};

/**
 * \brief Utility function for creating compound cursor RO leveraging type inference.
 */
template <typename... CursorTypes>
CompoundCursorRO<typename std::remove_reference<CursorTypes>::type...>
make_compound_cursor_ro(CursorTypes&&... cursors) {
  return CompoundCursorRO<typename std::decay<CursorTypes>::type...>(std::forward<CursorTypes>(cursors)...);
}

/**
 * \brief Utility function for creating compound cursor RO leveraging type inference.
 */
template <typename... CursorTypes>
CompoundCursor<typename std::remove_reference<CursorTypes>::type...>
make_compound_cursor(CursorTypes&&... cursors) {
  return CompoundCursor<typename std::decay<CursorTypes>::type...>(std::forward<CursorTypes>(cursors)...);
}

/**
 * \brief Similar to a CompoundCursorRO but ignores the first cursor in the list when considering whether the compound cursor is at a node/value or has child node/values.
 * 
 * The following (first in the tuple) cursor shadows the position of the rest of the cursors without
 * influencing traversal. This is convenient for operations that involve the creation of a new tree
 * based on the values at one or more other trees.
 */
template <typename... CursorTypes>
class CompoundFollowCursorRO
  : public CompoundCursorRO<CursorTypes...>
{
public:
  using BaseCursor = CompoundCursorRO<CursorTypes...>;
  static constexpr std::size_t Radix = BaseCursor::Radix;
  static constexpr std::size_t MaxDepth = BaseCursor::MaxDepth;

  CompoundFollowCursorRO() = delete;
  CompoundFollowCursorRO(const CursorTypes&... cursors) : BaseCursor(cursors...) {}
  CompoundFollowCursorRO(const CompoundFollowCursorRO& other) = default;
  CompoundFollowCursorRO(CompoundFollowCursorRO&& other) = default;
  CompoundFollowCursorRO& operator=(const CompoundFollowCursorRO& other) = default;
  CompoundFollowCursorRO& operator=(CompoundFollowCursorRO&& other) = default;

  // Interface method overrides start here
  bool atNode() const { return callOnAllTupleSkipFirst(CheckIfAny<CursorAtNode>{},this->allCursors_,CursorAtNode{}); }
  bool atValue() const { return callOnAllTupleSkipFirst(CheckIfAny<CursorAtValue>{},this->allCursors_,CursorAtValue{}); }
  bool canGoChildNode(std::size_t child) const { return callOnAllTupleSkipFirst(CheckIfAny<CursorCanGoChildNode>{},this->allCursors_,CursorCanGoChildNode{child}); }
  bool hasChildNode(std::size_t child) const { return canGoChildNode(child); }
};

/**
 * \brief Helper for creating a CompoundFollowCursorRO.
 */
template <typename... CursorTypes>
CompoundFollowCursorRO<typename std::remove_reference<CursorTypes>::type...>
make_compound_follow_cursor_ro(CursorTypes&&... cursors) {
  return CompoundFollowCursorRO<typename std::decay<CursorTypes>::type...>(std::forward<CursorTypes>(cursors)...);
}

/**
 * \brief Similar to CompoundFollowCursorRO except includes following cursor when deciding if a value is present.
 */
template <typename... CursorTypes>
class CompoundFollowOverCursorRO
  : public CompoundCursorRO<CursorTypes...>
{
public:
  using BaseCursor = CompoundCursorRO<CursorTypes...>;
  static constexpr std::size_t Radix = BaseCursor::Radix;
  static constexpr std::size_t MaxDepth = BaseCursor::MaxDepth;

  CompoundFollowOverCursorRO() = delete;
  CompoundFollowOverCursorRO(const CursorTypes&... cursors) : BaseCursor(cursors...) {}
  CompoundFollowOverCursorRO(const CompoundFollowOverCursorRO& other) = default;
  CompoundFollowOverCursorRO(CompoundFollowOverCursorRO&& other) = default;
  CompoundFollowOverCursorRO& operator=(const CompoundFollowOverCursorRO& other) = default;
  CompoundFollowOverCursorRO& operator=(CompoundFollowOverCursorRO&& other) = default;

  // Interface method overrides start here
  bool canGoChildNode(std::size_t child) const { return callOnAllTupleSkipFirst(CheckIfAny<CursorCanGoChildNode>{},this->allCursors_,CursorCanGoChildNode{child}); }
  bool hasChildNode(std::size_t child) const { return canGoChildNode(child); }
};

/**
 * \brief Helper for creating a CompoundFollowOverCursorRO
 */
template <typename... CursorTypes>
CompoundFollowOverCursorRO<typename std::remove_reference<CursorTypes>::type...>
make_compound_follow_over_cursor_ro(CursorTypes&&... cursors) {
  return CompoundFollowOverCursorRO<typename std::decay<CursorTypes>::type...>(std::forward<CursorTypes>(cursors)...);
}

} //  namespace RadixTree
} //  namespace Mapper
} //  namespace Akamai

#endif
