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

#include "verbose_lemma_processor.h"

#include <iostream>
#include <memory>
#include <utility>

#include "tamarin_interface.h"

using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;

namespace uttamarin {

VerboseLemmaProcessor::VerboseLemmaProcessor(
        unique_ptr<LemmaProcessor> decoratee) :
                                       decoratee_(std::move(decoratee)) {
}

TamarinOutput VerboseLemmaProcessor::DoProcessLemma(const string& lemma_name) {
  cout << "Decorator called for lemma " << lemma_name << endl;
  return decoratee_->ProcessLemma(lemma_name);
}

} // namespace uttamarin
