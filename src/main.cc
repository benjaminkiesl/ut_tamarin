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
#include <memory>
#include <string>

#include "cli11/CLI11.hpp"

#include "app.h"
#include "bash_lemma_processor.h"
#include "lemma_penetrator.h"
#include "terminator.h"
#include "verbose_lemma_processor.h"

int main (int argc, char *argv[])
{
  uttamarin::CmdParameters parameters;

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
  
  parameters.timeout = 600;
  cli.add_option("-t,--timeout", parameters.timeout,
                 "Per-lemma timeout in seconds "
                 "(0 means no timeout, default: 600 seconds).");

  parameters.abort_after_failure = true;
  cli.add_flag("-a,--abort_after_failure", 
               parameters.abort_after_failure,
               "Tells the tool to abort if Tamarin fails to prove a lemma.");

  CLI11_PARSE(cli, argc, argv);

  uttamarin::termination::registerSIGINTHandler(parameters.tamarin_path);

  auto lemma_processor = std::make_unique<uttamarin::VerboseLemmaProcessor>(
          std::make_unique<uttamarin::BashLemmaProcessor>(
            parameters.tamarin_path,
            parameters.proof_directory,
            parameters.timeout));

  if(parameters.penetration_lemma != ""){
    uttamarin::LemmaPenetrator lemma_penetrator(std::move(lemma_processor));
    lemma_penetrator.SetTimeout(parameters.timeout);
    lemma_penetrator.PenetrateLemma(parameters.spthy_file_path,
                                    parameters.penetration_lemma);
  } else {
    uttamarin::App app(std::move(lemma_processor));
    app.RunTamarinOnLemmas(parameters, std::cout);
  }
  return 0;
}
