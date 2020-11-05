#!/bin/bash

if [[ "$OSTYPE" == "linux"* ]]
then
  DEPLOYQT=$(which linuxdeployqt)
  PLATFORM="linux"
elif [[ "$OSTYPE" == "darwin"* ]]
then
  DEPLOYQT=$(which macdeployqt)
  PLATFORM="macos"
else
  echo "Unsupported $OSTYPE system"
  exit;
fi

TARGET_NAME="qml-parser"
PROJECT_DIR="."
QMAKE=$(which qmake)
INSTALL_PATH="$PROJECT_DIR/bin/packages/$PLATFORM"

if [[ "$1" != "" ]]
then
  cd "$1"
  QT_BIN_DIR="$PWD"
  cd -

  QMAKE="$QT_BIN_DIR/qmake"
  DEPLOYQT="$QT_BIN_DIR/macdeployqt"
fi

rm -rf $INSTALL_PATH
"$QMAKE" "./$TARGET_NAME.pro" -config release
make clean
make
mkdir -p $INSTALL_PATH

if [[ "$PLATFORM" == "linux" ]]
then
  mv "$PROJECT_DIR/bin/$TARGET_NAME" $INSTALL_PATH
  "$DEPLOYQT" "$INSTALL_PATH/$TARGET_NAME" -no-plugins -verbose=1
elif [[ "$PLATFORM" == "macos" ]]
then
  "$DEPLOYQT" "$PROJECT_DIR/bin/$TARGET_NAME.app" -no-plugins -verbose=1
  rmdir $INSTALL_PATH
  mv "$PROJECT_DIR/bin/$TARGET_NAME.app" $INSTALL_PATH
fi

