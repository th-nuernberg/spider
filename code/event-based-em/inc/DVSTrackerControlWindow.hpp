#ifndef __DVSTRACKERCONTROLWIDGET_HPP__1F654E45_7026_4F8E_ACB3_933E32B4ED82
#define __DVSTRACKERCONTROLWIDGET_HPP__1F654E45_7026_4F8E_ACB3_933E32B4ED82

#include <memory>
#include <QMdiSubWindow>


class QFrame;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QComboBox;
class QDateTime;
class QFileDialog;
class QLabel;
class QCustomPlot;

// user function vars
extern double rate_angle;
extern double rate_speed;
extern int smoothing_is_closed_loop ;

namespace nst {

  // forward declarations
  class DVSTrackerControl;

  namespace gui {

    // forward declarations
    class DVSVisualizerWindow;
    class DVSTrackerVisualizerWindow;
    struct DVSEvent;

    /**
 * RobotControlWindow - Widget to connect to and control a robot.
 */
    class DVSTrackerControlWindow : public QMdiSubWindow
    {
      Q_OBJECT
    public:
      DVSTrackerControlWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
      ~DVSTrackerControlWindow();

      void closeEvent(QCloseEvent *ev) override;

    signals:
      void closing(QMdiSubWindow *win);

    private slots:
      // ui slots
      void onBtnConnectClicked();
      void onBtnRecordClicked();
      void onBtnReplayClicked();
      void onCbUserFunctionStateChanged(int state);
      void onCmbUserFunctionIndexChanged(int index);
      void onParamRateAngleIncClicked();
      void onParamRateAngleDecClicked();
      void onParamRateSpeedIncClicked();
      void onParamRateSpeedDecClicked();
      void onParamSmoothingClosedLoop(int state);
      void onParamReplaySpeedIncClicked();
      void onParamReplaySpeedStopClicked();
      void onParamReplaySpeedPlayClicked();
      void onParamReplaySpeedDecClicked();
      void onParamTimestampClicked();
      void onParamTimestampMouseWheel();

      // control slots

      void onControlConnected();
      void onControlDisconnected();

    private:
      void setUserFunction(unsigned index);
      void unsetUserFunction();

      DVSTrackerControl *_control;

      QFrame *_centralWidget = nullptr;
      QLineEdit *_editPort = nullptr;
      QPushButton *_btnConnect = nullptr;

      QLineEdit *_editFileOut = nullptr;
      QPushButton *_btnRecord = nullptr;
      QString _editFileOutName;


      QLineEdit *_editFileIn= nullptr;
      QPushButton *_btnReplay= nullptr;

      QCheckBox *_cbShowEvents = nullptr;
      QCheckBox *_cbUserFunction = nullptr;

      QComboBox *_cmbUserFunction = nullptr;

      DVSVisualizerWindow *_winEventVisualizer = nullptr;
      DVSTrackerVisualizerWindow *_winTrackerVisualizer = nullptr;

      QLabel * _paramRateAngle = nullptr;
      QPushButton *_btnParamRateAngleInc = nullptr;
      QPushButton *_btnParamRateAngleDec = nullptr;
      QLabel *_valRateAngle = nullptr;


      QLabel * _paramRateSpeed = nullptr;
      QPushButton *_btnParamRateSpeedInc = nullptr;
      QPushButton *_btnParamRateSpeedDec = nullptr;
      QLabel *_valRateSpeed = nullptr;


      QLabel *_paramSmoothingClosedLoop = nullptr;
      QCheckBox *_cbParamSmoothingClosedLoop = nullptr;
      QLabel *_lbSmoothing = nullptr;

      QLabel * _paramReplaySpeed = nullptr;
      QPushButton *_btnParamReplaySpeedInc = nullptr;
      QPushButton *_btnParamReplaySpeedDec = nullptr;
      QPushButton *_btnParamReplaySpeedStop = nullptr;
      QPushButton *_btnParamReplaySpeedPlay = nullptr;

      QPushButton *_btnParamTimestamp = nullptr;
      QLabel * _paramTimestamp = nullptr;

      QCustomPlot *_evTmsPlot = nullptr;
    };


  }} // nst::gui::

#endif /* __DVSTRACKERCONTROLWIDGET_HPP__1F654E45_7026_4F8E_ACB3_933E32B4ED82 */

