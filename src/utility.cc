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

using std::string;
using std::vector;

namespace uttamarin {

string ToSecondsString(int duration) { 
  return std::to_string(duration) + " second" + (duration != 1 ? "s" : ""); 
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

string GetStringWithShortestEditDistance(const vector<string>& candidates,
                                         const string& target) {
  int min_edit_distance = std::numeric_limits<int>::max();
  string closest_lemma = "";
  for(auto lemma : candidates){
    int edit_distance = EditDistance(target, lemma);
    if(edit_distance < min_edit_distance) {
      min_edit_distance = edit_distance;
      closest_lemma = lemma;
    }
  }
  return closest_lemma;
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

} // namespace uttamarin
