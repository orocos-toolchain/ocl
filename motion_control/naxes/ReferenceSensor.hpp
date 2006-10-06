#ifndef _REFERENCESENSOR_HPP_
#define _REFERENCESENSOR_HPP_

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Method.hpp>

namespace Orocos
{
    /**
     * Implementation of a TaskContext that collects all the reference
     * dataports of a nAxesVelocityController and supplies a method to
     * get the value of each reference signal
     * 
     */

    class ReferenceSensor : public RTT::TaskContext
    {
        int nrofaxes;
    public:
        /** 
         * Constructor of the class
         * 
         * @param name name of the TaskContext
         * @param _nrofaxes number of axes
         * 
         */
        ReferenceSensor(std::string name="ReferenceSensor",int _nrofaxes=6);
        virtual ~ReferenceSensor() {};
    private:        
        virtual bool getReference(int axis);
    protected:
        /**
         * Method to get the Reference switch value of an axis
         *
         * @param axis axis to get reference switch value from
         *
         * @return reference switch value
         */
        RTT::Method<bool(int)> _getReference;
        /// vector of dataports containing reference switch value,
        /// shared with nAxesVelocityController. Looks for ports with
        /// names reference0, reference1, ...
        std::vector<RTT::ReadDataPort<bool>*>  reference;	
    };
}

#endif
