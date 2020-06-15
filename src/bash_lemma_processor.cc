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

#include "bash_lemma_processor.h"

#include <algorithm>
#include <fstream>
#include <string>

#include "lemma_job.h"
#include "utility.h"

using std::ifstream;
using std::istream;
using std::string;

namespace uttamarin {

const string kTempPath = "/tmp/uttamarintemp.ut";

BashLemmaProcessor::BashLemmaProcessor(const string& proof_directory,
                                       const int timeout) :
                                       proof_directory_(proof_directory),
                                       timeout_(timeout) {
}

BashLemmaProcessor::~BashLemmaProcessor() {
  std::remove(kTempPath.c_str());
}

TamarinOutput BashLemmaProcessor::DoProcessLemma(const LemmaJob& lemma_job) {
  string cmd = "timeout " + std::to_string(timeout_) + " ";

  string tamarin_args = "";

  if(lemma_job.GetHeuristic() != TamarinHeuristic::None) {
    tamarin_args += "--heuristic=" +
                    GetTamarinHeuristicArgument(lemma_job.GetHeuristic());
  }

  if(!proof_directory_.empty()) {
    tamarin_args += " --output=" + proof_directory_ + "/" +
                    lemma_job.GetLemmaName() + ".spthy";
  }

  cmd += "tamarin-prover --prove=" + lemma_job.GetLemmaName() + " "
         + tamarin_args + " " + lemma_job.GetSpthyFilePath()
         + " 1> " + kTempPath + " 2> /dev/null";

  TamarinOutput tamarin_output;
  tamarin_output.duration = ExecuteShellCommand(cmd);

  ifstream file_stream {kTempPath, ifstream::in};
  tamarin_output.result =
          ExtractResultForLemma(file_stream, lemma_job.GetLemmaName());

  std::remove(kTempPath.c_str());

  return tamarin_output;
}

string BashLemmaProcessor::GetTamarinHeuristicArgument(
        const TamarinHeuristic& heuristic) {
  switch(heuristic){
    case TamarinHeuristic::S: return "S";
    case TamarinHeuristic::s: return "s";
    case TamarinHeuristic::I: return "I";
    case TamarinHeuristic::i: return "i";
    case TamarinHeuristic::C: return "C";
    case TamarinHeuristic::c: return "c";
    case TamarinHeuristic::P: return "P";
    case TamarinHeuristic::p: return "p";
    case TamarinHeuristic::None: return "";
    default: return "";
  }
}

ProverResult BashLemmaProcessor::ExtractResultForLemma(
        istream& tamarin_stream,
        const string& lemma_name) {
  string line;

  // Move stream to begin of lemma results
  while(std::getline(tamarin_stream, line) && line.substr(0,5) != "=====");
  for(int i=1;i<=4;i++) std::getline(tamarin_stream, line);

  // Move to line for the lemma with name equal to lemma_name
  while(getline(tamarin_stream, line)
        && ExtractLemmaName(line) != lemma_name);

  if(line.find("falsified") != string::npos)
    return ProverResult::False;
  else if(line.find("verified") != string::npos)
    return ProverResult::True;
  return ProverResult::Unknown;
}

string BashLemmaProcessor::ExtractLemmaName(string line) {
  line = line.substr(line.find_first_not_of(" \f\n\r\t\v"));
  return line.substr(0, line.find(' '));
}

} // namespace uttamarin
