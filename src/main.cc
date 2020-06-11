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

#include <iostream>
#include <string>

#include "third-party/cli11/CLI11.hpp"

#include "app.h"
#include "terminator.h"

int main (int argc, char *argv[])
{
  ut_tamarin::CmdParameters parameters;

  CLI::App cli{
    "UT Tamarin is a small tool that runs the Tamarin prover on selected\n"
    "lemmas and outputs statistics." 
  };
  
  parameters.spthy_file_path = "";
  cli.add_option("spthy_file", parameters.spthy_file_path,
                 "Path to a .spthy file containing a Tamarin theory."
                )->required()->check(CLI::ExistingFile);

  parameters.config_file_path = "";
  cli.add_option("-c,--config_file", parameters.config_file_path,
                 "Configuration file for UT Tamarin."
                )->check(CLI::ExistingFile);

  parameters.output_file_path = "";
  cli.add_option("-o,--output_file", parameters.output_file_path,
                 "File where UT Tamarin saves its output.");

  parameters.starting_lemma = "";
  cli.add_option("-s,--start", parameters.starting_lemma,
                 "Name of the first lemma that should be verified."
                );

  parameters.tamarin_path = "tamarin-prover";
  cli.add_option("--tamarin_path", parameters.tamarin_path,
                 "Path to the Tamarin executable (default: tamarin-prover)."
                );

  parameters.proof_directory = "";
  cli.add_option("--proof_directory", parameters.proof_directory,
                 "Directory where the proofs should be stored."
                );
  
  parameters.penetration_lemma = "";
  cli.add_option("--penetrate", parameters.penetration_lemma,
                 "Name of a lemma that should be proved."
                );

  parameters.generate_lemma_file = false;
  cli.add_flag("-g,--generate_lemmas", 
               parameters.generate_lemma_file,
               "Tells the tool to output all lemmas specified in the "
               ".spthy file.");

  parameters.timeout = 600;
  cli.add_option("-t,--timeout", parameters.timeout,
                 "Per-lemma timeout in seconds "
                 "(0 means no timeout, default: 600 seconds).");

  parameters.abort_after_failure = true;
  cli.add_flag("-a,--abort_after_failure", 
               parameters.abort_after_failure,
               "Tells the tool to abort if Tamarin fails to prove a lemma.");

  parameters.verbose = false;

  cli.add_flag("-v,--verbose", 
               parameters.verbose,
               "Tells the tool to output debug information.");

  CLI11_PARSE(cli, argc, argv);


  ut_tamarin::App app;
  
  ut_tamarin::termination::registerSIGINTHandler(parameters.tamarin_path, 
                                                 app.GetTempfilePath());

  if(parameters.generate_lemma_file) {
    return app.PrintLemmaNames(parameters, std::cout);
  } else if(parameters.penetration_lemma != ""){
    return app.PenetrateLemma(parameters, std::cout);
  } else {
    return app.RunTamarinOnLemmas(parameters, std::cout) ? 0 : 1;
  }
}
