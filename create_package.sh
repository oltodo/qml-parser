#!/bin/bash

TARGET_NAME="qml-parser"

cd $(dirname $0)
PROJECT_DIR="$PWD"
cd -

QMAKE=$(which qmake)
MACDEPLOYQT=$(which macdeployqt)

if [[ "$1" != "" ]]
then

  cd "$1"
  QT_BIN_DIR="$PWD"
  cd -

  QMAKE="$QT_BIN_DIR/qmake"
  MACDEPLOYQT="$QT_BIN_DIR/macdeployqt"

fi

cd "$PROJECT_DIR"

echo "Operating system detected: $OSTYPE"
if [[ "$OSTYPE" == "darwin"* ]]
then

  rm -rf "./bin/$TARGET_NAME.app"
  "$QMAKE" "./$TARGET_NAME.pro"
  make -j8
  "$MACDEPLOYQT" "$PROJECT_DIR/bin/$TARGET_NAME.app" -no-plugins -verbose=1

else

  # todo : windows, linux, etc
  echo "Unsupported OS for package creation !"

fi

cd "$WORK_DIR"
