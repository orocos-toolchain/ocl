#ifndef IP_ENCODER_6_ENCINTERFACE_HPP
#define IP_ENCODER_6_ENCINTERFACE_HPP


//#include <rtt/TaskNonPreemptible.hpp>
#include <rtt/NonPreemptibleActivity.hpp>
#include <rtt/DataObjectInterfaces.hpp>
#include <rtt/dev/EncoderInterface.hpp>
#include <vector>


class IP_Encoder_6_Task : public RTT::NonPreemptibleActivity
{
public:
    IP_Encoder_6_Task( RTT::Seconds period );
    virtual ~IP_Encoder_6_Task() {};

    virtual bool initialize();
    virtual void step();

    virtual void positionSet(unsigned int encoderNr, int p);
    virtual void turnSet(unsigned int encoderNr, int t );
    const int positionGet(unsigned int encoderNr) const;
    const int turnGet(unsigned int encoderNr) const;

private:
    long               _virtualEncoder[6];
    unsigned short     _prevEncoderValue[6];
    RTT::DataObjectLockFree<long>  _virtual_encoder_1, _virtual_encoder_2, _virtual_encoder_3, 
                                           _virtual_encoder_4, _virtual_encoder_5, _virtual_encoder_6;
};


class IP_Encoder_6_EncInterface : public RTT::EncoderInterface
{
public:
    IP_Encoder_6_EncInterface( IP_Encoder_6_Task& IP_encoder_task, const unsigned int& encoder_nr, const std::string& name );
    IP_Encoder_6_EncInterface( IP_Encoder_6_Task& IP_encoder_task, const unsigned int& encoder_nr );
    virtual ~IP_Encoder_6_EncInterface() {};


    virtual int  positionGet() const { return _encoder_task->positionGet( _encoder_nr ); }
    virtual int  turnGet() const     { return _encoder_task->turnGet( _encoder_nr ); }
    virtual void positionSet( int p) { _encoder_task->positionSet( _encoder_nr, p ); }
    virtual void turnSet( int t )    { _encoder_task->turnSet( _encoder_nr, t ); }
    virtual int  resolution() const  { return 2000; }
    virtual bool upcounting() const  { return true; }

private:
    unsigned int         _encoder_nr;
    IP_Encoder_6_Task*   _encoder_task;
};

#endif
