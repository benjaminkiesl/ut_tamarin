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

#include "verbose_lemma_processor.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <utility>

#include "tamarin_interface.h"
#include "utility.h"

using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;

namespace uttamarin {

VerboseLemmaProcessor::VerboseLemmaProcessor(
        unique_ptr<LemmaProcessor> decoratee) :
                                       decoratee_(std::move(decoratee)) {
}

TamarinOutput VerboseLemmaProcessor::DoProcessLemma(const string& spthy_file_path,
                                                    const string& lemma_name) {
  auto start_time = std::chrono::high_resolution_clock::now();
  std::future<TamarinOutput> f = std::async(&LemmaProcessor::ProcessLemma,
                                             decoratee_.get(),
                                             spthy_file_path,
                                             lemma_name);
  do{
    auto seconds = DurationToString(
      std::chrono::duration_cast<std::chrono::seconds>(
         std::chrono::high_resolution_clock::now() - start_time).count());
    cout << "\r" << lemma_name << " " << seconds << " " << std::flush;
  } while(f.wait_for(std::chrono::seconds(1)) != std::future_status::ready);
  return f.get();
}

} // namespace uttamarin
