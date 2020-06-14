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

#ifndef UT_TAMARIN_OUTPUT_WRITER_H_
#define UT_TAMARIN_OUTPUT_WRITER_H_

#include <iostream>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

namespace uttamarin {

enum class TextColor { Red, Green, Yellow };

class OutputWriter {
 public:
  OutputWriter(std::vector<std::ostream*> streams);

  template<typename T>
  OutputWriter& operator<<(T input);

  template<typename T>
  OutputWriter& WriteColorized(T input, TextColor color);

  void Endl();

 private:

  template<typename T>
  OutputWriter& WriteColorized(std::ostream& stream, T input, TextColor color);

  std::vector<std::ostream*> streams_;
  std::unordered_map<TextColor, std::string> color_code_of_;
};

template<typename T>
OutputWriter& OutputWriter::operator<<(T input) {
  for(auto stream : streams_){
    *stream << input;
  }
  return *this;
}

template<typename T>
OutputWriter& OutputWriter::WriteColorized(T input, TextColor color) {
  for(auto stream : streams_){
    if(stream == &std::cout) WriteColorized(*stream, input, color);
    else *stream << input;
  }
  return *this;
}

template<typename T>
OutputWriter& OutputWriter::WriteColorized(std::ostream& stream,
                                           T input,
                                           TextColor color) {
  stream << "\033[" << color_code_of_[color] << "m" << input << "\033[m";
  return *this;
}

} // namespace uttamarin

#endif
