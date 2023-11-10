#include "inc/MainWindow.hpp"

#include <iostream>

#include <QMenuBar>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QApplication>
#include <QToolBar>
#include <QSizePolicy>
#include <QTextEdit>

#include "inc/Utils.hpp"
#include "inc/DVSTrackerControlWindow.hpp"
#include "inc/DVSVisualizerWindow.hpp"
#include "qlabel.h"

namespace nst {

  namespace gui {

    MainWindow::
    MainWindow(QWidget *parent)
      : QMainWindow(parent)
    {
      // initialize UI
      _mnuFile = menuBar()->addMenu("File");

      // build actions
      _actClose = new QAction("Quit", this);
      connect(_actClose, &QAction::triggered, this, &QApplication::quit);

      // build main menu
      _mnuFile->addAction(_actClose);

      _mdi = new QMdiArea(this);
      setCentralWidget(_mdi);
      setFixedSize(1200, 800);
      addDVSTrackerControl();
      addLogo();

    }

    MainWindow::
    ~MainWindow()
    {
      delete _win;
    }

    void MainWindow::
    addDVSTrackerControl()
    {
      auto rc = new DVSTrackerControlWindow(_mdi);
      rc->resize(600, 300);
      rc->show();
    }

    void MainWindow::
        addLogo()
    {
      auto rc = new QMdiSubWindow(_mdi);
      rc->resize(200, 800);
      QString filename = "../logo.svg";
      auto img = new QPixmap();
      img->load(filename);
      auto qlab = new QLabel();
      qlab->setPixmap(img->scaled(270, 880, Qt::KeepAspectRatio));
      rc->setWidget(qlab);
      rc->show();
    }


  }
} // nst::gui::
