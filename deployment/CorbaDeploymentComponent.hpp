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
        virtual bool componentLoaded(TaskContext* c);
    public:
        CorbaDeploymentComponent(const std::string& name);
        virtual ~CorbaDeploymentComponent();
        /**
         * Creates a ControlTask CORBA server for a given peer TaskContext.
         */
        bool createServer(const std::string& tc, bool use_naming);
    };

}

#endif /*CORBADEPLOYMENTCOMPONENT_H_*/
