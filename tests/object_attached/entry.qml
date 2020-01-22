import QtQuick 2.0

Rectangle {

  KeyNavigation.tab: this;

  Keys.onEscapePressed: {
    Qt.quit ()
  }

}
