#ifndef RTCANCONTROLLER_HPP_
#define RTCANCONTROLLER_HPP_

#include <rtt/NonPeriodicActivity.hpp>
#include "CANControllerInterface.hpp"
#include "CANBusInterface.hpp"
#include "CANMessage.hpp"

struct RTCANControllerPrivate;

namespace RTT
{
    namespace CAN
    {
        /**
         * A Controller which interacts with the Real-time CAN driver under Xenomai.
         * It is tested with the PCI CAN card from Peak-Systems.
         */
        class RTCANController: public CANControllerInterface,
                public NonPeriodicActivity
        {
        public:
            /**
             * @brief Create a real-time socket CAN controller
             * @param priority The priority of the activity
             * @param dev_name The name of the socket CAN device. E.g. if
             * you are using rtcan0, dev_name is rtcan0
             * @param timeout reception timeout of the socket in ms
             */
            RTCANController(int priority, std::string dev_name, int timeout);

            virtual ~RTCANController();

            // Redefine PeriodicActivity methods
            bool initialize();
            void loop();
            bool breakLoop();
            void finalize();

            // Redefine CANControllerInterface
            virtual void addBus(CANBusInterface* bus);
            virtual void process(const CANMessage* msg);
            bool readFromBuffer(void);

        private:
            RTCANControllerPrivate * const d;

        };
    }
}

#endif /* RTCANCONTROLLER_HPP_ */
