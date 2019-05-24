/***************************************************************************

                    command.hpp - Processes client commands
                           -------------------
    begin                : Wed Aug 9 2006
    copyright            : (C) 2006 Bas Kemper
    email                : kst@ <my name> .be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef ORO_COMP_TCP_REPORTING_COMMAND_HPP
#define ORO_COMP_TCP_REPORTING_COMMAND_HPP
#include <vector>
#include <rtt/os/Mutex.hpp>
#include "tcpreportingsession.hpp"
#include <iostream>

namespace OCL {
class Command;
class RealCommand;
class TcpReportingSession;

/**
 * Reads a line from the client and interprete it.
 */
class TcpReportingInterpreter {
 protected:
  unsigned int parseParameters(std::string& ipt, std::string& cmd,
                               std::string** params);
  TcpReportingSession* _parent;

 public:
  /**
   * After setup, the interpreter will only recognize the command
   * 'VERSION 1.0' by default.
   */
  TcpReportingInterpreter(TcpReportingSession* parent);
  ~TcpReportingInterpreter();
  void processLine(std::string line);
  std::vector<Command*> cmds;

  /**
   * Get the marshaller associated with the current connection.
   */
  TcpReportingSession* getSession() const;

  /**
   * Accept all valid commands (except 'VERSION 1.0')
   */
  void setVersion10();

  /**
   * Return a reference to the command list.
   */
  const std::vector<Command*>& giveCommands() const;

  /**
   * Add support for the given command.
   */
  void addCommand(Command* command);
};

/**
 * Command pattern
 */
class Command {
 protected:
  std::string _name;
  TcpReportingInterpreter* _parent;
  unsigned int _minargs;
  unsigned int _maxargs;
  std::string _syntax;

  /**
   * Send the correct syntax to the client.
   * Return false.
   */
  bool sendError102() const;

  /**
   * Return the socket for this command.
   * Fast shortcut for _parent->getConnection()->getSocket()
   */
  inline std::ostream& ostream() const;

 public:
  Command(std::string name, TcpReportingInterpreter* parent,
          unsigned int minargs, unsigned int maxargs,
          std::string syntax);
  Command(std::string name, TcpReportingInterpreter* parent,
          unsigned int minargs, unsigned int maxargs);
  Command(std::string name, TcpReportingInterpreter* parent);

  virtual bool is(std::string& cmd) const;

  /**
   * Find the command with the given name in the vector.
   */
  static Command* find(const std::vector<Command*>& cmds,
                       const std::string& cmp);

  /**
   * Compare on name
   */
  bool operator==(const std::string& cmp) const;
  bool operator!=(const std::string& cmp) const;
  bool operator<(const Command& cmp) const;

  /**
   * Get the name of this command.
   */
  const std::string& getName() const;
  std::string &getSyntax();

  virtual void maincode(int argc, std::string* args) = 0;

  /**
   * Return true if the syntax is correct, false otherwise.
   * Send an error message to the client on incorrect syntax.
   */
  bool correctSyntax(unsigned int argc);

  /**
   * Execute this command.
   */
  void execute(int argc, std::string* args);
};

/**
 * Real command which can be executed.
 */
class RealCommand : public Command {
 public:
};
}
#endif
