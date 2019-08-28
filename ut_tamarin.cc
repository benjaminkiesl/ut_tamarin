#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <csignal>
#include "third-party/cli11/CLI11.hpp"

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

const string kTempfileName = ".utemp";

struct CmdParameters{
  string tamarin_path;
  string spthy_file_path;
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

string to_string(const ProverResult& prover_result,
                 bool is_colorized=false) {
  string result_string;
  if(prover_result == ProverResult::True) result_string = "verified";
  else if(prover_result == ProverResult::False) result_string = "false";
  else result_string = "analysis incomplete";
  if(is_colorized){
    string prefix = "\033[";
    string color_code = prover_result == ProverResult::True ? "32" :
                        prover_result == ProverResult::False ? "31" : "33";
    result_string = prefix + color_code + "m" + result_string + "\033[m";
  }
  return result_string;
}

string to_string(const TamarinOutput& tamarin_output, 
                 bool is_colorized=false) {
  string formatted = to_string(tamarin_output.result, is_colorized);
  formatted += " (" + to_string(tamarin_output.duration) + " second"
               + (tamarin_output.duration != 1 ? "s" : "") + ")";
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
// Returns the path of the resulting temp file.
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
  ifstream file_stream {temp_file, ifstream::in};
  moveTamarinStreamToLemmaNames(file_stream);
  auto lemma_names = readLemmaNames(file_stream);
  std::remove(temp_file.c_str());
  std::remove("halama.txt");
  return lemma_names;
}

// Takes as input a stream of Tamarin output and the name of a lemma and
// returns the result ("verified", "falsified", "analysis incomplete").
ProverResult extractResultForLemma(istream& stream_of_tamarin_output,
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
                                  const string& lemma_name, int timeout){
  string cmd = "";
  if(timeout > 0) cmd += "timeout " + to_string(timeout) + " ";
  cmd += tamarin_path + " --prove=" + lemma_name + " " + spthy_file_path 
       + " 1> " + kTempfileName + " 2> /dev/null";

  TamarinOutput tamarin_output;
  tamarin_output.duration = executeShellCommand(cmd);

  ifstream file_stream {kTempfileName, ifstream::in};
  tamarin_output.result = extractResultForLemma(file_stream, lemma_name);

  std::remove(kTempfileName.c_str());
  return tamarin_output;
}

// Takes as input a Tamarin theory file (".spthy"), a list of lemmas, 
// a timeout, and an output stream and then runs Tamarin on all the 
// specified lemmas. The timeout is a per-lemma timeout. The results are 
// written to the given output stream.
void processTamarinLemmas(const string& tamarin_path,
                          const string& spthy_file_path,
                          const vector<string>& lemma_names,
                          int timeout, bool continue_after_failure, 
                          ostream& output_stream){
  int overall_duration = 0;
  for(int i=0;i < lemma_names.size();i++){
    output_stream << lemma_names[i] << " (" << (i+1) << "/" << 
                     lemma_names.size() << ") " << flush;
    auto stats = processTamarinLemma(tamarin_path, spthy_file_path, 
        lemma_names[i], timeout);
    output_stream << to_string(stats, &output_stream == &cout) << endl;
    overall_duration += stats.duration;
    if(!continue_after_failure && stats.result != ProverResult::True) break;
  }
  output_stream << endl << "Overall duration: " << overall_duration << 
    " second" << (overall_duration > 1 ? "s" : "") << endl;
}

// Runs the tool in the mode where a Tamarin theory file (".spthy") is read 
// and a list of the lemmas occurring in that file is written to a file.
int printLemmaNames(const CmdParameters& parameters, ostream& output_stream){
  auto lemma_names = readLemmaNamesFromSpthyFile(parameters.tamarin_path,
      parameters.spthy_file_path);
  writeLemmaNames(lemma_names, output_stream); 
  return 0;
}

// Takes the path of a Tamarin theory file (".spthy") and a lemma file
// (i.e., a file with a list of lemma names, one per line) and determines 
// which lemmas should be verified (namely those that are both in the lemma 
// file and the Tamarin file). If a lemma name is in the lemma file but not 
// in the Tamarin file, a warning message is printed.
vector<string> getNamesOfLemmasToVerify(const string& tamarin_path,
                                        const string& spthy_file_path,
                                        const string& lemma_file_path){
  vector<string> lemma_names;
  auto lemma_names_in_spthy_file = 
    readLemmaNamesFromSpthyFile(tamarin_path, spthy_file_path);
  if(lemma_file_path != ""){
    auto lemma_names_in_lemma_file = 
      readLemmaNamesFromLemmaFile(lemma_file_path);
    for(auto lemma_name : lemma_names_in_lemma_file){
      if(std::find(lemma_names_in_spthy_file.begin(),
                   lemma_names_in_spthy_file.end(), lemma_name) ==
                   lemma_names_in_spthy_file.end()){
        cerr << "Warning: lemma '" << lemma_name << "' is not declared in " <<
          "the file '" <<  spthy_file_path << "'." << endl;
      } else {
        lemma_names.push_back(lemma_name);
      }
    }
  } else {
    lemma_names = lemma_names_in_spthy_file;
  }
  return lemma_names;
}

// Runs Tamarin on lemmas. If the parameter 'lemma_names_path' is set
// Tamarin runs on all lemmas specified in the Tamarin theory file at
// 'spthy_file_path', otherwise it runs only on the lemmas that are listed
// in the file at 'lemma_names_path'. Prints statistics either to a file 
// (if 'output_path' is set) or to the standard output.
int runTamarinOnLemmas(const CmdParameters& parameters, 
                       ostream& output_stream){
  auto file_name = parameters.spthy_file_path;
  if(file_name.find('/') != string::npos)
    file_name = file_name.substr(file_name.find_last_of('/') + 1);

  output_stream << "Tamarin Tests for file '" << file_name << "':" << endl;
  output_stream << "Timeout: " << (parameters.timeout <= 0 ? "no timeout" : 
                   (to_string(parameters.timeout) + " second" + 
                   (parameters.timeout > 1 ? "s" : ""))) << " per lemma" 
                    << endl;

  auto lemma_names = getNamesOfLemmasToVerify(parameters.tamarin_path,
                                              parameters.spthy_file_path,
                                              parameters.lemma_names_path);
  output_stream << endl;
  processTamarinLemmas(parameters.tamarin_path,
      parameters.spthy_file_path, lemma_names, parameters.timeout, 
      parameters.continue_after_failure, output_stream);

  return 0;
}

void (*default_signal_handler)(int signal);
string tamarin_process = "tamarin-prover";

void signal_handler(int signal)
{
  std::system(("killall " + tamarin_process + " 2> /dev/null").c_str());
  std::remove(kTempfileName.c_str());
  std::signal(signal, default_signal_handler);
  std::raise(signal);
}

int main (int argc, char *argv[])
{
  CmdParameters parameters;

  CLI::App app{
    "UT Tamarin is a small tool that runs the Tamarin prover on selected\n"
    "files and lemmas and outputs statistics." 
  };
  
  parameters.spthy_file_path = "";
  app.add_option("spthy_file_path", parameters.spthy_file_path,
                 "Path to a .spthy file containing a Tamarin theory."
                )->required()->check(CLI::ExistingFile);

  parameters.lemma_names_path = "";
  app.add_option("-l,--lemma_file", parameters.lemma_names_path,
                 "Path to a file containing the names of the lemmas that "
                 "should be verified (one name per line). If not specified, "
                 "all lemmas are verified."
                )->check(CLI::ExistingFile);

  parameters.tamarin_path = "tamarin-prover";
  app.add_option("-p,--tamarin_path", parameters.tamarin_path,
                 "Path to the Tamarin executable. If not specified, Tamarin "
                 "is called with the command 'tamarin-prover'."
                );

  parameters.generate_lemma_file = false;
  app.add_flag("-g,--generate_lemmas", 
               parameters.generate_lemma_file,
               "If set, the tool just outputs a list of all lemmas that are "
               "specified in the given input theory file");

  parameters.timeout = 600;
  app.add_option("-t,--timeout", parameters.timeout,
                 "Timeout in seconds (per lemma) for Tamarin. If set "
                 "to 0, no timeout is used. By default, the timeout "
                 "is 600 seconds.");

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
  
  tamarin_process = parameters.tamarin_path;
  if(tamarin_process.find('/') != string::npos){
    tamarin_process = tamarin_process.substr(
        tamarin_process.find_last_of('/')+1);
  }

  default_signal_handler = std::signal(SIGINT, signal_handler);

  unique_ptr<ostream> output_file_stream = parameters.output_path == "" ? 
    nullptr : make_unique<ofstream>(parameters.output_path, ofstream::out);
  ostream& output_stream = output_file_stream ? *output_file_stream : cout;

  if(!parameters.generate_lemma_file){ 
    return runTamarinOnLemmas(parameters, output_stream);
  } else {
    return printLemmaNames(parameters, output_stream);
  }
}
