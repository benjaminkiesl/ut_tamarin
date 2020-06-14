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

#ifndef UT_TAMARIN_VERBOSE_LEMMA_PROCESSOR_H_
#define UT_TAMARIN_VERBOSE_LEMMA_PROCESSOR_H_

#include "lemma_processor.h"

#include <future>
#include <istream>
#include <memory>
#include <string>

namespace uttamarin {

class VerboseLemmaProcessor : public LemmaProcessor {
 public:
  VerboseLemmaProcessor(std::unique_ptr<LemmaProcessor> decoratee);
  virtual ~VerboseLemmaProcessor() = default;

 private:

  // Takes as input a tamarin file path and a lemma name and then runs Tamarin
  // on the given lemma. Returns some output/statistics (like Tamarin's result
  // and the execution duration). Additionally prints its statistics to cout.
  virtual TamarinOutput DoProcessLemma(const LemmaJob& lemma_job) override;

  std::unique_ptr<LemmaProcessor> decoratee_;
};

} // namespace uttamarin

#endif
