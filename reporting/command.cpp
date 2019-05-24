/***************************************************************************

        command.cpp - Processes client commands
           -------------------
    begin        : Wed Aug 9 2006
    copyright    : (C) 2006 Bas Kemper
    email        : kst@ <my name> .be

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

#include <string>
#include <cstdlib> /* strtoull */
#include <cctype>
#include <cerrno>
#include <rtt/Property.hpp>
#include <boost/algorithm/string.hpp>
#include "NiceHeaderMarshaller.hpp"
#include "TcpReporting.hpp"
#include "command.hpp"

namespace OCL {

/**
 * Output a XML file (prefixed with '300 ') containing the header to the socket.
 */
class HeaderCommand : public Command {
 protected:
  void maincode(int, std::string*) {
    if (_parent->getSession()->lastSerializedPropertyBag) {
      std::vector<std::string> available_items =
          _parent->getSession()->lastSerializedPropertyBag->list();
      for (unsigned int i = 0; i < available_items.size(); i++) {
        ostream() << "305 " << available_items[i] << std::endl;
      }
      ostream() << "306 End of list" << std::endl;
    }
  }

 public:
  HeaderCommand(TcpReportingInterpreter* parent) : Command("HEADERS", parent) {}

  ~HeaderCommand() {}

  void manualExecute() { maincode(0, 0); }
};

class HelpCommand : public Command {
 protected:
  /**
   * Print list of available commands to the socket.
   */
  void printCommands() {
    ostream() << "Use HELP <command>" << std::endl;
    for (unsigned int i = 0; i < _parent->cmds.size(); i++) {
      ostream() << _parent->cmds[i]->getName() << '\n';
    }
    ostream() << '.' << std::endl;
  }

  /**
   * Print help for <command> with <name> to the socket.
   */
  void printHelpFor(const std::string& name, Command* command) {
    ostream() << "Name: " << name << std::endl;
    ostream() << "Usage: " << name;
    ostream() << " " << command->getSyntax();
    ostream() << std::endl;
  }

  /**
   * Print help for the given command to the socket.
   */
  void printHelpFor(const std::string& cmd) {
    const std::vector<Command*>& cmds = _parent->giveCommands();
    for (unsigned int i = 0; i < cmds.size(); i++) {
      if (cmds[i]->getName() == cmd) {
        printHelpFor(cmd, cmds[i]);
        return;
      }
    }
    printCommands();
  }

  void maincode(int argc, std::string* params) {
    if (argc == 0) {
      printCommands();
    } else {
      printHelpFor(params[0]);
    }
  }

 public:
  HelpCommand(TcpReportingInterpreter* parent)
      : Command("HELP", parent, 0, 1, "[nothing | <command name>]") {}
};

class ListCommand : public Command {
 protected:
  void maincode(int, std::string*) { ostream() << "103 none" << std::endl; }

 public:
  ListCommand(TcpReportingInterpreter* parent)
      : Command("LISTEXTENSIONS", parent) {}
};

class QuitCommand : public Command {
 protected:
  void maincode(int, std::string*) { _parent->getSession()->terminate(); }

 public:
  QuitCommand(TcpReportingInterpreter* parent) : Command("QUIT", parent) {}
};

class SetLimitCommand : public Command {
 protected:
  void maincode(int, std::string* args) {
    int olderr = errno;
    char* tailptr;
    unsigned long long limit = strtoull(args[0].c_str(), &tailptr, 10);
    if (*tailptr != '\0' || (errno != olderr && errno == ERANGE)) {
      sendError102();
    } else {
      _parent->getSession()->frameLimit = limit;
      ostream() << "101 OK" << std::endl;
    }
  }

 public:
  SetLimitCommand(TcpReportingInterpreter* parent)
      : Command("SETLIMIT", parent, 1, 1, "<frameid>") {}
};

/**
 * Disable/enable output of data on the socket.
 */
class SilenceCommand : public Command {
 protected:
  void maincode(int, std::string* args) {
    boost::to_upper(args[0]);
    if (args[0] == "ON") {
      _parent->getSession()->silenced = true;
    } else if (args[0] == "OFF") {
      _parent->getSession()->silenced = false;
    } else {
      sendError102();
      return;
    }
    ostream() << "107 Silence " << args[0] << std::endl;
  }

 public:
  SilenceCommand(TcpReportingInterpreter* parent)
      : Command("SILENCE", parent, 1, 1, "[ON | OFF]") {}
};

/**
 * Add data stream to be printed
 */
class SubscribeCommand : public Command {
 protected:
  void maincode(int, std::string* args) {
    if (_parent->getSession()->addSubscription(args[0])) {
      ostream() << "302 " << args[0] << std::endl;
    } else {
      ostream() << "301 " << args[0] << std::endl;
    }
  }

 public:
  SubscribeCommand(TcpReportingInterpreter* parent)
      : Command("SUBSCRIBE", parent, 1, 1, "<source name>") {}
};

class SubscriptionsCommand : public Command {
 protected:
  void maincode(int, std::string*) {
    for (boost::unordered_set<std::string>::const_iterator elem =
             _parent->getSession()->getSubscriptions().begin();
         elem != _parent->getSession()->getSubscriptions().end(); elem++) {
      ostream() << "305 " << *elem << std::endl;
    }
    ostream() << "306 End of list" << std::endl;
  }

 public:
  SubscriptionsCommand(TcpReportingInterpreter* parent)
      : Command("SUBS", parent) {}
};

class UnsubscribeCommand : public Command {
 protected:
  void maincode(int, std::string* args) {
    if (_parent->getSession()->removeSubscription(args[0])) {
      ostream() << "303 " << args[0] << std::endl;
    } else {
      ostream() << "304 " << args[0] << std::endl;
    }
  }

 public:
  UnsubscribeCommand(TcpReportingInterpreter* parent)
      : Command("UNSUBSCRIBE", parent, 1, 1, "<source name>") {}
};

class VersionCommand : public Command {
 protected:
  void maincode(int, std::string* args) {
    if (args[0] == "1.0") {
      _parent->setVersion10();
      ostream() << "101 OK" << std::endl;
    } else {
      ostream() << "106 Not supported" << std::endl;
    }
  }

 public:
  VersionCommand(TcpReportingInterpreter* parent)
      : Command("VERSION", parent, 1, 1, "1.0") {}
};

/*
 * The default Orocos Command objects are not used since we
 * do not use Data Sources here.
 */
class Command;

Command::Command(std::string name, TcpReportingInterpreter* parent)
    : _name(name), _parent(parent), _minargs(0), _maxargs(0), _syntax("") {}

Command::Command(std::string name, TcpReportingInterpreter* parent,
                 unsigned int minargs, unsigned int maxargs)
    : _name(name),
      _parent(parent),
      _minargs(minargs),
      _maxargs(maxargs),
      _syntax("") {}

Command::Command(std::string name, TcpReportingInterpreter* parent,
                 unsigned int minargs, unsigned int maxargs, std::string syntax)
    : _name(name),
      _parent(parent),
      _minargs(minargs),
      _maxargs(maxargs),
      _syntax(syntax) {}

Command* Command::find(const std::vector<Command*>& cmds,
                       const std::string& cmp) {
  for (unsigned int j = 0; j < cmds.size(); j++) {
    if (*cmds[j] == cmp) {
      return cmds[j];
    }
  }
  return 0;
}

const std::string& Command::getName() const { return _name; }

bool Command::is(std::string& cmd) const { return cmd == _name; }

bool Command::operator==(const std::string& cmp) const { return cmp == _name; }

bool Command::operator!=(const std::string& cmp) const { return cmp != _name; }

bool Command::operator<(const Command& cmp) const {
  return _name < cmp.getName();
}

std::string& Command::getSyntax() { return _syntax; }

bool Command::sendError102() const {
  ostream() << "102 Syntax: " << _name << ' ' << _syntax << std::endl;
  _parent->getSession()->flushOstream();
  return false;
}

bool Command::correctSyntax(unsigned int argc) {
  if (argc < _minargs || argc > _maxargs) {
    return sendError102();
  }
  return true;
}

void Command::execute(int argc, std::string* args) {
  if (correctSyntax(argc)) {
    maincode(argc, args);
    _parent->getSession()->flushOstream();
  }
}

std::ostream& Command::ostream() const {
  return _parent->getSession()->getOstream();
}

TcpReportingInterpreter::TcpReportingInterpreter(TcpReportingSession* parent)
    : _parent(parent) {
  addCommand(new VersionCommand(this));
  addCommand(new HelpCommand(this));
  addCommand(new QuitCommand(this));
}

TcpReportingInterpreter::~TcpReportingInterpreter() {
  for (std::vector<Command*>::iterator i = cmds.begin(); i != cmds.end(); i++) {
    delete *i;
  }
}

void TcpReportingInterpreter::addCommand(Command* command) {
  // this method has a complexity of O(n) because we want
  // the list to be sorted.

  std::vector<Command*>::iterator i = cmds.begin();
  while (i != cmds.end() && *command < **i) {
    i++;
  }

  // avoid duplicates
  if (i != cmds.end() && *command == (*i)->getName()) {
    return;
  }
  cmds.insert(i, command);
}

const std::vector<Command*>& TcpReportingInterpreter::giveCommands() const {
  return cmds;
}

TcpReportingSession* TcpReportingInterpreter::getSession() const {
  return _parent;
}

void TcpReportingInterpreter::processLine(std::string line) {
  if (line.empty()) {
    return;
  }

  /* parseParameters returns data by reference */
  std::string cmd;
  std::string* params;

  unsigned int argc = parseParameters(line, cmd, &params);

  boost::to_upper(cmd);

  /* search the command to be executed */
  bool correct = false;
  Command* command = Command::find(cmds, cmd);
  if (command) /* command found */
  {
    command->execute(argc, params);
    correct = true;
  } else {
    Logger::log() << Logger::Error << "Invalid command: " << line
                  << Logger::endl;
  }

  if (!correct) {
    getSession()->getOstream() << "105 Command not found" << std::endl;
    getSession()->flushOstream();
  }
}

unsigned int TcpReportingInterpreter::parseParameters(std::string& ipt,
                                                      std::string& cmd,
                                                      std::string** params) {
  unsigned int argc = 0;
  std::string::size_type pos = ipt.find_first_of("\t ", 0);
  while (pos != std::string::npos) {
    pos = ipt.find_first_of("\t ", pos + 1);
    argc++;
  }
  if (argc > 0) {
    *params = new std::string[argc];
    pos = ipt.find_first_of("\t ", 0);
    cmd = ipt.substr(0, pos);
    unsigned int npos;
    for (unsigned int i = 0; i < argc; i++) {
      npos = ipt.find_first_of("\t ", pos + 1);
      (*params)[i] = ipt.substr(pos + 1, npos - pos - 1);
      pos = npos;
    }
  } else {
    cmd = ipt;
    *params = 0;
  }
  return argc;
}

void TcpReportingInterpreter::setVersion10() {
  //  removeCommand("VERSION"); //TODO: implement
  addCommand(new ListCommand(this));
  addCommand(new HeaderCommand(this));
  addCommand(new SilenceCommand(this));
  addCommand(new SetLimitCommand(this));
  addCommand(new SubscribeCommand(this));
  addCommand(new UnsubscribeCommand(this));
  addCommand(new SubscriptionsCommand(this));
}
}
