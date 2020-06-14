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

#ifndef UT_TAMARIN_UTILITY_H_ 
#define UT_TAMARIN_UTILITY_H_

#include <string>
#include <vector>

namespace uttamarin {

struct TamarinOutput;

// Takes a duration in seconds and converts it into a string saying "duration
// seconds"
std::string ToSecondsString(int duration);

// Computes the edit distance between two strings A and B
int EditDistance(const std::string& A, const std::string B);

// Takes a list of lemma names and a candidate name and returns the lemma
// from the list whose name is closes to the candidate name.
std::string GetStringWithShortestEditDistance(
        const std::vector<std::string>& candidates,
        const std::string& target);

// Executes a shell command and returns the duration of the execution in
// seconds.
int ExecuteShellCommand(const std::string& cmd);

} // namespace uttamarin

#endif
