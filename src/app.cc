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

#include "app.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "third-party/cli11/CLI11.hpp"

#include "tamarin_config.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::min;
using std::numeric_limits;
using std::cout;
using std::cerr;
using std::clog;
using std::endl;
using std::flush;
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;

namespace ut_tamarin {

const string kTempfilePath = "/tmp/uttamarintemp.ut";
const string kPreprocessedTempfilePath = "/tmp/preprocessed.spthy";
const string kM4TempfilePath = "/tmp/temp.m4";

// Converts a given ProverResult to a string. If the parameter 'is_colorized'
// is true, then the string is colored using color code for the bash
string to_string(const ProverResult& prover_result, bool is_colorized=false) {
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

// Takes a duration in seconds and converts it into a string of the form MM:SS
string durationToString(int seconds) { 
  string strMinutes = std::to_string(seconds / 60); 
  string strSeconds = std::to_string(seconds % 60); 
  if(strMinutes.size() < 2) strMinutes = "0" + strMinutes; 
  if(strSeconds.size() < 2) strSeconds = "0" + strSeconds; 
  return strMinutes + ":" + strSeconds; 
}

// Takes a duration in seconds and converts it into a string saying "duration
// seconds"
string toSecondsString(int duration) { 
  return std::to_string(duration) + " second" + (duration != 1 ? "s" : ""); 
}

// 
string to_string(const TamarinOutput& tamarin_output, 
                 bool is_colorized=false) { 
  string formatted = to_string(tamarin_output.result, is_colorized); 
  formatted += " (" + toSecondsString(tamarin_output.duration) + ")"; 
  return formatted; 
}

// Trims white space from the left of the given string.
void trimLeft(string& line) { 
  line.erase(line.begin(), std::find_if(line.begin(), line.end(), 
             [](int ch) { return !std::isspace(ch); })); 
}

// Computes the edit distance between the substring of A starting at a and the
// substring of B starting at b. The parameter 'dp' is used for memoization.
int editDistanceHelper(const string& A, int a, const string& B, int b,
                       vector<vector<int>>& dp) { 
  if(a == A.size()) return B.size() - b; 
  if(b == B.size()) return A.size() - a; 
  if(dp[a][b] == -1) { 
    int add = 1 + editDistanceHelper(A, a, B, b+1, dp); 
    int remove = 1 + editDistanceHelper(A, a+1, B, b, dp); 
    int modification_cost = A[a] == B[b] ? 0 : 1; 
    int keep_or_modify = modification_cost + 
      editDistanceHelper(A, a+1, B, b+1, dp); 
    dp[a][b] = min(add, min(remove, keep_or_modify)); 
  } 
  return dp[a][b]; 
}

// Computes the edit distance between two strings A and B
int editDistance(const string& A, const string B) { 
  vector<vector<int>> dp(A.size(), vector<int>(B.size(), -1)); 
  return editDistanceHelper(A, 0, B, 0, dp); 
}

// Executes a shell command and returns the duration of the execution in
// seconds.
int executeShellCommand(const string& cmd) { 
  auto start_time = std::chrono::high_resolution_clock::now();
  int status; 
  auto fp = popen(cmd.c_str(), "r"); 
  pclose(fp); 
  auto end_time = std::chrono::high_resolution_clock::now(); 
  return std::chrono::duration_cast<std::chrono::seconds> 
    (end_time - start_time).count(); 
}

string App::RunTamarinAndWriteOutputToNewTempfile(
                                    const string& tamarin_path,
                                    const string& spthy_file_path,
                                    const string& tamarin_parameters) {
  executeShellCommand(tamarin_path + " " + tamarin_parameters + 
       spthy_file_path + " 1> " + kTempfilePath + " 2> /dev/null");
  return kTempfilePath;
}

string App::ExtractLemmaName(string tamarin_line) {
  trimLeft(tamarin_line);
  return tamarin_line.substr(0, tamarin_line.find(' '));
}

vector<string> App::ReadLemmaNamesFromStream(istream& input_stream) {
  vector<string> lemma_names;
  string line;
  while(std::getline(input_stream, line) && line != "") {
    lemma_names.push_back(ExtractLemmaName(line));
  }
  return lemma_names;
}

vector<string> App::ReadLemmaNamesFromLemmaFile(const string& file_name) {
  ifstream file_stream {file_name, ifstream::in};
  return ReadLemmaNamesFromStream(file_stream);
}

void App::MoveTamarinStreamToLemmaNames(istream& tamarin_stream) {
  string line;
  while(std::getline(tamarin_stream, line) && line.substr(0,5) != "=====");
  for(int i=1;i<=4;i++) std::getline(tamarin_stream, line);
}

vector<string> App::ReadLemmaNamesFromSpthyFile(const string& tamarin_path,
                                                const string& spthy_file_path) {
  auto temp_file = RunTamarinAndWriteOutputToNewTempfile(tamarin_path,
                                                         spthy_file_path);
  ifstream tamarin_stream {temp_file, ifstream::in};
  MoveTamarinStreamToLemmaNames(tamarin_stream);
  auto lemma_names = ReadLemmaNamesFromStream(tamarin_stream);
  std::remove(temp_file.c_str());
  return lemma_names;
}

ProverResult App::GetTamarinResultForLemma(istream& stream_of_tamarin_output,
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

void App::WriteLemmaNames(const vector<string>& lemma_names, 
                     ostream& output_stream) {
  for(auto lemma_name : lemma_names) {
    output_stream << lemma_name << endl;
  }
}

TamarinOutput App::ProcessTamarinLemma(const string& lemma_name,
                                       const string& spthy_file_path,
                                       const CmdParameters& parameters,
                                       string tamarin_args) {

  string cmd = "";
  if(parameters.timeout > 0) 
    cmd += "timeout " + std::to_string(parameters.timeout) + " ";

  if(parameters.proof_directory != "") {
    tamarin_args += " --output=" + parameters.proof_directory + "/" + 
                    lemma_name + ".spthy";
  }
  
  cmd += parameters.tamarin_path + " --prove=" + lemma_name + " " 
       + tamarin_args + " " + spthy_file_path 
       + " 1> " + kTempfilePath + " 2> /dev/null";

  if(parameters.verbose) clog << endl << "Calling Tamarin: " << cmd << endl;
  
  TamarinOutput tamarin_output;
  tamarin_output.duration = executeShellCommand(cmd);

  ifstream file_stream {kTempfilePath, ifstream::in};
  tamarin_output.result = GetTamarinResultForLemma(file_stream, lemma_name);

  std::remove(kTempfilePath.c_str());

  return tamarin_output;
}

int App::PrintLemmaNames(const CmdParameters& parameters, 
                         ostream& output_stream) {
  auto lemma_names = ReadLemmaNamesFromSpthyFile(parameters.tamarin_path,
      parameters.spthy_file_path);
  WriteLemmaNames(lemma_names, output_stream); 
  return 0;
}

vector<string> App::GetLemmasInWhitelist(const vector<string>& all_lemmas, 
                                         const vector<string>& whitelist) {
  for(auto lemma_name : whitelist) {
    if(std::find(all_lemmas.begin(), all_lemmas.end(), lemma_name)
       == all_lemmas.end()) {
      cerr << "Warning: lemma '" << lemma_name << "' is not declared in " <<
        "the Tamarin theory." << endl;
    }
  }
  auto filtered_lemmas = all_lemmas;
  filtered_lemmas.erase(
      std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
      [&whitelist](const string& lemma_name) {
        return std::find(whitelist.begin(), whitelist.end(), lemma_name) 
        == whitelist.end();
      }), filtered_lemmas.end());
  return filtered_lemmas;
}

vector<string> App::RemoveLemmasInBlacklist(const vector<string>& all_lemmas, 
                                            const vector<string>& blacklist) {
  auto filtered_lemmas = all_lemmas;
  filtered_lemmas.erase(
      std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
      [&](const string& lemma_name) { 
        return std::find(blacklist.begin(), blacklist.end(), lemma_name) 
               != blacklist.end();
      }),
      filtered_lemmas.end());
  return filtered_lemmas;
}

vector<string> App::RemoveLemmasBeforeStart(const vector<string>& all_lemmas, 
                                            const string& starting_lemma) {
  int min_edit_distance = numeric_limits<int>::max();
  auto it_start = all_lemmas.begin();
  for(auto it = all_lemmas.begin();it != all_lemmas.end();++it) {
    auto lemma = *it;
    int edit_distance = editDistance(starting_lemma, lemma);
    if(edit_distance < min_edit_distance) {
      min_edit_distance = edit_distance; 
      it_start = it;
    }
  }

  vector<string> filtered_lemmas;
  if(it_start != all_lemmas.end()) {
    for(auto it = it_start;it != all_lemmas.end();++it) {
      filtered_lemmas.emplace_back(*it);
    }
  } else {
    cerr << "Warning: No lemma whose name starts with '" << starting_lemma << 
      "' declared in the Tamarin theory." << endl;
  }
  return filtered_lemmas;
}

vector<string> App::GetNamesOfLemmasToVerify(const string& tamarin_path,
                                             const string& spthy_file_path,
                                             const vector<string>& whitelist,
                                             const vector<string>& blacklist,
                                             const string& starting_lemma) {
  auto lemmas = ReadLemmaNamesFromSpthyFile(tamarin_path, spthy_file_path);
  if(!whitelist.empty()) {
    lemmas = GetLemmasInWhitelist(lemmas, whitelist);
  }
  if(!blacklist.empty()) {
    lemmas = RemoveLemmasInBlacklist(lemmas, blacklist); 
  }
  if(starting_lemma != "") {
    lemmas = RemoveLemmasBeforeStart(lemmas, starting_lemma);
  }
  return lemmas;
}

void App::PrintHeader(const CmdParameters& parameters, ostream& output_stream) {
  auto file_name = parameters.spthy_file_path;
  if(file_name.find('/') != string::npos)
    file_name = file_name.substr(file_name.find_last_of('/') + 1);

  output_stream << "Tamarin Tests for file '" << file_name << "':" << endl;
  output_stream << "Timeout: " << (parameters.timeout <= 0 ? "no timeout" : 
                   (std::to_string(parameters.timeout) + " second" + 
                   (parameters.timeout > 1 ? "s" : ""))) << " per lemma" 
                    << endl;
}

string App::AddPrefixViaM4(const string& prefix, const string& original) {
  return "define(" + original + ", " + prefix + original + "($*))";
}

vector<string> App::GetM4Commands(const string& lemma_name, 
                                  const TamarinConfig& config) {
  const string important_prefix = "F_";
  const string unimportant_prefix = "L_";

  vector<string> m4_commands;

  for(string fact : config.global_annotations.important_facts) {
    if(!config.FactIsAnnotatedLocally(fact, lemma_name)) {
      m4_commands.emplace_back(AddPrefixViaM4(important_prefix, fact));
    }
  }

  for(string fact : config.global_annotations.unimportant_facts) {
    if(!config.FactIsAnnotatedLocally(fact, lemma_name)) {
      m4_commands.emplace_back(AddPrefixViaM4(unimportant_prefix, fact));
    }
  }

  if(config.lemma_annotations.count(lemma_name)) {
    for(string fact : config.lemma_annotations.at(lemma_name).important_facts)
      m4_commands.emplace_back(AddPrefixViaM4(important_prefix, fact));

    for(string fact : config.lemma_annotations.at(lemma_name).unimportant_facts)
      m4_commands.emplace_back(AddPrefixViaM4(unimportant_prefix, fact));
  }

  return m4_commands;
}

string App::ApplyCustomHeuristics(const string& spthy_file_path, 
                             const string& lemma_name,
                             const TamarinConfig& config,
                             const CmdParameters& cmd_parameters) {
  auto m4_commands = GetM4Commands(lemma_name, config);

  ofstream tempfile_m4{kM4TempfilePath};

  if(cmd_parameters.verbose && !m4_commands.empty())
    clog << "Fact Annotations for " << lemma_name << ": " << endl;

  // Change quotes for M4, otherwise single quotes in spthy file lead to M4 bugs
  tempfile_m4 << "changequote(<!,!>)" << endl;
  tempfile_m4 << "changecom(<!/*!>, <!*/!>)" << endl;

  for(auto m4_command : m4_commands) {
    tempfile_m4 << m4_command << endl;
    if(cmd_parameters.verbose)
      clog << m4_command << endl;
  }

  ifstream spthy_file{spthy_file_path};
  string spthy_file_line = "";
  while(std::getline(spthy_file, spthy_file_line)) 
    tempfile_m4 << spthy_file_line << endl;

  executeShellCommand("m4 " + kM4TempfilePath + " > " + 
                      kPreprocessedTempfilePath);

  return kPreprocessedTempfilePath;
}

bool App::RunTamarinOnLemmas(const CmdParameters& parameters, 
                             ostream& output_stream) {
  PrintHeader(parameters, output_stream);

  auto config = ParseTamarinConfigFile(parameters.config_file_path);

  auto lemma_names = GetNamesOfLemmasToVerify(parameters.tamarin_path,
                                              parameters.spthy_file_path,
                                              config.lemma_whitelist,
                                              config.lemma_blacklist,
                                              parameters.starting_lemma);
  output_stream << endl;

  bool success = true; 

  unordered_map<ProverResult, int> count;
  int overall_duration = 0;
  for(int i=0;i < lemma_names.size();i++) {
    auto preprocessed_spthy_file = ApplyCustomHeuristics(
                        parameters.spthy_file_path, lemma_names[i], 
                        config, parameters);

    auto line = lemma_names[i] + " (" + std::to_string(i+1) + "/" + 
                std::to_string(lemma_names.size()) + ") ";
    // output_stream << line << "  " << flush;
    cout << line << "  " << flush;

    auto start_time = std::chrono::high_resolution_clock::now();
    std::future<TamarinOutput> f = std::async(&App::ProcessTamarinLemma, 
                                              this,
                                              lemma_names[i],
                                              preprocessed_spthy_file,
                                              parameters, "");
    do{
      auto seconds = durationToString(
        std::chrono::duration_cast<std::chrono::seconds>(
           std::chrono::high_resolution_clock::now() - start_time).count());
      cout << "\r" << line << seconds << " " << std::flush;
    } while(f.wait_for(std::chrono::seconds(1)) != std::future_status::ready);
    auto stats = f.get();
    output_stream << "\r" << line << 
      to_string(stats, &output_stream == &cout) << endl;
    overall_duration += stats.duration;
    count[stats.result]++;
    std::remove(kTempfilePath.c_str());
    if(stats.result != ProverResult::True) {
      success = false;
      if(parameters.abort_after_failure) break;
    }
  }
  output_stream << endl << "Summary: " << endl;
  output_stream << to_string(ProverResult::True) << ": " <<
    count[ProverResult::True] << ", " << to_string(ProverResult::False) << ": "
    << count[ProverResult::False] << ", " << to_string(ProverResult::Unknown)
    << ": " << count[ProverResult::Unknown] << endl;

  output_stream << "Overall duration: " << toSecondsString(overall_duration) 
   << endl;

  return success;
}

int App::PenetrateLemma(const CmdParameters& p, ostream& output_stream) {
  auto lemmas_in_file = ReadLemmaNamesFromSpthyFile(p.tamarin_path, 
                                                    p.spthy_file_path);
  auto lemma_it = std::find_if(lemmas_in_file.begin(), lemmas_in_file.end(), 
        [&p](const string& lemma) { 
          return lemma.size() >= p.penetration_lemma.size() &&
                 lemma.substr(0, p.penetration_lemma.size()) == 
                 p.penetration_lemma;
        });
  if(lemma_it == lemmas_in_file.end()) {
    cerr << "No lemma starting with '" << p.penetration_lemma << 
      "' contained in file " << p.spthy_file_path << "." << endl;
    return 1;
  }

  string penetration_lemma = *lemma_it;

  output_stream << "Penetrating lemma '" << penetration_lemma << 
    "' with a per-heuristic timeout of " << toSecondsString(p.timeout) <<
    "." << endl << endl;

  vector<string> heuristics = {"S", "C", "I", "s", "c", "i", "P", "p"};
  for(auto heuristic : heuristics) {
    output_stream << "Heuristic: " << heuristic << " " << flush;
    auto output = ProcessTamarinLemma(penetration_lemma, p.tamarin_path, p,
                                      "--heuristic=" + heuristic);
    output_stream << to_string(output.result, &output_stream == &cout) 
      << " (" << toSecondsString(output.duration) << ")" << endl;
    if(output.result == ProverResult::True) break; 
  }
  return 0;
}

string App::GetTempfilePath() {
  return kTempfilePath;
}

} // namespace ut_tamarin
