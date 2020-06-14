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

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "lemma_processor.h"
#include "output_writer.h"
#include "tamarin_interface.h"
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

bool App::RunTamarinOnLemmas(const CmdParameters& parameters) {
  PrintHeader(parameters);

  auto lemma_names = GetNamesOfLemmasToVerify(parameters.spthy_file_path,
                                              config_->lemma_allow_list,
                                              config_->lemma_deny_list,
                                              parameters.starting_lemma);

  bool success = true;

  unordered_map<ProverResult, int> count_of;
  int overall_duration = 0;
  for(int i=0;i < lemma_names.size();i++) {
    auto preprocessed_spthy_file =
            theory_preprocessor_->PreprocessAndReturnPathToResultingFile(
                    parameters.spthy_file_path, lemma_names[i]);

    auto output = lemma_processor_->ProcessLemma(preprocessed_spthy_file,
                                                lemma_names[i]);
    PrintLemmaResults(lemma_names[i], output, lemma_processor_->GetHeuristic());

    overall_duration += output.duration;
    count_of[output.result]++;
    std::remove(preprocessed_spthy_file.c_str());
    if(output.result != ProverResult::True) {
      success = false;
      if(parameters.abort_after_failure) break;
    }
  }

  PrintFooter(count_of[ProverResult::True], count_of[ProverResult::False],
              count_of[ProverResult::Unknown], overall_duration);

  return success;
}

void App::PrintHeader(const CmdParameters& parameters) {
  auto file_name = parameters.spthy_file_path;
  if(file_name.find('/') != string::npos) {
    file_name = file_name.substr(file_name.find_last_of('/') + 1);
  }

  *output_writer_ << "Tamarin Tests for file '" << file_name << "':\n"
    << "Timeout: " << (parameters.timeout <= 0 ?
    "no timeout" : ToSecondsString(parameters.timeout))
    << " per lemma\n";

  output_writer_->Endl();
}

void App::PrintLemmaResults(const string& lemma,
                            const TamarinOutput& tamarin_output,
                            const TamarinHeuristic& heuristic) {
  *output_writer_ << lemma << " ";
  if(tamarin_output.result == ProverResult::True) {
    output_writer_->WriteColorized("verified", TextColor::Green);
  } else if(tamarin_output.result == ProverResult::False) {
    output_writer_->WriteColorized("false", TextColor::Red);
  } else {
    output_writer_->WriteColorized("unverified", TextColor::Yellow);
  }
  *output_writer_ << " (" << ToSecondsString(tamarin_output.duration) << ")";
  if(heuristic != TamarinHeuristic::None) {
    *output_writer_ << " heuristic=" << ToOutputString(heuristic);
  }
  output_writer_->Endl();
}

void App::PrintFooter(int true_lemmas, int false_lemmas,
                      int unknown_lemmas, int overall_duration) {
  *output_writer_ << "\n"
    << "Summary: " << "\n"
    << to_string(ProverResult::True) << ": " << true_lemmas
    << ", " << to_string(ProverResult::False) << ": " << false_lemmas
    << ", " << to_string(ProverResult::Unknown) << ": " << unknown_lemmas
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
    case TamarinHeuristic::C: return "c";
    case TamarinHeuristic::P: return "P";
    case TamarinHeuristic::p: return "p";
    default: return "unknown";
  }
}

vector<string> App::GetLemmasInAllowList(const vector<string>& all_lemmas,
                                         const vector<string>& allow_list) {
  for(auto lemma_name : allow_list) {
    if(std::find(all_lemmas.begin(), all_lemmas.end(), lemma_name)
       == all_lemmas.end()) {
      cerr << "Warning: lemma '" << lemma_name << "' is not declared in " <<
        "the Tamarin theory." << endl;
    }
  }
  auto filtered_lemmas = all_lemmas;
  filtered_lemmas.erase(
      std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
      [&allow_list](const string& lemma_name) {
        return std::find(allow_list.begin(), allow_list.end(), lemma_name)
               == allow_list.end();
      }), filtered_lemmas.end());
  return filtered_lemmas;
}

vector<string> App::RemoveLemmasInDenyList(const vector<string>& all_lemmas,
                                           const vector<string>& deny_list) {
  auto filtered_lemmas = all_lemmas;
  filtered_lemmas.erase(
      std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
      [&](const string& lemma_name) { 
        return std::find(deny_list.begin(), deny_list.end(), lemma_name)
               != deny_list.end();
      }),
      filtered_lemmas.end());
  return filtered_lemmas;
}

vector<string> App::RemoveLemmasBeforeStart(const vector<string>& all_lemmas, 
                                            const string& starting_lemma) {
  auto it_start = std::find(all_lemmas.begin(), all_lemmas.end(),
          GetStringWithShortestEditDistance(all_lemmas, starting_lemma));

  vector<string> filtered_lemmas;
  if(it_start != all_lemmas.end()) {
    for(auto it = it_start;it != all_lemmas.end();++it) {
      filtered_lemmas.emplace_back(*it);
    }
  } else {
    cerr << "Warning: No lemma whose name starts with '" << starting_lemma << 
      "' declared in the Tamarin theory." << endl;
  }
  return filtered_lemmas;
}

vector<string> App::GetNamesOfLemmasToVerify(const string& spthy_file_path,
                                             const vector<string>& allow_list,
                                             const vector<string>& deny_list,
                                             const string& starting_lemma) {
  auto lemmas = ReadLemmaNamesFromSpthyFile(spthy_file_path);
  if(!allow_list.empty()) {
    lemmas = GetLemmasInAllowList(lemmas, allow_list);
  }
  if(!deny_list.empty()) {
    lemmas = RemoveLemmasInDenyList(lemmas, deny_list);
  }
  if(starting_lemma != "") {
    lemmas = RemoveLemmasBeforeStart(lemmas, starting_lemma);
  }
  return lemmas;
}

} // namespace uttamarin
