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

#include "lemma_penetrator.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "lemma_processor.h"
#include "tamarin_interface.h"
#include "utility.h"

using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;
using std::vector;

namespace uttamarin {

class LemmaProcessor;

// A LemmaPenetrator tries proving a lemma with various heuristics
LemmaPenetrator::LemmaPenetrator(unique_ptr<LemmaProcessor> lemma_processor)
  : lemma_processor_(std::move(lemma_processor)),
    timeout_(60){
}

LemmaPenetrator::~LemmaPenetrator() = default;

void LemmaPenetrator::PenetrateLemma(const string &spthy_file_path,
                                     const string &lemma_name) {
  std::ostream& output_stream = cout; // TODO: make this a parameter
  auto lemmas_in_file = ReadLemmaNamesFromSpthyFile(spthy_file_path);

  string lemma = GetStringWithShortestEditDistance(lemmas_in_file,
                                                   lemma_name);

  cout << "Penetrating lemma '" << lemma << "' with a per-heuristic timeout of "
    << ToSecondsString(timeout_) << "." << std::endl << std::endl;

  vector<TamarinHeuristic> heuristics =
          {TamarinHeuristic::S, TamarinHeuristic::s,
           TamarinHeuristic::I, TamarinHeuristic::i,
           TamarinHeuristic::C, TamarinHeuristic::c,
           TamarinHeuristic::P, TamarinHeuristic::p};

  for(auto heuristic : heuristics) {
    lemma_processor_->SetHeuristic(heuristic);

    auto output = lemma_processor_->ProcessLemma(spthy_file_path, lemma);

    output_stream << to_string(output.result, &output_stream == &cout)
                  << " (" << ToSecondsString(output.duration) << ")"
                  << " heuristic=" << ToOutputString(heuristic) << endl;
  }
}

void LemmaPenetrator::SetTimeout(int timeout_in_seconds) {
  timeout_ = timeout_in_seconds;
}

std::string LemmaPenetrator::ToOutputString(TamarinHeuristic heuristic) {
  switch(heuristic){
    case TamarinHeuristic::S: return "S";
    case TamarinHeuristic::s: return "s";
    case TamarinHeuristic::I: return "I";
    case TamarinHeuristic::i: return "i";
    case TamarinHeuristic::C: return "c";
    case TamarinHeuristic::P: return "P";
    case TamarinHeuristic::p: return "p";
    default: return "unknown";
  }
}

} // namespace uttamarin
