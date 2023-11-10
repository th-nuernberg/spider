#include "inc/DVSBytestreamParser.hpp"
#include <iostream>
#include <cstdlib>
#include "inc/Utils.hpp"

namespace nst {

  DVSBytestreamParser:: DVSBytestreamParser(uint8_t id, DVSEvent::timeformat_t fmt)
    : QObject(), _id(id), _timeformat(fmt), _state(0), _ev(nullptr)
  {
  }

  DVSBytestreamParser::
  ~DVSBytestreamParser()
  {
  }

  uint8_t DVSBytestreamParser::
  id() const
  {
    return _id;
  }

  void DVSBytestreamParser::
  parse(const unsigned char c)
  {
    switch (_state) {
      case 0:
        if ((c & 0x80) != 0) {
            _ev = new DVSEvent;
            _ev->id = _id;
            _ev->x = static_cast<uint16_t>(c) & 0x7F;
            _ev->y = 0;
            _ev->p = 0;
            _ev->t = 0;
            ++_state;
          }
        break;

      case 1:
        _ev->p = (static_cast<uint8_t>(c) & 0x80) >> 7;
        _ev->y = static_cast<uint16_t>(c) & 0x7F;
        if (_timeformat == DVSEvent::TIMEFORMAT_0BYTES) {
            emit eventReceived(std::move(_ev));
            _ev = nullptr;
            _state = 0;
          }
        else
          ++_state;
        break;

      case 2:
        if (_timeformat == DVSEvent::TIMEFORMAT_2BYTES)
          _ev->t |= static_cast<uint64_t>(c) << 8;
        else
          _ev->t |= static_cast<uint64_t>(c) << 16;
        ++_state;
        break;

      case 3:
        if (_timeformat == DVSEvent::TIMEFORMAT_2BYTES) {
            _ev->t |= static_cast<uint64_t>(c);
            emit eventReceived(std::move(_ev));
            _ev = nullptr;
            _state = 0;
          }
        else {
            _ev->t |= static_cast<uint64_t>(c) << 8;
            ++_state;
          }
        break;

      case 4:
        _ev->t |= static_cast<uint64_t>(c);
        emit eventReceived(std::move(_ev));
        _ev = nullptr;
        _state = 0;
        break;

      default:
        _state = 0;
        break;
      }

  }


  void DVSBytestreamParser::
  parseData(const QByteArray &data)
  {
    for (const char c: data)
      this->parse(static_cast<unsigned char>(c));
  }

  void DVSBytestreamParser::
  parseDataFile(const unsigned char c)
  {
      this->parse(c);
  }

  void DVSBytestreamParser::
  set_timeformat(DVSEvent::timeformat_t fmt)
  {
    _timeformat = fmt;
  }


} // nst::
