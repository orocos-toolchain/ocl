
#include <al/ethercat_slave_conf.h>
#include <dll/ethercat_slave_memory.h>

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
EC_FMMU fmmu0_EL5101_1(0x00010010,3,0,7,0x1000,0,false,true,true);
EC_FMMU fmmu1_EL5101_1(0x00010013,5,0,7,0x1100,0,true,false,true);

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

