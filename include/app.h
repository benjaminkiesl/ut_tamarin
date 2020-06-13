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

#ifndef UT_TAMARIN_APP_H_ 
#define UT_TAMARIN_APP_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace uttamarin {

class LemmaProcessor;

struct TamarinConfig;
struct TamarinOutput;

enum class ProverResult;

struct CmdParameters { 
  std::string tamarin_path;
  std::string spthy_file_path; 
  std::string config_file_path; 
  std::string output_file_path; 
  std::string starting_lemma; 
  std::string penetration_lemma; 
  std::string proof_directory; 
  int timeout; 
  bool abort_after_failure; 
};

class App {

 public:
  App(std::unique_ptr<LemmaProcessor> lemma_processor);
  ~App();

  // Executes Tamarin with the given parameters and writes the needed output to
  // a temp file while relocating the other output to /dev/null.  Returns the
  // path to the resulting temp file.
  std::string RunTamarinAndWriteOutputToNewTempfile(
      const std::string& tamarin_path,
      const std::string& spthy_file_path,
      const std::string& tamarin_parameters="");

  // Takes as input a  lemma name, the command line parameters, and dedicated
  // arguments for Tamarin and then runs Tamarin on the given lemma. Returns
  // some output/statistics (like Tamarin's result and the execution duration).
  TamarinOutput ProcessTamarinLemma(const std::string& lemma_name,
                                    const std::string& spthy_file_path,
                                    const CmdParameters& parameters,
                                    std::string tamarin_args="");

  // Takes as input a line of the Tamarin output (a line that shows the Tamarin
  // result for a particular lemma) and returns the name of the lemma.
  std::string ExtractLemmaName(std::string tamarin_line);

  // Takes as input a stream that points to a list of lemma names (one lemma
  // name per line) and returns these lemma names in a vector.
  std::vector<std::string> ReadLemmaNamesFromStream(std::istream& input_stream);

  // Takes a file that contains lemma names (one per line) and returns a vector
  // with all the lemma names.
  std::vector<std::string> ReadLemmaNamesFromLemmaFile(
      const std::string& file_name);

  // Takes as input a stream to output produced by Tamarin and moves the stream
  // to the first line of the part where the results for lemmas are displayed.
  void MoveTamarinStreamToLemmaNames(std::istream& tamarin_stream);

  // Takes as input a Tamarin theory file (".spthy") and returns a vector
  // containing all the names of the lemmas specified in the file.
  std::vector<std::string> ReadLemmaNamesFromSpthyFile(
      const std::string& tamarin_path, 
      const std::string& spthy_file_path);

  // Takes as input a stream of Tamarin output and the name of a lemma and
  // returns the result ("verified", "falsified", "analysis incomplete").
  ProverResult ExtractResultForLemma(std::istream& stream_of_tamarin_output,
                                     const std::string& lemma_name);

  // Takes as input two vectors of lemmas (the initial lemmas and the
  // "allow list", respectively) and removes from the initial lemmas all lemmas
  // that do not occur in the allow list.
  std::vector<std::string> GetLemmasInAllowList(
      const std::vector<std::string>& all_lemmas, 
      const std::vector<std::string>& allow_list);

  // Takes as input two vectors of lemmas (the initial lemmas and the
  // deny list, respectively) and removes from the initial lemmas all lemmas
  // that occur in the deny list.
  std::vector<std::string> RemoveLemmasInDenyList(
      const std::vector<std::string>& all_lemmas,
      const std::vector<std::string>& deny_list);

  // Takes as input a vector of lemmas and a lemma name. Removes from the
  // vector all lemmas that occur before the lemma with the given name (in
  // fact, removes all lemmas before the lemma whose name is closest to
  // 'starting_lemma').
  std::vector<std::string> RemoveLemmasBeforeStart(
      const std::vector<std::string>& all_lemmas,
      const std::string& starting_lemma);

  // Takes the path to a Tamarin theory file (".spthy") and a lemma file (i.e.,
  // a file with a list of lemma names, one per line) and determines which
  // lemmas should be verified (namely those that are both in the lemma file
  // and the Tamarin file). If a lemma name is in the lemma file but not in the
  // Tamarin file, a warning message is printed.
  std::vector<std::string> GetNamesOfLemmasToVerify(
      const std::string& tamarin_path, 
      const std::string& spthy_file_path, 
      const std::vector<std::string>& allow_list,
      const std::vector<std::string>& deny_list,
      const std::string& starting_lemma = "");

  // Prints the header for the Tamarin output based on the given command line
  // parameters. The header is printed to the given ostream output_stream.
  void PrintHeader(std::ostream& output_stream, const CmdParameters& parameters);

  // Prints the footer for the Tamarin output based on the given command line
  // parameters. The header is printed to the given ostream output_stream.
  void PrintFooter(std::ostream& output_stream,
                   int true_lemmas, int false_lemmas,
                   int unknown_lemmas, int overall_duration);

  // Takes a string 'prefix' and a string 'original' and returns a command that
  // tells the tool M4 to prepend 'prefix' to 'original'
  std::string AddPrefixViaM4(const std::string& prefix,
                             const std::string& original);

  // Takes as input the name of a lemma and a tamarin configuration and returns
  // a list of commands for the tool M4. This commands tell M4 to rename
  // certain fact symbols within the Tamarin theory file in order to apply
  // custom heuristics
  std::vector<std::string> GetM4Commands(const std::string& lemma_name, 
                                         const TamarinConfig& config);

  // Takes as input a Tamarin theory file and a lemma name and applies the
  // custom heuristics defined in the Tamarin config. Returns the path of the
  // temp file created by applying the custom heuristics to the Tamarin theory
  // file.
  std::string ApplyCustomHeuristics(const std::string& spthy_file_path, 
                                    const std::string& lemma_name, 
                                    const TamarinConfig& config, 
                                    const CmdParameters& cmd_parameters);

  // Runs Tamarin on lemmas in the given spthy file. The actual choice of
  // lemmas depends on the command-line parameters.  Returns true if Tamarin is
  // able to prove all lemmas.
  bool RunTamarinOnLemmas(const CmdParameters& parameters, 
                          std::ostream& output_stream);

  // Runs Tamarin on the given lemma, trying out all different heuristics until
  // either Tamarin succeeds or all strategies reach a timeout.
  int PenetrateLemma(const CmdParameters& p, std::ostream& output_stream);

 private:
  std::unique_ptr<LemmaProcessor> lemma_processor_;

};

} // namespace uttamarin

#endif
