#include "EthercatIO.hpp"
#include "slaveconfigurations.hpp"
#include "dev/DigitalEtherCATOutputDevice.hpp"
#include "dev/EtherCATEncoder.hpp"

#include <rtt/Logger.hpp>

#include <native/task.h>
#include <native/timer.h>
#include <posix/pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <math.h>
#include <stdlib.h>

#include <al/ethercat_master.h>
#include <al/ethercat_AL.h>
#include <al/ethercat_process_data.h>
#include <ethercat_xenomai_drv.h>
#include <ethercat/netif.h>
#include <dll/ethercat_dll.h>
#include <dll/ethercat_frame.h>
#include <dll/ethercat_slave_memory.h>
#include <al/ethercat_slave_conf.h>
#include <al/ethercat_slave_handler.h>

static void init_slave_db();

namespace OCL
{
    using namespace RTT;
    using namespace std;

		EthercatIO::EthercatIO(string name):
			TaskContext(name),
			rteth("rteth","for example: rteth0","rteth0"),
			get_pos("get_pos",&EthercatIO::mget_pos, this),
			set_pos("set_pos",&EthercatIO::mset_pos, this),
			get_turn("get_turn",&EthercatIO::mget_turn, this),
			set_turn("set_turn",&EthercatIO::mset_turn, this),
			upcounting("upcounting",&EthercatIO::mupcounting, this),
			fmmu_buffer(new unsigned char[40]),
			digoutputs(fmmu_buffer + 10,0,4),
			digoutputs2(fmmu_buffer + 10,4,4),
			diginputs(fmmu_buffer + 11,0,4),
			anaoutputs(fmmu_buffer,2,32676,0,10),
			anainputs(fmmu_buffer + 4,2,32676,0,10),
			enc(fmmu_buffer + 0x13,fmmu_buffer + 0x10, 65536, true)
		{

			this->properties()->addProperty(&rteth);
			this->methods()->addMethod(&get_pos,"get the position");
			this->methods()->addMethod(&set_pos,"set the position","position","the value of the desired position");
			this->methods()->addMethod(&get_turn,"get the turn");
			this->methods()->addMethod(&set_turn,"set the turn","turn","The value of the desired turn");
			this->methods()->addMethod(&upcounting,"get the counting direction (up = true, down = false)");
			for(int i = 0; i< 40; i++)
				fmmu_buffer[i] = 0x00;

			ni = init_ec( rteth.get().c_str() );
			init_slave_db();
			if(ni != 0) {

				EtherCAT_DataLinkLayer::instance()->attach(ni);
				printf("Master initializing \n\n");
				EtherCAT_Master * EM = EtherCAT_Master::instance();
				EtherCAT_SlaveHandler * sh;
				if(EtherCAT_AL::instance()->isReady()) {
					for(int i = 1001; i<1010; i++){
						//printf("Getting slave handler\n");
						sh = EM->get_slave_handler(i);
						if(!sh || !sh->to_state(EC_OP_STATE))
							printf("\nFailed to set slave %d to OP STATE\n", i);
					}
					int64_t timeout = 1000 * 50;
					if (set_socket_timeout(ni, timeout) == 0)
						printf("Socket timeout set to %d\n", (int)timeout);
				}
			}
		}

		EthercatIO::~EthercatIO() {
			if(ni != 0)
				close_socket(ni);
		}

		int EthercatIO::mget_pos(void) {
			return enc.positionGet();
		}

		void EthercatIO::mset_pos(int pos) {
			enc.positionSet(pos);
		}

		int EthercatIO::mget_turn(void) {
			return enc.turnGet();
		}

		void EthercatIO::mset_turn(int turn){
			enc.turnSet(turn);
		}

		bool EthercatIO::mupcounting(void) {
			return enc.upcounting();
		}

		bool EthercatIO::startup() {
			cnt_dig = 0;
			prevvoltage = 50;
			//ones = false;
			EtherCAT_AL * AL = EtherCAT_AL::instance();
			return ni && AL && AL->isReady();
		}
		void EthercatIO::update() {
			EtherCAT_Master *EM = EtherCAT_Master::instance();
			/*
			digoutputs.switchOff((cnt_dig - 1)%4);
			digoutputs.switchOn(cnt_dig%4);
			digoutputs2.switchOff((cnt_dig - 1)%4);
			digoutputs2.switchOn(cnt_dig%4);
			anaoutputs.write(0,5.0);
			double voltage;
			anainputs.read(0,voltage);
			if(voltage < (prevvoltage - 0.2) || voltage > (prevvoltage + 0.2) ) {
				printf("Voltage at Channel 0 = %f\n", voltage);
				prevvoltage = voltage;
		}*/
			//fmmu_buffer[0] = 0xff; fmmu_buffer[1] = 0x3f; fmmu_buffer[2] = 0xff; fmmu_buffer[3] = 0x3f;
			/*anaoutputs.write(0,5.0);
			if(!ones) {
				printf("fmmu_buffer[0] = %x\tfmmu_buffer[1] = %x\tfmmu_buffer[2] = %x\tfmmu_buffer[3] = %x\n", fmmu_buffer[0], fmmu_buffer[1], fmmu_buffer[2], fmmu_buffer[3]);
				ones = true;
			}
			cnt_dig++;
			*/
			enc.update();
			EM->txandrx_PD(40,fmmu_buffer);

		}

		void EthercatIO::shutdown() {

		}
}//namespace orocos


static void init_slave_db() {
	EtherCAT_SlaveDb * slave_db = EtherCAT_SlaveDb::instance(9);
	fmmu_config_EL4102[0] = fmmu0_EL4102;
	fmmu_config_EL4102[1] = fmmu1_EL4102;
	pd_config_EL4102[0] = syncman0_EL4102;
	pd_config_EL4102[1] = syncman1_EL4102;
	fmmu_config_EL3162[0] = fmmu0_EL3162;
	fmmu_config_EL3162[1] = fmmu1_EL3162;
	pd_config_EL3162[0] = syncman0_EL3162;
	pd_config_EL3162[1] = syncman1_EL3162;
	fmmu_config_EL2004_1[0] = fmmu0_EL2004_1;
	pd_config_EL2004_1[0] = syncman0_EL2004_1;
	fmmu_config_EL2004_2[0] = fmmu0_EL2004_2;
	pd_config_EL2004_2[0] = syncman0_EL2004_2;
	fmmu_config_EL1014_1[0] = fmmu0_EL1014_1;
	pd_config_EL1014_1[0] = syncman0_EL1014_1;
	fmmu_config_EL1014_2[0] = fmmu0_EL1014_2;
	pd_config_EL1014_2[0] = syncman0_EL1014_2;
	fmmu_config_EL5101_1[0] = fmmu0_EL5101_1;
	fmmu_config_EL5101_1[1] = fmmu1_EL5101_1;
	pd_config_EL5101_1[0] = syncman0_EL5101_1;
	pd_config_EL5101_1[1] = syncman1_EL5101_1;
	fmmu_config_EL5101_2[0] = fmmu0_EL5101_2;
	fmmu_config_EL5101_2[1] = fmmu1_EL5101_2;
	pd_config_EL5101_2[0] = syncman0_EL5101_2;
	pd_config_EL5101_2[1] = syncman1_EL5101_2;
	slave_db->set_conf(&EC_EK1100,0);
	slave_db->set_conf(&EC_EL4102,1);
	slave_db->set_conf(&EC_EL3162,2);
	slave_db->set_conf(&EC_EL2004_1,3);
	slave_db->set_conf(&EC_EL2004_2,4);
	slave_db->set_conf(&EC_EL1014_1,5);
	slave_db->set_conf(&EC_EL1014_2,6);
	slave_db->set_conf(&EC_EL5101_1,7);
	slave_db->set_conf(&EC_EL5101_2,8);
}
