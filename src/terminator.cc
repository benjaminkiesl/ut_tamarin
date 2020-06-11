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

#include "terminator.h"

#include <csignal>
#include <cstdio>

#include <iostream>
#include <string>

using std::string;

namespace ut_tamarin::termination {

// Holds the name of the tamarin process. This name is used for killing Tamarin 
// when the program receives a SIGINT signal (sent by Ctrl+C).
string tamarin_process = "tamarin-prover";

// Path to the tempfile created by UT Tamarin
string local_tempfile_path = "";

void (*default_sigint_handler)(int signal);

// Signal handler for SIGINT signal (sent by Ctrl+C)
void sigint_handler(int signal)
{
  std::cout << std::endl;
  std::system(("killall " + tamarin_process + " 2> /dev/null").c_str());
  std::remove(local_tempfile_path.c_str());
  std::signal(signal, default_sigint_handler);
  std::raise(signal);
}

// Sets the global 'tamarin_process' name. This is needed for killing Tamarin
// in case the program receives a SIGINT signal (sent by Ctrl+C).
void registerSIGINTHandler(const string& process_name, 
                           const string& tempfile_path){
  local_tempfile_path = tempfile_path;
  if(process_name.find('/') != string::npos){
    tamarin_process = process_name.substr(process_name.find_last_of('/')+1);
  } else {
    tamarin_process = process_name;
  }
  default_sigint_handler = std::signal(SIGINT, sigint_handler);
}

} // namespace ut_tamarin::terminator
