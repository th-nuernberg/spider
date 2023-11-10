#include <iostream>
#include <QApplication>
#include "inc/MainWindow.hpp"
#include "inc/Commands.hpp"
#include "inc/DVSTrackerDebugger.hpp"

int
main (int argc, char *argv[])
{
  using namespace nst;
  QApplication app(argc, argv);
  gui::MainWindow win;

  // register the command and logging infrastructure
  // as we pass along only pointers, use the base classes here
  qRegisterMetaType<commands::Command*>("commands::Command*");
  qRegisterMetaType<const commands::Command*>("const commands::Command*");
  qRegisterMetaType<QTextBlock>("QTextBlock");
  qRegisterMetaType<QTextCursor>("QTextCursor");

  win.show();
  return app.exec();
}
