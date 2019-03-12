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

#include <iostream>
#include <fstream>
#include <cstddef>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

#include "SimpleRadixTree.h"
#include "RadixTree.h"
#include "CursorTraversal.h"
#include "CursorOps.h"
#include "SimplePath.h"

// This example constructs a base 26 tree, one child for each letter
// in the English alphabet. Using some helper routines we store
// a small set of lower case words, traverse the trees, and then
// read prefixes from stdin and use the tree structure to display
// all of the words in our "dictionary" that have the provided prefix.

using namespace Akamai::Mapper::RadixTree;

// Typical round of typedefs to make the rest of the code more readable.
using AlphabetTree10 = SimpleRadixTree<bool,26,10,10>;
using Alphabet10Cursor = AlphabetTree10::CursorType;
using Alphabet10CursorRO = AlphabetTree10::CursorROType;
using AlphabetPath10 = SimplePath<26,10>;

// Helpers to convert word strings to/from our paths.
// The underlying path will throw an exception if we try to add
// an out-of-range value so we don't check for that here.
std::string alphabetPath10ToString(const AlphabetPath10& p) {
  std::string pathstr{};
  for (std::size_t i=0;i<p.size();++i) { pathstr += static_cast<char>(p.at(i) + 'a'); }
  return pathstr;
}
AlphabetPath10 stringToAlphabetPath10(const std::string& s) {
  AlphabetPath10 r{};
  std::size_t len = std::min(s.size(),static_cast<std::size_t>(10));
  for (std::size_t i=0;i<len;++i) { r.push_back(static_cast<std::size_t>(s.at(i) - 'a')); }
  return r;
}

int main(int /*argc*/,const char** /*argv*/) {
  // Our set of dictionary words.
  std::vector<AlphabetPath10> wordPaths{stringToAlphabetPath10("dog"),
                                        stringToAlphabetPath10("cat"),
                                        stringToAlphabetPath10("cartoon"),
                                        stringToAlphabetPath10("catalog"),
                                        stringToAlphabetPath10("cart"),
                                        stringToAlphabetPath10("aardvark"),
                                        stringToAlphabetPath10("ocelot"),
                                        stringToAlphabetPath10("sloth"),
                                        stringToAlphabetPath10("wombat"),
                                        stringToAlphabetPath10("dogged"),
                                        stringToAlphabetPath10("slothful"),
                                        stringToAlphabetPath10("carthorse"),
                                        stringToAlphabetPath10("dogsbody"),
                                        stringToAlphabetPath10("worker"),
                                        stringToAlphabetPath10("davenport"),
                                        stringToAlphabetPath10("chalkboard"),
                                        stringToAlphabetPath10("doghouse"),
                                        stringToAlphabetPath10("apple"),
                                        stringToAlphabetPath10("apricot")};

  std::cout << "=== BUILDING ALPHABET WORD TREE ===" << std::endl;
  // Add our words to the tree one by one.
  AlphabetTree10 alphabetTree{};
  for (std::size_t i=0;i<wordPaths.size();++i) {
    const AlphabetPath10& curPath = wordPaths.at(i);

    std::cout << std::to_string(i) << " " <<
              alphabetPath10ToString(curPath) <<
              std::endl;
    // We use "true" here for the value - if we used "false"
    // then a node would still be created but it wouldn't signal
    // the presence of a valid word.
    cursorAddValueAt(alphabetTree.cursor(),curPath,true);
  }

  // Go to each of our word paths, print whether or not they're considered valid.
  std::cout << std::endl;
  std::cout << "=== CHECKING ALPHABET TREE ===" << std::endl;
  for (std::size_t i = 0;i < wordPaths.size(); ++i) {
    std::cout <<
      std::to_string(i) << " " <<
      alphabetPath10ToString(wordPaths[i]) << " " <<
      (*(cursorGotoValue(alphabetTree.cursorRO(),wordPaths[i]).getPtrRO()) ? "(true)" : "(false)") <<
      std::endl;
  }

  std::cout << std::endl;
  std::cout << "===  TRAVERSING ALPHABET TREE ===" << std::endl;
  // We're using the bool tree, so anywhere there's a value the path forms a valid word.
  // We could also put this lambda inline with the call to the traversal function, but 
  // that would be more copy/paste.
  auto printAlphabetPath =
    [](const Alphabet10CursorRO& ac){ std::cout << alphabetPath10ToString(ac.getPath()) << std::endl; };
  std::cout << "----PRE ORDER----" << std::endl;
  preOrderWalk(printAlphabetPath,alphabetTree.cursorRO());
  std::cout << "----IN ORDER----" << std::endl;
  inOrderWalk(printAlphabetPath,alphabetTree.cursorRO());
  std::cout << "----POST ORDER----" << std::endl;
  postOrderWalk(printAlphabetPath,alphabetTree.cursorRO());

  // Now poll the user for prefixes to check in our dictionary.
  std::cout << std::endl;
  std::string wordPrefix{};
  while ((std::cout << "Please type a word prefix and return (! to quit): ") && (std::cin >> wordPrefix)) {
    if (wordPrefix == "!") { break; }
    if (wordPrefix.size() > 10) { wordPrefix.resize(10); }
    for (char& c : wordPrefix) { c = std::tolower((std::isalpha(c) > 0) ? c : 'x'); }
    std::cout << "All words starting with '" << wordPrefix << "' in our dictionary: " << std::endl;
    std::cout << "--------------" << std::endl;
    // Walk down to the prefix in the tree, then traverse all of the valid
    // words preorder that exist below it.
    AlphabetPath10 prefixPath = stringToAlphabetPath10(wordPrefix);
    Alphabet10CursorRO prefixCursor = alphabetTree.cursorRO();
    cursorGoto(prefixCursor,prefixPath);
    preOrderWalk(printAlphabetPath,prefixCursor);
    std::cout << "--------------" << std::endl;
  }

  return 0;
}