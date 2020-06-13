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

#ifndef UT_TAMARIN_TAMARIN_INTERFACE_H_ 
#define UT_TAMARIN_TAMARIN_INTERFACE_H_

#include <string>

namespace uttamarin {

enum class ProverResult { True, False, Unknown };

struct TamarinOutput {
  ProverResult result;
  int duration; // in seconds
};

// Converts a given ProverResult to a string. If the parameter 'is_colorized'
// is true, then the string is colored using color codes for the bash
std::string to_string(const ProverResult& prover_result,
                      bool is_colorized=false);

// Converts a given TamarinOutput into a string. If the parameter 'is_colorized'
// is true, then the string is colored using color codes for the bash
std::string to_string(const TamarinOutput& tamarin_output,
                      bool is_colorized=false);


} // namespace uttamarin

#endif
