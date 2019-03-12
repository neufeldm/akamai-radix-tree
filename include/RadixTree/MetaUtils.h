#ifndef AKAMAI_MAPPER_RADIX_TREE_METAUTILS_H_
#define AKAMAI_MAPPER_RADIX_TREE_METAUTILS_H_

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

#include <tuple>

/**
 * \file MetaUtils.h
 * Some template metaprogramming utilities that make it easier to build
 * functions that operate on an arbitrary number of trees.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

// XXX need to audit these, only include what we really need, also
// XXX separate out declaration/implementation and add comments


/**
 * \brief Calls a function object on each argument individually, returns true if any return true.
 *
 * Short circuits, just like the "||" operator, base compile-time recursive call.
 */
template <typename Func,typename FirstArg,typename... RestOfArgs>
bool checkIfAny(Func&& fn,FirstArg&& first,RestOfArgs&&... rest) {
  if (fn(std::forward<FirstArg>(first))) { return true; }
  return checkIfAny(std::forward<Func>(fn),std::forward<RestOfArgs>(rest)...);
}

/**
 * \brief Terminate recursion on checkIfAny.
 */
template <typename Func>
bool checkIfAny(Func&& /*fn*/) { return false; }

/**
 * \brief Callable wrapper around checkIfAny with template so you can specify the func type for the compiler.
 */
template <typename Func>
struct CheckIfAny {
  /**
   * \brief Callback as rvalue.
   */
  template <typename... Args>
  bool operator()(Func&& fn,Args&&... args) { return checkIfAny<Func>(std::move(fn),std::forward<Args>(args)...); }
  /**
   * \brief Callback as const lvalue.
   */
  template <typename... Args>
  bool operator()(const Func& fn,Args&&... args) { return checkIfAny<Func>(fn,std::forward<Args>(args)...); }
  /**
   * \brief Callback as lvalue.
   */
  template <typename... Args>
  bool operator()(Func& fn,Args&&... args) { return checkIfAny<Func>(fn,std::forward<Args>(args)...); }
};


/**
 * \brief Calls a function object on each argument individually, returns true if all return true.
 *
 * Short circuits, just like an "&&" operation. Base recursive call.
 */
template <typename Func,typename FirstArg,typename... RestOfArgs>
bool checkIfAll(Func&& fn,FirstArg&& first,RestOfArgs&&... rest) {
  if (!fn(std::forward<FirstArg>(first))) return false;
  return checkIfAll(std::forward<Func>(fn),std::forward<RestOfArgs>(rest)...);
}

/**
 * \brief Terminate recursion on checkIfAll.
 */
template <typename Func>
bool checkIfAll(Func&& /*fn*/) { return true; }

/**
 * \brief Callable wrapper around checkIfAll with template so you can specify the func type for the compiler.
 */
template <typename Func>
struct CheckIfAll {
  /**
   * \brief Callback as rvalue.
   */
  template <typename... Args>
  bool operator()(Func&& fn,Args&&... args) { return checkIfAll<Func>(std::move(fn),std::forward<Args>(args)...); }
  /**
   * \brief Callback as const lvalue.
   */
  template <typename... Args>
  bool operator()(const Func& fn,Args&&... args) { return checkIfAll<Func>(fn,std::forward<Args>(args)...); }
  /**
   * \brief Callback as lvalue.
   */
  template <typename... Args>
  bool operator()(Func& fn,Args&&... args) { return checkIfAll<Func>(fn,std::forward<Args>(args)...); }
};


/**
 * \brief Calls a function object on each argument individually, throws away return values.
 *
 * Base recursive call.
 */
template <typename Func,typename FirstArg,typename... RestOfArgs>
void callOnEach(Func&& fn,FirstArg&& first,RestOfArgs&&... rest) {
  fn(std::forward<FirstArg>(first));
  callOnEach(std::forward<Func>(fn),std::forward<RestOfArgs>(rest)...);
}

/**
 * \brief Terminate recursion for callOnEach.
 */
template <typename Func>
void callOnEach(Func&& /*fn*/) {}

/**
 * \brief Call a function on each element of a tuple
 *
 * Calling with arguments:
 * \verbatim
 * (func,tuple<...>{t1,t2,t3})
 * \endverbatim
 * would result in three separate calls:
 * \verbatim
 * func(t1), func(t2), func(t3)
 * \endverbatim
 * This is the base recursive call.
 */
template <std::size_t I=0,typename Func,typename Tuple>
inline
typename std::enable_if<I < std::tuple_size<typename std::remove_reference<Tuple>::type>::value, void>::type
callOnEachTuple(Func&& fn,Tuple&& t) {
  fn(std::get<I>(std::forward<Tuple>(t)));
  // Recurse to the next item in the tuple
  callOnEachTuple<I+1>(std::forward<Func>(fn),std::forward<Tuple>(t));
}

/**
 * \brief Terminate recursion for callOnEachTuple
 */
template <std::size_t I=0,typename Func,typename Tuple>
inline
typename std::enable_if<I == std::tuple_size<typename std::remove_reference<Tuple>::type>::value, void>::type
callOnEachTuple(Func&& /*fn*/,Tuple&& /*t*/) {}

/**
 * \brief Template for compile-time sequence generation/consumption.
 *
 * c++14 has this as a built-in.
 * The static_sequence class contains the numbers 0..(N-1) as its template parameters.
 */
template<std::size_t... NUMS>
struct static_sequence { };

/**
 * \brief Recursive definition for building a compile-time sequence (static_sequence).
 *
 * Pulls off the current limit, decrements it, and prepends it to the list of largest
 * numbers so far. The number is duplicated because the terminating case will
 * match on the extra initial 0 and get rid of it.
 */
template<std::size_t CUR, std::size_t... REST>
struct build_static_sequence : build_static_sequence<CUR - 1, CUR - 1, REST...> { };

/**
 * \brief Terminating definition for building a compile-time sequence (static_sequence).
 */
template<std::size_t... NUMS>
struct build_static_sequence<0, NUMS...> : static_sequence<NUMS...> {
    typedef static_sequence<NUMS...> type;
};

/**
 * \brief Same as build_static_sequence but start at 1 instead of 0.
 */
template<std::size_t CUR, std::size_t... REST>
struct build_static_sequence_from_one : build_static_sequence_from_one<CUR - 1, CUR - 1, REST...> { };

/**
 * \brief Terminating struct definition for build_static_sequence_from_one.
 */
template<std::size_t... NUMS>
struct build_static_sequence_from_one<1, NUMS...> : static_sequence<NUMS...> {
    typedef static_sequence<NUMS...> type;
};

/**
 * \brief Call a function on all of the tuple elements as an argument list, prepending any optional arguments provided after.
 *
 * This is the "helper" that actually makes the call. It takes
 * the expanded static_sequence from the initial call to call_on_all
 * and uses it to expand out the tuple into individual arguments.
 */
template <std::size_t ...S,typename Func,typename... TupleArgs,typename... PrependArgs>
inline
decltype(std::declval<Func>()(std::declval<PrependArgs>()...,std::declval<TupleArgs>()...))
callOnAllTupleSeq(static_sequence<S...> /*s*/,Func&& f,const std::tuple<TupleArgs...>& t,PrependArgs&&... prepend) {
  return f(std::forward<PrependArgs>(prepend)...,std::get<S>(t)...);
}

/**
 * \brief Call a function using all of the tuple elements as an argument list, prepending any optional arguments provided after.
 * 
 * Take the func/tuple, expand out a static sequence with
 * numbers for each tuple element, and then hand it over
 * to the version of call_on_all that takes the expanded
 * sequence and combines it with the tuple to produce
 * arguments.
 */
template <typename Func,typename... TupleArgs,typename... PrependArgs>
inline
decltype(std::declval<Func>()(std::declval<PrependArgs>()...,std::declval<TupleArgs>()...))
callOnAllTuple(Func&& f,const std::tuple<TupleArgs...>& t,PrependArgs&&... prepend) {
  build_static_sequence<sizeof...(TupleArgs)> sseq;
  return callOnAllTupleSeq(sseq,std::forward<Func>(f),t,std::forward<PrependArgs>(prepend)...);
}

/**
 * \brief Like callOnAllTuple, but skips first item of tuple
 */
template <typename Func,typename... TupleArgs,typename... PrependArgs>
inline
decltype(std::declval<Func>()(std::declval<PrependArgs>()...,std::declval<TupleArgs>()...))
callOnAllTupleSkipFirst(Func&& f,const std::tuple<TupleArgs...>& t,PrependArgs&&... prepend) {
  build_static_sequence_from_one<sizeof...(TupleArgs)> sseq;
  return callOnAllTupleSeq(sseq,std::forward<Func>(f),t,std::forward<PrependArgs>(prepend)...);
}

template <std::size_t... S,typename Head,typename... Tail>
inline std::tuple<Tail...>
tuple_skipfirst_seq(const std::tuple<Head,Tail...>& t,static_sequence<S...> /*s*/) {
  return std::tie(std::get<S+1>(t)...);
}

/**
 * \brief Make a new tuple with elements from the argument tuple with the first element removed.
 */
template <typename Head,typename... Tail>
inline std::tuple<Tail...>
tuple_skipfirst(const std::tuple<Head,Tail...>& t) {
  build_static_sequence<sizeof...(Tail)> sseq;
  return tuple_skipfirst_seq(t,sseq);
}


/**
 * \brief Call a function on each element of a tuple, assembling each individual return value into another tuple.
 */
template <std::size_t I,typename Func,typename ResultTuple,typename ArgsTuple>
inline
typename std::enable_if<I < std::tuple_size<typename std::remove_reference<ResultTuple>::type>::value,void>::type
addToResultTuple(Func&& fn,ResultTuple&& resultTuple,ArgsTuple&& argsTuple) {
  std::get<I>(std::forward<ResultTuple>(resultTuple)) = fn(std::get<I>(std::forward<ArgsTuple>(argsTuple)));
  addToResultTuple<I+1,Func>(std::forward<Func>(fn),std::forward<ResultTuple>(resultTuple),std::forward<ArgsTuple>(argsTuple));
}

/**
 * \brief Terminate addToResultTuple recursion.
 */
template <std::size_t I,typename Func,typename ResultTuple,typename ArgsTuple>
inline
typename std::enable_if<I == std::tuple_size<typename std::remove_reference<ResultTuple>::type>::value,void>::type
addToResultTuple(Func&& /*fn*/,ResultTuple&& /*resultTuple*/,ArgsTuple&& /*argsTuple*/) {}


/**
 * \brief Call an instance of Func on all arguments, assemble results into a tuple that is then returned.
 *
 * Assumes that an instance of Func()() is able to take/templated to take all of the types in the argument tuple.
 */
template <typename Func,typename... TupleArgs>
inline
std::tuple<decltype(std::declval<typename std::remove_reference<Func>::type>()(std::declval<TupleArgs>()))...>
callOnEachTupleResult(Func&& fn,const std::tuple<TupleArgs...>& t)
{
  // Type of a tuple containing the return types from the call made on each tuple item.
  std::tuple<decltype(std::declval<typename std::remove_reference<Func>::type>()(std::declval<TupleArgs>()))...> allResults;
  addToResultTuple<0>(std::forward<Func>(fn),allResults,t);
  return allResults;
}

/**
 * \brief Non-const lvalue of tuple version.
 */
template <typename Func,typename... TupleArgs>
inline
std::tuple<decltype(std::declval<typename std::remove_reference<Func>::type>()(std::declval<TupleArgs>()))...>
callOnEachTupleResult(Func&& fn,std::tuple<TupleArgs...>& t)
{
  // Type of a tuple containing the return types from the call made on each tuple item.
  std::tuple<decltype(std::declval<typename std::remove_reference<Func>::type>()(std::declval<TupleArgs>()))...> allResults;
  addToResultTuple<0>(std::forward<Func>(fn),allResults,t);
  return allResults;
}


/**
 * \brief Rvalue of tuple version.
 */
template <typename Func,typename... TupleArgs>
inline
std::tuple<decltype(std::declval<typename std::remove_reference<Func>::type>()(std::declval<TupleArgs>()))...>
callOnEachTupleResult(Func&& fn,std::tuple<TupleArgs...>&& t)
{
  // Type of a tuple containing the return types from the call made on each tuple item.
  std::tuple<decltype(std::declval<typename std::remove_reference<Func>::type>()(std::declval<TupleArgs>()))...> allResults;
  addToResultTuple<0>(std::forward<Func>(fn),allResults,std::move(t));
  return allResults;
}



} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
