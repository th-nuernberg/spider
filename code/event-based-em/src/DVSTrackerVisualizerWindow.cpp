#include "inc/DVSTrackerVisualizerWindow.hpp"

#include <iostream>

#include <QFrame>
#include <QLayout>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QCloseEvent>

#include "inc/Utils.hpp"
#include "inc/Datatypes.hpp"
#include "inc/DVSTrackerControl.hpp"
#include "inc/DVSTrackerWidget.hpp"

namespace nst {

  namespace gui {

    DVSTrackerVisualizerWindow::
    DVSTrackerVisualizerWindow(DVSTrackerControl *control, QWidget *parent, Qt::WindowFlags flags)
      : QMdiSubWindow(parent, flags), _control(control)
    {
      // add widgets for tracking visualization

      _wdgtTracker = new DVSTrackerWidget(this);
      layout()->addWidget(_wdgtTracker);

      // connect the tracker visualizers to the tracker control
      connect(_control, &DVSTrackerControl::TrackerEventReceived, _wdgtTracker, &DVSTrackerWidget::newPosition);

      setAttribute(Qt::WA_DeleteOnClose);
    }

    DVSTrackerVisualizerWindow::
    ~DVSTrackerVisualizerWindow()
    {
      disconnect(_control, &DVSTrackerControl::TrackerEventReceived, _wdgtTracker, &DVSTrackerWidget::newPosition);
    }


    void DVSTrackerVisualizerWindow::
    resizeEvent(QResizeEvent *ev)
    {
      // make sure to keep a fixed/square aspect when resizing
      int max = ev->size().width();
      if (ev->size().height() > max) max = ev->size().height();

      resize(max, max);
      QMdiSubWindow::resizeEvent(ev);
    }

    void DVSTrackerVisualizerWindow::
    closeEvent(QCloseEvent *ev)
    {
      emit closing();
      ev->accept();
    }

  }} // nst::gui::
