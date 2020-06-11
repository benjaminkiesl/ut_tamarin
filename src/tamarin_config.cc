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

#include "tamarin_config.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "third-party/nlohmann/json.hpp"

using std::string;
using std::vector;
using std::ifstream;
using json = nlohmann::json;

namespace ut_tamarin {

TamarinConfig ParseTamarinConfigFile(const string& config_file_path){
  TamarinConfig tamarin_config;
  json json_config;

  if(config_file_path != ""){
    ifstream config_file_stream{config_file_path};
    config_file_stream >> json_config;
  }

  tamarin_config.lemma_blacklist = json_config.count("lemma_blacklist") ?
    json_config["lemma_blacklist"].get<vector<string>>() : vector<string>{};

  tamarin_config.lemma_whitelist = json_config.count("lemma_whitelist") ?
    json_config["lemma_whitelist"].get<vector<string>>() : vector<string>{};

  if(json_config.count("global_annotations")){
    tamarin_config.global_annotations = 
      GetFactAnnotations(json_config["global_annotations"]);
  } 

  for(auto lemma_annotation : json_config["lemma_annotations"]){
    auto lemma_name = lemma_annotation["lemma_name"].get<string>();
    tamarin_config.lemma_annotations[lemma_name] = 
      GetFactAnnotations(lemma_annotation);
  }

  return tamarin_config;
}

FactAnnotations GetFactAnnotations(json json_annotation){
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

bool TamarinConfig::FactIsAnnotatedLocally(const string& fact, 
                                           const string& lemma_name) const {
  if(lemma_annotations.count(lemma_name) == 0) return false;
  auto local_fact_annotations = lemma_annotations.at(lemma_name);
  return local_fact_annotations.ContainFact(fact);
}

bool FactAnnotations::ContainFact(const std::string& fact) const { 
  return std::find(important_facts.begin(), important_facts.end(), fact) 
          != important_facts.end() || 
         std::find(unimportant_facts.begin(), unimportant_facts.end(), fact) 
          != unimportant_facts.end() || 
         std::find(neutral_facts.begin(), neutral_facts.end(), fact) 
          != neutral_facts.end(); 
} 

} // namespace ut_tamarin
