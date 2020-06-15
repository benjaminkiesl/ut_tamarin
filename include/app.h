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

#ifndef UT_TAMARIN_APP_H_ 
#define UT_TAMARIN_APP_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace uttamarin {

class LemmaProcessor;
class TheoryPreprocessor;
class OutputWriter;

struct LemmaJob;
struct UtTamarinConfig;
struct TamarinOutput;

//enum class ProverResult;
enum class TamarinHeuristic;

class App {

 public:
  App(std::unique_ptr<LemmaProcessor> lemma_processor,
      std::unique_ptr<TheoryPreprocessor> theory_preprocessor,
      std::shared_ptr<UtTamarinConfig> config,
      std::shared_ptr<OutputWriter> output_writer);

  ~App();

  // Runs Tamarin on lemmas in the given spthy file. The actual choice of
  // lemmas depends on the configuration parameters. Returns true if Tamarin is
  // able to prove all lemmas.
  bool RunOnLemmas(const std::vector<LemmaJob>& lemma_jobs);

 private:
  void PrintHeader();

  void PrintLemmaResults(const LemmaJob& lemma_job,
                         const TamarinOutput& tamarin_output,
                         int lemma_number,
                         int number_of_lemmas);

  void PrintFooter(int true_lemmas, int false_lemmas,
                   int unknown_lemmas, int overall_duration);

  std::string ToOutputString(const TamarinHeuristic& heuristic);

  std::unique_ptr<LemmaProcessor> lemma_processor_;
  std::unique_ptr<TheoryPreprocessor> theory_preprocessor_;
  std::shared_ptr<UtTamarinConfig> config_;
  std::shared_ptr<OutputWriter> output_writer_;
};

} // namespace uttamarin

#endif
