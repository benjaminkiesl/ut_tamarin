// MIT License
//
// Copyright (c) 2019 Benjamin Kiesl
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

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <future>
#include <csignal>
#include "third-party/cli11/CLI11.hpp"
#include "third-party/nlohmann/json.hpp"

using std::string;
using std::to_string;
using std::vector;
using std::unordered_map;
using std::min;
using std::numeric_limits;
using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using json = nlohmann::json;

const string kTempfileName = "/tmp/uttamarintemp.ut";
const string kPreprocessedTempfile = "/tmp/preprocessed.spthy";
const string kM4Tempfile = "/tmp/temp.m4";

struct CmdParameters{
  string tamarin_path;
  string spthy_file_path;
  string config_file_path;
  string starting_lemma;
  string penetration_lemma;
  int timeout;
  bool abort_after_failure;
  bool generate_lemma_file;
};

struct LemmaAnnotation{
  vector<string> important_facts;
  vector<string> unimportant_facts;
};

struct TamarinConfig{
  vector<string> lemma_blacklist;
  vector<string> lemma_whitelist;
  unordered_map<string, LemmaAnnotation> lemma_annotations;
};

enum class ProverResult {True, False, Unknown};

struct TamarinOutput{
  ProverResult result;
  int duration; // in seconds
};

string to_string(const ProverResult& prover_result,
                 bool is_colorized=false) {
  string result_string;
  if(prover_result == ProverResult::True) result_string = "verified";
  else if(prover_result == ProverResult::False) result_string = "false";
  else result_string = "timeout";
  if(is_colorized){
    string prefix = "\033[";
    string color_code = prover_result == ProverResult::True ? "32" :
                        prover_result == ProverResult::False ? "31" : "33";
    result_string = prefix + color_code + "m" + result_string + "\033[m";
  }
  return result_string;
}

string durationToString(int seconds){
  string strMinutes = to_string(seconds / 60);
  string strSeconds = to_string(seconds % 60);
  if(strMinutes.size() < 2) strMinutes = "0" + strMinutes;
  if(strSeconds.size() < 2) strSeconds = "0" + strSeconds;
  return strMinutes + ":" + strSeconds;
}

string toSecondsString(int duration){
  return to_string(duration) + " second" + (duration != 1 ? "s" : "");
}

string to_string(const TamarinOutput& tamarin_output, 
                 bool is_colorized=false) {
  string formatted = to_string(tamarin_output.result, is_colorized);
  formatted += " (" + toSecondsString(tamarin_output.duration) + ")";
  return formatted;
}

// Executes a shell command and returns the duration of the 
// execution in seconds.
int executeShellCommand(const string& cmd){
  auto start_time = std::chrono::high_resolution_clock::now();
  int status;
  auto fp = popen(cmd.c_str(), "r");
  pclose(fp);
  auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>
                            (end_time - start_time).count();
}

// Executes Tamarin with the given parameters and writes the needed output
// to a temp file while relocating the other output to /dev/null. 
// Returns the path to the resulting temp file.
string runTamarinAndWriteOutputToNewTempfile(
                                    const string& tamarin_path,
                                    const string& spthy_file_path,
                                    const string& tamarin_parameters=""){
  executeShellCommand(tamarin_path + " " + tamarin_parameters + 
       spthy_file_path + " 1> " + kTempfileName + " 2> /dev/null");
  return kTempfileName;
}

// Trims white space from the left of the given string.
void trimLeft(string& line) {
  line.erase(line.begin(), std::find_if(line.begin(), line.end(), 
             [](int ch) { return !std::isspace(ch); }));
}

// Takes as input a line of the Tamarin output (a line that shows the
// Tamarin result for a particular lemma) and returns the name of the lemma.
string getLemmaName(string tamarin_line){
  trimLeft(tamarin_line);
  return tamarin_line.substr(0, tamarin_line.find(' '));
}

// Takes as input a stream that points to a list of lemma names (one 
// lemma name per line) and returns these lemma names in a vector.
vector<string> readLemmaNamesFromStream(istream& input_stream){
  vector<string> lemma_names;
  string line;
  while(std::getline(input_stream, line) && line != ""){
    lemma_names.push_back(getLemmaName(line));
  }
  return lemma_names;
}

// Takes a file that contains lemma names (one per line) and returns a vector
// with all the lemma names.
vector<string> readLemmaNamesFromLemmaFile(const string& file_name){
  ifstream file_stream {file_name, ifstream::in};
  return readLemmaNamesFromStream(file_stream);
}

// Takes as input a stream to output produced by Tamarin and moves the stream
// to the first line of the part where the results for lemmas are displayed.
void moveTamarinStreamToLemmaNames(istream& tamarin_stream){
  string line;
  while(std::getline(tamarin_stream, line) && line.substr(0,5) != "=====");
  for(int i=1;i<=4;i++) std::getline(tamarin_stream, line);
}

// Takes as input a Tamarin theory file (".spthy") and returns a vector
// containing all the names of the lemmas specified in the file.
vector<string> readLemmaNamesFromSpthyFile(const string& tamarin_path,
                                           const string& spthy_file_path){
  auto temp_file = runTamarinAndWriteOutputToNewTempfile(tamarin_path,
                                                         spthy_file_path);
  ifstream tamarin_stream {temp_file, ifstream::in};
  moveTamarinStreamToLemmaNames(tamarin_stream);
  auto lemma_names = readLemmaNamesFromStream(tamarin_stream);
  std::remove(temp_file.c_str());
  return lemma_names;
}

// Takes as input a stream of Tamarin output and the name of a lemma and
// returns the result ("verified", "falsified", "analysis incomplete").
ProverResult getTamarinResultForLemma(istream& stream_of_tamarin_output,
                                      const string& lemma_name){
  string line;
  moveTamarinStreamToLemmaNames(stream_of_tamarin_output);
  while(getline(stream_of_tamarin_output, line) 
        && getLemmaName(line) != lemma_name);

  if(line.find("falsified") != string::npos)
    return ProverResult::False;
  else if(line.find("verified") != string::npos)
    return ProverResult::True;
  return ProverResult::Unknown;
}

// Takes as input a vector of lemma names and an output stream and writes 
// the lemma names to the stream.
void writeLemmaNames(const vector<string>& lemma_names, 
                     ostream& output_stream){
  for(auto lemma_name : lemma_names){
    output_stream << lemma_name << endl;
  }
}

// Takes as input a Tamarin theory file (".spthy"), the name of a lemma,
// and a timeout, and then runs Tamarin on the given lemma. Returns some
// output/statistics (like Tamarin's result and the execution duration).
TamarinOutput processTamarinLemma(const string& tamarin_path,
                                  const string& spthy_file_path,
                                  const string& lemma_name, int timeout,
                                  const string& tamarin_args=""){

  string cmd = "";
  if(timeout > 0) cmd += "timeout " + to_string(timeout) + " ";
  cmd += tamarin_path + " --prove=" + lemma_name + " " 
       + tamarin_args + " " + spthy_file_path 
       + " 1> " + kTempfileName + " 2> /dev/null";

  TamarinOutput tamarin_output;
  tamarin_output.duration = executeShellCommand(cmd);

  ifstream file_stream {kTempfileName, ifstream::in};
  tamarin_output.result = getTamarinResultForLemma(file_stream, lemma_name);

  std::remove(kTempfileName.c_str());
  return tamarin_output;
}

// Runs the tool in the mode where a Tamarin theory file (".spthy") is read 
// and a list of the lemmas occurring in that file is written to a file.
int printLemmaNames(const CmdParameters& parameters, ostream& output_stream){
  auto lemma_names = readLemmaNamesFromSpthyFile(parameters.tamarin_path,
      parameters.spthy_file_path);
  writeLemmaNames(lemma_names, output_stream); 
  return 0;
}


// Takes as input two vectors of lemmas (the initial lemmas and the 
// "whitelist", respectively) and removes from the initial lemmas all 
// lemmas that do not occur in the whitelist.
vector<string> getLemmasInWhitelist(const vector<string>& all_lemmas, 
                                    const vector<string>& whitelist){
  for(auto lemma_name : whitelist){
    if(std::find(all_lemmas.begin(), all_lemmas.end(), lemma_name)
       == all_lemmas.end()){
      cerr << "Warning: lemma '" << lemma_name << "' is not declared in " <<
        "the Tamarin theory." << endl;
    }
  }
  auto filtered_lemmas = all_lemmas;
  filtered_lemmas.erase(
      std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
      [&whitelist](const string& lemma_name){
        return std::find(whitelist.begin(), whitelist.end(), lemma_name) 
        == whitelist.end();
      }), filtered_lemmas.end());
  return filtered_lemmas;
}

// Takes as input two vectors of lemmas (the initial lemmas and the 
// "blacklist", respectively) and removes from the initial lemmas all 
// lemmas that occur in the blacklist.
vector<string> removeLemmasInBlacklist(const vector<string>& all_lemmas, 
                                       const vector<string>& blacklist){
  auto filtered_lemmas = all_lemmas;
  filtered_lemmas.erase(
      std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
      [&](const string& lemma_name){ 
        return std::find(blacklist.begin(), blacklist.end(), lemma_name) 
               != blacklist.end();
      }),
      filtered_lemmas.end());
  return filtered_lemmas;
}

int editDistanceHelper(const string& A, int a, const string& B, int b,
    vector<vector<int>>& dp){
  if(a == A.size()) return B.size() - b;
  if(b == B.size()) return A.size() - a;
  if(dp[a][b] == -1){
    int add = 1 + editDistanceHelper(A, a, B, b+1, dp);
    int remove = 1 + editDistanceHelper(A, a+1, B, b, dp);
    int modification_cost = A[a] == B[b] ? 0 : 1;
    int keep_or_modify = modification_cost + 
      editDistanceHelper(A, a+1, B, b+1, dp);
    dp[a][b] = min(add, min(remove, keep_or_modify));
  }
  return dp[a][b];
}

int editDistance(const string& A, const string B){
  vector<vector<int>> dp(A.size(), vector<int>(B.size(), -1));
  return editDistanceHelper(A, 0, B, 0, dp);
}

// Takes as input a vector of lemmas and a lemma name. Removes from the vector
// all lemmas that occur before the lemma with the given name (in fact, removes
// all lemmas before the lemma whose name is closest to 'starting_lemma').
vector<string> removeLemmasBeforeStart(const vector<string>& all_lemmas, 
                                       const string& starting_lemma){
  int min_edit_distance = numeric_limits<int>::max();
  auto it_start = all_lemmas.begin();
  for(auto it = all_lemmas.begin();it != all_lemmas.end();++it){
    auto lemma = *it;
    int edit_distance = editDistance(starting_lemma, lemma);
    if(edit_distance < min_edit_distance){
      min_edit_distance = edit_distance; 
      it_start = it;
    }
  }

  vector<string> filtered_lemmas;
  if(it_start != all_lemmas.end()){
    for(auto it = it_start;it != all_lemmas.end();++it){
      filtered_lemmas.emplace_back(*it);
    }
  } else {
    cerr << "Warning: No lemma whose name starts with '" << starting_lemma << 
      "' declared in the Tamarin theory." << endl;
  }
  return filtered_lemmas;
}

// Takes the path to a Tamarin theory file (".spthy") and a lemma file
// (i.e., a file with a list of lemma names, one per line) and determines 
// which lemmas should be verified (namely those that are both in the lemma 
// file and the Tamarin file). If a lemma name is in the lemma file but not 
// in the Tamarin file, a warning message is printed.
vector<string> getNamesOfLemmasToVerify(const string& tamarin_path,
                                        const string& spthy_file_path,
                                        const vector<string>& whitelist,
                                        const vector<string>& blacklist,
                                        const string& starting_lemma = ""){
  auto lemmas = readLemmaNamesFromSpthyFile(tamarin_path, spthy_file_path);
  if(!whitelist.empty()){
    lemmas = getLemmasInWhitelist(lemmas, whitelist);
  }
  if(!blacklist.empty()){
    lemmas = removeLemmasInBlacklist(lemmas, blacklist); 
  }
  if(starting_lemma != ""){
    lemmas = removeLemmasBeforeStart(lemmas, starting_lemma);
  }
  return lemmas;
}

void printHeader(const CmdParameters& parameters, ostream& output_stream){
  auto file_name = parameters.spthy_file_path;
  if(file_name.find('/') != string::npos)
    file_name = file_name.substr(file_name.find_last_of('/') + 1);

  output_stream << "Tamarin Tests for file '" << file_name << "':" << endl;
  output_stream << "Timeout: " << (parameters.timeout <= 0 ? "no timeout" : 
                   (to_string(parameters.timeout) + " second" + 
                   (parameters.timeout > 1 ? "s" : ""))) << " per lemma" 
                    << endl;
}

string applyCustomHeuristics(string spthy_file_path, 
                             const vector<string>& important_facts,
                             const vector<string>& unimportant_facts){
  ofstream tempfile_m4{kM4Tempfile};

  for(auto fact : important_facts){
    tempfile_m4 << 
      "define(" << fact << ", " << "F_" << fact << "($*))" << endl;
  }

  for(auto fact : unimportant_facts){
    tempfile_m4 << 
      "define(" << fact << ", " << "L_" << fact << "($*))" << endl;
  }

  ifstream spthy_file{spthy_file_path};
  string line = "";
  while(std::getline(spthy_file, line)){
    tempfile_m4 << line << endl;
  }

  executeShellCommand("m4 " + kM4Tempfile + " > " + kPreprocessedTempfile);

  return kPreprocessedTempfile;
}

TamarinConfig getTamarinConfig(const string& config_file_path){
  TamarinConfig tamarin_config;
  json json_config;

  if(config_file_path != ""){
    ifstream config_file_stream{config_file_path};
    config_file_stream >> json_config;
  }

  tamarin_config.lemma_blacklist = json_config.count("lemma_blacklist") ?
    json_config["lemma_blacklist"].get<vector<string>>() : vector<string>{};

  tamarin_config.lemma_whitelist = json_config.count("lemma_whitelist") ?
    json_config["lemma_whitelist"].get<vector<string>>() : vector<string>{};

  for(auto lemma_annotation : json_config["lemma_annotations"]){
    string lemma_name = lemma_annotation["name"].get<string>();
    if(lemma_annotation.count("important_facts")){
      tamarin_config.lemma_annotations[lemma_name].important_facts =
        lemma_annotation["important_facts"].get<vector<string>>();
    }
    if(lemma_annotation.count("unimportant_facts")){
      tamarin_config.lemma_annotations[lemma_name].unimportant_facts =
        lemma_annotation["unimportant_facts"].get<vector<string>>();
    }
  }

  return tamarin_config;
}

// Runs Tamarin on lemmas in the given spthy file. The actual choice
// of lemmas depends on the command-line parameters.
int runTamarinOnLemmas(const CmdParameters& parameters, 
                       ostream& output_stream){
  printHeader(parameters, output_stream);

  auto config = getTamarinConfig(parameters.config_file_path);

  auto lemma_names = getNamesOfLemmasToVerify(parameters.tamarin_path,
                                              parameters.spthy_file_path,
                                              config.lemma_whitelist,
                                              config.lemma_blacklist,
                                              parameters.starting_lemma);
  output_stream << endl;

  unordered_map<ProverResult, int> count;
  int overall_duration = 0;
  for(int i=0;i < lemma_names.size();i++){
    auto line = lemma_names[i] + " (" + to_string(i+1) + "/" + 
                to_string(lemma_names.size()) + ") ";
    output_stream << line << "  " << flush;

    auto preprocessed_spthy_file = 
      applyCustomHeuristics(parameters.spthy_file_path, 
          config.lemma_annotations[lemma_names[i]].important_facts,
          config.lemma_annotations[lemma_names[i]].unimportant_facts);

    auto start_time = std::chrono::high_resolution_clock::now();
    std::future<TamarinOutput> f = std::async(processTamarinLemma, 
                                              parameters.tamarin_path, 
                                              parameters.spthy_file_path, 
                                              lemma_names[i],
                                              parameters.timeout, "");
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
    std::remove(kTempfileName.c_str());
    if(parameters.abort_after_failure && 
       stats.result != ProverResult::True) break;
  }
  output_stream << endl << "Summary: " << endl;
  output_stream << to_string(ProverResult::True) << ": " <<
    count[ProverResult::True] << ", " << to_string(ProverResult::False) << ": "
    << count[ProverResult::False] << ", " << to_string(ProverResult::Unknown)
    << ": " << count[ProverResult::Unknown] << endl;

  output_stream << "Overall duration: " << toSecondsString(overall_duration) 
   << endl;

  return 0;
}

// Runs Tamarin on the given lemma, trying out all different heuristics until
// either Tamarin succeeds or all strategies reach a timeout.
int penetrateLemma(const CmdParameters& p, ostream& output_stream){
  auto lemmas_in_file = readLemmaNamesFromSpthyFile(p.tamarin_path, 
                                                    p.spthy_file_path);
  auto lemma_it = std::find_if(lemmas_in_file.begin(), lemmas_in_file.end(), 
        [&p](const string& lemma){ 
          return lemma.size() >= p.penetration_lemma.size() &&
                 lemma.substr(0, p.penetration_lemma.size()) == 
                 p.penetration_lemma;
        });
  if(lemma_it == lemmas_in_file.end()){
    cerr << "No lemma starting with '" << p.penetration_lemma << 
      "' contained in file " << p.spthy_file_path << "." << endl;
    return 1;
  }

  string penetration_lemma = *lemma_it;

  output_stream << "Penetrating lemma '" << penetration_lemma << 
    "' with a per-heuristic timeout of " << toSecondsString(p.timeout) <<
    "." << endl << endl;

  vector<string> heuristics = {"S", "C", "I", "s", "c", "i", "P", "p"};
  for(auto heuristic : heuristics){
    output_stream << "Heuristic: " << heuristic << " " << flush;
    auto output = processTamarinLemma(p.tamarin_path, p.spthy_file_path, 
        penetration_lemma, p.timeout, "--heuristic=" + heuristic);
    output_stream << to_string(output.result, &output_stream == &cout) 
      << " (" << toSecondsString(output.duration) << ")" << endl;
    if(output.result == ProverResult::True) break; 
  }
  return 0;
}

// Holds the name of the tamarin process. This name is used for killing Tamarin 
// when the program receives a SIGINT signal (sent by Ctrl+C).
string tamarin_process = "tamarin-prover";

void (*default_sigint_handler)(int signal);

// Signal handler for SIGINT signal (sent by Ctrl+C)
void sigint_handler(int signal)
{
  cout << endl;
  std::system(("killall " + tamarin_process + " 2> /dev/null").c_str());
  std::remove(kTempfileName.c_str());
  std::signal(signal, default_sigint_handler);
  std::raise(signal);
}

// Sets the global 'tamarin_process' name. This is needed for killing Tamarin
// in case the program receives a SIGINT signal (sent by Ctrl+C).
void registerSIGINTHandler(const string& process_name){
  if(process_name.find('/') != string::npos){
    tamarin_process = process_name.substr(process_name.find_last_of('/')+1);
  } else {
    tamarin_process = process_name;
  }
  default_sigint_handler = std::signal(SIGINT, sigint_handler);
}


int main (int argc, char *argv[])
{
  CmdParameters parameters;

  CLI::App app{
    "UT Tamarin is a small tool that runs the Tamarin prover on selected\n"
    "lemmas and outputs statistics." 
  };
  
  parameters.spthy_file_path = "";
  app.add_option("spthy_file", parameters.spthy_file_path,
                 "Path to a .spthy file containing a Tamarin theory."
                )->required()->check(CLI::ExistingFile);

  parameters.config_file_path = "";
  app.add_option("-c,--config_file", parameters.config_file_path,
                 "Configuration file for UT Tamarin."
                )->check(CLI::ExistingFile);

  parameters.starting_lemma = "";
  app.add_option("-s,--start", parameters.starting_lemma,
                 "Name of the first lemma that should be verified."
                );

  parameters.tamarin_path = "tamarin-prover";
  app.add_option("--tamarin_path", parameters.tamarin_path,
                 "Path to the Tamarin executable (default: tamarin-prover)."
                );
  
  parameters.penetration_lemma = "";
  app.add_option("--penetrate", parameters.penetration_lemma,
                 "Name of a lemma that should be proved."
                );

  parameters.generate_lemma_file = false;
  app.add_flag("-g,--generate_lemmas", 
               parameters.generate_lemma_file,
               "Tells the tool to output all lemmas specified in the "
               ".spthy file.");

  parameters.timeout = 600;
  app.add_option("-t,--timeout", parameters.timeout,
                 "Per-lemma timeout in seconds "
                 "(0 means no timeout, default: 600 seconds).");

  parameters.abort_after_failure = true;
  app.add_flag("-a,--abort_after_failure", 
               parameters.abort_after_failure,
               "Tells the tool to abort if Tamarin fails to prove a lemma.");

  CLI11_PARSE(app, argc, argv);
  
  registerSIGINTHandler(parameters.tamarin_path);

  if(parameters.generate_lemma_file){
    return printLemmaNames(parameters, cout);
  } else if(parameters.penetration_lemma != ""){
    return penetrateLemma(parameters, cout);
  } else {
    return runTamarinOnLemmas(parameters, cout);
  }
}
