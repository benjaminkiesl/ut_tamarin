// MIT License
//
// Copyright (c) 2020 Benjamin Kiesl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "lemma_penetrator.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "lemma_processor.h"
#include "tamarin_interface.h"
#include "utility.h"

using std::ifstream;
using std::istream;
using std::string;
using std::unique_ptr;

namespace uttamarin {

class LemmaProcessor;

// A LemmaPenetrator tries proving a lemma with various heuristics
LemmaPenetrator::LemmaPenetrator(unique_ptr<LemmaProcessor> lemma_processor)
  : lemma_processor_(std::move(lemma_processor)),
    timeout_(60){
}

LemmaPenetrator::~LemmaPenetrator() = default;

ProverResult LemmaPenetrator::PenetrateLemma(const string &spthy_file_name,
                                             const string &lemma_name) {

}

void LemmaPenetrator::SetTimeout(int timeout_in_seconds) {
  timeout_ = timeout_in_seconds;
}

} // namespace uttamarin
