#pragma once

#ifndef LOCATION_H
#define LOCATION_H

#include <private/qqmljsast_p.h>

#include "parser.h"

using namespace QQmlJS;
using namespace QQmlJS::AST;
using namespace Parser;

class Location {
protected:
  void set(int column, int line, int offset, int length_);

public:
  int startColumn = 0;
  int startLine = 0;
  int startOffset = 0;
  int endColumn = 0;
  int endLine = 0;
  int endOffset = 0;
  int length = 0;

  Location();

  Location(int column, int line, int offset, int length);

  Location(int startColumn, int startLine, int startOffset, int endColumn,
           int endLine, int endOffset);

  Location(SourceLocation const &loc);

  Location(SourceLocation const &locStart, SourceLocation const &locEnd);

  Location(json const &loc);

  bool isValid() const;

  operator json();

  Location operator+(int size);
  Location operator-(int size);

  Location mergeWith(const Location &loc) const;

  json toJson() const;
};

#endif // LOCATION_H
