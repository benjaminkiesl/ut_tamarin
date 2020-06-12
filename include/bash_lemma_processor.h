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

#ifndef UT_TAMARIN_BASH_LEMMA_PROCESSOR_H_
#define UT_TAMARIN_BASH_LEMMA_PROCESSOR_H_

#include "lemma_processor.h"

#include <istream>
#include <string>

#include "tamarin_interface.h"

namespace uttamarin {

class BashLemmaProcessor : public LemmaProcessor {
 public:
  BashLemmaProcessor(const std::string& spthy_file_path,
                     const std::string& tamarin_path="tamarin-prover",
                     const std::string& proof_directory="",
                     const int timeout=600);

  // Takes as input a  lemma name, the command line parameters, and dedicated
  // arguments for Tamarin and then runs Tamarin on the given lemma. Returns
  // some output/statistics (like Tamarin's result and the execution duration).
  virtual TamarinOutput ProcessLemma(const std::string& lemma_name);

 private:

  // Takes as input a stream of Tamarin output and the name of a lemma and
  // returns the result ("verified", "falsified", "analysis incomplete").
  ProverResult ExtractResultForLemma(std::istream& stream_of_tamarin_output,
                                     const std::string& lemma_name);

  // Takes as input a stream to output produced by Tamarin and moves the stream
  // to the first line of the part where the results for lemmas are displayed.
  void MoveTamarinStreamToLemmaNames(std::istream& tamarin_stream);

  // Takes as input a line of the Tamarin output (a line that shows the Tamarin
  // result for a particular lemma) and returns the name of the lemma.
  std::string ExtractLemmaName(std::string tamarin_line);

  std::string spthy_file_path_;
  std::string tamarin_path_;
  std::string proof_directory_;
  int timeout_;
};

} // namespace uttamarin

#endif
