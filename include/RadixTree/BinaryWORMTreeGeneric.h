#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_GENERIC_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_GENERIC_H_

#include <stdint.h>
#include <cstddef>
#include <memory>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

template <typename PathT,typename ValueT>
class BinaryWORMCursorROGenericImpl;

template <typename PathT,typename ValueT>
class BinaryWORMCursorROGeneric {
public:
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t MaxDepth = PathT::MaxDepth;
  using PathType = PathT;
  using ValueType = ValueT;
  using NodeValueRO = BinaryWORMValueCopyRO<uint64_t>;
  using NodeValue = NodeValueRO;
  using NodeHeader = BinaryWORMNodeHeaderRO<sizeof(uint64_t),false>;
  using OffsetType = typename NodeHeader::OffsetType;
  using EdgeWordType = typename NodeHeader::EdgeWordType;
  using CursorImplType = BinaryWORMCursorROGenericImpl<PathT,ValueT>;
  
  BinaryWORMCursorROGeneric() = default;
  virtual ~BinaryWORMCursorROGeneric() = default;

  bool atNode() const { return cursorImpl_->atNode(); }
  bool atLeafNode() const { return cursorImpl_->atLeafNode(); }
  bool atValue() const { return cursorImpl_->atValue(); }
  bool goChild(std::size_t child) { return cursorImpl_->goChild(child); }
  bool canGoChild(std::size_t child) const { return cursorImpl_->canGoChild(child); }
  bool canGoChildNode(std::size_t child) const { return cursorImpl_->canGoChildNode(child); }
  PathType getPath() const { return cursorImpl_->getPath(); }
 
  bool goParent() { return cursorImpl_->goParent(); }
  bool canGoParent() const { return cursorImpl_->canGoParent(); }

  NodeValueRO nodeValueRO() const {
    if (cursorImpl_->atValue()) { return NodeValueRO(cursorImpl_->valueCopy()); }
    else { return NodeValueRO{}; }
  }
  NodeValue nodeValue() const { return nodeValueRO(); }

  ValueType valueCopy() const { return cursorImpl_->valueCopy(); }

  template <typename NewValueType>
  NewValueType valueCopyAs() const { return static_cast<NewValueType>(valueCopy()); }

private:
  std::unique_ptr<CursorImplType> cursorImpl_{};
};

template <typename PathT,typename ValueT>
class BinaryWORMCursorROGenericImpl {
  using PathType = PathT;
  using ValueType = ValueT;

  BinaryWORMCursorROGenericImpl() = default;
  virtual ~BinaryWORMCursorROGenericImpl() = default;

  virtual bool atNode() const = 0;
  virtual bool atLeafNode() const = 0;
  virtual bool atValue() const = 0;
  virtual bool goChild(std::size_t child) = 0;
  virtual bool canGoChild(std::size_t child) const = 0;
  virtual bool canGoChildNode(std::size_t child) const = 0;
  virtual bool goParent() = 0;
  virtual bool canGoParent() const = 0;
  virtual PathType getPath() const = 0;

  virtual ValueType valueCopy() const = 0;
};

template <typename PathT,typename ValueT>
class BinaryWORMTreeGenericImpl;

template <typename PathT,typename ValueT>
class BinaryWORMTreeGeneric
{
public:
  using PathType = PathT;
  using CursorROType = BinaryWORMCursorROGeneric<PathT,ValueT>;
  using CursorType = CursorROType;
  using LookupCursorROType = CursorROType;
  using ValueTypeRO = typename CursorROType::ValueType;
  using ValueType = ValueTypeRO;
  using TreeImplType = BinaryWORMTreeGenericImpl<PathT,ValueT>;

  BinaryWORMTreeGeneric() = default;
  BinaryWORMTreeGeneric(std::unique_ptr<TreeImplType>&& t) : treeImpl_(std::move(t)) {}
  virtual ~BinaryWORMTreeGeneric() = default;

  CursorType cursor() const { return walkCursorRO(); }
  CursorROType cursorRO() const { return walkCursorRO(); }
  CursorROType walkCursorRO() const { return treeImpl_->walkCursorRO(); }
  LookupCursorROType lookupCursorRO() const { return treeImpl_->lookupCursorRO(); }

private:
  std::shared_ptr<TreeImplType> treeImpl_{};
};

template <typename PathT,typename ValueT>
class BinaryWORMTreeGenericImpl
{
public:
  using PathType = PathT;
  using CursorROType = BinaryWORMCursorROGeneric<PathT,ValueT>;
  using CursorType = CursorROType;
  using LookupCursorROType = CursorROType;
  using ValueTypeRO = typename CursorROType::ValueType;
  using ValueType = ValueTypeRO;
  
  BinaryWORMTreeGenericImpl() = default;
  virtual ~BinaryWORMTreeGenericImpl() = default;

  virtual CursorROType walkCursorRO() const = 0;
  virtual LookupCursorROType lookupCursorRO() const = 0;
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif