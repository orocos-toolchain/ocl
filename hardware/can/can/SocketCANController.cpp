#include "SocketCANController.hpp"

#include <rtt/Logger.hpp>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

/* Define socket CAN protocol family */
#define PF_CAN 29
/* Define address family */
#define AF_CAN PF_CAN
/* Define invalid socket file descriptor */
#define INVALID	-1

struct SocketCANControllerPrivate
{

    /* File descriptor set */
    fd_set fdset;
    /* File descriptor for socket */
    int fd;
    /* struct timeval */
    struct timeval tv;
    /* Interface request structure for ioctl call */
    struct ifreq ifr;
    /* Socket can address structure */
    struct sockaddr_can addr;
    bool exit;

    RTT::CAN::CANBusInterface * bus;

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

        SocketCANController::SocketCANController(int priority,
                std::string dev_name, int timeout) :
            NonPeriodicActivity(ORO_SCHED_OTHER, priority),
            d(new SocketCANControllerPrivate)
        {
            int ifindex;

            Logger::In in("SocketCANController");

            /* clear file descriptor set */
            FD_ZERO(&d->fdset);
            log(Info) << "Trying to open " << dev_name << endlog();
            d->fd = socket(PF_CAN,SOCK_RAW, CAN_RAW);
            if (d->fd < 0)
            {
                log(Error) << d->fd << " error" << endlog();
                log(Error) << dev_name << ": failed. Could not open device."
                        << endlog();
            }
            else
            {
                /* Add the file descriptor into the file descriptor set */
                FD_SET(d->fd, &d->fdset);
                /* Set timeout of the select call */
                d->tv.tv_sec = 0;
                d->tv.tv_usec = timeout * 1000;
                strcpy(d->ifr.ifr_name, dev_name.c_str());
                /* name ifindex */
                ioctl(d->fd, SIOCGIFINDEX, &d->ifr);
                ifindex = d->ifr.ifr_ifindex;
                /* Fill in address structure for CAN sockets */
                d->addr.can_family = AF_CAN;
                d->addr.can_ifindex = ifindex;
                /* Bind the device name to a socket */
                if (bind(d->fd, (struct sockaddr *) &d->addr, sizeof(d->addr)) < 0)
                    log(Error) << dev_name << ": failed. Could not bind to device." << endlog();
            }
        }

        SocketCANController::~SocketCANController()
        {
            Logger::In in("SocketCANController");

            /* Delete member variables */
            delete (d);
            /* Stop SocketCANController component */
            this->stop();

            /* Close socket if file descriptor is valid */
            if (d->fd >= 0)
            {
                close(d->fd);
                d->fd = INVALID;
            }
        }

        bool SocketCANController::initialize()
        {
            Logger::In in("SocketCANController");

            if (d->fd == INVALID)
                return false;

            d->exit = false;

            d->total_RX = d->total_TX = 0;
            d->failed_RX = d->failed_TX = 0;

            return true;
        }

        void SocketCANController::loop()
        {
            while (!d->exit)
            {
                this->readFromBuffer();
            }
        }

        bool SocketCANController::breakLoop()
        {
            d->exit = true;
            return true;
        }

        void SocketCANController::finalize()
        {
            Logger::In in("SocketCANController");
            log(Info) << "Socket CAN Controller stopped. Last run statistics :"
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

        void SocketCANController::addBus(CANBusInterface* bus)
        {
            Logger::In in("SocketCANController");

            d->bus = bus;
            bus->setController(this);
        }

        void SocketCANController::process(const CANMessage* msg)
        {
            Logger::In in("SocketCANController");

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
            ret = write(d->fd, &frame, sizeof(can_frame));
            if (ret > 0)
                d->total_TX++;
            else if (ret < 0)
                d->failed_TX++;

        }

        bool SocketCANController::readFromBuffer()
        {
            Logger::In in("SocketCANController");

            struct can_frame frame;
            CANMessage msg;
            int ret;
            struct timeval tv;
            tv.tv_sec = d->tv.tv_sec;
            tv.tv_usec = d->tv.tv_usec;

            FD_ZERO(&d->fdset);
            FD_SET(d->fd, &d->fdset);

            if(d->bus == NULL) {
                log(Error) << "No CANBus added! Use SocketCANController::addBus " << endlog();
                return false;
            }

            /* Use select in order to see if fd is changed. */
            ret = select(d->fd+1, &d->fdset,
                    NULL, NULL, &tv);
            if (ret > 0) /* Bytes written into buffer */
            {
                ret = recv(d->fd, &frame, sizeof(can_frame),
                        MSG_DONTWAIT);
                if (ret > 0) /* Bytes written into buffer */
                {
                    msg.clear();
                    /* Set DLC */
                    msg.setDLC(frame.can_dlc);
                    /* Is message a remote transmission request? */
                    if (frame.can_id == CAN_RTR_FLAG)
                        msg.setRemote();
                    // Extended or Standard CAN frame format
                    if ((frame.can_id & CAN_EFF_MASK) > CAN_SFF_MASK) /* Extendend CAN frame format */
                        msg.setExtId(frame.can_id);
                    else
                        /* Standard CAN frame format */
                        msg.setStdId(frame.can_id);
                    /* Copy data from buffer into CANMessage */
                    for (unsigned int i = 0; i < msg.getDLC(); i++)
                        msg.setData(i, frame.data[i]);

#if 0 /* Some debugging code, printing each received CAN message */
                    log(Debug) << "Receiving CAN Message: Id=";
                    if (msg.isStandard())
                    log(Debug) << msg.getStdId();
                    else
                    log(Debug) << msg.getExtId();
                    log(Debug) << ", DLC=" << msg.getDLC() << ", DATA = ";
                    for (unsigned int i=0; i < msg.getDLC(); i++)
                    log(Debug) << (unsigned int) msg.getData(i) << " ";
                    log(Debug) << endlog();
#endif
                    /* set origin */
                    msg.origin = this;
                    d->bus->write(&msg);
                    d->total_RX++;
                }
                else /* Error while receiving CAN message */
                    d->failed_RX++;
            }
            /**
             *  If ret < 0, something went wrong while receiving the CAN message
             */
            else if (ret < 0)
                d->failed_RX++;

            /* else if (ret == 0) timeout without receiving a CAN message */

            return true;
        }

    }
}
