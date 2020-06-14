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

#include "tamarin_interface.h"

namespace uttamarin {

enum class TamarinHeuristic {S, s, C, c, I, i, P, p, None};

enum class ProverResult { True, False, Unknown };

struct TamarinOutput {
  ProverResult result;
  int duration; // in seconds
};

class LemmaProcessor {
 public:
  virtual ~LemmaProcessor() = default;

  // Takes as input a  lemma name and then runs Tamarin on the given lemma.
  // Returns some output/statistics (like Tamarin's result and the execution
  // duration).
  TamarinOutput ProcessLemma(const std::string& spthy_file_name,
                             const std::string& lemma_name) {
    DoProcessLemma(spthy_file_name, lemma_name);
  }


  // Returns the heuristics used by the Tamarin prover
  TamarinHeuristic GetHeuristic() {
    DoGetHeuristic();
  }


  // Sets the heuristics to use by the Tamarin prover
  void SetHeuristic(TamarinHeuristic heuristic) {
    DoSetHeuristic(heuristic);
  }

 private:
  virtual TamarinOutput DoProcessLemma(const std::string& spthy_file_name,
                                       const std::string& lemma_name) = 0;

  virtual TamarinHeuristic DoGetHeuristic() = 0;
  virtual void DoSetHeuristic(TamarinHeuristic heuristic) = 0;
};

} // namespace uttamarin

#endif
