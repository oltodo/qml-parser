import QtQuick 2.0 as QQ

QQ.Rectangle {

  property QQ.Item test1 : null;

  property list<QQ.Text> test2;

  signal test3 (QQ.Image arg);

}
