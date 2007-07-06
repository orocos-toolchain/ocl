/***************************************************************************

                     datasender.cpp -  description
                           -------------------
    begin                : Wed August 9 2006
    copyright            : (C) 2006 Bas Kemper
    email                : kst@baskemper.be
 
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

#include <list>
#include <queue>
#include <rtt/Logger.hpp>
#include <rtt/os/Mutex.hpp>
#include <rtt/Property.hpp>
#include <rtt/PropertyIntrospection.hpp>
#include "socket.hpp"
#include "socketmarshaller.hpp"
#include "datasender.hpp"
#include "command.hpp"
#include "../TcpReporting.hpp"

using RTT::Logger;

namespace {
    /**
     * A Subscription is a class indicating that someone wants
     * data with the given attribute.
     */
    class Subscription
    {
        private:
            std::string _name;

        public: /* bug in gcc? */ // TODO:...
            /**
             * Remove the subscription with the given name from
             * all dependents.
             *
             * Instruct the caller to delete this Subscription if
             * removethis is set to True.
             *
             * Return true if the given subscription should be deleted
             * because it is the subscription with the given name or
             * has no dependants anymore.
             */
            virtual bool remove( const std::string& name, bool& removethis ) = 0;
            virtual void printDataElement( std::ostream& opt ) = 0;

        public:
            Subscription( std::string name );
            virtual ~Subscription();

            /**
             * Return true if this instance is an instance
             * of SubscriptionManager. Return false otherwise.
             */
            virtual bool isCollection() const;

            /**
             * Give the name of this subscription.
             */
            const std::string& getName() const;

            bool operator==(std::string &cmp) const;
    };

    /**
     * Basic subscription.
     */
    class BasicSubscription : public Subscription
    {
        private:
            const std::string& _parentName;

        protected:
            virtual bool remove( const std::string& name, bool& removethis );
            virtual void printDataElement( std::ostream& opt );

        public:
            BasicSubscription( const std::string& parent, std::string name );
    };

    /**
     * A SubscriptionManager is a list of Subscriptions.
     * It manages a list of subscriptions.
     */
    class SubscriptionManager : public Subscription
    {
        public: // TODO: make protected
            std::list<Subscription*> subscriptions;
            std::string _fullName;

            /**
             * Get the Subscription with the given name.
             */
            Subscription* getMember( std::string property ) const;

            virtual void printDataElement( std::ostream& opt );

            /**
             * Add this subscription to this subscription manager.
             */
            void addSubscription( Subscription* subcription );

            /**
             * Create the subscription manager with the given name
             * (can be recursively)
             */
            SubscriptionManager* createGroup( std::string name );

            /**
             * Create a subscription manager without a parent.
             */
            SubscriptionManager( std::string name );

            virtual bool remove( const std::string& name, bool& removethis );

        public:
            SubscriptionManager( std::string name, const std::string& parentname );
            ~SubscriptionManager();

            /**
             * Check if there is a subscription for the given property
             * (non-recursive)
             */
            bool contains( const std::string& property ) const;

            /**
             * RTTI. Returns always true.
             */
            virtual bool isCollection() const;

            /**
             * Give the subscription manager child with the given name.
             * Return 0 if this subscription manager does not exist.
             */
            SubscriptionManager* giveSubscriptionManager( std::string& name ) const;

            /**
             * Return true if this SubscriptionManager does not contain
             * any subscription.
             */
            bool isEmpty() const;

            /**
             * Print full name.
             */
            virtual const std::string& fullNameWithDot() const;
    };

    class TopSubscriptionManager : public SubscriptionManager
    {
        private:
            static const std::string _emptyString;
            Orocos::TcpReporting* _report;

            /**
             * Search the apprpriate subscription manager and
             * add a subscription for the given name.
             */
            class SubscriptionVisitor : public RTT::PropertyBagVisitor
            {
                private:
                    bool _result;
                    TopSubscriptionManager* topmngr;
                    const std::string& _name;

                    /**
                     * Create a subscription in the given subscription manager and the
                     * given name. Ignore the request if such a subscription already
                     * exists.
                     */
                    void createSubscription( SubscriptionManager* sm, const std::string& name );

                public:
                    /**
                     * Constructor with the name of the subscription to add.
                     */
                    SubscriptionVisitor( TopSubscriptionManager* manager, const std::string& name );

                    virtual ~SubscriptionVisitor();

                    /**
                     * Subscribe to the stream.
                     */
                    bool subscribe( const RTT::PropertyBag* base );

                    /**
                     * Final property
                     */
                    virtual void introspect(PropertyBase* p);

                    /**
                     * Go deeper
                     */
                    virtual void introspect(Property<PropertyBag>& p);

                    bool parseName(std::string& toadd);
            };

        public:
            /**
             * Manages all the subscriptions for a connection.
             * The PropertyBag contains the available streams.
             */
            TopSubscriptionManager( Orocos::TcpReporting* reporting );

            /**
             * Add a subscription for the stream with given name.
             */
            bool add( const std::string& toadd );

            /**
             * Print the contents of this SubscriptionManager to the given output
             * in human-readable format.
             */
            void printData( std::ostream& opt );

            /**
             * Print full name.
             */
            virtual const std::string& fullNameWithDot() const;

            /**
             * Remove a subscription
             */
            bool remove( const std::string& name );
    };
    const std::string TopSubscriptionManager::_emptyString = "";

    bool TopSubscriptionManager::add( const std::string& toadd )
    {
        SubscriptionVisitor sv( this, toadd );
        bool r = sv.subscribe( _report->getReport() );
        _report->cleanReport();
        return r;
    }

    const std::string& TopSubscriptionManager::fullNameWithDot() const
    {
        return _emptyString;
    }

    void TopSubscriptionManager::printData( std::ostream& opt )
    {
        printDataElement( opt );
    }

    TopSubscriptionManager::SubscriptionVisitor::SubscriptionVisitor(
            TopSubscriptionManager* manager, const std::string& name )
    : topmngr(manager), _name(name)
    {
    }

    TopSubscriptionManager::SubscriptionVisitor::~SubscriptionVisitor()
    {
    }

    void TopSubscriptionManager::SubscriptionVisitor::createSubscription(
            SubscriptionManager* sm, const std::string& name )
    {
        if( !sm->contains( name ) )
        {
            sm->addSubscription( new BasicSubscription( sm->fullNameWithDot(), name ) );
        }
        _result = true;
    }

    bool TopSubscriptionManager::SubscriptionVisitor::subscribe( const RTT::PropertyBag* base )
    {
        _result = false;
        base->identify(this);
        return _result;
    }

    void TopSubscriptionManager::SubscriptionVisitor::introspect(PropertyBase* p)
    {
        if( p->getName() == _name )
        {
            std::string::size_type t = _name.find_last_of( '.' );
            if( t == std::string::npos )
            {
                createSubscription( topmngr, _name );
            } else if( t > 0 && t + 1 != _name.length() ) {
                createSubscription( topmngr->createGroup( _name.substr( 0, t ) ),
                    _name.substr( t + 1, _name.length() - t - 1 ) );
            } else {
                return;
            }
        }
    }

    void TopSubscriptionManager::SubscriptionVisitor::introspect(Property<PropertyBag>& p)
    {
        if( p.getName().length() + 1 < _name.length() &&
            !_name.compare( 0, p.getName().length(), p.getName() ) &&
            _name[ p.getName().length() ] == '.' )
        {
            if( p.get().getType() == "std::vector<double>" || p.get().getType() == "std::vector<int>" )
            {
                if( _name.substr( p.getName().length() + 1 ).find('.') == std::string::npos )
                {
                    PropertyBase* f = p.get().find( _name.substr( p.getName().length() + 1 ) );
                    if( f )
                    {
                        createSubscription( topmngr->createGroup( p.getName() ),
                            _name.substr( p.getName().length() + 1 ) );
                    }
                }
            } else {
                Logger::log() << Logger::Info << "TopSubscriptionManager::SubscriptionVisitor::introspect: Unknown bag type " << p.get().getType() << Logger::endl;
            }
        }
    }

    bool TopSubscriptionManager::remove( const std::string& name )
    {
        bool r;
        return SubscriptionManager::remove( name, r );
    }

    Subscription::Subscription( std::string name )
    : _name( name )
    {
    }

    Subscription::~Subscription()
    {
    }

    bool Subscription::isCollection() const
    {
        return false;
    }

    const std::string& Subscription::getName() const
    {
        return _name;
    }

    bool Subscription::operator==(std::string &cmp) const
    {
        return getName() == cmp;
    }

    BasicSubscription::BasicSubscription( const std::string& parent, std::string name )
    : Subscription( name ), _parentName(parent)
    {
    }

    bool BasicSubscription::remove( const std::string& name, bool& removethis )
    {
        if( name == getName() )
        {
            removethis = true;
            return true;
        }
        return false;
    }

    void BasicSubscription::printDataElement( std::ostream& opt )
    {
        opt << "305 " << _parentName << getName() << '\n';
    }

    SubscriptionManager::SubscriptionManager( std::string name )
        : Subscription( name ), subscriptions()
    {
        _fullName = name;
    }

    SubscriptionManager::SubscriptionManager( std::string name, const std::string& parentName )
    : Subscription( name ), subscriptions()
    {
        _fullName = parentName + name + '.';
    }

    SubscriptionManager::~SubscriptionManager()
    {
        while( !subscriptions.empty() )
        {
            delete subscriptions.front();
            subscriptions.pop_front();
        }
    }

    void SubscriptionManager::addSubscription( Subscription* subscription )
    {
        subscriptions.push_back( subscription );
    }

    bool SubscriptionManager::contains(const std::string& property) const
    {
        return ( getMember( property.c_str() ) != 0 );
    }

    SubscriptionManager* SubscriptionManager::createGroup( std::string name )
    {
        std::string::size_type t = name.find('.');
        std::string pname;

        if( t == std::string::npos )
        {
            pname = name;
        } else {
            pname = name.substr( 0, t );
        }

        SubscriptionManager* r = dynamic_cast<SubscriptionManager*>(getMember( pname ));
        if( !r )
        {
            r = new SubscriptionManager( pname, fullNameWithDot() );
            subscriptions.push_back(r);
        }
        if( t != std::string::npos )
        {
            r = r->createGroup( name.substr( t + 1, name.length() - t - 1 ) );
        }
        return r;
    }

    const std::string& SubscriptionManager::fullNameWithDot() const
    {
        return _fullName;
    }

    bool SubscriptionManager::isCollection() const
    {
        return true;
    }

    bool SubscriptionManager::isEmpty() const
    {
        return subscriptions.empty();
    }

    Subscription* SubscriptionManager::getMember( std::string property ) const
    {
        for( std::list<Subscription*>::const_iterator i = subscriptions.begin();
             i != subscriptions.end();
             i++ )
        {
            if( **i == property )
            {
                return *i;
            }
        }
        return 0;
    }

    bool SubscriptionManager::remove( const std::string& name, bool& removethis )
    {
        std::string::size_type partpos = name.find('.');
        const std::string* what;
        Subscription* s = 0;
        std::string t;

        if( partpos == std::string::npos )
        {
            s = getMember(name);
            what = &name;
        } else if( partpos + 1 < name.length() ) {
            s = getMember(name.substr(0, partpos));
            t = name.substr(partpos + 1, name.length() - partpos - 1);
            what = &t;
        }
        bool ret = false;
        if( s )
        {
            ret = s->remove( *what, removethis );
            if( removethis )
            {
                subscriptions.remove(s);
                removethis = subscriptions.empty();
            }
        }
        return ret;
    }

    SubscriptionManager* SubscriptionManager::giveSubscriptionManager( std::string& name ) const
    {
        if( getName() != name.substr( 0, getName().length() ) )
        {
            return 0;
        }
        std::string::size_type prevnext = getName().length();
        std::string::size_type next = name.find_first_of( '.', getName().length() );
        Subscription* ret = (Subscription*)( (next == std::string::npos) ? 0 : this );
        while( ret != 0 )
        {
            ret = dynamic_cast<SubscriptionManager*>(ret)->getMember( name.substr( prevnext, next ) );
            if( !ret || !ret->isCollection() )
            {
                return 0;
            }
            prevnext = next + 1;
            next = name.find_first_of( '.', prevnext );
            if( next == std::string::npos ) /* final case */
            {
                ret = dynamic_cast<SubscriptionManager*>(ret)->getMember( name.substr( prevnext, name.length() ) );
                return ( ret && ret->isCollection() ?
                        dynamic_cast<SubscriptionManager*>(ret) :
                        0 );
            }
        }
        return 0;
    }

    void SubscriptionManager::printDataElement( std::ostream& opt )
    {
        for( std::list<Subscription*>::const_iterator i = subscriptions.begin();
             i != subscriptions.end();
             i++ )
        {
            (*i)->printDataElement( opt );
        }
    }

    TopSubscriptionManager::TopSubscriptionManager(Orocos::TcpReporting* report)
    : SubscriptionManager( _emptyString ), _report(report)
    {
    }
}

namespace Orocos
{
namespace TCP
{
    Datasender::Datasender(RTT::SocketMarshaller* marshaller, Orocos::TCP::Socket* os)
        : NonPeriodicActivity(10), _os( os ), _marshaller(marshaller)
    {
        limit = 0;
        curframe = 0;
        subs = new TopSubscriptionManager(marshaller->getReporter());
        cursubs = subs;
        _silenced = true;
        interpreter = new TcpReportingInterpreter(this);
    }

    Datasender::~Datasender()
    {
        delete subs;
        delete interpreter;
        delete _os;
    }

    void Datasender::loop()
    {
        *_os << "100 Orocos 1.0 TcpReporting Server 1.0" << std::endl;
        while( _os->isValid() )
        {
            log(Debug)<<"Lets process incomming messages"<<endlog();
            
            interpreter->process();
        }
        Logger::log() << Logger::Info << "Connection closed!" << Logger::endl;
    }

    bool Datasender::breakloop()
    {
        _os->close();
        return true;
    }

    RTT::SocketMarshaller* Datasender::getMarshaller() const
    {
        return _marshaller;
    }

    Socket& Datasender::getSocket() const
    {
        return *_os;
    }

    bool Datasender::isValid() const
    {
        return _os && _os->isValid();
    }

    int Datasender::addSubscription( std::string name )
    {
        lock.lock();
        int ret = subs->add( name );
        lock.unlock();
        return ret;
    }

    void Datasender::remove()
    {
      getMarshaller()->removeConnection( this );
    }

    bool Datasender::removeSubscription( const std::string& name )
    {
        lock.lock();
        bool ret = subs->remove( name );
        lock.unlock();
        return ret;
    }

    void Datasender::listSubscriptions()
    {
        subs->printData( *_os );
        *_os << "306 End of list" << std::endl;
    }

    template< class T >
            void Datasender::writeOut(const Property<T> &v) 
    { 
        std::stringstream buffer;
        buffer << v.get();
        *_os << "202 ";
        *_os << cursubs->fullNameWithDot() << v.getName().c_str()
                << '\n' << "205 " << buffer.str().c_str() << std::endl;
    }

    template< class T >
    bool Datasender::hasSubscription(Property<T> &v)
    {
        if( cursubs->contains( v.getName() ) )
        {
            return true;
        }
        return false;
    }

    void Datasender::introspect(PropertyBase* pb)
    {
        Logger::log() << Logger::Error << "Datasender::introspect(PropertyBase* pb)" << Logger::endl;
    }

    void Datasender::introspect(Property<bool> &v) 
    {
        if( hasSubscription(v) )
        {
            writeOut(v);
        }
    }

    void Datasender::introspect(Property<char> &v) 
    { 
        if( hasSubscription(v) )
        {
            writeOut(v);
        }
    }

    void Datasender::introspect(Property<int> &v) 
    { 
        if( hasSubscription(v) )
        {
            writeOut(v);
        }
    }

    void Datasender::introspect(Property<unsigned int> &v) 
    { 
        if( hasSubscription(v) )
        {
            writeOut(v);
        }
    }

    void Datasender::introspect(Property<double> &v) 
    {
        if( hasSubscription(v) )
        {
            writeOut(v);
        }
    }

    void Datasender::introspect(Property<std::string> &v) 
    {
        if( hasSubscription(v) )
        {
            writeOut(v);
        }
    }

    void Datasender::introspect(Property<PropertyBag> &v) 
    {
        introspect(v.get(), v.getName());
    }

    void Datasender::introspect(const PropertyBag &v, std::string name)
    {
        SubscriptionManager* old = cursubs;
        cursubs = old->giveSubscriptionManager(name);
        if( cursubs )
        {
            checkbag(v);
        }
        cursubs = old;
    }

    void Datasender::checkbag(const PropertyBag &v)
    {
        for (
             unsigned int i = 0;
             i < v.getProperties().size();
             i++ )
        {
            v.getProperties().at(i)->identify(this);
        }
    }

    void Datasender::silence(bool newstate)
    {
        _silenced = newstate;
    }

    void Datasender::setLimit(unsigned long long newlimit)
    {
        limit = newlimit;
    }

    void Datasender::serialize(const PropertyBag &v)
    {
        if( _silenced ) {
            return;
        }

        lock.lock();
        if( !subs->isEmpty() && ( limit == 0 || curframe <= limit ) )
        {
            checkbag(v);
            *_os << "203 " << curframe << " -- end of frame" << std::endl;
            curframe++;
            if( curframe > limit && limit != 0 )
            {
                *_os << "204 Limit reached" << std::endl;
            }
        }
        lock.unlock();
    }

}
}
