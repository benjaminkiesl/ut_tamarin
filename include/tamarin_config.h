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

#include <string>
#include <unordered_map>
#include <vector>

#include "third-party/nlohmann/json.hpp"

namespace ut_tamarin {

struct FactAnnotations { 
  std::vector<std::string> important_facts; 
  std::vector<std::string> unimportant_facts; 
  std::vector<std::string> neutral_facts;
  
  bool ContainFact(const std::string& fact) const;
};

struct TamarinConfig { 
  std::vector<std::string> lemma_blacklist; 
  std::vector<std::string> lemma_whitelist; 
  FactAnnotations global_annotations; 
  std::unordered_map<std::string, FactAnnotations> lemma_annotations; 
  
  // Returns true if the given fact 'fact' is annotaed locally for the given
  // lemma 'lemma_name'. Here, a local annotation means that the scope of the 
  // annotation is only the lemma itself and not the whole Tamarin theory file.
  bool FactIsAnnotatedLocally(const std::string& fact, 
                              const std::string& lemma_name) const;
};

// Takes as input the path of the Tamarin config file and returns an object
// representing the configuration options deifned in the config file.
TamarinConfig ParseTamarinConfigFile(const std::string& config_file_path);

// Gets the fact annotations from a JSON annotation.
FactAnnotations GetFactAnnotations(nlohmann::json json_annotation);

} // namespace ut_tamarin

#endif
