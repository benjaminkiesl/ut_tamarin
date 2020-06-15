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

#include "m4_theory_preprocessor.h"

#include <cstdio>

#include <fstream>
#include <memory>
#include <string>

#include "ut_tamarin_config.h"
#include "utility.h"

using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

namespace uttamarin {

const string kPreprocessedTempfilePath = "/tmp/preprocessed.spthy";
const string kM4TempfilePath = "/tmp/temp.m4";

M4TheoryPreprocessor::M4TheoryPreprocessor(
        std::shared_ptr<UtTamarinConfig> config) : config_(config) {
  std::remove(kM4TempfilePath.c_str());
}

M4TheoryPreprocessor::~M4TheoryPreprocessor() {

}

std::string M4TheoryPreprocessor::DoPreprocessAndReturnPathToResultingFile(
                  const std::string& spthy_file_path,
                  const std::string& lemma_name) {

  auto m4_commands = GetM4Commands(lemma_name);

  ofstream tempfile_m4{kM4TempfilePath};

  // Change quotes for M4, otherwise single quotes in spthy file lead to M4 bugs
  tempfile_m4 << "changequote(<!,!>)" << std::endl;
  tempfile_m4 << "changecom(<!/*!>, <!*/!>)" << std::endl;

  for(auto m4_command : m4_commands) {
    tempfile_m4 << m4_command << std::endl;
  }

  ifstream spthy_file{spthy_file_path};
  string spthy_file_line = "";
  while(std::getline(spthy_file, spthy_file_line))
    tempfile_m4 << spthy_file_line << std::endl;

  ExecuteShellCommand("m4 " + kM4TempfilePath + " > " +
                      kPreprocessedTempfilePath);

  std::remove(kM4TempfilePath.c_str());

  return kPreprocessedTempfilePath;
}

string M4TheoryPreprocessor::AddPrefixViaM4(const string& prefix,
                                            const string& original) {
  return "define(" + original + ", " + prefix + original + "($*))";
}

vector<string> M4TheoryPreprocessor::GetM4Commands(const string& lemma_name) {
  const string important_prefix = "F_";
  const string unimportant_prefix = "L_";

  vector<string> m4_commands;

  for(string fact : config_->GetGlobalAnnotations().important_facts) {
    if(!config_->FactIsAnnotatedLocally(fact, lemma_name)) {
      m4_commands.emplace_back(AddPrefixViaM4(important_prefix, fact));
    }
  }

  for(string fact : config_->GetGlobalAnnotations().unimportant_facts) {
    if(!config_->FactIsAnnotatedLocally(fact, lemma_name)) {
      m4_commands.emplace_back(AddPrefixViaM4(unimportant_prefix, fact));
    }
  }

  for(string fact : config_->GetLocalAnnotations(lemma_name).important_facts)
    m4_commands.emplace_back(AddPrefixViaM4(important_prefix, fact));

  for(string fact : config_->GetLocalAnnotations(lemma_name).unimportant_facts)
    m4_commands.emplace_back(AddPrefixViaM4(unimportant_prefix, fact));

  return m4_commands;
}

} // namespace uttamarin
