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

#ifndef UT_TAMARIN_M4_THEORY_PREPROCESSOR_H_
#define UT_TAMARIN_M4_THEORY_PREPROCESSOR_H_

#include "theory_preprocessor.h"

#include <memory>
#include <string>
#include <vector>

namespace uttamarin {

struct UtTamarinConfig;

class M4TheoryPreprocessor : public TheoryPreprocessor {

 public:
  M4TheoryPreprocessor(std::shared_ptr<UtTamarinConfig> config);
  virtual ~M4TheoryPreprocessor();

 private:

  // Preprocesses the given Tamarin theory file with M4, applying prefixes
  // to different fact names according to the specification defined in the
  // config file. Returns the path to the resulting file.
  virtual std::string DoPreprocessAndReturnPathToResultingFile(
                  const std::string& spthy_file_path,
                  const std::string& lemma_name) override;

  // Takes a string 'prefix' and a string 'original' and returns a command that
  // tells the tool M4 to prepend 'prefix' to 'original'.
  std::string AddPrefixViaM4(const std::string& prefix,
                             const std::string& original);

  // Takes as input the name of a lemma and a tamarin configuration and returns
  // a list of commands for the tool M4. These commands tell M4 to rename
  // certain fact symbols within the Tamarin theory file in order to apply
  // custom heuristics.
  std::vector<std::string> GetM4Commands(const std::string& lemma_name);

  std::shared_ptr<UtTamarinConfig> config_;
};

} // namespace uttamarin

#endif
