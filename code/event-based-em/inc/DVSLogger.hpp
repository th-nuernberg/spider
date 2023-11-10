#ifndef DVSLOGGER_HPP__3E71037A_2D7F_456C_B919_BF491
#define DVSLOGGER_HPP__3E71037A_2D7F_456C_B919_BF491

#include <QObject>
#include <QFile>

/**
 * File types for logging
 */
typedef enum{
  RECORD_FILE,
  REPLAY_FILE
}filetype_t;

class DVSLogger : public QObject
{
  Q_OBJECT
public:
  explicit DVSLogger(QString source, filetype_t ft, QObject *parent=0);
  ~DVSLogger();
  bool isRecording();
  bool endFile();

public slots:
  void writeFile(const QByteArray &data);
  QByteArray readFile();
  void closeFile();

private:
  QFile *_file = nullptr;
  bool _isRecording = false;
};

#endif // DVSLOGGER_HPP__3E71037A_2D7F_456C_B919_BF491
