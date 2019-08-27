#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <memory>
#include "cli11/CLI11.hpp"

using std::string;
using std::to_string;
using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::vector;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::unique_ptr;
using std::make_unique;

struct CmdParameters{
  string input_theory_path;
  string lemma_names_path;
  string output_path;
  int timeout;
  bool continue_after_failure;
  bool generate_lemma_file;
};

enum class ProverResult {True, False, Unknown};

struct TamarinOutput{
  ProverResult result;
  int duration; // in seconds
};

string to_string(const ProverResult& prover_result) {
  switch(prover_result)
  {
    case ProverResult::True: return "verified";
    case ProverResult::False: return "FALSE";
    default: return "ANALYSIS INCOMPLETE";
  }
}

string to_string(const TamarinOutput& tamarin_output) {
  string formatted = to_string(tamarin_output.result);
  formatted += " (duration: " + to_string(tamarin_output.duration) + " second"
               + (tamarin_output.duration != 1 ? "s" : "") + ")";
  return formatted;
}

// Executes a shell command and returns the duration of the 
// execution in seconds.
int executeShellCommand(const string& cmd){
  auto start_time = std::chrono::high_resolution_clock::now();
  std::system(cmd.c_str());
  auto end_time = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>
                            (end_time - start_time).count();
}

// Executes Tamarin with the given parameters and writes the needed output
// to a temp file while relocating the other output to /dev/null. 
// Returns the path of the resulting temp file.
string runTamarinAndWriteOutputToNewTempfile(const string& tamarin_file_path,
                                    const string& tamarin_parameters=""){
  string temp_file = ".temputm";
  executeShellCommand("tamarin-prover " + tamarin_parameters + 
      tamarin_file_path + " 1> " + temp_file + " 2> /dev/null");
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

// Takes as input a stream that points to a list of lemma names (one 
// lemma name per line) and returns these lemma names in a vector.
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
vector<string> readLemmaNamesFromTamarin(const string& tamarin_file_name){
  auto temp_file = runTamarinAndWriteOutputToNewTempfile(tamarin_file_name);
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
TamarinOutput processTamarinLemma(const string& tamarin_file,
                                  const string& lemma_name, int timeout){
  string temp_file = ".temptam";
  string cmd = "";
  if(timeout > 0) cmd += "timeout " + to_string(timeout) + " ";
  cmd += "tamarin-prover --prove=" + lemma_name + " " + tamarin_file 
       + " 1> " + temp_file + " 2> /dev/null";

  TamarinOutput tamarin_output;
  tamarin_output.duration = executeShellCommand(cmd);

  ifstream file_stream {temp_file, ifstream::in};
  tamarin_output.result = extractResultForLemma(file_stream, lemma_name);

  std::remove(temp_file.c_str());
  return tamarin_output;
}

// Takes as input a Tamarin theory file (".spthy"), a list of lemmas, 
// a timeout, and an output stream and then runs Tamarin on all the 
// specified lemmas. The timeout is a per-lemma timeout. The results are 
// written to the given output stream.
void processTamarinLemmas(const string& tamarin_file,
                           const vector<string>& lemma_names,
                           int timeout, ostream& output_stream,
                           bool continue_after_failure){
  int overall_duration = 0;
  for(int i=0;i < lemma_names.size();i++){
    output_stream << "lemma " << lemma_names[i] << " (" << (i+1) << "/" << 
                     lemma_names.size() << ") " << flush;
    auto stats = processTamarinLemma(tamarin_file, lemma_names[i], timeout);
    output_stream << to_string(stats) << endl;
    overall_duration += stats.duration;
    if(!continue_after_failure && stats.result != ProverResult::True) break;
  }
  output_stream << endl << "Overall duration: " << overall_duration << 
    " second" << (overall_duration > 1 ? "s" : "") << endl;
}

// Runs the tool in the mode where a Tamarin theory file (".spthy") is read 
// and a list of the lemmas occurring in that file is written to a file.
int printLemmaNames(const CmdParameters& parameters){
  auto lemma_names = readLemmaNamesFromTamarin(parameters.input_theory_path);
  if(parameters.output_path != ""){
    ofstream output_file_stream(parameters.output_path, std::ofstream::out);
    writeLemmaNames(lemma_names, output_file_stream); 
  } else {
    writeLemmaNames(lemma_names, cout);
  }
  return 0;
}

// Takes the path of a Tamarin theory file (".spthy") and a lemma file
// (i.e., a file with a list of lemma names, one per line) and determines 
// which lemmas should be verified (namely those that are both in the lemma 
// file and the Tamarin file). If a lemma name is in the lemma file but not 
// in the Tamarin file, a warning message is printed.
vector<string> getNamesOfLemmasToVerify(const string& tamarin_file_path,
                                        const string& lemma_file_path){
  vector<string> lemma_names;
  auto lemma_names_in_tamarin_file = 
    readLemmaNamesFromTamarin(tamarin_file_path);
  if(lemma_file_path != ""){
    auto lemma_names_in_lemma_file = 
      readLemmaNamesFromLemmaFile(lemma_file_path);
    for(auto lemma_name : lemma_names_in_lemma_file){
      if(std::find(lemma_names_in_tamarin_file.begin(),
                   lemma_names_in_tamarin_file.end(), lemma_name) ==
                   lemma_names_in_tamarin_file.end()){
        cerr << "Warning: lemma '" << lemma_name << "' is not declared in " <<
          "the file '" << tamarin_file_path << "'." << endl;
      } else {
        lemma_names.push_back(lemma_name);
      }
    }
  } else {
    lemma_names = lemma_names_in_tamarin_file;
  }
  return lemma_names;
}

// Runs Tamarin on lemmas. If the parameter 'lemma_names_path' is set
// Tamarin runs on all lemmas specified in the Tamarin theory file 
// 'input_theory_path', otherwise it runs only on the lemmas that are listed
// in the file at 'lemma_names_path'. Prints statistics either to a file 
// (if 'output_path' is set) or to the standard output.
int runTamarinOnLemmas(const CmdParameters& parameters){
  unique_ptr<ostream> output_file_stream = parameters.output_path == "" ? 
    nullptr : make_unique<ofstream>(parameters.output_path, ofstream::out);
  ostream& output_stream = output_file_stream ? *output_file_stream : cout;

  auto file_name = parameters.input_theory_path;
  if(file_name.find('/') != string::npos)
    file_name = file_name.substr(file_name.find_last_of('/') + 1);

  output_stream << "Tamarin Tests for file '" << file_name << "':" << endl;
  output_stream << "Timeout: " << (parameters.timeout <= 0 ? "no timeout" : 
                   (to_string(parameters.timeout) + " second" + 
                   (parameters.timeout > 1 ? "s" : ""))) << " per lemma" 
                    << endl;

  auto lemma_names = getNamesOfLemmasToVerify(parameters.input_theory_path,
                                              parameters.lemma_names_path);
  output_stream << endl;
  processTamarinLemmas(parameters.input_theory_path, lemma_names, 
      parameters.timeout, output_stream, parameters.continue_after_failure);

  return 0;
}

int main (int argc, char *argv[])
{
  CmdParameters parameters;

  CLI::App app{
    "UT Tamarin is a small tool that runs the Tamarin prover on selected\n"
    "files and lemmas and outputs statistics." 
  };
  
  parameters.input_theory_path = "";
  app.add_option("input_theory_path", parameters.input_theory_path,
                 "Path to a file containing a Tamarin theory."
                )->required()->check(CLI::ExistingFile);

  parameters.lemma_names_path = "";
  app.add_option("-l,--lemma_file", parameters.lemma_names_path,
                 "Path to a file containing the names of the lemmas that "
                 "should be verified (one name per line). If not specified, "
                 "all lemmas are verified."
                )->check(CLI::ExistingFile);

  parameters.generate_lemma_file = false;
  app.add_flag("-g,--generate_lemmas", 
               parameters.generate_lemma_file,
               "If set, the tool just outputs a list of all lemmas that are "
               "specified in the given input theory file");

  parameters.timeout = 0;
  app.add_option("-t,--timeout", parameters.timeout,
                 "Timeout in seconds (per lemma) for Tamarin. If not "
                 "specified, no timeout is used.");

  parameters.continue_after_failure = false;
  app.add_flag("-c,--continue_after_failure", 
               parameters.continue_after_failure,
               "If set, the test procedure continues in case Tamarin "
               "fails to prove a lemma (due to a timeout or a false lemma), "
               "otherwise Tamarin terminates after failure.");

  parameters.output_path = "";
  app.add_option("-o,--output_file", parameters.output_path,
                 "Optional file to which the results should be printed. If "
                 "not specified, results are written to the standard output.")
                 ->type_name("FILE");

  CLI11_PARSE(app, argc, argv);

  if(!parameters.generate_lemma_file) return runTamarinOnLemmas(parameters);
  else return printLemmaNames(parameters);
}
