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

#ifndef UTTAMARIN_LEMMA_JOB_H_
#define UTTAMARIN_LEMMA_JOB_H_

#include <string>

namespace uttamarin {

enum class TamarinHeuristic { S, s, C, c, I, i, P, p, None };

class LemmaJob {
 public:
  LemmaJob(std::string spthy_file_path,
           std::string lemma_name,
           const TamarinHeuristic& heuristic=TamarinHeuristic::None);

  const std::string GetSpthyFilePath() const;
  void SetSpthyFilePath(const std::string& spthy_file_path);

  const std::string GetLemmaName() const;
  void SetLemmaName(const std::string& lemma_name);

  TamarinHeuristic GetHeuristic() const;
  void SetHeuristic(TamarinHeuristic heuristic);

 private:
  std::string spthy_file_path_;
  std::string lemma_name_;
  TamarinHeuristic heuristic_;
};

} // namespace uttamarin

#endif
