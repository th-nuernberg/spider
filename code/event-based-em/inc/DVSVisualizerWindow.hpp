#ifndef __DVSVISUALIZERWINDOW_HPP__A93CCD32_800E_47B5_B2ED_BFE5EB16A55E
#define __DVSVISUALIZERWINDOW_HPP__A93CCD32_800E_47B5_B2ED_BFE5EB16A55E

#include <memory>
#include <QMdiSubWindow>

// forward declarations
class QFrame;
class QResizeEvent;
class QCloseEvent;

namespace nst {

  // forward declarations
  class DVSTrackerControl;
  struct DVSEvent;

  namespace gui {

    class DVSEventWidget;

    class DVSVisualizerWindow : public QMdiSubWindow
    {
      Q_OBJECT
    public:
      DVSVisualizerWindow(DVSTrackerControl *control, QWidget *parent = 0, Qt::WindowFlags flags = 0);
      ~DVSVisualizerWindow();

      void resizeEvent(QResizeEvent *ev);

    public slots:
      void closeEvent(QCloseEvent *ev) Q_DECL_OVERRIDE;

    signals:
      void closing();


    private:
      DVSTrackerControl * _control;
      DVSEventWidget *_wdgtEvents = nullptr;
    };


  }} // nst::gui::

#endif /* __DVSVISUALIZERWINDOW_HPP__A93CCD32_800E_47B5_B2ED_BFE5EB16A55E */

