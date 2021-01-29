#ifndef AKAMAI_MAPPER_RADIXTREE_CURSOR_OPS_H_
#define AKAMAI_MAPPER_RADIXTREE_CURSOR_OPS_H_

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
#include <cstddef>
#include <type_traits>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Move cursor along the specified path.
 */
template <typename CursorType,typename PathType>
void cursorGoto(CursorType&& c,PathType&& p) {
  for (std::size_t i = 0;i < p.size(); ++i) { c.goChild(p[i]); }
}

/**
 * \brief Return the node value object at path, leaves cursor at position.
 */
template <typename CursorType,typename PathType>
typename std::decay<CursorType>::type::NodeValue
cursorGotoValue(CursorType&& c,PathType&& p) {
  cursorGoto(std::forward<CursorType>(c),std::forward<PathType>(p));
  return c.nodeValue();
}

/**
 * \brief Move cursor along the specified path until it either reaches a leaf node or the end of the path.
 *
 * Returns the covering node value.
 */
template <typename CursorType,typename PathType>
typename std::decay<CursorType>::type::NodeValue
cursorLookupCoveringValueRO(CursorType&& c,PathType&& p) {
  std::size_t curDepth{0},size = p.size();
  while ((curDepth < size) && c.canGoChildNode(p[curDepth])) {
    c.goChild(p[curDepth++]);
  }
  return c.coveringNodeValueRO();
}

/**
 * \brief Move cursor along the specified path, stop when at the covering value for that path.
 * Returns the depth along the path that the cursor actually stopped.
 */
template <typename CursorType,typename PathType>
std::size_t cursorGotoCovering(CursorType&& c,PathType&& p) {
  std::size_t valDepth{0},curDepth{0},size = p.size();
  while ((curDepth < size) && c.canGoChildNode(p[curDepth])) {
    c.goChild(p[curDepth++]);
    if (c.atValue()) { valDepth = curDepth; }
  }
  // The tree structure may have values or extra nodes (without values)
  // below this covering value - wind back up the tree to make the
  // cursor position match.
  while (curDepth > valDepth) { c.goParent(); --curDepth; }
  return valDepth;
}

/**
 * \brief Move cursor to covering value position, return node value, leave cursor at position.
 */
template <typename CursorType,typename PathType>
typename std::decay<CursorType>::type::NodeValue
cursorGotoCoveringValue(CursorType&& c,PathType&& p) {
  cursorGotoCovering(std::forward<CursorType>(c),std::forward<PathType>(p));
  return c.nodeValue();
}

/**
 * \brief Add a value at a particular path, replaces whatever was there.
 */
template <typename CursorType,typename PathType,typename ValueType>
void cursorAddValueAt(CursorType&& c,PathType&& p,ValueType&& v) {
  cursorGoto(std::forward<CursorType>(c),std::forward<PathType>(p));
  c.addNode();
  c.nodeValue().set(std::forward<ValueType>(v));
}

/**
 * \brief Remove a value and delete the node at a specific path.
 * Only does anything if there's a value at the exact path specified.
 * Performs cleanup on the tree - attempts to remove the node associated
 * with the value and walk back up the tree deleting all of the nodes
 * along the path that it can. Cursor is left at the point where it stopped
 * being able to remove nodes.
 */
template <typename CursorType,typename PathType>
bool cursorRemoveValueAt(CursorType&& c,PathType&& p) {
  std::size_t valueDepth = cursorGotoCovering(std::forward<CursorType>(c),std::forward<PathType>(p));
  if (valueDepth != p.size()) { return false; }
  // Clear our value, remove the node if possible,
  // then wind back up the parent chain removing nodes
  // until we can't remove any more of them.
  c.nodeValue().clear();
  while (!c.atNode() || c.removeNode()) { c.goParent(); }
  return true;
}

/**
 * \brief Deletes the covering value for a particular path.
 * Unlike cursorDeleteValueAt this will remove whatever the covering value
 * of the path is, which might be at the actual path value but might be above it.
 * Returns the depth at which the deletion happened.
 */
template <typename CursorType,typename PathType>
std::size_t cursorRemoveCoveringValue(CursorType&& c,PathType&& p) {
  std::size_t valueDepth = cursorGotoCovering(std::forward<CursorType>(c),std::forward<PathType>(p));
  // Clear our value, remove the node if possible,
  // then wind back up the parent chain removing nodes
  // until we can't remove any more of them.
  c.nodeValue().clear();
  while (!c.atNode() || c.removeNode()) { c.goParent(); }
  return valueDepth;
}

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif