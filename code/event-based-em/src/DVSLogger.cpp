#include <QTextStream>
#include <QDir>
#include <iostream>

#include "inc/DVSLogger.hpp"


DVSLogger::DVSLogger(QString source, filetype_t ft, QObject *parent) :
  QObject(parent)
{
  if(ft==RECORD_FILE){
      QDir filePath("./");
      this->_file = new QFile();
      this->_file->setFileName(filePath.relativeFilePath(source));
      this->_isRecording = true;
      if(!this->_file->open(QIODevice::WriteOnly)){
          std::cout << (QObject::tr("DVSLogger: Error opening DVS recording file %1, error: %2\n").arg(source).arg(this->_file->errorString())).toStdString()<<std::endl;
        }
    }
  else{
      this->_file = new QFile(source);
      this->_isRecording = false;
      if(!this->_file->open(QIODevice::ReadOnly)){
          std::cout << ( QObject::tr("DVSLogger: Error opening DVS replay file %1, error: %2\n").arg(source).arg(this->_file->errorString())).toStdString()<<std::endl;
        }
    }
}

DVSLogger::~DVSLogger()
{
}

void DVSLogger::closeFile()
{
  if(this->_file->isOpen()){
      this->_file->close();
      this->_file = nullptr;
    }
  else this->_file = nullptr;
}

void DVSLogger::writeFile(const QByteArray &data)
{
    this->_file->QIODevice::write(data);
}

QByteArray DVSLogger::readFile()
{
  QByteArray events = this->_file->readAll();
  return events;
}

bool DVSLogger::endFile()
{
  return this->_file->atEnd();
}

bool DVSLogger::isRecording()
{
  return _isRecording;
}
