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

#include "penetration_lemma_job_generator.h"

#include <string>
#include <vector>

#include "lemma_name_reader.h"
#include "utility.h"

using std::string;
using std::vector;

namespace uttamarin {

PenetrationLemmaJobGenerator::PenetrationLemmaJobGenerator(
                             const string& spthy_file_path,
                             const string& lemma_name) :
                              spthy_file_path_(spthy_file_path),
                              lemma_name_(lemma_name){

}

vector<LemmaJob> PenetrationLemmaJobGenerator::DoGenerateLemmaJobs() {
  auto lemmas_in_file = ReadLemmaNamesFromSpthyFile(spthy_file_path_);

  string lemma_name = GetStringWithShortestEditDistance(lemmas_in_file,
                                                        lemma_name_);
  vector<TamarinHeuristic> all_heuristics =
                    {TamarinHeuristic::S, TamarinHeuristic::s,
                     TamarinHeuristic::I, TamarinHeuristic::i,
                     TamarinHeuristic::C, TamarinHeuristic::c,
                     TamarinHeuristic::P, TamarinHeuristic::p};

  vector<LemmaJob> lemma_jobs;

  for(auto heuristic : all_heuristics) {
    lemma_jobs.push_back(LemmaJob(spthy_file_path_, lemma_name, heuristic));
  }
  return lemma_jobs;
}

} // namespace uttamarin
