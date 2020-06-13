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

#include <fstream>
#include <string>

#include "tamarin_interface.h"
#include "utility.h"

using std::ifstream;
using std::istream;
using std::string;

namespace uttamarin {

const string kTempPath = "/tmp/uttamarintemp.ut";

BashLemmaProcessor::BashLemmaProcessor(const string& tamarin_path,
                                       const string& proof_directory,
                                       const int timeout) :
                                       tamarin_path_(tamarin_path),
                                       proof_directory_(proof_directory),
                                       timeout_(timeout),
                                       heuristic_(TamarinHeuristic::S){

}

TamarinOutput BashLemmaProcessor::DoProcessLemma(const string& spthy_file_path,
                                                 const string& lemma_name) {
  string cmd = "timeout " + std::to_string(timeout_) + " ";

  string tamarin_args = "--heuristic=" + GetTamarinHeuristicArgument();

  if(!proof_directory_.empty()) {
    tamarin_args += " --output=" + proof_directory_ + "/" +
                    lemma_name + ".spthy";
  }

  cmd += tamarin_path_ + " --prove=" + lemma_name + " "
         + tamarin_args + " " + spthy_file_path
         + " 1> " + kTempPath + " 2> /dev/null";

  TamarinOutput tamarin_output;
  tamarin_output.duration = ExecuteShellCommand(cmd);

  ifstream file_stream {kTempPath, ifstream::in};
  tamarin_output.result = ExtractResultForLemma(file_stream, lemma_name);

  std::remove(kTempPath.c_str());

  return tamarin_output;
}

void BashLemmaProcessor::DoSetHeuristic(TamarinHeuristic heuristic) {
  heuristic_ = heuristic;
}

string BashLemmaProcessor::GetTamarinHeuristicArgument() {
  switch(heuristic_){
    case TamarinHeuristic::S: return "S";
    case TamarinHeuristic::s: return "s";
    case TamarinHeuristic::I: return "I";
    case TamarinHeuristic::i: return "i";
    case TamarinHeuristic::C: return "c";
    case TamarinHeuristic::P: return "P";
    case TamarinHeuristic::p: return "p";
    default: return "S";
  }
}

ProverResult BashLemmaProcessor::ExtractResultForLemma(
        istream& stream_of_tamarin_output,
        const string& lemma_name) {
  string line;
  MoveTamarinStreamToLemmaNames(stream_of_tamarin_output);
  while(getline(stream_of_tamarin_output, line)
        && ExtractLemmaName(line) != lemma_name);

  if(line.find("falsified") != string::npos)
    return ProverResult::False;
  else if(line.find("verified") != string::npos)
    return ProverResult::True;
  return ProverResult::Unknown;
}

void BashLemmaProcessor::MoveTamarinStreamToLemmaNames(istream& tamarin_stream) {
  string line;
  while(std::getline(tamarin_stream, line) && line.substr(0,5) != "=====");
  for(int i=1;i<=4;i++) std::getline(tamarin_stream, line);
}

string BashLemmaProcessor::ExtractLemmaName(string tamarin_line) {
  TrimLeft(tamarin_line);
  return tamarin_line.substr(0, tamarin_line.find(' '));
}

} // namespace uttamarin
