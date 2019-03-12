#ifndef AKAMAI_MAPPER_RADIX_TREE_TEST_CURSOR_FUNCS_H_
#define AKAMAI_MAPPER_RADIX_TREE_TEST_CURSOR_FUNCS_H_

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

template <typename CursorType>
void cursorGotoRoot(CursorType&& c) { while (c.canGoParent()) { c.goParent(); } }

template <typename CursorType,typename PathType>
void cursorGoto(CursorType&& c,PathType&& path,bool gotoRootFirst = true) {
  if (gotoRootFirst) { cursorGotoRoot(std::forward<CursorType>(c)); }
  for (unsigned i = 0; i<path.size(); ++i) { c.goChild(path[i]); }
}

#endif