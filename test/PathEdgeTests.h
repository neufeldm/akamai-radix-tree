#ifndef AKAMAI_MAPPER_RADIX_TREE_TEST_PATH_EDGE_TESTS_H_
#define AKAMAI_MAPPER_RADIX_TREE_TEST_PATH_EDGE_TESTS_H_

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
 * \file PathEdgeTests.h
 * 
 * Templated googletest routines that can be applied to individual path/edge implementations.
 */

#include <stdint.h>
#include <vector>
#include <stdexcept>

#include "PathEdgeTestUtils.h"
#include "RandomUtils.h"

/**
 * \brief Perform and check a series of randomized operations on an edge/path object.
 */
template <typename PathType>
std::string pathRandomOps(std::size_t opCount) {
  // sequence of target sizes
  // sequence of ops to use to reach target (only if target is smaller - one option for increasing size exists)
  // get blocks of random steps as required - rotate through the available random seeds for each block
  enum {OP_POP=0,OP_TRIM_BACK=1,OP_TRIM_FRONT=2};
  RandomSeeds seeds;
  std::vector<std::size_t> refPath;
  PathType path{};

  std::size_t curStep{0};
  std::vector<std::size_t> steps{};
  std::vector<std::size_t> sizes = 
    generateUniformRandomSequence<std::size_t>(seeds.next(),opCount,0,PathType::MaxDepth);
  std::vector<uint32_t> ops = generateUniformRandomSequence<uint32_t>(seeds.next(),opCount,0,2);

  for (std::size_t curOp=0;curOp < opCount;++curOp) {
    std::size_t size = sizes.at(curOp);
    if (size > path.size()) {
      while (path.size() < size) {
        if (curStep >= steps.size()) { 
          curStep = 0;
          steps =
            generateUniformRandomSequence<std::size_t>(seeds.next(),4096,0,PathType::Radix - 1);
        }
        std::size_t step = steps.at(curStep);
        path.push_back(step);
        refPath.push_back(step);
        ++curStep;
      }
    } else if (size < path.size()) {
      if (ops[curOp] == OP_POP) {
        while (path.size() > size) { path.pop_back(); refPath.pop_back(); }
      } else if (ops[curOp] == OP_TRIM_BACK) {
        while (path.size() > size) { path.pop_back(); refPath.pop_back(); }
        //path.trim_back(path.size() - size);
        //vectorTrimBack(refPath,refPath.size() - size);
      }
      else if (ops[curOp] == OP_TRIM_FRONT) {
        path.trim_front(path.size() - size);
        vectorTrimFront(refPath,refPath.size() - size);
      }
      else { throw std::length_error("pathRandomOps: got invalid op"); }
    }
    std::size_t refPathSize = refPath.size();
    std::size_t pathSize = path.size();
    if (refPathSize != pathSize) {
      return std::to_string(curOp) + ": " +
             "refPathSize == " + std::to_string(refPathSize) + " != " +
             "pathSize == " + std::to_string(pathSize) +
             " expected == " + std::to_string(size);
    }
    if (!pathsEqual(refPath,path)) {
      return std::to_string(curOp) + " " + std::to_string(ops[curOp]) + ": refPath != path (" +
             pathToString(refPath) + ") (" + pathToString(path) + ")";
    }
  }
  return "OK";
}


#endif