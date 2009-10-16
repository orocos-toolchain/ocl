#include "RTCANController.hpp"

#include <rtt/Logger.hpp>

#include <rtdm/rtcan.h>

/* Define invalid socket file descriptor */
#define INVALID	-1

struct RTCANControllerPrivate
{

    /* File descriptor for socket */
    int fd;
    /* Interface request structure for ioctl call */
    struct ifreq ifr;
    /* Socket can address structure */
    struct sockaddr_can addr;
    bool exit;
    RTT::CAN::CANMessage msg;

    RTT::CAN::CANBusInterface * mbus;

    /* Total transmit CAN messages */
    unsigned int total_TX;
    /* Total received CAN messages */
    unsigned int total_RX;
    /* Failed TX messages */
    unsigned int failed_TX;
    /* Failed RX messages */
    unsigned int failed_RX;
};

namespace RTT
{
    namespace CAN
    {

        RTCANController::RTCANController(int priority, std::string dev_name, int timeout)
            :   NonPeriodicActivity(ORO_SCHED_RT, priority),
                d(new RTCANControllerPrivate)
        {
            int ifindex;
            int ret;
            nanosecs_rel_t rxtimeout = timeout * 1000000;

            Logger::In in("RTCANController");

            log(Info) << "Trying to open " << dev_name << endlog();
            d->fd = rt_dev_socket(PF_CAN, SOCK_RAW, CAN_RAW);
            if (d->fd < 0)
            {
                log(Error) << dev_name << ": failed. Could not open device."
                        << endlog();
            }
            else
            {
                strcpy(d->ifr.ifr_name, dev_name.c_str());
                /* name ifindex */
                ret = rt_dev_ioctl(d->fd, SIOCGIFINDEX,
                        &d->ifr);
                if (ret < 0)
                    log(Error) << dev_name
                            << ": failed. Unable to find CAN interface. "
                            << endlog();
                ifindex = d->ifr.ifr_ifindex;
                /* Fill in address structure for CAN sockets */
                d->addr.can_family = AF_CAN;
                d->addr.can_ifindex = ifindex;
                /* Bind the device name to a socket */
                ret = rt_dev_bind(d->fd,
                        (struct sockaddr *) &d->addr,
                        sizeof(struct sockaddr_can));
                if (ret < 0)
                    log(Error) << dev_name
                            << ": failed. Could not bind to device." << ret
                            << endlog();

                // Enable timeout for receive operations
                ret = rt_dev_ioctl(d->fd, RTCAN_RTIOC_RCV_TIMEOUT, &rxtimeout);
                if (ret < 0)
                    log(Error) << dev_name << ": failed. Could not enable timeout for RX CAN messages." << endlog();
            }
        }

        RTCANController::~RTCANController()
        {
            Logger::In in("RTCANController");

            /* Delete member variables */
            delete (d);
            /* Stop RTCANController component */
            this->stop();

            /* Close socket if file descriptor is valid */
            if (d->fd >= 0)
            {
                rt_dev_close(d->fd);
                d->fd = INVALID;
            }
        }

        bool RTCANController::initialize()
        {
            Logger::In in("RTCANController");

	        log(Info) << "Initialize CAN device" << endlog();
            if (d->fd < 0)
                return false;

            d->exit = false;

            d->total_RX = d->total_TX = 0;
            d->failed_RX = d->failed_TX = 0;

            return true;
        }

        void RTCANController::loop()
        {
            Logger::In in("RTCANController");

	        log(Info) << "Start loop" << endlog();
            while (!d->exit)
            {
                this->readFromBuffer();
            }
        }

        bool RTCANController::breakLoop()
        {
            d->exit = true;
            return true;
        }

        void RTCANController::finalize()
        {
            Logger::In in("RTCANController");

            log(Info) << "RTCAN Controller stopped. Last run statistics :"
                    << endlog();
            log(Info) << " Total Received    : " << d->total_RX
                    << ".  Failed to Receive : " << d->failed_RX
                    << endlog();
            log(Info) << " Total Transmitted : " << d->total_TX
                    << ".  Failed to Transmit : " << d->failed_TX
                    << endlog();
            d->total_RX = d->failed_RX
                    = d->total_TX = d->failed_TX = 0;
        }

        void RTCANController::addBus(CANBusInterface* bus)
        {
            Logger::In in("RTCANController");

            if(bus == NULL)
                log(Error) << "Can not add CANBus, CANBusInterface is not valid!" << endlog();
            else {
                d->mbus = bus;
                bus->setController(this);
            }
        }

        void RTCANController::process(const CANMessage* msg)
        {
            Logger::In in("RTCANController");

            int ret = 0;

            /* Construct message */
            struct can_frame frame;
            for (unsigned int i = 0; i < msg->getDLC(); i++)
                frame.data[i] = msg->getData(i);

            /* Set CAN DLC */
            frame.can_dlc = msg->getDLC();

            /* Extendend frame format or standard frame format */
            if (msg->isExtended()) /* Extended frame format */
                frame.can_id = msg->getExtId() | CAN_EFF_FLAG;
            else
                /* Standard frame format */
                frame.can_id = msg->getStdId();
            // Remote Transmission request?
            if (msg->isRemote())
                frame.can_id = CAN_RTR_FLAG;

            /* Send CAN message */
            ret = rt_dev_send(d->fd, &frame, sizeof(can_frame),
                    MSG_DONTWAIT);
            if (ret > 0)
                d->total_TX++;
            else if (ret < 0)
                d->failed_TX++;

        }

        bool RTCANController::readFromBuffer()
        {
            Logger::In in("RTCANController");

            struct can_frame frame;
    	    struct sockaddr_can addr;
	        socklen_t addrlen = sizeof(addr);
	        int ret;

            if(d->mbus == NULL) {
                log(Error) << "No CANBus added! Use RTCANController::addBus "
                << "else received CAN messages on the device will be lost." << endlog();
                return false;
            }

            ret = rt_dev_recvfrom(d->fd, (void *) &frame, sizeof(can_frame_t), 0,
				  (struct sockaddr *) &addr, &addrlen);
            if (ret > 0) /* Bytes written into buffer */
            {
                d->msg.clear();
                /* Set DLC */
                d->msg.setDLC(frame.can_dlc);
                /* Is message a remote transmission request? */
                if (frame.can_id == CAN_RTR_FLAG)
                    d->msg.setRemote();
                // Extended or Standard CAN frame format
                if ((frame.can_id & CAN_EFF_MASK) > CAN_SFF_MASK) /* Extendend CAN frame format */
                    d->msg.setExtId(frame.can_id);
                else
                    /* Standard CAN frame format */
                    d->msg.setStdId(frame.can_id);
                /* Copy data from buffer into CANMessage */
                for (unsigned int i = 0; i < d->msg.getDLC(); i++)
                    d->msg.setData(i, frame.data[i]);

#if 0 /* Some debugging code, printing each received CAN message */
                log(Debug) << "Receiving CAN Message: Id=";
                if (d->msg.isStandard())
                log(Debug) << d->msg.getStdId();
                else
                log(Debug) << d->msg.getExtId();
                log(Debug) << ", DLC=" << d->msg.getDLC() << ", DATA = ";
                for (unsigned int i=0; i < d->msg.getDLC(); i++)
                log(Debug) << (unsigned int) d->msg.getData(i) << " ";
                log(Debug) << endlog();
#endif
                /* set origin */
                d->msg.origin = this;
                d->mbus->write(&d->msg);
                d->total_RX++;
            }
            else if (ret == -ETIMEDOUT) { /* Timeout */ }
            else
                d->failed_RX++;

            return true;
        }

    }
}

#endif /* RTCANCONTROLLER */
