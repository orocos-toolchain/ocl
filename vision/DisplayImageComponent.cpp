/***************************************************************************

                        ImageDisplayComponent.cxx -  description
                        -------------------------

    begin                : Jan 2008
    copyright            : (C) 2008 Francois Cauwe
    based on             : ReporterComponent.cpp made by Peter Soetens
    email                : francois@cauwe.org

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

#include "ImageDisplayComponent.hpp"
#include <rtt/marsh/PropertyDemarshaller.hpp>

namespace OCL{

    using namespace RTT;
    using namespace std;


    ImageDisplayComponent::ImageDisplayComponent(std::string name):
        TaskContext(name),
        config("Configuration","The name of the property file which lists which image dataports to be displayed.",
               "cpf/imagedisplay.cpf")
    {
        // add properties
        properties()->addProperty(&config);

        // Tell Orocos to load the Vision toolkit
        RTT::TypeInfoRepository::Instance()->addType( new IplImageTypeInfo() );

    }

    bool ImageDisplayComponent::configureHook(){
        Logger::In in("ImageDisplayComponent");

        // Load config file
        if ( this->marshalling()->readProperties( this->getName() + ".cpf" ) == false)
            return false;

        log(Info) << "Loading Ports to display from file." <<endlog();
        PropertyDemarshaller dem( config.get() );
        PropertyBag bag;
        if (dem.deserialize( bag ) == false ) {
            log(Error) << "Reading file "<< config.get() << " failed."<<endlog();
            return false;
        }

        bool ok = true;
        PropertyBag::const_iterator it = bag.getProperties().begin();
        while ( it != bag.getProperties().end() ){
            Property<std::string>* compName = dynamic_cast<Property<std::string>* >( *it );
            if ( !compName )
                log(Error) << "Expected Property \""
                           << (*it)->getName() <<"\" to be of type string."<< endlog();
            else if ( compName->getName() == "ImagePort" ) {
                string cname = compName->value().substr(0, compName->value().find("."));
                string pname = compName->value().substr( compName->value().find(".")+1, string::npos);
                ok &= this->addDisplayPort(cname, pname);
            }
            else {
                log(Error) << "Expected \"Component\", \"Port\" or \"Data\", got "
                           << compName->getName() << endlog();
                ok = false;
            }
            ++it;
        }
        deleteProperties( bag );

        // Create window for every dataport
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            // Update the dataport
            (it->get<2>())->execute();
            // Get the base dataport
            DataSourceBase::shared_ptr source = it->get<3>();
            // Convert to Dataport<IplImage>
            DataSource<IplImage>::shared_ptr clone = AdaptDataSource<IplImage>()( source.get() );
            IplImage localImage = clone->get();
            string dataportName = it->get<0>();
            cvNamedWindow(dataportName.data(),CV_WINDOW_AUTOSIZE);
            cvShowImage(dataportName.data(),&localImage);
        }

        // Enter main loop of the window, and update the window if needed
        int key;
        key = cvWaitKey(3);
        // Magic number 3


        return ok;
    }


    // Exactly the same as th reportPort from the Reporter component
    bool ImageDisplayComponent::addDisplayPort(std::string& component, std::string& port)
    {

        Logger::In in("ImageDisplayComponent");
        TaskContext* comp = this->getPeer(component);
        if ( !comp ) {
            log(Error) << "Could not display Component " << component <<" : no such peer."<<endlog();
            return false;
        }
        PortInterface* porti   = comp->ports()->getPort(port);
        if ( !porti ) {
            log(Error) << "Could not display Port " << port
                       <<" : no such port on Component "<<component<<"."<<endlog();
            return false;
        }
        if ( porti->connected() ) {
            this->addDisplaySource( component + "." + port, "Port", porti->connection()->getDataSource() );
            log(Info) << "Reporting port " << port <<" : ok."<<endlog();
        } else {
            // create new port temporarily
            // this port is only created with the purpose of
            // creating a connection object.
            PortInterface* ourport = porti->antiClone();
            assert(ourport);

            ConnectionInterface::shared_ptr ci = porti->createConnection( ourport );
            if ( !ci )
                ci = ourport->createConnection( porti );
            if ( !ci )
                return false;
            ci->connect();

            delete ourport;
            this->addDisplaySource( component + "." + porti->getName(), "Port", ci->getDataSource() );
            log(Info) << "Created connection for port " << porti->getName()<<" : ok."<<endlog();
        }
        return true;

    }

    bool ImageDisplayComponent::addDisplaySource(std::string tag, std::string type, DataSourceBase::shared_ptr orig)
    {

        // creates a copy of the data and an update command to
        // update the copy from the original.
        //
        // Added IplImage type
        Logger::In in("ImageDisplayComponent");

        // Check if the type is IplImage, if this fail
        if(orig->getTypeName()!="IplImage"){
            log(Error) << "Could not display '"<< tag <<"': This is not a IplImage type, it the Vision tookkit loaded?" << endlog();
            return false;
        }
        DataSourceBase::shared_ptr clone = orig->getTypeInfo()->buildValue();
        if ( !clone ) {
            log(Error) << "Could not display '"<< tag <<"' : unknown type." << endlog();
            return false;
        }
        try {
            boost::shared_ptr<CommandInterface> comm( clone->updateCommand( orig.get() ) );
            assert( comm );
            root.push_back( boost::make_tuple( tag, orig, comm, clone, type ) );
        } catch ( bad_assignment& ba ) {
            log(Error) << "Could not display '"<< tag <<"' : failed to create Command." << endlog();
            return false;
        }
        return true;
    }


    bool ImageDisplayComponent::startHook()
    {
        // Do nothing
        return true;
    }

    void ImageDisplayComponent::updateHook()
    {
        // For every dataport
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            // Update the dataport
            (it->get<2>())->execute();
            // Get the base dataport
            DataSourceBase::shared_ptr source = it->get<3>();
            // Convert to Dataport<IplImage>
            DataSource<IplImage>::shared_ptr clone = AdaptDataSource<IplImage>()( source.get() );
            IplImage localImage = clone->get();
            string dataportName = it->get<0>();
            cvShowImage(dataportName.data(),&localImage);
        }

        // Enter main loop of the window, and update the window if needed
        int key;
        key = cvWaitKey(3);
        // Magic number 3
    }

    void ImageDisplayComponent::stopHook()
    {
        // Do nothing
    }


    void ImageDisplayComponent::cleanupHook()
    {
        // Rmove all windows
        for(Reports::iterator it = root.begin(); it != root.end(); ++it ) {
            string dataportName = it->get<0>();
            cvDestroyWindow(dataportName.data());
            int key;
            key = cvWaitKey(3);
        }

        // delete all datasource port objects
        root.clear();
    }

    ImageDisplayComponent::~ImageDisplayComponent()
    {
        // Do nothing
    }

} //end namespace OCL

