// ============================================================================
//
//
// = FILENAME
//    LiasClient.cpp
//
// = DESCRIPTION
//    client-classes. simply call connect(host,port) and sendCommand(string)
//	to send command
//	BE SURE TO END EACH COMMAND WITH \n
//
// = AUTHOR
//    Dan Lihtetchi, based on client.cpp
//
// ============================================================================
//
// ============================================================================
#include "liasclient.hpp"
#include <sstream>
#include <iostream>

#include <ace/Message_Block.h>
#include <ace/Date_Time.h>
#include <ace/OS.h>

#define PORT	9999
#define HOST	"localhost"

using namespace std;

//LiasClientN::Client::Client():_clientIn(_serverData,200){
LiasClientN::Client::Client(){
}

LiasClientN::Client::~Client()
{
	if(_myconnector){
		_myconnector->close();
        delete _myconnector;
        _myconnector=0;
	}
};

int
LiasClientN::Client::open(void*)
{
	ACE_DEBUG( (LM_DEBUG, " (%t) connection ESTABLISHED \n") );

	return 0; // keep registered
};



int
LiasClientN::Client::connect(std::string host, unsigned int port)
{
	ACE_INET_Addr addr(port,host.c_str());
        _myconnector = new Connector();

	Client *pointer_to_self = this;

    int result = _myconnector->connect(pointer_to_self, addr);
	if (result == -1)
                ACE_DEBUG((LM_DEBUG,"Connection FAILED \n"));
	return result;
};

int
LiasClientN::Client::sendCommand(std::string command)
{
	//manage the call time

	const char* charstr = command.c_str();
	//pfclog << _clientlog << "Sending command to the host: " << command << endm;
    //std::cout << "Sending command to the host: " << command << "\n";

	//int sent = peer().send_n(charstr, strlen(charstr));
	 peer().send_n(charstr, strlen(charstr));

	//pfclog << _clientlog << sent<< " bytes sent "<< endm;
	return 0;
};

/*
	Once the client blocks until all data is received. The method returns
	when the client receives a null character.
  */
std::string
LiasClientN::Client::receiveData(){
	// Receive one char at a time and push into a string
	int count;
	count=0;
	while(peer().recv_n ((&_buff), 1)!=0) {
		if (_buff == '\n')                        // Is it enter?
		{
		 // _clientIn.writeUChar(0);               // Finish the string correctly
		 _serverData[count++]=_buff;
		  //_dataString = (char*) (&(_serverData));
		  //pfclog << _clientlog << "Received data from server: \""<< _dataString<< "\"" << endm;
		 // _clientIn.reset();                     // ready for more commands
		 _dataString = _serverData;
		  return _dataString;//_dataString;
		}
		else if (_buff == '\0' || _buff == 10) {
			//ignore string terminators and windows carriage returns
		}
		else ;
		  //_clientIn.writeUChar(_buff);           // One more received
		  //_dataString += (char *)_buff;
		  _serverData[count++]=_buff;
	}
	return "\0";

};


void
LiasClientN::Client::close(void* param=0){

   if(_myconnector)
   {
       _myconnector->close();
       delete _myconnector;
       _myconnector=0;
   }
};
