#include <ocl/OCL.hpp>
#include <rtt/os/StartStopManager.hpp>
#include <rtt/Logger.hpp>

#include "dev/ComediDevice.hpp"
#include "dev/ComediSubDeviceAOut.hpp"
#include "dev/ComediSubDeviceDOut.hpp"
#include "dev/ComediSubDeviceDIn.hpp"
#include "dev/ComediEncoder.hpp"

#ifndef __COMEDIDEVICES__
#define __COMEDIDEVICES__

namespace OCL{
    using namespace RTT;

#if (defined OROPKG_OS_LXRT)
    class ComediLoader{
    private:
        static ComediLoader* mt;

        ComediDevice *ni6713,*ni6527,*ni6602;
        
        AnalogOutInterface* SubAOut;
        DigitalInInterface* SubDIn;
        DigitalOutInterface* SubDOut;
        EncoderInterface *enc1,*enc2,*enc3,*enc4,*enc5,*enc6,*enc7,*enc8;

        unsigned int in_use;

    public:
        ComediLoader(){
            in_use=0;
        }
        
        static inline ComediLoader* Instance(){
            if ( mt==NULL)
                mt = new ComediLoader();
            return mt;
        }

        void CreateComediDevices(void){
            if(in_use++==0){
                log(Info)<<"CreateComediDevices"<<endlog();
                
                ni6713 = new ComediDevice(0);
                ni6527 = new ComediDevice(1);
                ni6602 = new ComediDevice(2);
                
                SubAOut = new ComediSubDeviceAOut(ni6713,"AnalogOut",1);
                SubDIn = new ComediSubDeviceDIn(ni6527,"DigitalIn",0);
                SubDOut = new ComediSubDeviceDOut(ni6527,"DigitalOut",1);
                
                enc1 = new ComediEncoder(ni6602,2,"Counter0");
                enc2 = new ComediEncoder(ni6602,3,"Counter1");
                enc3 = new ComediEncoder(ni6602,4,"Counter2");
                enc4 = new ComediEncoder(ni6602,5,"Counter3");
                enc5 = new ComediEncoder(ni6602,6,"Counter4");
                enc6 = new ComediEncoder(ni6602,7,"Counter5");
                enc7 = new ComediEncoder(ni6602,8,"Counter6");
                enc8 = new ComediEncoder(ni6602,9,"Counter7");
                
            }else
                in_use++;
        };
        
        void DestroyComediDevices(void){
            if(--in_use==0){
                log(Info)<<"DestroyComediDevices"<<endlog();
                
                delete enc1;
                delete enc2;
                delete enc3;
                delete enc4;
                delete enc5;
                delete enc6;
                delete enc7;
                delete enc8;
                
                delete SubDOut;
                delete SubDIn;
                delete SubAOut;
                
                delete ni6713;
                delete ni6527;
                delete ni6602;

                delete mt;
                mt = NULL;
            }
        };
    };
    ComediLoader* ComediLoader::mt=NULL;
#endif
}
#endif
