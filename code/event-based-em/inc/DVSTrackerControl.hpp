#ifndef __DVSTRACKERCONTROL_HPP__32EABE1A_2F0D_4AAB_B831_EFC05DE84126
#define __DVSTRACKERCONTROL_HPP__32EABE1A_2F0D_4AAB_B831_EFC05DE84126

#include <memory>
#include <QObject>

// forward declarations
class QTimer;
class QThread;
class QString;

namespace nst {

  // forward declarations
  class DVSTrackerConnection;
  class DVSBytestreamParser;

  struct UserFunction;
  struct DVSEvent;
  struct TrackerEvent;

  namespace commands {
    struct Command;
  } // commands;


  class DVSTrackerControl : public QObject
  {
    Q_OBJECT

  public:
    DVSTrackerControl();
    ~DVSTrackerControl();

    void connectDVSTracker(const QString port);
    void disconnectDVSTracker();
    bool isConnected();

    void resetDVSTracker();

    /*
         * set a user function which will be called everytime an event is
         * received.
         */
    void setUserFunction(const UserFunction *fn);
    void unsetUserFunction();

    void actuateLaser(const float x, const float y);

    /*
         * enable/disable event streaming
         */
    void enableEventstream();
    void disableEventstream();

    void setTrackerEvent(TrackerEvent *trkev);
    std::vector<DVSEvent*> getEvBuffer();

  signals:
    void connected();
    void disconnected();

    void DVSEventReceived(std::shared_ptr<DVSEvent> ev);
    void TrackerEventReceived(TrackerEvent *trkev);

  private slots:
    void onDVSTrackerConnected();
    void onDVSTrackerDisconnected();
    void onDVSEventReceived(DVSEvent *ev);
    void onTimerUFTimeout();

  private:
    QTimer *_timer_uf = nullptr;
    QThread *_con_thread = nullptr;
    QThread *_parser_thread = nullptr;
    QThread *_reader_thread = nullptr;

    DVSTrackerConnection *_con = nullptr;
    DVSBytestreamParser *_parser = nullptr;

    const UserFunction *_userfn = nullptr;

    TrackerEvent *_trkev = nullptr;
    std::vector<DVSEvent*> _evBuffer;

    bool _is_connected = false;
    uint8_t _id;
  };


} // nst::



#endif /* __DVSTRACKERCONTROL_HPP__32EABE1A_2F0D_4AAB_B831_EFC05DE84126 */

