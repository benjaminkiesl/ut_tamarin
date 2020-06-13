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
class TheoryPreprocessor;

struct UtTamarinConfig;
struct TamarinOutput;

enum class ProverResult;

struct CmdParameters { 
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
  App(std::unique_ptr<LemmaProcessor> lemma_processor,
      std::unique_ptr<TheoryPreprocessor> theory_preprocessor,
      std::shared_ptr<UtTamarinConfig> config);

  ~App();

  // Runs Tamarin on lemmas in the given spthy file. The actual choice of
  // lemmas depends on the command-line parameters. Returns true if Tamarin is
  // able to prove all lemmas.
  bool RunTamarinOnLemmas(const CmdParameters& parameters, 
                          std::ostream& output_stream);

 private:

  // Prints the header for the Tamarin output based on the given command line
  // parameters. The header is printed to the given ostream output_stream.
  void PrintHeader(std::ostream& output_stream, const CmdParameters& parameters);

  // Prints the footer for the Tamarin output based on the given command line
  // parameters. The header is printed to the given ostream output_stream.
  void PrintFooter(std::ostream& output_stream,
                   int true_lemmas, int false_lemmas,
                   int unknown_lemmas, int overall_duration);

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
          const std::string& spthy_file_path,
          const std::vector<std::string>& allow_list,
          const std::vector<std::string>& deny_list,
          const std::string& starting_lemma = "");

  std::unique_ptr<LemmaProcessor> lemma_processor_;
  std::unique_ptr<TheoryPreprocessor> theory_preprocessor_;
  std::shared_ptr<UtTamarinConfig> config_;
};

} // namespace uttamarin

#endif
