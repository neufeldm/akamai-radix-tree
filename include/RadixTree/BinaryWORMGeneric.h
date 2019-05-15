#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_GENERIC_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_GENERIC_H_

#include <stdint.h>
#include <cstddef>
#include <string>
#include <array>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

template <typename Buffer,typename PathType,typename ValueType>
class GenericBinaryWORMTreeBuilder
{
public:
  using HasChild = std::array<bool,2>;
  /**
   * \brief Begin construction of a binary WORM tree, use buffer, optionally stats only.
   */
  virtual bool start(Buffer&& buffer,bool statsOnly = false) = 0;
  /**
   * \brief Begin construction of a binary WORM tree, optionally stats only.
   */
  virtual bool start(bool statsOnly = false) = 0;
  /**
   * \brief Has construction of a tree been started?
   */
  virtual bool started() const = 0;
  /**
   * \brief Add a node at a particular path in the tree.
   * 
   * Nodes must be added in pre-order. Any value pointer must be valid until
   * the call returns. Will throw an exception if constraints are violated.
   */
  virtual void addNode(const PathType& path,bool hasValue,const ValueType* v,const HasChild& hasChild) = 0;

  /**
   * \brief Indicate that the tree is complete.
   * 
   * The builder tracks what added nodes still require children, and if finish
   * is called before all outstanding children have been added then finish will fail.
   */
  virtual bool finish() = 0;
  /**
   * \brief Did we start and subsequently finish a tree?
   */
  virtual bool finished() const = 0;
  /**
   * \brief Return current buffer size.
   */
  virtual std::size_t sizeofBuffer() const = 0;
  /**
   * \brief Moves the current buffer manager out, clears the internal tree state.
   */
  virtual Buffer extractBuffer() = 0;
  /**
   * \brief Const access to the current buffer manager.
   */
  virtual const Buffer& buffer() const = 0;
  /**
   * \brief Statistics for whatever tree is under construction.
   */
  // XXX need to define this...
  virtual const TreeStats& treeStats() const = 0;
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif