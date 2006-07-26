// ============================================================================
//
// = FILENAME
//    LiasClient.h
//
// = DESCRIPTION
//    Client-class. Sends commands (ascii) over TCP/IP ports
//
// = AUTHOR
//    Dan Lihtetchi , based on client.h
//
//
// ============================================================================
#ifndef LIASCLIENT_H
#define LIASCLIENT_H

//-----------------------------------------------------------------------------
#include "ace/SOCK_Connector.h"
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Synch.h"
#include "ace/Connector.h"
#include "ace/Acceptor.h"
#include "ace/Thread.h"
#include "ace/Reactor.h"

#include "ace/Task.h"
#include "ace/Activation_Queue.h"

#include <string>
#include <vector>

namespace LiasClientN
{

	// forward decls
	class Client;

	typedef ACE_Connector<Client,ACE_SOCK_CONNECTOR> Connector;

	class Client:
	public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
	{
		public:
			Client();
			~Client();
			int open(void*);
			int connect(std::string host, unsigned int port);
			int sendCommand(std::string command);
			// This call blocks until data is received from the server.
			std::string receiveData();
			void close(void*);
		private:
			Connector* _myconnector;
			// Received data from the server
			//LiasClientN::DataWriter _clientIn;
			char _serverData[200];
			char _buff;
			std::string _dataString;
	};

}
#endif //LIASCLIENT_H
