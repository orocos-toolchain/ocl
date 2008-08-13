
#include <rtt/RTT.hpp>
#include <rtt/RTT.hpp>
#include <rtt/os/main.h>
#include <rtt/os/threads.hpp>
#include <iostream>

#if defined (OROPKG_OS_LXRT)

#include <rtt/PeriodicActivity.hpp>
#include "dev/EncoderSSIapci1710.hpp"
#include <fstream>
#include <math.h>

//#include <Kuka361Constants.hpp>

//#define KUKA361_ENCODEROFFSETS { 1000004, 1000000, 1000002, 449784, 1035056 , 1230656 }
#define KUKA361_ENCODEROFFSETS { 1000004, 1000000, 502652, 1001598, 984660 , 1230656 }

#define KUKA361_CONV1  94.14706
#define KUKA361_CONV2  -103.23529
#define KUKA361_CONV3  51.44118
#define KUKA361_CONV4  175
#define KUKA361_CONV5  150
#define KUKA361_CONV6  131.64395

#define KUKA361_ENC_RES  4096

  // Conversion from encoder ticks to radiants
#define KUKA361_TICKS2RAD { 2*M_PI / (KUKA361_CONV1 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV2 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV3 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV4 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV5 * KUKA361_ENC_RES), 2*M_PI / (KUKA361_CONV6 * KUKA361_ENC_RES)}


using namespace Orocos;
using namespace RTT;



class EncWriter : public PeriodicActivity
{
/*1{{{*/
public:
    EncWriter( EncoderSSI_apci1710_board& ssiBoard, bool radiants = false)
        :  PeriodicActivity(RTT::OS::HighestPriority, 0.001 ),  enc1( 1, &ssiBoard ),  enc2( 2, &ssiBoard ),  enc3( 7, &ssiBoard )
                                      ,  enc4( 8, &ssiBoard ),  enc5( 5, &ssiBoard ),  enc6( 6, &ssiBoard )
                                      ,  output("encoderKuka361.txt", std::ios::out)
    {
        results = new double[360000];
        resultcounter = 0;

        rad = radiants;
        double temp_offsets[]   = KUKA361_ENCODEROFFSETS;
        double temp_ticks2rad[] = KUKA361_TICKS2RAD;

        for(unsigned int i = 0; i < 6; i++)
        {
            offsets[i]   = temp_offsets[i];
            ticks2rad[i] = temp_ticks2rad[i];
        }
    };


    virtual ~EncWriter()
    {
        for (unsigned int i = 0; i < resultcounter; i++)
        {
          for (int j = 0; j < 6; j++)
          output << results[i*6 + j] << " ";
          output << std::endl;
        }
        output.close();
    };


    virtual void step()
    {
        if (rad)
        {
            if (resultcounter < 60000)
            {
                results[resultcounter*6    ] = ( enc1.positionGet() - offsets[0] ) * ticks2rad[0];
                results[resultcounter*6 + 1] = ( enc2.positionGet() - offsets[1] ) * ticks2rad[1];
                results[resultcounter*6 + 2] = ( enc3.positionGet() - offsets[2] ) * ticks2rad[2];
                results[resultcounter*6 + 3] = ( enc4.positionGet() - offsets[3] ) * ticks2rad[3];
                results[resultcounter*6 + 4] = ( enc5.positionGet() - offsets[4] ) * ticks2rad[4];
                results[resultcounter*6 + 5] = ( enc6.positionGet() - offsets[5] ) * ticks2rad[5];

                resultcounter++;
            }
        }
        else
        {
            if (resultcounter < 60000)
            {
                results[resultcounter*6    ] = enc1.positionGet();
                results[resultcounter*6 + 1] = enc2.positionGet();
                results[resultcounter*6 + 2] = enc3.positionGet();
                results[resultcounter*6 + 3] = enc4.positionGet();
                results[resultcounter*6 + 4] = enc5.positionGet();
                results[resultcounter*6 + 5] = enc6.positionGet();

                resultcounter++;
            }
        }
    }

private:
    EncoderSSI_apci1710   enc1, enc2, enc3, enc4, enc5, enc6;
    double  ticks2rad[6];
    double  offsets[6];
    bool    rad;

    double* results;
    unsigned int resultcounter;
    std::ofstream output;/*}}}1*/
};



class EncReader : public PeriodicActivity
{
/*{{{*/
public:
    EncReader( EncoderSSI_apci1710_board& ssiBoard, bool radiants = false)
        :  PeriodicActivity(RTT::OS::LowestPriority, 0.1 ),  enc1( 1, &ssiBoard ),  enc2( 2, &ssiBoard ),  enc3( 7, &ssiBoard )
                                 ,  enc4( 8, &ssiBoard ),  enc5( 5, &ssiBoard ),  enc6( 6, &ssiBoard )
    {
        rad = radiants;
        double temp_offsets[]   = KUKA361_ENCODEROFFSETS;
        double temp_ticks2rad[] = KUKA361_TICKS2RAD;

        for(unsigned int i = 0; i < 6; i++)
        {
            offsets[i]   = temp_offsets[i];
            ticks2rad[i] = temp_ticks2rad[i];
        }

        this->thread()->setScheduler(ORO_SCHED_OTHER);
    };


    virtual ~EncReader() {};


    virtual void step()
    {
        if (rad)
        {
            std::cout << ( enc1.positionGet() - offsets[0] ) * ticks2rad[0] << "\t"
                      << ( enc2.positionGet() - offsets[1] ) * ticks2rad[1] << "\t"
                      << ( enc3.positionGet() - offsets[2] ) * ticks2rad[2] << "\t"
                      << ( enc4.positionGet() - offsets[3] ) * ticks2rad[3] << "\t"
                      << ( enc5.positionGet() - offsets[4] ) * ticks2rad[4] << "\t"
                      << ( enc6.positionGet() - offsets[5] ) * ticks2rad[5] << std::endl;

        }
        else
        {
            std::cout << enc1.positionGet() << "\t" << enc2.positionGet() << "\t" << enc3.positionGet() << "\t" <<
                         enc4.positionGet() << "\t" << enc5.positionGet() << "\t" << enc6.positionGet() << std::endl;
        }
    }

private:
    EncoderSSI_apci1710   enc1, enc2, enc3, enc4, enc5, enc6;
    double  ticks2rad[6];
    double  offsets[6];
    bool    rad;/*}}}*/
};



int ORO_main(int argc, char** argv)
{

    EncoderSSI_apci1710_board  encSSIboard(0, 1, 2);

    std::cout << "Read in encoder ticks?  (y/n) " << std::endl;
    char c, d;
    std::cin >> c;

    std::cout << "Write to file?  (y/n) " << std::endl;
    std::cin >> d;


    EncReader* encReader = NULL;
    EncWriter* encWriter = NULL;

    if ( (c == 'y')||(c == 'Y') )  encReader = new EncReader(encSSIboard, false);
    else                           encReader = new EncReader(encSSIboard, true );


    if ( (d == 'y')||(d == 'Y') )
    {
        if ( (c == 'y')||(c == 'Y') )  encWriter = new EncWriter(encSSIboard, false);
        else                           encWriter = new EncWriter(encSSIboard, true );
        encWriter->start();
    }
    else encReader->start();


    std::cin >> c;


    if (encReader) encReader->stop();
    if (encWriter) encWriter->stop();

    if (encReader) delete encReader;
    if (encWriter) delete encWriter;
    return 0;
}

#else

int ORO_main(int argc, char** argv)
{
    std::cout << "To use this program, it should be compiled for RTAI/LXRT." << std::endl;
    return 0;
}

#endif
