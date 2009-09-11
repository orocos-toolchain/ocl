#ifndef SOCKET_CAN_CONTROLLER_HPP
#define SOCKET_CAN_CONTROLLER_HPP

#include <rtt/NonPeriodicActivity.hpp>
#include "CANControllerInterface.hpp"
#include "CANBusInterface.hpp"
#include "CANMessage.hpp"

struct SocketCANControllerPrivate;

namespace RTT
{
    namespace CAN
    {
        /**
         * A Controller which interacts with the socket CAN linux driver.
         * It is tested with the NON REALTIME CAN driver from Peak-Systems.
         * See http://peak-systems.com/linux for more information on how
         * to compile this driver with socket CAN support.
         */
        class SocketCANController: public CANControllerInterface,
                public NonPeriodicActivity
        {
        public:
            /**
             * @brief Create a socket CAN controller
             * @param priority The priority of the activity
             * @param dev_name The name of the socket CAN device. E.g. if
             * you are using can0, dev_name is can0
             * @param timeout reception timeout of the socket in ms
             */
            SocketCANController(int priority, std::string dev_name,
                            int timeout);

            virtual ~SocketCANController();

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
            SocketCANControllerPrivate * const d;

        };
    }
}
#endif
