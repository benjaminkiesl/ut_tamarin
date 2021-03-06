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

#ifndef UT_TAMARIN_LEMMA_PROCESSOR_H_
#define UT_TAMARIN_LEMMA_PROCESSOR_H_

#include <string>

namespace uttamarin {

class LemmaJob;

enum class ProverResult { True, False, Unknown };

struct TamarinOutput {
  ProverResult result;
  int duration; // in seconds
};

class LemmaProcessor {
 public:
  virtual ~LemmaProcessor() = default;

  // Takes as input a  lemma job and then runs Tamarin with the information
  // given by the lemma job. Returns some statistics (like Tamarin's result
  // and the execution duration).
  TamarinOutput ProcessLemma(const LemmaJob& lemma_job) {
    DoProcessLemma(lemma_job);
  }

 private:
  virtual TamarinOutput DoProcessLemma(const LemmaJob& lemma_job) = 0;

};

} // namespace uttamarin

#endif
