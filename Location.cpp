#include <nlohmann/json.hpp>
#include <qmljs/qmljsdocument.h>

#include "AstGeneratorBase.h"
#include "Location.h"

using namespace QmlJS;
using namespace QmlJS::AST;
using namespace nlohmann;

// int startColumn = 0;
// int startLine = 0;
// int startOffset = 0;
// int endColumn = 0;
// int endLine = 0;
// int endOffset = 0;
// int length = 0;

Location::Location() {}

Location::Location(int column, int line, int offset, int length) {
  set(column, line, offset, length);
}

Location::Location(int startColumn, int startLine, int startOffset,
                   int endColumn, int endLine, int endOffset)
    : startColumn(startColumn), startLine(startLine), startOffset(startOffset),
      endColumn(endColumn), endLine(endLine), endOffset(endOffset),
      length(endOffset - startOffset) {}

Location::Location(SourceLocation const &loc) {
  set(loc.startColumn, loc.startLine, loc.offset, loc.length);
}

void Location::set(int column, int line, int offset, int length_) {
  startColumn = column;
  startLine = line;
  startOffset = offset;
  endColumn = column;
  endLine = line;
  endOffset = offset + length_;
  length = length_;
}

bool Location::isValid() const { return length > 0; }

Location::operator json() { return toJson(); }

Location Location::operator+(int size) {
  return Location(startColumn, startLine, startOffset, endColumn + size,
                  endLine, endOffset + size);
}

Location Location::mergeWith(const Location &loc) const {
  return Location(
      min(startColumn, loc.startColumn), min(startLine, loc.startLine),
      min(startOffset, loc.startOffset), max(endColumn, loc.endColumn),
      max(endLine, loc.endLine), max(endOffset, loc.endOffset));
}

json Location::toJson() const {
  return json{
      {"start",
       {{"offset", startOffset}, {"line", startLine}, {"column", startColumn}}},
      {"end",
       {{"offset", endOffset}, {"line", endLine}, {"column", endColumn}}}};
}
