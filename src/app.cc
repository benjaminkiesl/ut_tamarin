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

#include "app.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "lemma_job.h"
#include "lemma_processor.h"
#include "output_writer.h"
#include "theory_preprocessor.h"
#include "ut_tamarin_config.h"
#include "utility.h"

using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

namespace uttamarin {

App::App(unique_ptr<LemmaProcessor> lemma_processor,
         unique_ptr<TheoryPreprocessor> theory_preprocessor,
         shared_ptr<UtTamarinConfig> config,
         shared_ptr<OutputWriter> output_writer) :
  lemma_processor_(std::move(lemma_processor)),
  theory_preprocessor_(std::move(theory_preprocessor)),
  config_(config),
  output_writer_(output_writer) {

}

App::~App() = default;

bool App::RunOnLemmas(const vector<LemmaJob>& lemma_jobs) {
  PrintHeader();

  bool success = true;
  unordered_map<ProverResult, int> count_of;
  int overall_duration = 0;

  for(int i=0;i < lemma_jobs.size();i++) {
    auto lemma_job = lemma_jobs[i];
    auto preprocessed_spthy_file =
            theory_preprocessor_->PreprocessAndReturnPathToResultingFile(
                    lemma_job.GetSpthyFilePath(), lemma_job.GetLemmaName());

    lemma_job.SetSpthyFilePath(preprocessed_spthy_file);
    auto output = lemma_processor_->ProcessLemma(lemma_job);

    PrintLemmaResults(lemma_job, output, i+1, lemma_jobs.size());

    overall_duration += output.duration;
    count_of[output.result]++;
    std::remove(preprocessed_spthy_file.c_str());
    if(output.result != ProverResult::True) {
      success = false;
      if(config_->IsAbortAfterFailure()) break;
    }
  }

  PrintFooter(count_of[ProverResult::True], count_of[ProverResult::False],
              count_of[ProverResult::Unknown], overall_duration);

  return success;
}

void App::PrintHeader() {
  auto file_name = config_->GetSpthyFilePath();
  if(file_name.find('/') != string::npos) {
    file_name = file_name.substr(file_name.find_last_of('/') + 1);
  }

  *output_writer_ << "Tamarin Tests for file '" << file_name << "':\n"
    << "Timeout: " << (config_->GetTimeout() <= 0 ?
    "no timeout" : ToSecondsString(config_->GetTimeout()))
    << " per lemma\n";

  output_writer_->Endl();
}

void App::PrintLemmaResults(const LemmaJob& lemma_job,
                            const TamarinOutput& tamarin_output,
                            int lemma_number,
                            int number_of_lemmas) {
  *output_writer_ << lemma_job.GetLemmaName() << " ";
  if(tamarin_output.result == ProverResult::True) {
    output_writer_->WriteColorized("verified", TextColor::Green);
  } else if(tamarin_output.result == ProverResult::False) {
    output_writer_->WriteColorized("false", TextColor::Red);
  } else {
    output_writer_->WriteColorized("unverified", TextColor::Yellow);
  }
  *output_writer_ << " (" << ToSecondsString(tamarin_output.duration) << ")";
  if(lemma_job.GetHeuristic() != TamarinHeuristic::None) {
    *output_writer_ << " heuristic="
                    << ToOutputString(lemma_job.GetHeuristic());
  }
  *output_writer_ << " (" << lemma_number << "/" << number_of_lemmas << ")";
  output_writer_->Endl();
}

void App::PrintFooter(int true_lemmas, int false_lemmas,
                      int unknown_lemmas, int overall_duration) {
  *output_writer_ << "\n"
    << "Summary: " << "\n"
    << "verified: " << true_lemmas
    << ", false: " << false_lemmas
    << ", timeout: " << unknown_lemmas
    << "\n"
    << "Overall duration: " << ToSecondsString(overall_duration);
  output_writer_->Endl();
}

std::string App::ToOutputString(const TamarinHeuristic& heuristic) {
  switch(heuristic){
    case TamarinHeuristic::S: return "S";
    case TamarinHeuristic::s: return "s";
    case TamarinHeuristic::I: return "I";
    case TamarinHeuristic::i: return "i";
    case TamarinHeuristic::C: return "C";
    case TamarinHeuristic::c: return "c";
    case TamarinHeuristic::P: return "P";
    case TamarinHeuristic::p: return "p";
    default: return "unknown";
  }
}

} // namespace uttamarin
