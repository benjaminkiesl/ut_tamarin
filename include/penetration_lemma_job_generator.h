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

#ifndef UT_TAMARIN_PENETRATION_LEMMA_JOB_GENERATOR_H_
#define UT_TAMARIN_PENETRATION_LEMMA_JOB_GENERATOR_H_

#include "lemma_job_generator.h"

#include <string>
#include <vector>

namespace uttamarin {

class PenetrationLemmaJobGenerator : public LemmaJobGenerator {

 public:
  PenetrationLemmaJobGenerator(const std::string& spthy_file_path,
                               const std::string& lemma_name);

  virtual ~PenetrationLemmaJobGenerator() = default;

 private:
  virtual std::vector<LemmaJob> DoGenerateLemmaJobs() override;

  std::string spthy_file_path_;
  std::string lemma_name_;
};

} // namespace uttamarin

#endif
