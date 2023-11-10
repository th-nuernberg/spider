#ifndef __DVSTRACKERCONNECTION_HPP__3E71037A_2D7F_456C_B919_BF491C99A569
#define __DVSTRACKERCONNECTION_HPP__3E71037A_2D7F_456C_B919_BF491C99A569

#include <iostream>
#include <memory>
#include <QMutex>
#include <QThread>
#include <QMetaObject>
#include <QString>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>

// add legacy acces for configuring custom baud rates
#include <termio.h>
#include <fcntl.h>
#include <linux/serial.h>

class DVSLogger;

namespace nst {

  // forward declarations
  namespace commands {
    struct Command;
  } // commands::

  class DVSTrackerConnection : public QObject
  {
    Q_OBJECT

  public:
    DVSTrackerConnection(QObject *parent = 0);
    virtual ~DVSTrackerConnection();

  signals:
    void dataReady(const QByteArray &data);
    void dataReadyFile(const unsigned char c);
    void connected();
    void disconnected();

  public slots:
    void connect(const QString source);
    void record(const QString source);
    void replay(const QString source);
    void disconnect();

    void sendCommand(commands::Command *cmd);
    bool isOffline();

  private slots:
    void _port_readyRead();

  private:
    QSerialPort *_port = nullptr;
    DVSLogger *_logger = nullptr;
  };


} // nst::

#endif /* __DVSTRACKERCONNECTION_HPP__3E71037A_2D7F_456C_B919_BF491C99A569 */

