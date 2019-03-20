#include <qmljs/qmljsdocument.h>

#include "AstGeneratorBase.h"
#include "Location.h"
#include "parser.h"

using namespace QmlJS;
using namespace QmlJS::AST;

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
  set(loc.startColumn - 1, loc.startLine, loc.offset, loc.length);
}

void Location::set(int column, int line, int offset, int length_) {
  startColumn = column;
  startLine = line;
  startOffset = offset;
  endColumn = column + length_;
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
  Location startLoc;
  Location endLoc;

  if (startOffset < loc.startOffset)
    startLoc = *this;
  else
    startLoc = loc;

  if (endOffset > loc.endOffset)
    endLoc = *this;
  else
    endLoc = loc;

  return Location(startLoc.startColumn, startLoc.startLine,
                  startLoc.startOffset, endLoc.endColumn, endLoc.endLine,
                  endLoc.endOffset);
}

json Location::toJson() const {
  return json{
      {"start",
       {{"offset", startOffset}, {"line", startLine}, {"column", startColumn}}},
      {"end",
       {{"offset", endOffset}, {"line", endLine}, {"column", endColumn}}}};
}
