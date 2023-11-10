#include "inc/DVSTrackerControl.hpp"
#include "inc/DVSTrackerConnection.hpp"
#include "inc/DVSBytestreamParser.hpp"
#include "inc/Datatypes.hpp"
#include "inc/Commands.hpp"
#include "inc/Utils.hpp"
#include "inc/UserFunction.hpp"

#include <QString>
#include <QThread>
#include <QTimer>

namespace nst {

  DVSTrackerControl::
  DVSTrackerControl()
  {

    // initialize required timers, threads and connections
    _timer_uf = new QTimer();
    _timer_uf->setTimerType(Qt::PreciseTimer);
    _timer_uf->setInterval(CONTROL_LOOP_TS);

    // control loop sample time TS
    connect(_timer_uf, &QTimer::timeout, this, &DVSTrackerControl::onTimerUFTimeout);

    _con_thread = new QThread();
    _parser_thread = new QThread();

    _con = new DVSTrackerConnection();
    _con->moveToThread(_con_thread);

    _parser = new DVSBytestreamParser(_id);
    // enable timestamp
    _parser->set_timeformat(DVSEvent::TIMEFORMAT_3BYTES);
    _parser->moveToThread(_parser_thread);

    // connect the worker objects
    connect(_con, &DVSTrackerConnection::dataReady, _parser, &DVSBytestreamParser::parseData, Qt::QueuedConnection);
    connect(_con, &DVSTrackerConnection::connected, this, &DVSTrackerControl::onDVSTrackerConnected, Qt::QueuedConnection);
    connect(_con, &DVSTrackerConnection::disconnected, this, &DVSTrackerControl::onDVSTrackerDisconnected, Qt::QueuedConnection);
    if(_con->isOffline())
      connect(_con, &DVSTrackerConnection::dataReadyFile, _parser, &DVSBytestreamParser::parseDataFile, Qt::QueuedConnection);

    // forward events from the lower level
    connect(_parser, &DVSBytestreamParser::eventReceived, this, &DVSTrackerControl::onDVSEventReceived, Qt::QueuedConnection);

    // manage cleanup
    connect(_parser_thread, &QThread::finished, _parser, &DVSBytestreamParser::deleteLater);
    connect(_parser_thread, &QThread::finished, _parser_thread, &QThread::deleteLater);
    connect(_con_thread, &QThread::finished, _con, &DVSTrackerConnection::deleteLater);
    connect(_con_thread, &QThread::finished, _con_thread, &QThread::deleteLater);

    // start the threads
    _con_thread->start();
    _parser_thread->start();
  }


  DVSTrackerControl::
  ~DVSTrackerControl()
  {
    // shut down objects
    _con->disconnect();

    // shut down threads
    _parser_thread->quit();
    _con_thread->quit();
  }


  void DVSTrackerControl::
  resetDVSTracker()
  {
    // enable event streaming
    _con->sendCommand(new commands::DVS(true));

    // enable laser actuator driving circuitry
    _con->sendCommand(new commands::LaserMotorDriver(true));

    // reset motor velocities to 0
    _con->sendCommand(new commands::MV0(0));
    _con->sendCommand(new commands::MV1(0));
  }

  void DVSTrackerControl::
  onDVSTrackerConnected()
  {
    _is_connected = true;
    resetDVSTracker();
    emit connected();
  }

  void DVSTrackerControl::
  onDVSTrackerDisconnected()
  {
    _is_connected = false;
    emit disconnected();
  }

  std::vector<DVSEvent*> DVSTrackerControl::
  getEvBuffer()
  {
    return this->_evBuffer;
  }


  void DVSTrackerControl::
  onDVSEventReceived(DVSEvent *ev)
  {
    // turn the pointer into a shared memory object. data comes from the
    // parser and is now in our thread.
    auto _ev = std::make_shared<DVSEvent>(std::move(*ev));
    //push back new event in the global buffer for timestamp analysis
    _evBuffer.push_back(ev);
    // user function is executed every TS ms
    if (_userfn)   _userfn->fn(this, _ev);
    // emit relevant signals for event viewer
    emit DVSEventReceived(_ev);
  }

  void DVSTrackerControl::
  setTrackerEvent(TrackerEvent *trkev)
  {
    this->_trkev = trkev;
    // update the tracker visualization
    emit TrackerEventReceived(this->_trkev);
  }

  void DVSTrackerControl::
  connectDVSTracker(const QString source)
  {
    // check the source of DVS events and the direction
    if(source.contains("/dev/")){
        _con->connect(source);
      }
    else{ // we are working with files, either to record or to replay (relative path only set to max chars)
        if(source.size() < EVENT_FILENAME_SIZE ){
            _con->record(source);
          }
        else{
            // otherwise is for replaying (contains the full path to the file)
            _con->replay(source);
          }
      }
  }

  void DVSTrackerControl::
  disconnectDVSTracker()
  {
    // turn off everything
    _con->disconnect();
  }

  bool DVSTrackerControl::
  isConnected()
  {
    return this->_is_connected;
  }


  void DVSTrackerControl::
  actuateLaser(float x, float y)
  {
    if (!_is_connected) return;
    x = 0.0;  y = 0.0;
    // finally send commands. use decaying ones
    _con->sendCommand(new commands::MVD0(static_cast<int>(x)));
    _con->sendCommand(new commands::MVD1(static_cast<int>(y)));
  }


  void DVSTrackerControl::
  enableEventstream()
  {
    if (!_is_connected) return;
    _con->sendCommand(new commands::DVS(true));
  }

  void DVSTrackerControl::
  disableEventstream()
  {
    if (!_is_connected) return;
    _con->sendCommand(new commands::DVS(false));
  }


  void DVSTrackerControl::
  setUserFunction(const UserFunction *fn)
  {
    _userfn = fn;
    _timer_uf->start();
  }


  void DVSTrackerControl::
  unsetUserFunction()
  {
    _userfn = nullptr;
    _timer_uf->stop();
  }


  void DVSTrackerControl::
  onTimerUFTimeout()
  {
    if (_userfn) _userfn->fn(this, std::shared_ptr<DVSEvent>());
  }

} // nst::
