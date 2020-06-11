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

#include "utility.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <string>
#include <vector>

#include "tamarin_config.h"
#include "tamarin_interface.h"

using std::string;
using std::vector;

namespace ut_tamarin {

string ToSecondsString(int duration) { 
  return std::to_string(duration) + " second" + (duration != 1 ? "s" : ""); 
}

string to_string(const ProverResult& prover_result, bool is_colorized) {
  string result_string; 
  if(prover_result == ProverResult::True) 
    result_string = "verified"; 
  else if(prover_result == ProverResult::False) 
    result_string = "false"; 
  else result_string = "timeout"; 
  
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

string DurationToString(int seconds) { 
  string strMinutes = std::to_string(seconds / 60); 
  string strSeconds = std::to_string(seconds % 60); 
  if(strMinutes.size() < 2) strMinutes = "0" + strMinutes; 
  if(strSeconds.size() < 2) strSeconds = "0" + strSeconds; 
  return strMinutes + ":" + strSeconds; 
}

void TrimLeft(string& line) { 
  line.erase(line.begin(), std::find_if(line.begin(), line.end(), 
             [](int ch) { return !std::isspace(ch); })); 
}

// Computes the edit distance between the substring of A starting at a and the
// substring of B starting at b. The parameter 'dp' is used for memoization.
int EditDistanceHelper(const string& A, int a, const string& B, int b,
                       vector<vector<int>>& dp) { 
  if(a == A.size()) return B.size() - b; 
  if(b == B.size()) return A.size() - a; 
  if(dp[a][b] == -1) { 
    int add = 1 + EditDistanceHelper(A, a, B, b+1, dp); 
    int remove = 1 + EditDistanceHelper(A, a+1, B, b, dp); 
    int modification_cost = A[a] == B[b] ? 0 : 1; 
    int keep_or_modify = modification_cost + 
      EditDistanceHelper(A, a+1, B, b+1, dp); 
    dp[a][b] = std::min(add, std::min(remove, keep_or_modify)); 
  } 
  return dp[a][b]; 
}

int EditDistance(const string& A, const string B) { 
  vector<vector<int>> dp(A.size(), vector<int>(B.size(), -1)); 
  return EditDistanceHelper(A, 0, B, 0, dp); 
}

int ExecuteShellCommand(const string& cmd) { 
  auto start_time = std::chrono::high_resolution_clock::now();
  int status; 
  auto fp = popen(cmd.c_str(), "r"); 
  pclose(fp); 
  auto end_time = std::chrono::high_resolution_clock::now(); 
  return std::chrono::duration_cast<std::chrono::seconds> 
    (end_time - start_time).count(); 
}

} // namespace ut_tamarin
