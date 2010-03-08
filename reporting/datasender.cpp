/***************************************************************************

                     datasender.cpp -  description
                           -------------------
    begin                : Wed August 9 2006
    copyright            : (C) 2006 Bas Kemper
                           (C) 2007 Ruben Smits //Changed subscription structure
    email                : kst@baskemper.be
                           first dot last at mech dot kuleuven dot be
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

#include <vector>
#include <rtt/Logger.hpp>
#include <rtt/os/Mutex.hpp>
#include <rtt/Property.hpp>
#include <rtt/base/PropertyIntrospection.hpp>
#include "socket.hpp"
#include "socketmarshaller.hpp"
#include "datasender.hpp"
#include "command.hpp"
#include "TcpReporting.hpp"
#include <rtt/types/TemplateTypeInfo.hpp>

namespace OCL
{
namespace TCP
{
    Datasender::Datasender(RTT::SocketMarshaller* _marshaller, Orocos::TCP::Socket* _os):
        Activity(10), os( _os ), marshaller(_marshaller)
    {
        limit = 0;
        curframe = 0;
        reporter = marshaller->getReporter();
        silenced = true;
        interpreter = new TcpReportingInterpreter(this);
    }

    Datasender::~Datasender()
    {
        subscriptions.clear();
        delete interpreter;
        delete os;
    }

    void Datasender::loop()
    {
        *os << "100 Orocos 1.0 TcpReporting Server 1.0" << std::endl;
        while( os->isValid() )
        {
            interpreter->process();
        }
        Logger::log() << Logger::Info << "Connection closed!" << Logger::endl;
    }

    bool Datasender::breakloop()
    {
        os->close();
        return true;
    }

    RTT::SocketMarshaller* Datasender::getMarshaller() const
    {
        return marshaller;
    }

    Socket& Datasender::getSocket() const
    {
        return *os;
    }

    bool Datasender::isValid() const
    {
        return os && os->isValid();
    }

    bool Datasender::addSubscription(const std::string name )
    {
        lock.lock();
        log(Debug)<<"Datasender::addSubscription: "<<name<<endlog();
        //Check if a property is available with that name?
        if(reporter->getReport()->find(name)!=NULL){
            //check if subscription already exists
            std::vector<std::string>::const_iterator pos =
                find(subscriptions.begin(),subscriptions.end(),name);
            if(pos!=subscriptions.end()){
                Logger::In("DataSender");
                log(Info)<<"Already subscribed to "<<name<<endlog();
                lock.unlock();
                return false;
            }else{
                Logger::In("DataSender");
                log(Info)<<"Adding subscription for "<<name<<endlog();
                subscriptions.push_back(name);
                lock.unlock();
                return true;
            }
        }else{
            Logger::In("DataSender");
            log(Error)<<name<<" is not available for reporting"<<endlog();
            lock.unlock();
            return false;
        }
    }

    void Datasender::remove()
    {
      getMarshaller()->removeConnection( this );
    }

    bool Datasender::removeSubscription( const std::string& name )
    {
        lock.lock();
        //check if subscription exists
        std::vector<std::string>::iterator pos =
            find(subscriptions.begin(),subscriptions.end(),name);
        if(pos!=subscriptions.end()){
            Logger::In("DataSender");
            log(Info)<<"Removing subscription for "<<name<<endlog();
            subscriptions.erase(pos);
            lock.unlock();
            return true;
        }else{
            Logger::In("DataSenser");
            log(Error)<<"No subscription found for "<<name<<endlog();
            lock.unlock();
            return false;
        }
    }

    void Datasender::listSubscriptions()
    {
        for(std::vector<std::string>::const_iterator elem=subscriptions.begin();
            elem!=subscriptions.end();elem++)
            *os<<"305 "<< *elem<<std::endl;
        *os << "306 End of list" << std::endl;
    }

    void Datasender::writeOut(base::PropertyBase* v)
    {
        *os<<"202 "<<v->getName()<<"\n";
        Property<PropertyBag>* bag = dynamic_cast< Property<PropertyBag>* >( v );
        if ( bag )
            this->writeOut( bag->value() );
        else {
            *os<<"205 " <<v->getDataSource()<<"\n";
        }

    }

    void Datasender::writeOut(const PropertyBag &v)
    {
        for (
             PropertyBag::const_iterator i = v.getProperties().begin();
             i != v.getProperties().end();
             i++ )
            {
                this->writeOut( *i );
            }

    }


    void Datasender::checkbag(const PropertyBag &v)
    {
        log(Debug)<<"Let's check the subscriptions"<<endlog();
        for(std::vector<std::string>::iterator elem = subscriptions.begin();
            elem!=subscriptions.end();elem++){
            base::PropertyBase* prop = reporter->getReport()->find(*elem);
            if(prop!=NULL){
                writeOut(prop);
            }else{
                Logger::In("DataSender");
                log(Error)<<*elem<<" not longer available for reporting,"<<
                    ", removing the subscription."<<endlog();
                subscriptions.erase(elem);
                elem--;
            }
        }
    }

    void Datasender::silence(bool newstate)
    {
        silenced = newstate;
    }

    void Datasender::setLimit(unsigned long long newlimit)
    {
        limit = newlimit;
    }

    void Datasender::serialize(const PropertyBag &v)
    {
        if( silenced ) {
            return;
        }

        lock.lock();
        if( !subscriptions.empty() && ( limit == 0 || curframe <= limit ) ){
            *os << "201 " <<curframe << " -- begin of frame\n";
            checkbag(v);
            *os << "203 " << curframe << " -- end of frame" << std::endl;
            curframe++;
            if( curframe > limit && limit != 0 )
            {
                *os << "204 Limit reached" << std::endl;
            }
        }
        lock.unlock();
    }

}
}
