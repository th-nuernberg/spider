#ifndef __COMMANDS_HPP__DE8555E9_1B8E_47A6_BF34_8AE88A27C9BE
#define __COMMANDS_HPP__DE8555E9_1B8E_47A6_BF34_8AE88A27C9BE

#include <iostream>
#include <string>
#include <sstream>
#include <QSerialPort>

namespace nst {
  namespace commands {

    /**
 * Command - Abstract command interface
 */
    struct Command
    {
      virtual ~Command() {}
      virtual const std::string toString() const = 0;
    };


    /**
 * MotorDriver - Enable or Disable the Motor Driver of the DVS Tracker
 */
    struct LaserMotorDriver : Command
    {
      LaserMotorDriver(bool enabled = true) : _enabled(enabled) {}

      const std::string toString() const override
      {
        if (this->_enabled)
          return std::string("!L=+\n");
        else
          return std::string("!L=-\n");
      }

      void enable() { this->_enabled = true; }
      void disable() { this->_enabled = false; }

    private:
      bool _enabled = true;
    };


    /**
 * DVS - Enable or disable event streaming from the retina
 */
    struct DVS : Command
    {
      DVS(bool enabled = true) : _enabled(enabled) {}

      const std::string toString() const override
      {
        if (this->_enabled)
           // FIXME make this available in the constructor using the timeformat_t
          return std::string("E+\n!E3\n");
        else
          return std::string("E-\n");
      }

      void enable() { this->_enabled = true; }
      void disable() { this->_enabled = false; }

    private:
      bool _enabled = true;
    };

    /**
 * mdw_base_t - Base class for all motor duty width / velocity commands
 *
 * TODO: check if the widths are really correct for DW
 */
    template <unsigned LASERMOTORID>
    struct md_base_t
    {

      md_base_t(int width) : _width(width) { validate(); }
      void setWidth(int width) { _width = width; validate(); }

    protected:
      const unsigned _id = LASERMOTORID;
      int _width;

    private:
      void validate()
      {
        if (_width >  100) _width =  100;
        if (_width < -100) _width = -100;
      }
    };


    /**
 * Macro to define any kind of motor command for duty width and velocity
 */
#define MAKE_LASER_MOTOR_COMMAND(TYPE, ID, CMD) \
  struct M ## TYPE ## ID: Command, md_base_t<ID> \
    { \
  M ## TYPE ## ID(unsigned width) : md_base_t(width) {} \
  \
  const std::string toString() const override \
    { \
  std::stringstream ss; \
  ss << "!" << (CMD) << std::to_string(_id) << "=" << std::to_string(_width) << "\n"; \
  return ss.str(); \
  } \
  };

    /*
 *	DW : Duty Width
 *	DWD: Duty Width with Decay
 *	V  : Velocity
 *	VD : Velicity with Decay
 */

    // commands::MDW0
    MAKE_LASER_MOTOR_COMMAND(DW,  0, "M")

    // commands::MDW1
    MAKE_LASER_MOTOR_COMMAND(DW,  1, "M")

    // commands::MDWD0
    MAKE_LASER_MOTOR_COMMAND(DWD, 0, "MD")

    // commands::MDWD1
    MAKE_LASER_MOTOR_COMMAND(DWD, 1, "MD")

    // commands::MV0
    MAKE_LASER_MOTOR_COMMAND(V,   0, "MV")

    // commands::MV1
    MAKE_LASER_MOTOR_COMMAND(V,   1, "MV")

    // commands::MVD0
    MAKE_LASER_MOTOR_COMMAND(VD,  0, "MVD")

    // commands::MVD1
    MAKE_LASER_MOTOR_COMMAND(VD,  1, "MVD")

    // finalize
#undef MAKE_LASER_MOTOR_COMMAND


    /*
 * Empty - The empty command is represented by a simple newline.
 */
    struct Empty : Command
    {
                     const std::string toString() const override
    {
                     return "\n";
  }
  };

  }} // nst::commands


inline
QSerialPort* operator<< (QSerialPort* port, const nst::commands::Command &cmd)
{
  port->write(cmd.toString().c_str());
  return port;
}


#endif /* __COMMANDS_HPP__DE8555E9_1B8E_47A6_BF34_8AE88A27C9BE */

