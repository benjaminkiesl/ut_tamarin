#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include "cli11/CLI11.hpp"

using std::string;
using std::to_string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;

struct CmdParameters{
  string input_theory_path;
  string lemma_names_path;
  string output_path;
  int timeout;
  bool continue_after_failure;
};

enum ProverResult {kTrue, kFalse, kUnknown};

struct TamarinOutput{
  ProverResult result;
  int duration; // in seconds
};

// Returns the duration of the execution in seconds
int executeShellCommand(const string& cmd){
  auto start_time = std::chrono::high_resolution_clock::now();
  std::system(cmd.c_str());
  auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>
                            (end_time - start_time).count();
}

// Executes Tamarin with the given parameters and writes the output
// to a temp file. Returns the path of the temp file.
string writeTamarinOutputToTempfile(const string& tamarin_file_path,
                                    const string& tamarin_parameters=""){
  string temp_file = ".temputm";
  executeShellCommand("tamarin-prover " + tamarin_parameters + 
                      tamarin_file_path + " > " + temp_file);
  return temp_file;
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

// Takes as input a stream that contains a list of lemma names (one lemma name
// per line) and returns these lemma names in a vector.
vector<string> readLemmaNames(istream& input_stream){
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
  return readLemmaNames(file_stream);
}

// Takes as input a stream to output produced by Tamarin and moves the stream
// to the first line of the part where the results for lemmas are displayed.
void moveTamarinOutputStreamToLemmaNames(istream& output_stream){
  string line;
  while(std::getline(output_stream, line) && line.substr(0,5) != "=====");
  for(int i=1;i<=4;i++) std::getline(output_stream, line);
}

// Takes as input a Tamarin theory file (".spthy") and returns a vector
// containing all the names of the lemmas specified in the file.
vector<string> readLemmaNamesFromTamarin(const string& file_name){
  auto temp_file = writeTamarinOutputToTempfile(file_name);
  ifstream file_stream {temp_file, ifstream::in};
  moveTamarinOutputStreamToLemmaNames(file_stream);
  auto lemma_names = readLemmaNames(file_stream);
  std::remove(temp_file.c_str());
  return lemma_names;
}

// Takes as input a stream of Tamarin output and the name of a lemma and
// returns the result ("verified", "falsified", "analysis incomplete").
ProverResult extractResultForLemma(istream& stream_of_tamarin_output,
                                   const string& lemma_name){
  string line;
  moveTamarinOutputStreamToLemmaNames(stream_of_tamarin_output);
  while(getline(stream_of_tamarin_output, line) 
        && getLemmaName(line) != lemma_name);

  if(line.find("falsified") != string::npos)
    return kFalse;
  else if(line.find("verified") != string::npos)
    return kTrue;
  return kUnknown;
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
// and a timeout and then runs Tamarin on the given lemma. Returns some
// output/statistics (like Tamarin's result or the duration).
TamarinOutput processTamarinLemma(const string& tamarin_file,
                                  const string& lemma_name, int timeout){
  string temp_file = ".temptam";
  string cmd = "";
  if(timeout >= 0) cmd += "timeout " + to_string(timeout) + " ";
  cmd += "tamarin-prover --prove=" + lemma_name + " " + tamarin_file 
       + " > " + temp_file;

  TamarinOutput tamarin_output;
  tamarin_output.duration = executeShellCommand(cmd);

  ifstream file_stream {temp_file, ifstream::in};
  tamarin_output.result = extractResultForLemma(file_stream, lemma_name);

  std::remove(temp_file.c_str());
  return tamarin_output;
}

// Takes as input a TamarinOutput object and transforms it to a readable string.
string to_string(const TamarinOutput& tamarin_output) {
  string formatted = "";
  if(tamarin_output.result == kTrue) formatted += "verified";
  if(tamarin_output.result == kFalse) formatted += "FALSE";
  if(tamarin_output.result == kUnknown) formatted += "UNKNOWN";
  formatted += " (duration: " + to_string(tamarin_output.duration) + " second"
               + (tamarin_output.duration != 1 ? "s" : "") + ")";
  return formatted;
}

// Takes as input a Tamarin theory file (".spthy"), a list of lemmas, a timeout,
// and an outputstream and then runs Tamarin on all the specified lemmas.
// The timeout is a per-lemma timeout. The results are written to the given
// output stream.
void processTamarinLemmas(const string& tamarin_file,
                           const vector<string>& lemma_names,
                           int timeout, ostream& output_stream,
                           bool continue_after_failure){
  vector<string> results;
  for(auto lemma_name : lemma_names){
    auto stats = processTamarinLemma(tamarin_file, lemma_name, timeout);
    results.push_back("lemma " + lemma_name + ": " + to_string(stats));
    if(!continue_after_failure && stats.result != kTrue) break;
  }
  string decoration = "=========================";
  output_stream << decoration << " RESULTS " << decoration << endl;
  output_stream << "Tamarin File: " << tamarin_file << endl;
  output_stream << "Timeout: " << (timeout <= 0 ? "no timeout" : 
                                   to_string(timeout) + " second" +
                                   (timeout > 1 ? "s" : "")) << endl;
  for(auto result : results){
    output_stream << result << endl;
  }
}

// Runs the tool in the mode where a Tamarin theory file (".spthy") is read 
// and a list of the lemmas occurring in that file is written to a file.
int printLemmaNames(const CmdParameters& parameters){
  auto lemma_names = readLemmaNamesFromTamarin(parameters.input_theory_path);
  if(parameters.output_path != ""){
    ofstream output_file_stream (parameters.output_path, std::ofstream::out);
    writeLemmaNames(lemma_names, output_file_stream); 
  } else {
    writeLemmaNames(lemma_names, cout);
  }
  return 0;
}


// Runs the tool in the mode where a Tamarin theory file (".spthy") and a file
// with a list of lemmas are given and Tamarin is then executed on all the
// lemmas. Statistics are then printed to either a file or to standard output.
int runTamarinOnLemmas(const CmdParameters& parameters){
  auto lemma_names = readLemmaNamesFromLemmaFile(parameters.lemma_names_path);
  if(parameters.output_path != ""){
    ofstream ofs (parameters.output_path, std::ofstream::out);
    processTamarinLemmas(parameters.input_theory_path, lemma_names,
                     parameters.timeout, ofs, parameters.continue_after_failure);
  } else {
    processTamarinLemmas(parameters.input_theory_path, lemma_names, 
            parameters.timeout, cout, parameters.continue_after_failure);
  }
  return 0;
}

int main (int argc, char *argv[])
{
  CmdParameters parameters;

  CLI::App app{
    "UT Tamarin is a small tool that runs the Tamarin prover on selected\n"
    "files and lemmas and outputs the result, allowing you to specify a" 
    "per-lemma timeout" 
  };
  
  parameters.input_theory_path = "";
  app.add_option("input_theory_path", parameters.input_theory_path,
                 "Path to a file containing a Tamarin theory."
                )->required()->check(CLI::ExistingFile);

  parameters.lemma_names_path = "";
  app.add_option("-l,--lemma_file", parameters.lemma_names_path,
                 "Path to a file containing the names of the lemmas that "
                 "should be verified (one name per line)."
                )->check(CLI::ExistingFile);

  parameters.timeout = 0;
  app.add_option("-t,--timeout", parameters.timeout,
                 "Timeout (per lemma) for Tamarin. If not specified, "
                 "there is no timeout.");

  parameters.continue_after_failure = false;
  app.add_flag("-c,--continue_after_failure", parameters.continue_after_failure,
               "If set, the test procedure continues if Tamarin "
               "fails to prove a lemma (due to timeout or a false lemma), "
               "otherwise Tamarin terminates.");

  parameters.output_path = "";
  app.add_option("output_file", parameters.output_path,
                 "File to which the results should be printed. If not "
                 "specified, results are written to standard output.")
                 ->type_name("FILE");

  CLI11_PARSE(app, argc, argv);

  if(parameters.lemma_names_path != "") return runTamarinOnLemmas(parameters);
  else return printLemmaNames(parameters);
}
