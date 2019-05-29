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
  using NodeValueRO = BinaryWORMValueCopyRO<ValueType>;
  using NodeValue = NodeValueRO;
  using BinaryWORMNodeType = BinaryWORMNodeT;
  using OffsetType = typename BinaryWORMNodeT::OffsetType;
  using EdgeWordType = typename BinaryWORMNodeT::EdgeWordType;
  using CursorImplType = GenericBinaryWORMCursorROImpl<MAXDEPTH,ValueT>;
  
  BinaryWORMCursorROGeneric() = default;
  virtual ~BinaryWORMCursorROGeneric() = default;

  PathType getPath() const { return curPath_; }
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

  template <typename NewValueType>
  NewValueType valueCopyAs() const { return static_cast<NewValueType>(valueCopy()); }

private:
  std::unique_ptr<CursorImplType> cursorImpl_{};
};

template <typename PathT,typename ValueT>
class BinaryWORMCursorROGenericImpl {
  using PathType = PathT;
  using ValueType = ValueT;

  GenericBinaryWORMCursorROGenericImpl() = default;
  virtual ~GenericBinaryWORMCursorROGenericImpl() = default;

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

template <typename CursorImplT>
class BinaryWORMCursorROGenericGlue
  : public BinaryWORMCursorROGenericImpl<typename CursorImplT::PathType,typename CursorImplT::ValueType>
  , public CursorImplT
{
public:
  using PathType = typename CursorImplT::PathType;
  using ValueType = typename CursorImplT::ValueType;
  using CursorImpl = CursorImplT;
  using GenericImpl = BinaryWORMCursorROGenericImpl<PathType,ValueType>;
  BinaryWORMCursorROGenericGlue() = default;
  template <typename... ConstructorArgs>
  BinaryWORMCursorROGenericGlue(ConstructorArgs&& cargs...) : CursorImpl(std::forward<ConstructorArgs>(cargs)...) {}
  virtual ~BinaryWORMCursorROGenericGlue() = default;

  virtual bool GenericImpl::atNode() const override { return CursorImpl::atNode(); }
  virtual bool GenericImpl::atLeafNode() const override { return CursorImpl::atLeafNode(); }
  virtual bool GenericImpl::atValue() const override { return CursorImpl::atValue(); }
  virtual bool GenericImpl::goChild(std::size_t child) override { return CursorImpl::goChild(child); }
  virtual bool GenericImpl::canGoChild(std::size_t child) const override { return CursorImpl::canGoChild(child); }
  virtual bool GenericImpl::canGoChildNode(std::size_t child) const override { return CursorImpl::canGoChildNMode(child); }
  virtual bool GenericImpl::goParent() override { return CursorImpl::goParent(); }
  virtual bool GenericImpl::canGoParent() const override { return CursorImpl::canGoParent(); }
  virtual ValueType GenericImpl::valueCopy() const override { return *(CursorImpl::nodeValueRO().getPtr()); }
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

template <typename TreeImplT>
class BinaryWORMTreeGenericGlue
  : public BinaryWORMTreeGenericImpl<typename TreeImplT::PathType,typename TreeImplT::ValueType>
  , public TreeImplT
{
public:
  using PathType = typename TreeImplT::PathType;
  using ValueType = typename TreeImplT::ValueType;
  using GenericImpl = BinaryWORMTreeGenericImpl<PathType,ValueType>;
  using TreeImpl = TreeImplT;
  BinaryWORMTreeGenericGlue() = default;
  template <typename... ConstructorArgs>
  BinaryWORMTreeGenericGlue(ConstructorArgs&& cargs...) : CursorImpl(std::forward<ConstructorArgs>(cargs)...) {}
  virtual ~BinaryWORMTreeROGenericGlue() = default;

  virtual CursorROType GenericImpl::walkCursorRO() const override { return TreeImpl::walkCursorRO(); }
  virtual LookupCursorROType GenericImpl::lookupCursorRO() const override { return TreeImpl::lookupCursorRO(); }
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif