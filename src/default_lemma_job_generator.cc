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

#include "default_lemma_job_generator.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "lemma_name_reader.h"
#include "utility.h"

using std::shared_ptr;
using std::string;
using std::vector;

namespace uttamarin {

DefaultLemmaJobGenerator::DefaultLemmaJobGenerator(
                             const string& spthy_file_path,
                             const string& starting_lemma,
                             shared_ptr<UtTamarinConfig> config) :
                              spthy_file_path_(spthy_file_path),
                              config_(config) {
}

vector<LemmaJob> DefaultLemmaJobGenerator::DoGenerateLemmaJobs() {
  vector<LemmaJob> lemma_jobs;
  for(auto lemma_name : GetNamesOfLemmasToVerify()) {
    lemma_jobs.emplace_back(LemmaJob(spthy_file_path_, lemma_name));
  }
  return lemma_jobs;
}

vector<string> DefaultLemmaJobGenerator::GetNamesOfLemmasToVerify() {
  auto lemmas = ReadLemmaNamesFromSpthyFile(spthy_file_path_);
  if(!config_->GetLemmaAllowList().empty()) {
    lemmas = GetLemmasInAllowList(lemmas, config_->GetLemmaAllowList());
  }
  if(!config_->GetLemmaDenyList().empty()) {
    lemmas = RemoveLemmasInDenyList(lemmas, config_->GetLemmaDenyList());
  }
  if(starting_lemma_ != "") {
    lemmas = RemoveLemmasBeforeStart(lemmas, starting_lemma_);
  }
  return lemmas;
}


vector<string> DefaultLemmaJobGenerator::GetLemmasInAllowList(
                                            const vector<string>& all_lemmas,
                                            const vector<string>& allow_list) {
  for(auto lemma_name : allow_list) {
    if(std::find(all_lemmas.begin(), all_lemmas.end(), lemma_name)
       == all_lemmas.end()) {
      std::cerr << "Warning: lemma '" << lemma_name << "' is not declared in "
                << "the Tamarin theory." << std::endl;
    }
  }
  auto filtered_lemmas = all_lemmas;
  filtered_lemmas.erase(
    std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
                   [&allow_list](const string& lemma_name) {
                     return std::find(allow_list.begin(),
                                    allow_list.end(), lemma_name)
                            == allow_list.end();
                   }), filtered_lemmas.end());
  return filtered_lemmas;
}

vector<string> DefaultLemmaJobGenerator::RemoveLemmasInDenyList(
                                            const vector<string>& all_lemmas,
                                            const vector<string>& deny_list) {
  auto filtered_lemmas = all_lemmas;
  filtered_lemmas.erase(
    std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
                   [&](const string& lemma_name) {
                     return std::find(deny_list.begin(),
                                      deny_list.end(),
                                      lemma_name) != deny_list.end();
                   }),
    filtered_lemmas.end());
  return filtered_lemmas;
}

vector<string> DefaultLemmaJobGenerator::RemoveLemmasBeforeStart(
        const vector<string>& all_lemmas,
        const string& starting_lemma) {
  auto it_start = std::find(all_lemmas.begin(), all_lemmas.end(),
                            GetStringWithShortestEditDistance(all_lemmas,
                                                              starting_lemma));

  vector<string> filtered_lemmas;
  for(auto it = it_start;it != all_lemmas.end();++it) {
    filtered_lemmas.emplace_back(*it);
  }
  return filtered_lemmas;
}

} // namespace uttamarin
