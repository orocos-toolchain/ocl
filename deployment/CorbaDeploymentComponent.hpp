/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:34:46 CEST 2008  CorbaDeploymentComponent.hpp

                        CorbaDeploymentComponent.hpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be

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


#ifndef CORBADEPLOYMENTCOMPONENT_H_
#define CORBADEPLOYMENTCOMPONENT_H_

#include "DeploymentComponent.hpp"

namespace OCL
{

    class OCL_API CorbaDeploymentComponent
        :public DeploymentComponent
    {
    protected:
        /**
         * Check if \a c is a proxy or a local object.
         * If it is a local object, make it a server.
         */
        virtual bool componentLoaded(RTT::TaskContext* c);
        /**
         * Removes the CORBA server for this component.
         * @param c a valid TaskContext object.
         */
        virtual void componentUnloaded(RTT::TaskContext* c);
    public:
        CorbaDeploymentComponent(const std::string& name, const std::string& siteFile = "");
        virtual ~CorbaDeploymentComponent();
        /**
         * Creates a ControlTask CORBA server for a given peer TaskContext.
         */
        bool createServer(const std::string& tc, bool use_naming);
    };

}

#endif /*CORBADEPLOYMENTCOMPONENT_H_*/
