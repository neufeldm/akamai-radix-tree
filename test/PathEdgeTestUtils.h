#ifndef AKAMAI_MAPPER_RADIX_TREE_PATH_EDGE_TEST_UTILS_H_
#define AKAMAI_MAPPER_RADIX_TREE_PATH_EDGE_TEST_UTILS_H_

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

#include <cstddef>
#include <inttypes.h>
#include <string>
#include <random>
#include <algorithm>

#include "RandomUtils.h"

/**
 * \brief Make a new path or edge from an initializer list
 */
template <typename PathType>
inline PathType makePath(std::initializer_list<std::size_t> pathSteps);

template <typename PathType,typename PatternType>
inline PathType makePath(PatternType&& pathSteps);

template <typename PathType>
inline PathType makeRandomPath(uint64_t seed,std::size_t length);

template <typename PathType>
inline std::string pathToString(const PathType& p);

template <typename PathType>
inline std::vector<std::size_t> pathToVector(const PathType& p);

template <typename PathType>
inline bool
pathCompareAt(const PathType& p,std::initializer_list<std::size_t> pattern,std::size_t at = 0);

/**
 * Take a pattern represented by a size_t container and compare with the pattern
 * at offset 'at' in a path/edge object.
 */
template <typename PathType,typename PatternType>
inline bool pathCompareAt(PathType&& path,PatternType&& pattern,std::size_t at = 0);

template <typename PathType,typename PatternType>
inline void pathPushPattern(PathType&& path,PatternType&& pattern);

template <typename PathType>
inline void pathPushPattern(PathType&& path,std::initializer_list<std::size_t> pattern);

template <typename PatternType>
inline void vectorPushPattern(std::vector<std::size_t>& v,std::size_t max,PatternType&& pattern);

inline void
vectorPushPattern(std::vector<std::size_t>& v,std::size_t max,std::initializer_list<std::size_t> pattern);

template <typename PathType,typename PatternType>
inline void pathFillPattern(PathType&& path,PatternType&& pattern);

template <typename PathType>
inline void pathFillPattern(PathType&& path,std::initializer_list<std::size_t> pattern);

template <typename PatternType>
inline void vectorFillPattern(std::vector<std::size_t>& v,std::size_t max,PatternType&& pattern);

inline void
vectorFillPattern(std::vector<std::size_t>& v,std::size_t max,std::initializer_list<std::size_t> pattern);

template <typename PathType>
inline void clearPathPop(PathType&& path);

template <typename PathTypeA,typename PathTypeB>
inline bool pathsEqual(PathTypeA&& pathA,PathTypeB&& pathB);

inline void vectorTrimBack(std::vector<std::size_t>& v,std::size_t c);

inline void vectorTrimFront(std::vector<std::size_t>& v,std::size_t c);


/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <typename PathType>
PathType makePath(std::initializer_list<std::size_t> pathSteps) {
  PathType newPath;
  for (std::size_t curStep : pathSteps) { newPath.push_back(curStep); }
  return newPath;
}

template <typename PathType,typename PatternType>
PathType makePath(PatternType&& pathSteps) {
  PathType newPath;
  std::size_t stepCount = pathSteps.size();
  for (std::size_t i=0; i < stepCount; ++i) { newPath.push_back(pathSteps.at(i)); }
  return newPath;
}

template <typename PathType>
PathType makeRandomPath(uint64_t seed,std::size_t length) {
 return PathType(generateUniformRandomSequence(static_cast<std::size_t>(seed),length,0,PathType::Radix - 1));
}

template <typename PathType>
std::string pathToString(const PathType& p) {
  std::string pstr;
  std::size_t psize{p.size()};
  for (std::size_t i = 0; i < psize; ++i) { pstr += (std::to_string(p.at(i)) + '-'); }
  if (pstr.empty()) { pstr.push_back('-'); }
  else { pstr.pop_back(); }
  pstr += ('/' + std::to_string(psize));
  return pstr;
}

template <typename PathType>
std::vector<std::size_t> pathToVector(const PathType& p) {
  std::vector<std::size_t> pvec;
  std::size_t psize{p.size()};
  pvec.reserve(psize);
  for (std::size_t i = 0; i < psize; ++i) { pvec.push_back(p.at(i)); }
  return pvec;
}

template <typename PathType>
bool pathCompareAt(const PathType& p,std::initializer_list<std::size_t> pattern,std::size_t at)
{
  std::size_t curPos = at;
  for (std::size_t step : pattern) { if (p.at(curPos) != step) { return false; } }
  return true;
}

template <typename PathType,typename PatternType>
bool pathCompareAt(PathType&& path,PatternType&& pattern,std::size_t at) {
  std::size_t patternsize = pattern.size();
  for (std::size_t i = 0; i < patternsize; ++i) {
    if (pattern[i] != path.at(i+at)) { return false; }
  }
  return true;
}

template <typename PathType,typename PatternType>
void pathPushPattern(PathType&& path,PatternType&& pattern) {
  std::size_t patternSize = pattern.size();
  if (patternSize == 0) { throw std::range_error("pathPushPattern: cannot push empty pattern"); }
  for (std::size_t i = 0;i < patternSize; ++i) { path.push_back(pattern[i]); }
}

template <typename PathType>
void pathPushPattern(PathType&& path,std::initializer_list<std::size_t> pattern) {
  if (pattern.size() == 0) { throw std::range_error("pathPushPattern: cannot push empty pattern"); }
  for (std::size_t step : pattern) { path.push_back(step); }
}

template <typename PatternType>
void vectorPushPattern(std::vector<std::size_t>& v,std::size_t max,PatternType&& pattern) {
  std::size_t patternSize = pattern.size();
  if (pattern.size() == 0) { throw std::range_error("vectorPushPattern: cannot push empty pattern"); }
  if ((v.size() + patternSize) > max) { throw std::range_error("vectorPushPattern: exceeded desired max"); }
  for (std::size_t i = 0;i < patternSize;++i) { v.push_back(pattern[i]); }
}

void vectorPushPattern(std::vector<std::size_t>& v,std::size_t max,std::initializer_list<std::size_t> pattern) {
  std::size_t patternSize = pattern.size();
  if (pattern.size() == 0) { throw std::range_error("vectorPushPattern: cannot push empty pattern"); }
  if ((v.size() + patternSize) > max) { throw std::range_error("vectorPushPattern: exceeded desired max"); }
  for (std::size_t step : pattern) { v.push_back(step); }
}

template <typename PathType,typename PatternType>
void pathFillPattern(PathType&& path,PatternType&& pattern) {
  path.clear();
  std::size_t patternSize = pattern.size();
  if (patternSize == 0) { throw std::range_error("pathFillPattern: cannot fill with empty pattern"); }
  std::size_t pathCapacity = path.capacity();
  if (patternSize > pathCapacity) { throw std::range_error("pathFillPattern: pattern too large"); }
  for (std::size_t i = 0; i <= (pathCapacity - patternSize); i += patternSize) {
    pathPushPattern(std::forward<PathType>(path),std::forward<PatternType>(pattern));
  }
}

template <typename PathType>
void pathFillPattern(PathType&& path,std::initializer_list<std::size_t> pattern) {
  path.clear();
  std::size_t patternSize = pattern.size();
  if (patternSize == 0) { throw std::range_error("pathFillPattern: cannot fill with empty pattern"); }
  std::size_t pathCapacity = path.capacity();
  if (patternSize > pathCapacity) { throw std::range_error("pathFillPattern: pattern too large"); }
  for (std::size_t i = 0; i <= (pathCapacity - patternSize); i += patternSize) {
    pathPushPattern(std::forward<PathType>(path),pattern);
  }  
}

template <typename PatternType>
void vectorFillPattern(std::vector<std::size_t>& v,std::size_t max,PatternType&& pattern) {
  v.clear();
  std::size_t patternSize = pattern.size();
  if (patternSize == 0) { throw std::range_error("vectorFillPattern: cannot push empty pattern"); }
  if (patternSize > max) { throw std::range_error("vectorFillPattern: pattern too large"); }
  for (std::size_t i = 0; i <= (max - patternSize); i += patternSize) {
    vectorPushPattern(v,max,std::forward<PatternType>(pattern));
  }
}

void vectorFillPattern(std::vector<std::size_t>& v,std::size_t max,std::initializer_list<std::size_t> pattern) {
  v.clear();
  std::size_t patternSize = pattern.size();
  if (patternSize == 0) { throw std::range_error("vectorFillPattern: cannot fill with empty pattern"); }
  if (patternSize > max) { throw std::range_error("vectorFillPattern: pattern too large"); }
  for (std::size_t i = 0; i <= (max - patternSize); i += patternSize) {
    vectorPushPattern(v,max,pattern);
  }  
}

template <typename PathType>
void clearPathPop(PathType&& path) { while (!path.empty()) { path.pop_back(); } }

template <typename PathTypeA,typename PathTypeB>
bool pathsEqual(PathTypeA&& pathA,PathTypeB&& pathB) {
  if (pathA.size() != pathB.size()) { return false; }
  std::size_t size = pathA.size();
  for (std::size_t i=0; i < size; ++i) { if (pathA.at(i) != pathB.at(i)) { return false; }}
  return true;
}

void vectorTrimBack(std::vector<std::size_t>& v,std::size_t c) {
  if (c == 0) { return; }
  if (c > v.size()) { throw std::range_error("vectorTrimBack: trim size too large"); }
  v.resize(v.size() - c);
}

void vectorTrimFront(std::vector<std::size_t>& v,std::size_t c) {
  if (c == 0) { return; }
  if (c > v.size()) { throw std::range_error("vectorTrimFront: trim size too large"); }
  v.erase(v.begin(),v.begin() + c);
}

#endif
