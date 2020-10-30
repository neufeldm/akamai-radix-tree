#ifndef AKAMAI_MAPPER_RADIX_TREE_CURSOR_METAUTILS_H_
#define AKAMAI_MAPPER_RADIX_TREE_CURSOR_METAUTILS_H_

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

/**
 * \file CursorMetaUtils.h
 * Wrap some cursor calls within callable objects for use with our std::tuple based
 * metaprogramming utilities.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Call canGoChildNode(c) on the cursor passed in, c specified at construction.
 */
struct CursorCanGoChildNode {
  CursorCanGoChildNode() = delete;
  CursorCanGoChildNode(std::size_t c) : child(c) {}
  template <typename T>
  bool operator()(const T& cursor) { return cursor.canGoChildNode(child); }
  std::size_t child;
};

/**
 * \brief Call atNode() on the cursor passed in.
 */
struct CursorAtNode {
  template <typename T>
  bool operator()(const T& cursor) { return cursor.atNode(); }
};

/**
 * \brief Call atLeafNode() on the cursor passed in.
 */
struct CursorAtLeafNode {
  template <typename T>
  bool operator()(const T& cursor) { return cursor.atLeafNode(); }
};


/**
 * \brief Call atValue() on the cursor passed in.
 */
struct CursorAtValue {
  template <typename T>
  bool operator()(T&& cursor) { return cursor.atValue(); }
};

/**
 * \brief Call goChild(c) on the cursor passed in, c specified at construction.
 */
struct CursorGoChild {
  CursorGoChild() = delete;
  CursorGoChild(std::size_t c) : child(c) {}
  template <typename T>
  void operator()(T&& cursor) { cursor.goChild(child); }
  std::size_t child;
};

/**
 * \brief Call canGoChild(c) on the cursor passed in, c specified in construction.
 */
struct CursorCanGoChild {
  CursorCanGoChild() = delete;
  CursorCanGoChild(std::size_t c) : child(c) {}
  template <typename T>
  bool operator()(const T& cursor) { return cursor.canGoChild(child); }
  std::size_t child;
};


/**
 * \brief Call canGoParent() on the cursor passed in.
 */
struct CursorCanGoParent {
  template<typename T>
  bool operator()(const T& cursor) {
    return cursor.canGoParent();
  } 
};

/**
 * \brief Call goParent() on the cursor passed in.
 */
struct CursorGoParent {
  template <typename T>
  void operator()(T&& cursor) {
    cursor.goParent();
  }
};

/**
 * \brief Call nodeValueRO() on the cursor passed in.
 */
struct CursorGetNodeValueRO {
  template <typename CursorType>
  typename std::remove_reference<CursorType>::type::NodeValueRO operator()(const CursorType& c) { return c.nodeValueRO(); }
};

/**
 * \brief Call getPath() on the cursor passed in.
 */
struct CursorGetPath {
  template <typename CursorType>
  typename std::remove_reference<CursorType>::type::PathType operator()(const CursorType& c) { return c.getPath(); }

};

/**
 * \brief Call coveringNodeValueRO() on the cursor passed in.
 */
struct CursorGetCoveringNodeValueRO {
  template <typename CursorType>
  typename std::remove_reference<CursorType>::type::NodeValueRO operator()(const CursorType& c) { return c.coveringNodeValueRO(); }
};

/**
 * \brief Call coveringNodeValueDepth() on the cursor passed in.
 */
struct CursorGetCoveringNodeValueDepth {
  template <typename CursorType>
  std::size_t operator()(const CursorType& c) { return c.coveringNodeValueDepth(); }
};

/**
 * \brief Call nodeValue() on the cursor passed in.
 */
struct CursorGetNodeValue {
  template <typename CursorType>
  typename std::decay<CursorType>::type::NodeValue operator()(CursorType&& c) { return c.nodeValue(); }
};


/**
 * \brief Call addNode() on the cursor passed in.
 */
struct CursorAddNode {
  template <typename CursorType>
  typename std::decay<CursorType>::type::NodeValue operator()(CursorType&& c) { return c.addNode(); }
};

/**
 * \brief Call canRemoveNode() on the cursor passed in.
 */
struct CursorCanRemoveNode {
  template <typename CursorType>
  bool operator()(CursorType&& c) { return c.canRemoveNode(); }
};

/**
 * \brief Call removeNode() on the cursor passed in.
 */
struct CursorRemoveNode {
  template <typename CursorType>
  bool operator()(CursorType&& c) { return c.removeNode(); }
};

  
template <typename CursorTuple,std::size_t I=std::tuple_size<CursorTuple>::value - 1>
struct CheckCursorRadixMatch
  : public CheckCursorRadixMatch<CursorTuple,I-1>
{
  static_assert(std::tuple_element<I,CursorTuple>::type::Radix == std::tuple_element<I-1,CursorTuple>::type::Radix,
                "all cursors must have same radix");
};

template <typename CursorTuple>
struct CheckCursorRadixMatch<CursorTuple,0> 
{
  static constexpr bool value = true;
};

template <typename CursorTuple,std::size_t I=std::tuple_size<CursorTuple>::value - 1>
struct CheckCursorMaxDepthMatch
  : public CheckCursorMaxDepthMatch<CursorTuple,I-1>
{
  static_assert(std::tuple_element<I,CursorTuple>::type::MaxDepth == std::tuple_element<I-1,CursorTuple>::type::MaxDepth,
                "all cursors must have same maximum depth");
};

template <typename CursorTuple>
struct CheckCursorMaxDepthMatch<CursorTuple,0>
{
  static constexpr bool value = true;
};


template <typename CursorTuple,std::size_t I=std::tuple_size<CursorTuple>::value - 1>
struct CheckCursorPathSizeMatch
  : public CheckCursorPathSizeMatch<CursorTuple,I-1>
{
  static_assert(std::tuple_element<I,CursorTuple>::type::PathType::MaxDepth == std::tuple_element<I-1,CursorTuple>::type::PathType::MaxDepth,
                "all cursors must have same path size");
};

template <typename CursorTuple>
struct CheckCursorPathSizeMatch<CursorTuple,0> 
{
  static constexpr bool value = true;
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
