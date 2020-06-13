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

#include "tamarin_interface.h"

#include <fstream>
#include <string>
#include <vector>

#include "utility.h"

using std::string;
using std::vector;

namespace uttamarin {

const string kTamarinTempfilePath = "/tmp/uttamarintemp.ut";

string to_string(const ProverResult& prover_result, bool is_colorized) {
  string result_string = "";
  switch(prover_result) {
    case ProverResult::True: result_string = "verified"; break;
    case ProverResult::False: result_string = "false"; break;
    case ProverResult::Unknown: result_string = "timeout";
  }

  if(is_colorized) {
    string prefix = "\033[";
    string color_code = prover_result == ProverResult::True ?
      "32" : prover_result == ProverResult::False ? "31" : "33";
    result_string = prefix + color_code + "m" + result_string + "\033[m";
  }
  return result_string;
}

string to_string(const TamarinOutput& tamarin_output,
                 bool is_colorized) {
  string formatted = to_string(tamarin_output.result, is_colorized);
  formatted += " (" + ToSecondsString(tamarin_output.duration) + ")";
  return formatted;
}

// Takes as input a line of the Tamarin output (a line that shows the Tamarin
// result for a particular lemma) and returns the name of the lemma.
string ExtractLemmaName(string line) {
  line = line.substr(line.find_first_not_of(" \f\n\r\t\v"));
  return line.substr(0, line.find(' '));
}

string RunTamarinAndWriteOutputToNewTempfile(const string& spthy_file_path,
                                             const string& tamarin_parameters) {
  ExecuteShellCommand("tamarin-prover " + tamarin_parameters +
                      spthy_file_path + " 1> " + kTamarinTempfilePath +
                      " 2> /dev/null");
  return kTamarinTempfilePath;
}

vector<string> ReadLemmaNamesFromSpthyFile(const string& spthy_file_path) {
  auto temp_file = RunTamarinAndWriteOutputToNewTempfile(spthy_file_path);

  std::ifstream tamarin_stream {temp_file, std::ifstream::in};
  vector<string> lemma_names;
  string line;

  // Move stream to begin of lemma names
  while(std::getline(tamarin_stream, line) && line.substr(0,5) != "=====");
  for(int i=1;i<=4;i++) std::getline(tamarin_stream, line);

  // Read lemma names
  while(std::getline(tamarin_stream, line) && line != "") {
    lemma_names.push_back(ExtractLemmaName(line));
  }

  // Remove temp file
  std::remove(temp_file.c_str());

  return lemma_names;
}

} // namespace uttamarin