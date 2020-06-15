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

#include "ut_tamarin_config.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "cmd_parameters.h"

using std::string;
using std::vector;
using std::ifstream;
using json = nlohmann::json;

namespace uttamarin {

UtTamarinConfig::UtTamarinConfig(const CmdParameters &cmd_parameters)
  : spthy_file_path_(cmd_parameters.spthy_file_path),
    config_file_path_(cmd_parameters.config_file_path),
    output_file_path_(cmd_parameters.output_file_path),
    starting_lemma_(cmd_parameters.starting_lemma),
    penetration_lemma_(cmd_parameters.penetration_lemma),
    proof_directory_(cmd_parameters.proof_directory),
    timeout_(cmd_parameters.timeout),
    abort_after_failure_(cmd_parameters.abort_after_failure)
    {
  ParseJsonConfigFile(cmd_parameters.config_file_path);
}

bool UtTamarinConfig::FactIsAnnotatedLocally(const string& fact,
                                             const string& lemma_name) const {
  if(local_annotations_.count(lemma_name) == 0) return false;
  auto local_annotations = local_annotations_.at(lemma_name);
  return std::find(local_annotations.important_facts.begin(),
                   local_annotations.important_facts.end(), fact)
         != local_annotations.important_facts.end() ||
         std::find(local_annotations.unimportant_facts.begin(),
                   local_annotations.unimportant_facts.end(), fact)
         != local_annotations.unimportant_facts.end() ||
         std::find(local_annotations.neutral_facts.begin(),
                   local_annotations.neutral_facts.end(), fact)
         != local_annotations.neutral_facts.end();
}

void UtTamarinConfig::ParseJsonConfigFile(const std::string &config_file_path) {
  json json_config;

  if(config_file_path != ""){
    ifstream config_file_stream{config_file_path};
    config_file_stream >> json_config;
  }

  lemma_deny_list_ = json_config.count("lemma_deny_list") ?
    json_config["lemma_deny_list"].get<vector<string>>() : vector<string>{};

  lemma_allow_list_ = json_config.count("lemma_allow_list") ?
    json_config["lemma_allow_list"].get<vector<string>>() : vector<string>{};

  if(json_config.count("global_annotations")){
    global_annotations_ = GetFactAnnotations(json_config["global_annotations"]);
  } 

  for(auto lemma_annotation : json_config["lemma_annotations"]){
    auto lemma_name = lemma_annotation["lemma_name"].get<string>();
    local_annotations_[lemma_name] = GetFactAnnotations(lemma_annotation);
  }
}

FactAnnotations UtTamarinConfig::GetFactAnnotations(json json_annotation){
  FactAnnotations fact_annotations;
  if(json_annotation.count("important_facts")){
    fact_annotations.important_facts =
      json_annotation["important_facts"].get<vector<string>>();
  }
  if(json_annotation.count("unimportant_facts")){
    fact_annotations.unimportant_facts =
      json_annotation["unimportant_facts"].get<vector<string>>();
  }
  if(json_annotation.count("neutral_facts")){
    fact_annotations.neutral_facts =
      json_annotation["neutral_facts"].get<vector<string>>();
  }
  return fact_annotations;
}

string UtTamarinConfig::GetSpthyFilePath() const {
  return spthy_file_path_;
}

string UtTamarinConfig::GetConfigFilePath() const {
  return config_file_path_;
}

string UtTamarinConfig::GetOutputFilePath() const {
  return output_file_path_;
}

string UtTamarinConfig::GetStartingLemma() const {
  return starting_lemma_;
}

string UtTamarinConfig::GetPenetrationLemma() const {
  return penetration_lemma_;
}

string UtTamarinConfig::GetProofDirectory() const {
  return proof_directory_;
}

int UtTamarinConfig::GetTimeout() const {
  return timeout_;
}

bool UtTamarinConfig::IsAbortAfterFailure() const {
  return abort_after_failure_;
}

const vector<std::string>& UtTamarinConfig::GetLemmaAllowList() const {
  return lemma_allow_list_;
}

const vector<std::string>& UtTamarinConfig::GetLemmaDenyList() const {
  return lemma_deny_list_;
}

const FactAnnotations& UtTamarinConfig::GetGlobalAnnotations() const {
  return global_annotations_;
}

FactAnnotations UtTamarinConfig::GetLocalAnnotations(const string& lemma_name)
const {
  if(local_annotations_.count(lemma_name) > 0) {
    return local_annotations_.at(lemma_name);
  }
  return FactAnnotations{vector<string>{}, vector<string>{}, vector<string>{}};
}

} // namespace uttamarin
