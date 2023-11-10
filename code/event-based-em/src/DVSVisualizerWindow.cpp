#include "inc/DVSVisualizerWindow.hpp"

#include <iostream>

#include <QFrame>
#include <QLayout>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QCloseEvent>

#include "inc/Utils.hpp"
#include "inc/Datatypes.hpp"
#include "inc/DVSTrackerControl.hpp"
#include "inc/DVSEventWidget.hpp"

namespace nst {

  namespace gui {

    DVSVisualizerWindow::
    DVSVisualizerWindow(DVSTrackerControl *control, QWidget *parent, Qt::WindowFlags flags)
      : QMdiSubWindow(parent, flags), _control(control)
    {
      // add widgets for event visualization
      _wdgtEvents = new DVSEventWidget(this);
      layout()->addWidget(_wdgtEvents);

      // connect the event and tracker visualizers to the tracker control
      connect(_control, &DVSTrackerControl::DVSEventReceived, _wdgtEvents, &DVSEventWidget::newEvent);

      setAttribute(Qt::WA_DeleteOnClose);
    }

    DVSVisualizerWindow::
    ~DVSVisualizerWindow()
    {
      disconnect(_control, &DVSTrackerControl::DVSEventReceived, _wdgtEvents, &DVSEventWidget::newEvent);
    }

    void DVSVisualizerWindow::
    resizeEvent(QResizeEvent *ev)
    {
      // make sure to keep a fixed/square aspect when resizing
      int max = ev->size().width();
      if (ev->size().height() > max) max = ev->size().height();

      resize(max, max);
      QMdiSubWindow::resizeEvent(ev);
    }


    void DVSVisualizerWindow::
    closeEvent(QCloseEvent *ev)
    {
      emit closing();
      ev->accept();
    }


  }} // nst::gui::
