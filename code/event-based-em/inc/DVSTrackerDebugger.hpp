#ifndef __DVSTRACKERDEBUGSTREAM_H__5E71037A_2D7F_456C_B919_BF491C99A569
#define __DVSTRACKERDEBUGSTREAM_H__5E71037A_2D7F_456C_B919_BF491C99A569

#include <iostream>
#include <streambuf>
#include <string>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextEdit>

class DVSTrackerDebugStream : public std::basic_streambuf<char>
{
public:
  DVSTrackerDebugStream(std::ostream &stream, QTextEdit* text_edit) : m_stream(stream)
  {
    log_window = text_edit;
    m_old_buf = stream.rdbuf();
    stream.rdbuf(this);
  }

  ~DVSTrackerDebugStream()
  {
    m_stream.rdbuf(m_old_buf);
  }

  static void registerQDebugMessageHandler(){
    qInstallMessageHandler(myQDebugMessageHandler);
  }

private:

  static void myQDebugMessageHandler(QtMsgType, const QMessageLogContext &, const QString &msg)
  {
    std::cout << msg.toStdString().c_str();
  }

protected:

  //This is called when a std::endl has been inserted into the stream
  virtual int_type overflow(int_type v)
  {
    if (v == '\n')
      {
        log_window->append("");
      }
    return v;
  }


  virtual std::streamsize xsputn(const char *p, std::streamsize n)
  {
    QString str(p);
    if(str.contains("\n")){
        QStringList strSplitted = str.split("\n");

        log_window->moveCursor (QTextCursor::End);
        log_window->insertPlainText (strSplitted.at(0)); //Index 0 is still on the same old line

        for(int i = 1; i < strSplitted.size(); i++){
            log_window->append(strSplitted.at(i));
          }
      }else{
        log_window->moveCursor (QTextCursor::End);
        log_window->insertPlainText (str);
      }
    return n;
  }

private:
  std::ostream &m_stream;
  std::streambuf *m_old_buf;
  QTextEdit* log_window;
};


#endif // __DVSTRACKERDEBUGSTREAM_H__5E71037A_2D7F_456C_B919_BF491C99A569
