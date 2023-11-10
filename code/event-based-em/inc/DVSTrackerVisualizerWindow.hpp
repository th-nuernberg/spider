#ifndef __DVSTRACKERVISUALIZERWINDOW_HPP__A93CCD32_800E_47B5_B2ED_BFE5EB16A55E
#define __DVSTRACKERVISUALIZERWINDOW_HPP__A93CCD32_800E_47B5_B2ED_BFE5EB16A55E

#include <memory>
#include <QMdiSubWindow>

// forward declarations
class QFrame;
class QResizeEvent;
class QCloseEvent;

namespace nst {

  // forward declarations
  class DVSTrackerControl;

  namespace gui {

    class DVSTrackerWidget;

    class DVSTrackerVisualizerWindow : public QMdiSubWindow
    {
      Q_OBJECT
    public:
      DVSTrackerVisualizerWindow(DVSTrackerControl *control, QWidget *parent = 0, Qt::WindowFlags flags = 0);
      ~DVSTrackerVisualizerWindow();

      void resizeEvent(QResizeEvent *ev);

    public slots:
      void closeEvent(QCloseEvent *ev) Q_DECL_OVERRIDE;

    signals:
      void closing();

    private:
      DVSTrackerControl * _control;
      DVSTrackerWidget *_wdgtTracker = nullptr;
    };


  }} // nst::gui::

#endif /* __DVSTRACKERVISUALIZERWINDOW_HPP__A93CCD32_800E_47B5_B2ED_BFE5EB16A55E */

