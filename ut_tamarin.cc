#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <csignal>
#include "third-party/cli11/CLI11.hpp"

using std::string;
using std::to_string;
using std::vector;
using std::unordered_map;
using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::ifstream;
using std::istream;
using std::ostream;

const string kTempfileName = "/tmp/uttamarintemp.ut";

struct CmdParameters{
  string tamarin_path;
  string spthy_file_path;
  string whitelist_path;
  string blacklist_path;
  string starting_lemma;
  string penetration_lemma;
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
  else result_string = "timeout";
  if(is_colorized){
    string prefix = "\033[";
    string color_code = prover_result == ProverResult::True ? "32" :
                        prover_result == ProverResult::False ? "31" : "33";
    result_string = prefix + color_code + "m" + result_string + "\033[m";
  }
  return result_string;
}

string durationToString(int duration){
  return to_string(duration) + " second" + (duration != 1 ? "s" : "");
}

string to_string(const TamarinOutput& tamarin_output, 
                 bool is_colorized=false) {
  string formatted = to_string(tamarin_output.result, is_colorized);
  formatted += " (" + durationToString(tamarin_output.duration) + ")";
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


// Takes as input a vector of lemmas and the path to a file containing a
// whitelist of lemmas (one per line) and removes from the vector all lemmas
// that do not occur on the whitelist.
vector<string> getLemmasInWhitelist(const vector<string>& all_lemmas, 
                                    const string& whitelist_path){
  auto whitelist = 
    readLemmaNamesFromLemmaFile(whitelist_path);
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

// Takes as input a vector of lemmas and the path to a file containing a
// blacklist of lemmas (one per line) and removes from the vector all lemmas
// occurring on the blacklist.
vector<string> removeLemmasInBlacklist(const vector<string>& all_lemmas, 
                                       const string& blacklist_path){
  auto filtered_lemmas = all_lemmas;
  auto blacklist = readLemmaNamesFromLemmaFile(blacklist_path);
  filtered_lemmas.erase(
      std::remove_if(filtered_lemmas.begin(), filtered_lemmas.end(),
      [&](const string& lemma_name){ 
        return std::find(blacklist.begin(), blacklist.end(), lemma_name) 
               != blacklist.end();
      }),
      filtered_lemmas.end());
  return filtered_lemmas;
}

// Takes as input a vector of lemmas and a lemma name. Removes from the vector
// all lemmas that occur before the lemma with the given name (in fact, removes
// all lemmas before the first lemma whose name starts with the starting lemma
// name).
vector<string> removeLemmasBeforeStart(const vector<string>& all_lemmas, 
                                       const string& starting_lemma){
  auto it_start = 
    std::find_if(all_lemmas.begin(), all_lemmas.end(), 
        [&starting_lemma](const string& lemma){ 
          return lemma.size() >= starting_lemma.size() &&
                 lemma.substr(0, starting_lemma.size()) == starting_lemma;
        });
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
                                        const string& whitelist_path = "",
                                        const string& blacklist_path = "",
                                        const string& starting_lemma = ""){
  auto lemmas = readLemmaNamesFromSpthyFile(tamarin_path, spthy_file_path);
  if(whitelist_path != ""){
    lemmas = getLemmasInWhitelist(lemmas, whitelist_path);
  }
  if(blacklist_path != ""){
    lemmas = removeLemmasInBlacklist(lemmas, blacklist_path); 
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

// Runs Tamarin on lemmas in the given spthy file. The actual choice
// of lemmas depends on the command-line parameters.
int runTamarinOnLemmas(const CmdParameters& parameters, 
                       ostream& output_stream){
  printHeader(parameters, output_stream);

  auto lemma_names = getNamesOfLemmasToVerify(parameters.tamarin_path,
                                              parameters.spthy_file_path,
                                              parameters.whitelist_path,
                                              parameters.blacklist_path,
                                              parameters.starting_lemma);
  output_stream << endl;

  unordered_map<ProverResult, int> count;
  int overall_duration = 0;
  for(int i=0;i < lemma_names.size();i++){
    output_stream << lemma_names[i] << " (" << (i+1) << "/" << 
                     lemma_names.size() << ") " << flush;
    auto stats = processTamarinLemma(parameters.tamarin_path, 
        parameters.spthy_file_path, 
        lemma_names[i], parameters.timeout);
    output_stream << to_string(stats, &output_stream == &cout) << endl;
    overall_duration += stats.duration;
    count[stats.result]++;
    if(!parameters.continue_after_failure && 
       stats.result != ProverResult::True) break;
  }
  output_stream << endl << "Summary: " << endl;
  output_stream << to_string(ProverResult::True) << ": " <<
    count[ProverResult::True] << ", " << to_string(ProverResult::False) << ": "
    << count[ProverResult::False] << ", " << to_string(ProverResult::Unknown)
    << ": " << count[ProverResult::Unknown] << endl;

  output_stream << "Overall duration: " << durationToString(overall_duration) 
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
    "' with a per-heuristic timeout of " << durationToString(p.timeout) <<
    "." << endl << endl;

  vector<string> heuristics = {"S", "C", "I", "s", "c", "i", "P", "p"};
  for(auto heuristic : heuristics){
    output_stream << "Heuristic: " << heuristic << " " << flush;
    auto output = processTamarinLemma(p.tamarin_path, p.spthy_file_path, 
        penetration_lemma, p.timeout, "--heuristic=" + heuristic);
    output_stream << to_string(output.result, &output_stream == &cout) 
      << " (" << durationToString(output.duration) << ")" << endl;
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

  parameters.whitelist_path = "";
  app.add_option("-w,--whitelist", parameters.whitelist_path,
                 "Lemma whitelist file (one lemma per line)."
                )->check(CLI::ExistingFile);

  parameters.blacklist_path = "";
  app.add_option("-b,--blacklist", parameters.blacklist_path,
                 "Lemma blacklist file (one lemma per line)."
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

  parameters.continue_after_failure = false;
  app.add_flag("-c,--continue_after_failure", 
               parameters.continue_after_failure,
               "Tells the tool to continue if Tamarin fails to prove a lemma.");

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
