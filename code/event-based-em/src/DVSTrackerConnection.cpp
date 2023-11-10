#include "inc/DVSTrackerConnection.hpp"
#include "inc/Commands.hpp"
#include "inc/Utils.hpp"
#include "inc/DVSLogger.hpp"

#include <QMutexLocker>
#include <iostream>
#include <QTimer>

#include <unistd.h>

namespace nst {

  DVSTrackerConnection::
  DVSTrackerConnection(QObject *parent)
    : QObject(parent)
  {
  }

  DVSTrackerConnection::
  ~DVSTrackerConnection()
  {
  }


  void DVSTrackerConnection::
  connect(const QString source)
  {
    // make sure to call in the correct thread
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "connect", Qt::QueuedConnection, Q_ARG(const QString, source));
        return;
      }

    // create object
    this->_port = new QSerialPort(source);

    // connect the signals
    QObject::connect(_port, &QSerialPort::readyRead, this, &DVSTrackerConnection::_port_readyRead);

    // atempt to open port and configure it
    if(this->_port->open(QIODevice::ReadWrite)){
        int fd = this->_port->handle();

        // add custom code for serial port
        struct serial_struct ser;
        int flags;
        struct termios tty;
        ioctl(fd, TIOCGSERIAL, &ser);                                  // get the current port options
        ser.flags |= ASYNC_SPD_CUST;                             // activate custom speed option
        ser.flags |= ASYNC_LOW_LATENCY;                      // activate low latency option
        // compute the divisor to apply to base baudrate
        ser.custom_divisor = ser.baud_base / DVS_DATA_RATE;
        ioctl(fd, TIOCSSERIAL, &ser);                                  // set the options
        flags = B38400;                                                         // default flags
        tcgetattr(fd, &tty);                                                       // get current options from port
        cfsetispeed(&tty, flags);                                             // set input speed
        cfsetospeed(&tty, flags);                                            // set output speed
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;               // transmission size to 8bit
        tty.c_cflag |= (CLOCAL | CREAD);                            // enable receiver and set local mode
        tty.c_cflag |= CRTSCTS;                                           // use hardware handshaking (RTS/CTS)
        // clear input options: Send a SIGINT when a break condition is detected
        tty.c_iflag = IGNBRK;
        tty.c_lflag = 0;                                                            // clear local options
        tty.c_oflag = 0;                                                           // clear output options
        tty.c_cc[VMIN] = 1;                                                    // set minimum character count to receive to 1
        tty.c_cc[VTIME] = 5;                                                  // set inter character timeout to 500 ms
        tcsetattr(fd, TCSANOW, &tty);                                   // set new options to port
        tcflush(fd, TCIFLUSH);                                              // flush input buffer
        // just to be sure, set O_NONBLOCK
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
        emit connected();
      }
    else{
        std::cout << ( "DVSConnection: Error opening DVS port "  + (this->_port->portName()) +  ", error:" + (this->_port->errorString())).toStdString() <<std::endl;
      }
  }

  void DVSTrackerConnection::
  disconnect()
  {
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "disconnect", Qt::QueuedConnection);
        return;
      }
    // check if the log support is activated
    if(!this->_logger){
        if (!this->_port)
          return;
        this->_port->close();
        this->_port = nullptr;
      }
    else{ // if logging was on, check disconnection for recording or replay
        if (!this->_port)
          return;
        this->_port->close();
        this->_port = nullptr;
        // close file
        this->_logger->closeFile();
        this->_logger = nullptr;
        delete this->_logger;
      }
    emit disconnected();
  }

  void DVSTrackerConnection::
  record(const QString source)
  {
    // make sure to call in the correct thread
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "record", Qt::QueuedConnection, Q_ARG(const QString, source));
        return;
      }
    _logger = new DVSLogger(source, RECORD_FILE);
  }

  void DVSTrackerConnection::
  replay(const QString source)
  {
    // make sure to call in the correct thread
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "replay", Qt::QueuedConnection, Q_ARG(const QString, source));
        return;
      }
    _logger = new DVSLogger(source, REPLAY_FILE);
    // here we can implement the slowmo for debugging
    auto events = _logger->readFile();
    uint evCount = 0;
    for(auto evid = 0; evid<events.size(); ++evid){
        if(++evCount>VISUAL_UPDATE*replayDelay){
            QThread::usleep(DEF_REPLAY_DELAY);
            evCount = 0;
        }
        // emit signal to process event
        emit dataReadyFile(std::move((events.at(evid))));
    }
  }

  void DVSTrackerConnection::
  _port_readyRead()
  {
    if (!_port) return;
    // check here how can we read and save the data into file properly
    auto data = _port->readAll();
    if(!this->isOffline() && _logger->isRecording()){
        _logger->writeFile(data);
      }
    emit dataReady(std::move(data));
  }

  void DVSTrackerConnection::
  sendCommand(commands::Command *cmd)
  {
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "sendCommand", Qt::QueuedConnection, Q_ARG(commands::Command*, cmd));
        return;
      }
    if (!this->_port || !cmd)
      return;
    this->_port << *cmd;
    delete cmd;
  }

  bool DVSTrackerConnection::
  isOffline()
  {
    return (_logger==nullptr);
  }

} // nst::
