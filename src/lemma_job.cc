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

#include "lemma_job.h"

#include <string>

using std::string;

namespace uttamarin {

LemmaJob::LemmaJob(string spthy_file_path,
                   string lemma_name,
                   const TamarinHeuristic& heuristic)
                   : spthy_file_path_(spthy_file_path),
                     lemma_name_(lemma_name),
                     heuristic_(heuristic) {

}

const string LemmaJob::GetSpthyFilePath() const {
  return spthy_file_path_;
}

void LemmaJob::SetSpthyFilePath(const string& spthy_file_path) {
  spthy_file_path_ = spthy_file_path;
}

const string LemmaJob::GetLemmaName() const {
  return lemma_name_;
}

void LemmaJob::SetLemmaName(const string& lemma_name) {
  lemma_name_ = lemma_name;
}

TamarinHeuristic LemmaJob::GetHeuristic() const {
  return heuristic_;
}

void LemmaJob::SetHeuristic(TamarinHeuristic heuristic) {
  heuristic_ = heuristic;
}

} // namespace uttamarin
