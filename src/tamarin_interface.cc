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

#include <string>

#include "utility.h"

using std::string;

namespace uttamarin {

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

} // namespace uttamarin
