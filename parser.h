/*
Copyright (c) 2015-2018, Jesper Helles� Hansen
jesperhh@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the <organization> nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <nlohmann/json.hpp>
#include <tsl/ordered_map.h>

template <class Key, class T, class Ignore, class Allocator,
          class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
          class AllocatorPair = typename std::allocator_traits<
              Allocator>::template rebind_alloc<std::pair<Key, T>>,
          class ValueTypeContainer =
              std::vector<std::pair<Key, T>, AllocatorPair>>
using ordered_map =
    tsl::ordered_map<Key, T, Hash, KeyEqual, AllocatorPair, ValueTypeContainer>;

namespace parser {
using json = nlohmann::basic_json<ordered_map>;
} // namespace parser

class Parser {
public:
  static bool debug;

  enum class Option {
    // ListFileName = 0x1,
    // OverwriteFile = 0x2,
    // PrintError = 0x4,
    Debug = 0x8
  };
  Q_DECLARE_FLAGS(Options, Option)

  Parser(Options options);

  int Run();
  int Run(QStringList paths);

private:
  Options m_options;
  int InternalRun(QIODevice &input, const QString &path);

  void setDebug(bool debug_);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Parser::Options)

#endif // PARSER_H