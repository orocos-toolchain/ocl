// Copyright (C) 2006 Wim Meeussen <wim dot meeussen at mech dot kuleuven dot be>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  

#include "gcodeReceiver.hpp"
#include <ocl/ComponentLoader.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>

namespace OCL{
    
    using namespace std;
    using namespace RTT;
    using namespace KDL;
    
    gcodeReceiver::gcodeReceiver(string name):
        TaskContext(name,PreOperational),
        // name the properties
        port_prop("port","port to receive positions",50006),
        initial_pos("CartesianStartingPosition",Frame::Identity()),
        cart_pos("CartesianDesiredPosition",Frame::Identity()),
        received(false)
    {
        // register and read properties
        log(Info) << this->getName() << ": registring and reading Properties" << endlog();
        properties()->addProperty(&port_prop);
        attributes()->addAttribute(&initial_pos);
        
        // register ports
        log(Info) << this->getName() << ": registring ports" << endlog();
        ports()->addPort(&cart_pos);
    }
    
    
    gcodeReceiver::~gcodeReceiver()
    {
    }
  
    bool gcodeReceiver::configureHook()
    {
        int portno;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) return false;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = port_prop;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            return false;
        listen(sockfd,5);
        clilen = sizeof(cli_addr);
        
        Logger::In in(this->getName().c_str());
        log(Warning)<<"Waiting for connection on port: "<<portno<<endlog();
        
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) return false;
        bzero(buffer,256);
        int flags = fcntl(newsockfd,F_GETFL);
        fcntl(newsockfd,F_SETFL,flags | O_NONBLOCK);
        
        //Ask initial position
        while(!received)
            this->updateHook();
        initial_pos.set(f);
        
        return true;
    }
    
    bool gcodeReceiver::startHook()    
    {
        this->updateHook();
        return true;
    }
    
    
    void gcodeReceiver::updateHook()
    {
        int i=0;
        int j=0;
        int n =read(newsockfd,buffer,255);
        if(n>0){
            if(strchr(buffer,'\n') != NULL){
                log(Warning)<<"End of gcode reached. Stopping component"<<endlog();
                this->stop();
            }
            char * pch;
            pch = strtok (buffer,", ");
            j=0;
            while (pch != NULL){
                data[j]=strtod(pch,NULL);
                pch = strtok (NULL, ", ");
                if (j<3){
                    f.p[j]=data[j];
                }
                if (j==5){
                    f.M=Rotation::RPY(data[3], data[4], data[5]);
                } 
                j++;
            }
            bzero(buffer,256);
            n = write(newsockfd,"ack",7);
            //if (n =< 0 )
                //this->fatal();
            i++;
            cart_pos.Set(f);
            received=true;
        }
    }
    
  
    void gcodeReceiver::stopHook()
    {
        close(newsockfd);
        close(sockfd);
        log(Info) << this->getName() << ": shutdown" << endlog();
    }
}//namespace OCL

ORO_CREATE_COMPONENT(OCL::gcodeReceiver);
  
