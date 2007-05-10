#include "EthercatDemoIO.hpp"

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




///SlaveConfig EK1100
EtherCAT_FMMU_Config fmmu_config_EK1100(0);
EtherCAT_PD_Config pd_config_EK1100(0);
EtherCAT_SlaveConfig EC_EK1100(0x044c2c52,0x00010000,1001,&fmmu_config_EK1100,&pd_config_EK1100);

///SlaveConfig EL4102
EtherCAT_FMMU_Config fmmu_config_EL4102(2);
EC_FMMU fmmu0_EL4102(0x00080000,1,0,0,0x080D,0,true,false,true);
EC_FMMU fmmu1_EL4102(0x00010000,4,0,7,0x1000,0,false,true,true);

EC_SyncMan syncman_mbx0_EL4102(0x1800,246,EC_QUEUED,EC_WRITTEN_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EC_SyncMan syncman_mbx1_EL4102(0x18f6,246,EC_QUEUED,EC_READ_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EtherCAT_MbxConfig mbx_conf_EL4102 = {syncman_mbx0_EL4102, syncman_mbx1_EL4102};

EtherCAT_PD_Config pd_config_EL4102(2);
EC_SyncMan
		syncman0_EL4102(0x1000,4,EC_BUFFERED,EC_WRITTEN_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EC_SyncMan
		syncman1_EL4102(0x1100,0,EC_BUFFERED,EC_READ_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);

EtherCAT_SlaveConfig EC_EL4102(0x10063052,0,1002,&fmmu_config_EL4102,&pd_config_EL4102,&mbx_conf_EL4102);


///SlaveConfig EL3162
EtherCAT_FMMU_Config fmmu_config_EL3162(2);
EC_FMMU fmmu0_EL3162(0x00080000,1,1,1,0x080D,0,true,false,true);
EC_FMMU fmmu1_EL3162(0x00010004,6,0,7,0x1100,0,true,false,true);

EC_SyncMan syncman_mbx0_EL3162(0x1800,246,EC_QUEUED,EC_WRITTEN_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EC_SyncMan syncman_mbx1_EL3162(0x18f6,246,EC_QUEUED,EC_READ_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EtherCAT_MbxConfig mbx_conf_EL3162 = {syncman_mbx0_EL3162, syncman_mbx1_EL3162};

EtherCAT_PD_Config pd_config_EL3162(2);
EC_SyncMan syncman0_EL3162(0x1000,0,EC_BUFFERED,EC_WRITTEN_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,false);
EC_SyncMan syncman1_EL3162(0x1100,6,EC_BUFFERED,EC_READ_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);

EtherCAT_SlaveConfig EC_EL3162(0x0C5A3052,0,1003,&fmmu_config_EL3162, &pd_config_EL3162, &mbx_conf_EL3162);


///SlaveConfig EL2004(1)
EtherCAT_FMMU_Config fmmu_config_EL2004_1(1);
EC_FMMU fmmu0_EL2004_1(0x0001000A,1,0,3,0x0F00,0,false,true,true);

EtherCAT_PD_Config pd_config_EL2004_1(1);
EC_SyncMan syncman0_EL2004_1(0x0F00,1,EC_QUEUED,EC_WRITTEN_FROM_MASTER,false,true,false,false,false,false,EC_FIRST_BUFFER,true);

EtherCAT_SlaveConfig EC_EL2004_1(0x7d43052,0,1004,&fmmu_config_EL2004_1,&pd_config_EL2004_1);

///SlaveConfig EL2004(2)
EtherCAT_FMMU_Config fmmu_config_EL2004_2(1);
EC_FMMU fmmu0_EL2004_2(0x0001000A,1,4,7,0x0F00,0,false,true,true);

EtherCAT_PD_Config pd_config_EL2004_2(1);
EC_SyncMan syncman0_EL2004_2(0x0F00,1,EC_QUEUED,EC_WRITTEN_FROM_MASTER,false,true,false,false,false,false,EC_FIRST_BUFFER,true);

EtherCAT_SlaveConfig EC_EL2004_2(0x7d43052,0,1005,&fmmu_config_EL2004_2,&pd_config_EL2004_2);

///SlaveConfig EL1014(1)
EtherCAT_FMMU_Config fmmu_config_EL1014_1(1);
EC_FMMU fmmu0_EL1014_1(0x0001000B,1,0,3,0x1000,0,true,false,true);

EtherCAT_PD_Config pd_config_EL1014_1(1);
EC_SyncMan syncman0_EL1014_1(0x1000,1,EC_BUFFERED,EC_READ_FROM_MASTER,false,false,false,false,false,false,EC_FIRST_BUFFER,true);

EtherCAT_SlaveConfig EC_EL1014_1(0x03F63052,0,1006,&fmmu_config_EL1014_1, &pd_config_EL1014_1);

///SlaveConfig EL1014(2)
EtherCAT_FMMU_Config fmmu_config_EL1014_2(1);
EC_FMMU fmmu0_EL1014_2(0x0001000B,1,4,7,0x1000,0,true,false,true);

EtherCAT_PD_Config pd_config_EL1014_2(1);
EC_SyncMan syncman0_EL1014_2(0x1000,1,EC_BUFFERED,EC_READ_FROM_MASTER,false,false,false,false,false,false,EC_FIRST_BUFFER,true);

EtherCAT_SlaveConfig EC_EL1014_2(0x03F63052,0,1007,&fmmu_config_EL1014_2, &pd_config_EL1014_2);

///SlaveConfig EL5101(1)
EtherCAT_FMMU_Config fmmu_config_EL5101_1(2);
EC_FMMU fmmu0_EL5101_1(0x00010800,3,0,7,0x1000,0,false,true,true);
EC_FMMU fmmu1_EL5101_1(0x00011000,5,0,7,0x1100,0,true,false,true);

EC_SyncMan syncman_mbx0_EL5101_1(0x1800,246,EC_QUEUED,EC_WRITTEN_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EC_SyncMan syncman_mbx1_EL5101_1(0x18f6,246,EC_QUEUED,EC_READ_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EtherCAT_MbxConfig mbx_conf_EL5101_1 = {syncman_mbx0_EL5101_1, syncman_mbx1_EL5101_1};

EtherCAT_PD_Config pd_config_EL5101_1(2);
EC_SyncMan syncman0_EL5101_1(0x1000,3,EC_BUFFERED,EC_WRITTEN_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EC_SyncMan syncman1_EL5101_1(0x1100,5,EC_BUFFERED,EC_READ_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);

EtherCAT_SlaveConfig EC_EL5101_1(0x13ED3052,0x270B0000,1008,&fmmu_config_EL5101_1, &pd_config_EL5101_1, &mbx_conf_EL5101_1);

///SlaveConfig EL5101(2)
EtherCAT_FMMU_Config fmmu_config_EL5101_2(2);
EC_FMMU fmmu0_EL5101_2(0x00010803,3,0,7,0x1000,0,false,true,true);
EC_FMMU fmmu1_EL5101_2(0x00011005,5,0,7,0x1100,0,true,false,true);

EC_SyncMan syncman_mbx0_EL5101_2(0x1800,246,EC_QUEUED,EC_WRITTEN_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EC_SyncMan syncman_mbx1_EL5101_2(0x18f6,246,EC_QUEUED,EC_READ_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EtherCAT_MbxConfig mbx_conf_EL5101_2 = {syncman_mbx0_EL5101_2, syncman_mbx1_EL5101_2};

EtherCAT_PD_Config pd_config_EL5101_2(2);
EC_SyncMan syncman0_EL5101_2(0x1000,3,EC_BUFFERED,EC_WRITTEN_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);
EC_SyncMan syncman1_EL5101_2(0x1100,5,EC_BUFFERED,EC_READ_FROM_MASTER,true,false,false,false,false,false,EC_FIRST_BUFFER,true);

EtherCAT_SlaveConfig EC_EL5101_2(0x13ED3052,0x270B0000,1009,&fmmu_config_EL5101_2, &pd_config_EL5101_2, &mbx_conf_EL5101_2);


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



namespace OCL
{
    using namespace RTT;
    using namespace std;
 
		EthercatDemoIO::EthercatDemoIO(string name):
			TaskContext(name), 
			rteth("rteth","for example: rteth0","rteth0"), 
			gen_sin("gen_sin",&EthercatDemoIO::mgen_sin, this), 
			stop_master("stop_master", &EthercatDemoIO::mstop_master, this)
		{
			this->properties()->addProperty(&rteth);
			this->methods()->addMethod(&gen_sin);
			this->methods()->addMethod(&stop_master);
			
			ni = init_ec( rteth.get().c_str() );
			init_slave_db();
			if(ni != 0) {
		
				EtherCAT_DataLinkLayer::instance()->attach(ni);
				printf("Master initializing \n\n");
				EtherCAT_Master * EM = EtherCAT_Master::instance();
				EtherCAT_SlaveHandler * sh;
				for(int i = 1001; i<1010; i++){
					printf("Getting slave handler\n");
					sh = EM->get_slave_handler(i);
					if(sh && sh->to_state(EC_OP_STATE))
						printf("Slave %d succesfully set to OP STATE\n", i);
					else
						printf("\nFailed to set slave %d to OP STATE\n", i);
				}
				int64_t timeout = 1000 * 50;
				if (set_socket_timeout(ni, timeout) == 0)
					printf("Socket timeout set to %d\n", (int)timeout);
			}
		}
		
		EthercatDemoIO::~EthercatDemoIO() {
			if(ni != 0)
				close_socket(ni);
		}

		bool EthercatDemoIO::mgen_sin(double f) {
			if(f <= 0)
				return false;
			sinus_freq = f;
			return true;
		}

		void EthercatDemoIO::mstop_master(void) {
		
		}

		bool EthercatDemoIO::startup() {
			cnt = 0; cnt_dig = 0;
			voltage_i = 0;
			voltage_f = 0; time = 0;
			sinus_freq = 1000;
			EtherCAT_AL * AL = EtherCAT_AL::instance();
			return ni && AL && AL->isReady();
		}
		void EthercatDemoIO::update() {
			EtherCAT_Master *EM = EtherCAT_Master::instance();
			//printf("Got master\n");
			unsigned char msg[10] = {0};		//Data to Send
			int samples = (int)(1000.0 / this->engine()->getActivity()->getPeriod() / sinus_freq);
			if(cnt > samples)
				cnt = 0;
			time = M_PI * 2 * cnt / samples;
			voltage_f = (sin(time) * 5 + 5) * 32767 / 10;
			voltage_i = (int) voltage_f;
			msg[0] = voltage_i;	msg[1] = voltage_i>>8;
			//msg[4] = cnt_dig & 0x01;
			//printf("mdg[4] = %d\n",msg[4]);
			//printf("samples = %d cnt = %d   time = %f    voltage = %f  msg[0] =0x%x     msg[1] = 0cx%x \n", samples,cnt,time, voltage_f,msg[0], msg[1]);							// Digital Output Channel 1 			
			cnt++;	cnt_dig++;
			EM->txandrx_PD(sizeof(msg),msg);
			int volt = msg[6];
			volt = volt<<8;
			volt = volt | msg[5];
			
			//printf("voltage at channel0: 0x%x	0x%x	%d\n",msg[5],msg[6], volt);
		}
		
		void EthercatDemoIO::shutdown() {
			
		}
}//namespace orocos
