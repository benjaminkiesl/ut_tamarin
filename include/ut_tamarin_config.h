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

#ifndef UT_TAMARIN_CONFIG_H_ 
#define UT_TAMARIN_CONFIG_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "nlohmann/json.hpp"

namespace uttamarin {

struct CmdParameters;

struct FactAnnotations { 
  std::vector<std::string> important_facts; 
  std::vector<std::string> unimportant_facts; 
  std::vector<std::string> neutral_facts;
};

class UtTamarinConfig {

 public:
  UtTamarinConfig(const CmdParameters& cmd_parameters);

  // Returns true if the given fact 'fact' is annotated locally for the given
  // lemma 'lemma_name'. Here, a local annotation means that the scope of the 
  // annotation is only the lemma itself and not the whole Tamarin theory file.
  bool FactIsAnnotatedLocally(const std::string& fact, 
                              const std::string& lemma_name) const;

  std::string GetSpthyFilePath() const;
  std::string GetConfigFilePath() const;
  std::string GetOutputFilePath() const;
  std::string GetStartingLemma() const;
  std::string GetPenetrationLemma() const;
  std::string GetProofDirectory() const;
  int GetTimeout() const;
  bool IsAbortAfterFailure() const;
  const std::vector<std::string>& GetLemmaAllowList() const;
  const std::vector<std::string>& GetLemmaDenyList() const;
  const FactAnnotations& GetGlobalAnnotations() const;
  FactAnnotations GetLocalAnnotations(const std::string& lemma) const;

 private:
  void ParseJsonConfigFile(const std::string& config_file_path);
  static FactAnnotations GetFactAnnotations(nlohmann::json json_annotation);

  std::string spthy_file_path_;
  std::string config_file_path_;
  std::string output_file_path_;
  std::string starting_lemma_;
  std::string penetration_lemma_;
  std::string proof_directory_;
  int timeout_;
  bool abort_after_failure_;
  std::vector<std::string> lemma_allow_list_;
  std::vector<std::string> lemma_deny_list_;
  FactAnnotations global_annotations_;
  std::unordered_map<std::string, FactAnnotations> local_annotations_;
};

} // namespace uttamarin

#endif
