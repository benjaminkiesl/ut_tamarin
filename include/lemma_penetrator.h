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

#ifndef UT_TAMARIN_LEMMA_PENETRATOR_H_
#define UT_TAMARIN_LEMMA_PENETRATOR_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "lemma_processor.h"
#include "tamarin_interface.h"

namespace uttamarin {

class OutputWriter;

// A LemmaPenetrator tries proving a lemma with various heuristics
class LemmaPenetrator {

 public:
  LemmaPenetrator(std::unique_ptr<LemmaProcessor> lemma_processor,
                  std::shared_ptr<OutputWriter> output_writer);
  ~LemmaPenetrator();

  void PenetrateLemma(const std::string& spthy_file_path,
                      const std::string& lemma_name);

  void SetTimeout(int timeout_in_seconds);

 private:
  void PrintHeader(const std::string& lemma);

  void PrintLemmaResults(const std::string& lemma,
                         const TamarinOutput& tamarin_output,
                         TamarinHeuristic heuristic);

  std::string ToOutputString(TamarinHeuristic heuristic);

  std::unique_ptr<LemmaProcessor> lemma_processor_;
  std::shared_ptr<OutputWriter> output_writer_;
  std::vector<TamarinHeuristic> all_heuristics_;
  int timeout_;

};

} // namespace uttamarin

#endif
