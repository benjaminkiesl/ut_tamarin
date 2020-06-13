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
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "lemma_processor.h"
#include "ut_tamarin_config.h"
#include "tamarin_interface.h"
#include "utility.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;

namespace uttamarin {

const string kTempfilePath = "/tmp/uttamarintemp.ut";
const string kPreprocessedTempfilePath = "/tmp/preprocessed.spthy";
const string kM4TempfilePath = "/tmp/temp.m4";

App::App(std::unique_ptr<LemmaProcessor> lemma_processor,
         std::shared_ptr<UtTamarinConfig> config) :
  lemma_processor_(std::move(lemma_processor)),
  config_(config) {

}

App::~App() {
  std::remove(kTempfilePath.c_str());
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

void App::PrintHeader(ostream& output_stream, const CmdParameters& parameters) {
  auto file_name = parameters.spthy_file_path;
  if(file_name.find('/') != string::npos)
    file_name = file_name.substr(file_name.find_last_of('/') + 1);

  output_stream << "Tamarin Tests for file '" << file_name << "':" << endl;
  output_stream << "Timeout: " << (parameters.timeout <= 0 ? "no timeout" : 
                   (std::to_string(parameters.timeout) + " second" + 
                   (parameters.timeout > 1 ? "s" : ""))) << " per lemma" 
                    << endl << endl;
}

void App::PrintFooter(ostream& output_stream,
                      int true_lemmas, int false_lemmas,
                      int unknown_lemmas, int overall_duration) {

  output_stream << endl << "Summary: " << endl;
  output_stream << to_string(ProverResult::True) << ": " <<
                true_lemmas << ", " << to_string(ProverResult::False) << ": "
                << false_lemmas << ", " << to_string(ProverResult::Unknown)
                << ": " << unknown_lemmas << endl;

  output_stream << "Overall duration: " << ToSecondsString(overall_duration)
                << endl;
}

string App::AddPrefixViaM4(const string& prefix, const string& original) {
  return "define(" + original + ", " + prefix + original + "($*))";
}

vector<string> App::GetM4Commands(const string& lemma_name) {
  const string important_prefix = "F_";
  const string unimportant_prefix = "L_";

  vector<string> m4_commands;

  for(string fact : config_->global_annotations.important_facts) {
    if(!config_->FactIsAnnotatedLocally(fact, lemma_name)) {
      m4_commands.emplace_back(AddPrefixViaM4(important_prefix, fact));
    }
  }

  for(string fact : config_->global_annotations.unimportant_facts) {
    if(!config_->FactIsAnnotatedLocally(fact, lemma_name)) {
      m4_commands.emplace_back(AddPrefixViaM4(unimportant_prefix, fact));
    }
  }

  if(config_->lemma_annotations.count(lemma_name)) {
    for(string fact : config_->lemma_annotations.at(lemma_name).important_facts)
      m4_commands.emplace_back(AddPrefixViaM4(important_prefix, fact));

    for(string fact : config_->lemma_annotations.at(lemma_name).unimportant_facts)
      m4_commands.emplace_back(AddPrefixViaM4(unimportant_prefix, fact));
  }

  return m4_commands;
}

string App::ApplyCustomHeuristics(const string& spthy_file_path, 
                                  const string& lemma_name) {
  auto m4_commands = GetM4Commands(lemma_name);

  ofstream tempfile_m4{kM4TempfilePath};

  // Change quotes for M4, otherwise single quotes in spthy file lead to M4 bugs
  tempfile_m4 << "changequote(<!,!>)" << endl;
  tempfile_m4 << "changecom(<!/*!>, <!*/!>)" << endl;

  for(auto m4_command : m4_commands) {
    tempfile_m4 << m4_command << endl;
  }

  ifstream spthy_file{spthy_file_path};
  string spthy_file_line = "";
  while(std::getline(spthy_file, spthy_file_line)) 
    tempfile_m4 << spthy_file_line << endl;

  ExecuteShellCommand("m4 " + kM4TempfilePath + " > " + 
                      kPreprocessedTempfilePath);

  return kPreprocessedTempfilePath;
}

bool App::RunTamarinOnLemmas(const CmdParameters& parameters, 
                             ostream& output_stream) {
  PrintHeader(output_stream, parameters);

  auto lemma_names = GetNamesOfLemmasToVerify(parameters.spthy_file_path,
                                              config_->lemma_allow_list,
                                              config_->lemma_deny_list,
                                              parameters.starting_lemma);

  bool success = true;

  unordered_map<ProverResult, int> count_of;
  int overall_duration = 0;
  for(int i=0;i < lemma_names.size();i++) {
    auto preprocessed_spthy_file =
            ApplyCustomHeuristics(parameters.spthy_file_path, lemma_names[i]);

    auto stats = lemma_processor_->ProcessLemma(preprocessed_spthy_file,
                                                lemma_names[i]);
    output_stream << "\r" << lemma_names[i] << " (" << (i+1) << "/" <<
                lemma_names.size() << ") " <<
                to_string(stats, &output_stream == &cout) << endl;
    overall_duration += stats.duration;
    count_of[stats.result]++;
    std::remove(preprocessed_spthy_file.c_str());
    if(stats.result != ProverResult::True) {
      success = false;
      if(parameters.abort_after_failure) break;
    }
  }

  PrintFooter(output_stream, count_of[ProverResult::True],
              count_of[ProverResult::False], count_of[ProverResult::Unknown],
              overall_duration);

  return success;
}

} // namespace uttamarin
