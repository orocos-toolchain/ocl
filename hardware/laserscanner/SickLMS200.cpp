#include "SickLMS200.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
//#include <stdio.h>
//#include <sys/time.h>
//#include <time.h>
#include <signal.h>
//#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <rtt/Logger.hpp>

#define FALSE 0
#define TRUE 1
#define MAXRETRY 100
#define MAXNDATA 802
#define STX 0x02   /*every PC->LMS packet is started by STX*/ 
#define ACKSTX 0x06 /*every PC->LMS packet is started by ACKSTX*/
typedef unsigned char uchar;

using namespace RTT;


namespace SickDriver {
/*the cmd and ack packets for the 5 different range/resolution modes*/
const uchar PCLMS_RES1[]={0x02,0x00,0x05,0x00,0x3b,0x64,0x00,0x64,0x00,0x1d,0x0f};
const uchar PCLMS_RES2[]={0x02,0x00,0x05,0x00,0x3b,0x64,0x00,0x32,0x00,0xb1,0x59};
const uchar PCLMS_RES3[]={0x02,0x00,0x05,0x00,0x3b,0x64,0x00,0x19,0x00,0xe7,0x72};
const uchar PCLMS_RES4[]={0x02,0x00,0x05,0x00,0x3b,0xb4,0x00,0x64,0x00,0x97,0x49};
const uchar PCLMS_RES5[]={0x02,0x00,0x05,0x00,0x3b,0xb4,0x00,0x32,0x00,0x3b,0x1f};
const uchar LMSPC_RES1_ACK[]={0x06,0x02,0x80,0x07,0x00,0xbb,0x01,0x64,0x00,0x64,0x00,0x10,0x4f,0xbd};
const uchar LMSPC_RES2_ACK[]={0x06,0x02,0x80,0x07,0x00,0xbb,0x01,0x64,0x00,0x32,0x00,0x10,0x17,0x10};
const uchar LMSPC_RES3_ACK[]={0x06,0x02,0x80,0x07,0x00,0xbb,0x01,0x64,0x00,0x19,0x00,0x10,0xbb,0x46};
const uchar LMSPC_RES4_ACK[]={0x06,0x02,0x80,0x07,0x00,0xbb,0x01,0xb4,0x00,0x64,0x00,0x10,0x5b,0x30};
const uchar LMSPC_RES5_ACK[]={0x06,0x02,0x80,0x07,0x00,0xbb,0x01,0xb4,0x00,0x32,0x00,0x10,0x03,0x9d};

/*the cmd and ack packets different measurement unit modes*/
const uchar PCLMS_SETMODE[]={0x02,0x00,0x0a,0x00,0x20,0x00,0x53,0x49,0x43,0x4b,0x5f,0x4c,0x4d,0x53,0xbe,0xc5};
const uchar PCLMS_CM[]={0x02,0x00,0x21,0x00,0x77,0x00,0x00,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,\
			0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0xcb};
const uchar PCLMS_MM[]={0x02,0x00,0x21,0x00,0x77,0x00,0x00,0x00,0x00,0x00,0x0d,0x01,0x00,0x00,\
			0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x34,0xc7};
const uchar LMSPC_CM_ACK[]={0x06,0x02,0x80,0x25,0x00,0xf7,0x00,0x00,0x00,0x46,0x00,0x00,0x0d,0x00,\
			    0x00,0x00,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
			    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xcb,0x10};
const uchar LMSPC_MM_ACK[]={0x06,0x02,0x80,0x25,0x00,0xf7,0x00,0x00,0x00,0x46,0x00,0x00,0x0d,0x01,\
			    0x00,0x00,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
			    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xc7,0x10};

/*the cmd packets for setting transfer speedn and  controlling the start and stop of measurement*/
const uchar PCLMS_START[]={0x02,0x00,0x02,0x00,0x20,0x24,0x34,0x08};
const uchar PCLMS_STOP[]={0x02,0x00,0x02,0x00,0x20,0x25,0x35,0x08};
const uchar PCLMS_STATUS[]={0x02,0x00,0x01,0x00,0x31,0x15,0x12};
const uchar PCLMS_B9600[] ={0x02,0x00,0x02,0x00,0x20,0x42,0x52,0x08};
const uchar PCLMS_B19200[]={0x02,0x00,0x02,0x00,0x20,0x41,0x51,0x08};
const uchar PCLMS_B38400[]={0x02,0x00,0x02,0x00,0x20,0x40,0x50,0x08};
/*the ack packet for the above*/
const uchar LMSPC_CMD_ACK[]={0x06,0x02,0x80,0x03,0x00,0xa0,0x00,0x10,0x16,0x0a};





bool SickLMS200::msgcmp(int len1, const uchar *s1, int len2, const uchar *s2)
{
  int i;
  if(len1!=len2) return FALSE;
  for(i=0; i<len1; i++)
    if(s1[i]!=s2[i]) return FALSE;
  return TRUE;
}


void SickLMS200::wtLMSmsg(int fd, int len, const uchar *msg)
{
  write(fd,(const void*) msg,len);
}

int SickLMS200::rdLMSmsg(int fd, int len, const uchar *buf)
{
  int sumRead=0,nRead=0,toRead=len,n;
  while(toRead>0){
    n=toRead>255 ? 255:toRead;
    toRead-=n;
    nRead=read(fd,(void *)(buf+sumRead),n);
    sumRead+=nRead;
    if(nRead!=n) break;
  }
  return nRead;
}

uchar SickLMS200::rdLMSbyte(int fd)
{
  uchar buf;
  read(fd,&buf,1);
  return buf;
}



/*return true if the ACK packet is as expected*/
bool SickLMS200::chkAck(int fd, int ackmsglen, const uchar *ackmsg)
{
  int i,buflen;
  uchar buf[MAXNDATA];

  /*the following is to deal with a possibly timing issue*/
  /*the communication occasionally breaks without this*/
  usleep(100000); 
  for(i=0;i<MAXRETRY;i++) {
    if(rdLMSbyte(fd)==0x06)
    {
       break;
    }
  }
  buflen=rdLMSmsg(fd,ackmsglen-1,buf);
  return msgcmp(ackmsglen-1,ackmsg+1,buflen,buf);
}

/*set the communication speed and terminal properties*/
int SickLMS200::initLMS(const char *serialdev, struct termios *oldtio)

{
  int fd;
  struct termios newtio_struct, *newtio=&newtio_struct;

  fd = open(serialdev, O_RDWR | O_NOCTTY ); 
  if (fd <0) {
      throw IOException(strerror(errno));
  }
        
  tcgetattr(fd,oldtio); /* save current port settings */

  /*after power up, the laser scanner will reset to 9600bps*/
  memset(newtio, 0, sizeof(struct termios));
  newtio->c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  newtio->c_iflag = IGNPAR;
  newtio->c_oflag = 0;
        
  /* set input mode (non-canonical, no echo,...) */
  newtio->c_lflag = 0;
  newtio->c_cc[VTIME]    = 10;   /* inter-character timer unused */
  newtio->c_cc[VMIN]     = 255;   /* blocking read until 1 chars received */
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,newtio);


  /* step to the 38400bps mode*/
  wtLMSmsg(fd,sizeof(PCLMS_B38400)/sizeof(uchar),PCLMS_B38400);
  if(!chkAck(fd,sizeof(LMSPC_CMD_ACK)/sizeof(uchar),LMSPC_CMD_ACK))
      throw BaudRateChangeException();

  /* set the PC side as well*/
  close(fd);
  fd = open(serialdev, O_RDWR | O_NOCTTY );
  if (fd <0) {
      throw IOException(strerror(errno));
  }
  newtio->c_cflag = B38400 | CS8 | CLOCAL | CREAD;
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,newtio);

  return fd;
}


/*set both the angular range and resolutions*/
void SickLMS200::setmode(int fd, int mode)
{
  const uchar *msg, *ackmsg;
  int msglen, ackmsglen, unit, res;

  /*change the resolution*/
  res=mode & 0x3e;
  switch(res){
  case (RES_1_DEG | RANGE_100): 
    msg=PCLMS_RES1;
    ackmsg=LMSPC_RES1_ACK;
    break;
  case (RES_0_5_DEG | RANGE_100):
    msg=PCLMS_RES2;
    ackmsg=LMSPC_RES2_ACK;
    break;
  case (RES_0_25_DEG | RANGE_100):
    msg=PCLMS_RES3;
    ackmsg=LMSPC_RES3_ACK;
    break;
  case (RES_1_DEG | RANGE_180):
    msg=PCLMS_RES4;
    ackmsg=LMSPC_RES4_ACK;      
    break;
  case (RES_0_5_DEG | RANGE_180):
    msg=PCLMS_RES5;
    ackmsg=LMSPC_RES5_ACK;
    break;
  default:
    msg=PCLMS_RES1;
    ackmsg=LMSPC_RES1_ACK;
    throw InvalidResolutionException();
    break;
  }

  /*the following two line works only because msg and ackmsg are const uchar str*/
  msglen=sizeof(PCLMS_RES1)/sizeof(uchar);
  ackmsglen=sizeof(LMSPC_RES1_ACK)/sizeof(uchar);
  wtLMSmsg(fd,msglen,msg);
  if(!chkAck(fd,ackmsglen,ackmsg))
      throw ResolutionFailureException();


  /*change the measurement unit*/
  unit=mode & 0x1;  
  /*may need to increase the timeout to 7sec here*/
  if(unit==MMMODE){
    msg=PCLMS_MM;
    ackmsg=LMSPC_MM_ACK;
  }else if(unit==CMMODE){
    msg=PCLMS_CM;
    ackmsg=LMSPC_CM_ACK;
  }
  /*invoking setting mode*/
  msglen=sizeof(PCLMS_SETMODE)/sizeof(uchar);
  ackmsglen=sizeof(LMSPC_CMD_ACK)/sizeof(uchar);
  wtLMSmsg(fd,msglen,PCLMS_SETMODE);
  if(!chkAck(fd,ackmsglen,LMSPC_CMD_ACK))
      throw ModeFailureException();
  /*the following two line works only because msg and ackmsg are const uchar str*/
  msglen=sizeof(PCLMS_MM)/sizeof(uchar);
  ackmsglen=sizeof(LMSPC_MM_ACK)/sizeof(uchar);
  wtLMSmsg(fd,msglen,msg);
  if(!chkAck(fd,ackmsglen,ackmsg))
      throw ModeFailureException();

}

/*tell the scanner to enter the continuous measurement mode*/
void SickLMS200::startLMS(int fd)
{
  log(Debug)<< " SickLMS200::startLMS entered."<<endlog();
  int ackmsglen;

  wtLMSmsg(fd,sizeof(PCLMS_START)/sizeof(uchar),PCLMS_START);
  ackmsglen=sizeof(LMSPC_CMD_ACK)/sizeof(uchar);
  if(!chkAck(fd,ackmsglen,LMSPC_CMD_ACK))
      throw StartFailureException();
}

/*stop the continuous measurement mode*/
void SickLMS200::stopLMS(int fd)
{
  int ackmsglen;

  wtLMSmsg(fd,sizeof(PCLMS_STOP)/sizeof(uchar),PCLMS_STOP);
  ackmsglen=sizeof(LMSPC_CMD_ACK)/sizeof(uchar);
  if(!chkAck(fd,ackmsglen,LMSPC_CMD_ACK))
      throw StopFailureException();
}


bool SickLMS200::checkErrorMeasurement() {
  switch(meas_state & 0x07){
    case 0: return false;
    case 1: return false;
    case 2: return false; 
    case 3: return true;
    case 4: throw FatalMeasurementException();
    default: throw FatalMeasurementException();
  }
}

bool SickLMS200::checkPlausible() {
    return (meas_state & 0xC0)==0;
}



/*reset terminal and transfer speed of laser scanner before quitting*/
void SickLMS200::resetLMS(int fd, struct termios *oldtio)
{
  wtLMSmsg(fd,sizeof(PCLMS_B9600)/sizeof(uchar),PCLMS_B9600);
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,oldtio);
  close(fd);
   if(!chkAck(fd,sizeof(LMSPC_CMD_ACK)/sizeof(uchar),LMSPC_CMD_ACK))
      throw BaudRateChangeException();
  
}

SickLMS200::SickLMS200(const char* _port, uchar _range_mode, uchar _res_mode, uchar _unit_mode) {
    port = _port;
    range_mode = _range_mode;
    res_mode   = _res_mode;
    unit_mode  = _unit_mode;
}

void SickLMS200::start() {
    log(Debug)<< " SickLMS200::start() entered."<<endlog();
    fd = initLMS(port,&oldtio);
    setmode(fd, range_mode | res_mode | unit_mode);
    startLMS(fd);
    log(Debug)<< " SickLMS200::start() exit."<<endlog();
}
void SickLMS200::stop() {
    stopLMS(fd);
    resetLMS(fd,&oldtio);
}
void SickLMS200::reset() {
    try {
        resetLMS(fd,&oldtio);
    } catch (Exception&) {
    }
}
SickLMS200::~SickLMS200() {
}

/**
 * returns :
 *   -1 : when the input does not begin with the correct header
 */
int SickLMS200::readMeasurement(uchar* buf,int& datalen) {
    if(rdLMSbyte(fd)!=STX) return -1;
    rdLMSbyte(fd); /*should be the ADR byte, ADR=0X80 here*/
    /*LEN refers to the packet length in bytes, datalen 
      refers to the number of measurements, each of them are 16bits long*/
    rdLMSbyte(fd); /*should be the LEN low byte*/
    rdLMSbyte(fd); /*should be the LEN high byte*/
    rdLMSbyte(fd); /*should be the CMD byte, CMD=0xb0 in this mode*/
    datalen=rdLMSbyte(fd);
    datalen |= (rdLMSbyte(fd) & 0x1f) << 8; /*only lower 12 bits are significant*/
    datalen *= 2; /*each reading is 2 bytes*/
    rdLMSmsg(fd,datalen,buf);
    meas_state = rdLMSbyte(fd); 
    rdLMSbyte(fd); /*should be CRC low byte*/
    rdLMSbyte(fd); /*should be CRC high byte*/
    return 0;
}


static SickLMS200* sick=0;
static void SickLMS200_trap(int sig) {
    if (sick!=0) {
        sick->reset();
    }
};

bool registerSickLMS200SignalHandler() {
  return !(signal(SIGTERM, SickLMS200_trap) ==SIG_ERR || signal(SIGHUP, SickLMS200_trap) ==SIG_ERR || 
	   signal(SIGINT, SickLMS200_trap) ==SIG_ERR || signal(SIGQUIT, SickLMS200_trap) ==SIG_ERR || 
	   signal(SIGABRT, SickLMS200_trap) ==SIG_ERR);
};


Exception::Exception(const char* _descr):descr(_descr) {}
const char* Exception::getDescription() { return descr; }

BaudRateChangeException::BaudRateChangeException():Exception("Failure to set the baud rate at Sick side") {};

InvalidResolutionException::InvalidResolutionException():Exception("Invalid resolution selected") {}

ResolutionFailureException::ResolutionFailureException():Exception("Failure to set the resolution mode at Sick side") {}

ModeFailureException::ModeFailureException():Exception("Failure to set the measurement mode at Sick side") {}

StartFailureException::StartFailureException():Exception("Failure to start the Sick sensor") {}

StopFailureException::StopFailureException():Exception("Failure to stop the Sick sensor") {}

RegisterException::RegisterException():Exception("Failure to register a signal handler") {}

IOException::IOException(const char* _descr):Exception(_descr) {}

FatalMeasurementException::FatalMeasurementException():Exception("Fatal measurement exception") {}

};

