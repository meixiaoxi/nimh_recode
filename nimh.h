#ifndef __NIMH_H__
#define __NIMH_H__

#include "MC96F8316.h"

typedef  unsigned char		u8;
typedef	 unsigned short 		u16;		
typedef 	 unsigned long		u32;

//#define EVT_BOARD
#define DVT_BOARD

#define GET_FACTORY_STATUS()	(P2&0x20)

#define		cli()		do{IE &= ~0x80;}while(0)
#define		sei()		do{IE |=  0x80;}while(0)
#define		NOP()		_nop_()
#define 		ClrWdt()		WDTCR = 0xE0
#define LED_ALL_OFF()	LED_OFF(BT_1),LED_OFF(BT_2),LED_OFF(BT_3),LED_OFF(BT_4)

#define CHANGE_TO_IO()		P0FSR &= 0xEF
#define CHANGE_TO_VIN5V()	P0FSR |= 0x10

#ifdef EVT_BOARD
#warning "evt version"
#define 		GET_SYS_STATUS()	P1&0x01

#define		CHANGE_TO_OUTPUT()	(P0IO |= (1<<2),P02 = 0,P2IO |= (1<<4),P24 = 0)
#define		CHANGE_TO_INPUT()		(P0IO &= ~(1<<2),P2IO &=~(1<<4))

#define SYS_CHARGING_STATE	0x00
#define SYS_DISCHARGE_STATE	0x01
#else
#warning "dvt version"
#define		GET_SYS_STATUS()	P0&0x04

#define		CHANGE_TO_OUTPUT()	NOP() //(P2IO |= (1<<4),P24 = 0)
#define		CHANGE_TO_INPUT()		NOP() //(P2IO &=~(1<<4))

#define SYS_CHARGING_STATE	0x00
#define SYS_DISCHARGE_STATE	0x04
#endif

#define toNextBattery()	if(gIsChargingBatPos >= BT_4)\
			gIsChargingBatPos = BT_1;\
		else\
			gIsChargingBatPos++;\


#define STATE_DEAD_BATTERY 							0
#define STATE_BATTERY_DETECT							1
#define STATE_NORMAL_CHARGING 						2
#define STATE_BATTERY_FULL								3
#define STATE_BATTERY_TYPE_ERROR						4
#define STATE_BATTERY_TEMPERATURE_ERROR 				5
//#define STATE_BT_1_TEMPERATURE_ERROR 				6
//#define STATE_BT_1_BATTERY_CHARGE_ERROR				7

#define CHARGING_TIME_0MS	0
#define CHARGING_TIME_10MS	1
#define CHARGING_TIME_20MS	2
#define CHARGING_TIME_30MS	3
#define CHARGING_TIME_200MS	20
//#define CHARGING_TIME_500MS	50

//#define CHARGING_TIME_1S		100


#define MIN_DETECT_PRE_BATTERY	10     //100ms


#define CURRENT_LEVEL_1		1
#define CURRENT_LEVEL_2		2
#define CURRENT_LEVEL_3		3
//#define CURRENT_LEVEL_4		4

//#define SYS_CHARGING_STATUS_DETECT	0
//#define SYS_CHARGING_STATUS_NORMAL	1


#define SYS_CHARGE_WAIT_TO_PICK_BATTERY		0
#define SYS_CHARGE_IS_CHARGING				1

#define CHARGE_STATE_PRE	0
#define CHARGE_STATE_FAST	1

#define BT_1		0
#define BT_2		1
#define BT_3		2	
#define BT_4		3
#define BT_NULL	4


#define UP_CURRENT		0
#define DN_CURRENT		1

#define PWM_ON	1
#define PWM_OFF	 0

#define SYS_CHARGING_STATE_DELAY	0x02

#define	BAT_MAX_LABEL 		4



#define BAT_ZERO_DETECT_TICK			6	  // 100ms 100/16.384
#define BAT_ZERO_VOLT_OPEN			124

#define LDO_LEVEL	3300

#if (LDO_LEVEL == 3300)
#warning "3.3v ldo"

#define ADC_DV_VOLT						6			//5mv   0.005/3.3*4096

#define BAT_HAS_BATTERY_VOLT			868		// 0.7  (0.7/3.3)*4096

#define BAT_MIN_VOLT_33_OPEN	472		//(0.3/2.6)*4096
#define BAT_MIN_VOLT_OPEN	372 			//(0.3/3.3)*4096
#define BAT_MAX_VOLT_OPEN	1923		//(1.55/3.3)*4096

#define BAT_MIN_VOLT_OPEN_SPE	124 			//(0.1/3.3)*4096

#define BAT_MAX_VOLT_CLOSE 2445		//(1.97/3.3*4096)        (1.8/3.3)*4096	
#define BAT_MAX_VOLT_CLOSE_CHANNEL_4		2234	//(1.8/3.3)*4096

#define BAT_MAX_CLOS_LEVEL_2		2142				//(1.65+0.076)  (1.726/3.3*4096)
#define BAT_MAX_CLOSE_CHANNEL_4_LEVEL_2	 2048		// (1.65   1.65/3.3)*4096

#define BAT_MAX_CLOS_LEVEL_3			1990						// 1.57 + 0.034
#define BAT_MAX_CLOSE_CHANNEL_4_LEVEL_3	1948				// 1.57  1.57/3.3*4096

#define BAT_LEVEL_LOW_TO_MIDD		1737			// 1.4     (1.4/3.3)*4096
#define BAT_LEVEL_MIDD_TO_HIGH		1750				//  1.41	(1.41/3.3)*4096

#define CHARGING_PRE_END_VOLT		1117			//(0.9/3.3)*4096
#define CHARGING_FAST_END_VOLT	1799			//(1.45/3.3)*4096
#define CHARGING_FAST_MAX_VOLT	1986			//(1.6/3.3)*4096

#define CHARGING_FAST_TEMP_END_VOLT	1799			// (1.45/3.3)*4096

#define DV_ENABLE_MIN_VOLT		1837						//  (1.48/3.3)*4096
#define BAT_VOLT_CHANGE_FASTER	1750						//  (1.41/3.3)*4096

#define BAT_VOLT_ALK_MIN		620				// 1.5	(1.5/3/3.3*4096)
#define BAT_VOLT_ALK_MIN_CHANNEL_4	1861		// 1.5     (1.5/3.3*4096)

#define BAT_LEVEL_12_CHARGING		1700		// 1.37   (1.37/3.3*4096)
#define BAT_LEVEL_23_CHARGING		1725		// 1.39    (1.39/3.3*4096)
#define BAT_LEVEL_34_CHARGING		1747		// 1.408   (1.408/3.3*4096)

#define BAT_INITIAL_VOLT_FULL		1824		//  1.47(1.47/3.3*4096)

//output
#define MIN_VBAT_CHANNEL_1_IDLE	 1944  //(4.7/3=2.4V   2.4/3.3*4096)
#define MIN_VBAT_OUPUT				351	//(0.85/3=0.283	0.3/3.3*4096)
#define MIN_VBAT_OUTPUT_CHANNEL_4	372	// 0.9/3=0.3      0.3/3.3*4096
#define MIN_VBAT_OUTPUT_IDLE		455	// (1.1/3 = 0.366  0.366/3.3*4096)
#define	OUTPUT_SHOW_LEVEL_3		537	//(1.3/3  0.65/3.3*4096 )
#define	OUTPUT_SHOW_LEVEL_2		517	//(1.25/3 0.625/3.3*4096)	

#define BAT_MIN_VOLT_NO_BATTERY	206		// 0.5/3  0.16/3.3*4096
#define BAT_MIN_VOLT_NO_BATTERY_CHANNEL_4	620	//0.5  0.5/3.3*4096

#define MIN_VBAT_OUTPUT_ALK	330		// 0.8 (0.8/3/3.3*4096)

#define MIN_VBAT_1		1406				//  3.4/3 for LDO   3.4/3/3.3*4096


//#define	MIN_OUTPUT_DISPLAY_VOLT	289	 // 0.7  0.7/3/3.3*4096

#define	BAT_LEVEL_43_OUTPUT		521      // 1.26/3 (1.26/3.3/2*4096)
#define 	BAT_LEVEL_32_OUTPUT		508      //  1.23/3 (1.23/3.3/2*4096)
#define   BAT_LEVEL_21_OUTPUT		500	//  1.21/3(1.21/3.3/2*4096)

#define	BAT_LEVEL_43_IDLE		1613      // 1.3 (1.3/3.3*4096)
#define 	BAT_LEVEL_32_IDLE		1576      //  1.27 (1.27/3.3*4096)
#define   BAT_LEVEL_21_IDLE		1551	//  1.25 (1.25/3.3*4096)

#define BAT_ZERO_SPEC_VOLT		3103		// 2.3 (2.3/3.3*4096) 


#define VIN_5V_TEST_MAX		3872			// 3.12V   3.12/3.3*4096
#define VIN_5V_TEST_MIN		3611				// 2.91	(2.91/3.3*4096)

#define VIN_5V_MINUM			3425			// 4.6  (4.6*3/5/3.3*4096)
#define BAT_VOLT_NEAR_FULL		1812			// 1.46  1.46/3.3*4096
#define BAT_VOLT_NEAR_FULL_AAA		1824				// 1.47
#define VIN_5V_NO_EXIST			3351				// 4.5 (4.5*0.6/3.3*4096)

#define BAT_START_FAST_CHARGE		1799		// 1.45  1.45/3.3*4096

#define BAT_REMOVE_VOLT		868


#define TEST_VOLT_YUNFANG_MIN		1075			// 2.6 (2.6/3/3.3*4096)
#define TEST_VOLT_YUNFANG_MAX	1241				// 3  (3.1/3/3.3*4096)

#define TEST_DIVIDE_1V3_MIN	1179				// 0.95   1.628/3.3*4096
#define TEST_DIVIDE_1V3_MAX	1303					//  1.05	1.705/3.3*4096

#define TEST_VOLT_YUNFANG_MIN_4		2854			// 2.3 (2.3/3/3.3*4096)
#define TEST_VOLT_YUNFANG_MAX_4		3723				// 3  (3.1/3/3.3*4096)

#define TEST_3V3_VOLT_MAX			1415				//  1.137 1.14     1.14/3.3*4096
#define TEST_3V3_VOLT_MIN			1316					// 1.063 1.06      1.06/3.3*4096


#define VBAT_IDLE_VOLT_MAX		62					// 0.05V 0.05/3.3*4096
#define VBAT_IDLE_VOLT_3V3_MIN	3723			// 3V 3/(150/(150+300+510))/3.3*4096
#define VBAT_IDLE_VOLT_3V3_MAX    	659			// 3.4V  3.4/(150/(150+300+510))/3.3*4096

//temperature adc
#define 	ADC_TEMP_MAX	1121    //53
#define	ADC_TEMP_MIN	3177	//-5

#define ADC_TEMP_DOWN_CURRENT  1203	// 50

#define ADC_TEMP_MAX_RECOVERY    1260	  // 48
#define ADC_TEMP_MIN_RECOVERY	    3111	 // -3


//output temp
#define ADC_TEMP_OUTPUT_MAX	   839       //65
#define ADC_TEMP_OUTPUT_MIN	   3331		 // -10

#define ADC_TEMP_OUTPUT_MAX_RECOVERY	   881       //63
#define ADC_TEMP_OUTPUT_MIN_RECOVERY	   3271		 // -8


//factory test
 //   1.8 - 2.5A     700 -1100   20 /30       36mV~ 50mV    35mv~55mV 
  //  600 - 1200     150 - 550	20/30    12mV~24mV      7mV~28mV		
  // 100 - 650   50 -300               20/30        2mV~13mV     2mV ~15mV

#define CURRENT_MAX_LEVEL_1		67			//50mV   50/3300*4096	
#define CURRENT_MIN_LEVEL_1             44                    //36      36/3300*4096

#define CURRENT_MAX_LEVEL_2		34			// 24mV   24/3300*4096
#define CURRENT_MIN_LEVEL_2		12			// 12mV  12/3300*4096

#define CURRENT_MAX_LEVEL_3		20			//  13   13/3300*4096
#define CURRENT_MIN_LEVEL_3		2			// 2		2/3300*4096

#define CURRENT_MAX_LEVEL_1_AAA		70			//55mV   55/3300*4096	
#define CURRENT_MIN_LEVEL_1_AAA             40                    //35      35/3300*4096

#define CURRENT_MAX_LEVEL_2_AAA		38			// 28mV   28/3300*4096
#define CURRENT_MIN_LEVEL_2_AAA		6			// 7mV  8/3300*4096

#define CURRENT_MAX_LEVEL_3_AAA		20			//  15   15/3300*4096
#define CURRENT_MIN_LEVEL_3_AAA		1			// 2		2/3300*4096

#elif (LDO_LEVEL == 4000)
#warning "4v ldo"

#define ADC_DV_VOLT						6			//5mv   0.005/4*4096

#define BAT_MIN_VOLT_OPEN	307 			//(0.3/4)*4096
#define BAT_MAX_VOLT_OPEN	1587		//(1.55/4)*4096

#define BAT_MIN_VOLT_OPEN_SPE	307 			//(0.3/4)*4096

#define BAT_MAX_VOLT_CLOSE 2048		//(2/4*4096)
#define BAT_MAX_VOLT_CLOSE_CHANNEL_4		1843	//(1.8/4)*4096


#define BAT_LEVEL_LOW_TO_MIDD		1433			// 1.4     (1.4/4)*4096
#define BAT_LEVEL_MIDD_TO_HIGH		1443				//  1.41	(1.41/4)*4096

#define CHARGING_PRE_END_VOLT		921			//(0.9/4)*4096
#define CHARGING_FAST_END_VOLT	1484			//(1.45/4)*4096
#define CHARGING_FAST_MAX_VOLT	1638			//(1.6/4)*4096

#define CHARGING_FAST_TEMP_END_VOLT	1484			// (1.45/4)*4096

#define DV_ENABLE_MIN_VOLT		1464						//  (1.43/4)*4096
#define BAT_VOLT_CHANGE_FASTER	1443						//  (1.41/4)*4096


#define BAT_LEVEL_12_CHARGING		1402		// 1.37   (1.37/4*4096)
#define BAT_LEVEL_23_CHARGING		1423		// 1.39    (1.39/4*4096)
#define BAT_LEVEL_34_CHARGING		1441		// 1.408   (1.408/4*4096)

#define BAT_INITIAL_VOLT_FULL		1484		// 1.45 (1.45/4*4096)

//output
#define MIN_VBAT_CHANNEL_1_IDLE	 1604  //(4.7/3=2.4V   2.4/4*4096)
#define MIN_VBAT_OUPUT				341	//(1/3=0.5	0.5/4*4096)
#define MIN_VBAT_OUTPUT_IDLE		402	// (1.18/3 = 0.41  0.65/4*4096)
#define	OUTPUT_SHOW_LEVEL_3		443	//(1.3/3  0.65/4*4096 )
#define	OUTPUT_SHOW_LEVEL_2		426	//(1.25/3 0.625/4*4096)	

#define	BAT_LEVEL_43_OUTPUT		430      // 1.26/3 (1.26/3.3/2*4096)
#define 	BAT_LEVEL_32_OUTPUT		419      //  1.23/3 (1.23/3.3/2*4096)
#define   BAT_LEVEL_21_OUTPUT		413	//  1.21/3(1.21/3.3/2*4096)

#define	BAT_LEVEL_43_IDLE		1331      // 1.3 (1.3/4*4096)
#define 	BAT_LEVEL_32_IDLE		1300      //  1.27 (1.27/4*4096)
#define   BAT_LEVEL_21_IDLE		1280	//  1.25 (1.25/4*4096)

#define BAT_ZERO_SPEC_VOLT		2560		// 2.5 (2.5/4*4096) 

//temperature adc
#define 	ADC_TEMP_MAX	1203    //50
#define	ADC_TEMP_MIN	3177	//-5
#elif (LDO_LEVEL == 2600)
#warning "2.6v ldo"

#define ADC_DV_VOLT						8			//5mv   0.005/2.6*4096

#define BAT_MIN_VOLT_OPEN	472 			//(0.3/2.6)*4096
#define BAT_MAX_VOLT_OPEN	2441		//(1.55/2.6)*4096

#define BAT_MIN_VOLT_OPEN_SPE	472 			//(0.3/2.6)*4096

#define BAT_MAX_VOLT_CLOSE 3150		//(2/2.6*4096)
#define BAT_MAX_VOLT_CLOSE_CHANNEL_4		2835	//(1.8/2.6)*4096


#define BAT_LEVEL_LOW_TO_MIDD		2205			// 1.4     (1.4/2.6)*4096
#define BAT_LEVEL_MIDD_TO_HIGH		2221				//  1.41	(1.41/2.6)*4096

#define CHARGING_PRE_END_VOLT		1417			//(0.9/2.6)*4096
#define CHARGING_FAST_END_VOLT	2284			//(1.45/2.6)*4096
#define CHARGING_FAST_MAX_VOLT	2520			//(1.6/2.6)*4096

#define CHARGING_FAST_TEMP_END_VOLT	2284			// (1.45/2.6)*4096

#define DV_ENABLE_MIN_VOLT		2252						//  (1.43/2.6)*4096
#define BAT_VOLT_CHANGE_FASTER	2221						//  (1.41/2.6)*4096


#define BAT_LEVEL_12_CHARGING		2158		// 1.37   (1.37/2.6*4096)
#define BAT_LEVEL_23_CHARGING		2189		// 1.39    (1.39/2.6*4096)
#define BAT_LEVEL_34_CHARGING		2218		// 1.408   (1.408/2.6*4096)

#define BAT_INITIAL_VOLT_FULL		2284		// 1.45 (1.45/2.6*4096)

//output
#define MIN_VBAT_CHANNEL_1_IDLE	 2468  //(4.7/3=2.4V   2.4/2.6*4096)
#define MIN_VBAT_OUPUT				525	//(1/3=0.5	0.5/2.6*4096)
#define MIN_VBAT_OUTPUT_IDLE		619	// (1.18/3 = 0.41  0.65/4*4096)
#define	OUTPUT_SHOW_LEVEL_3		682	//(1.3/3  0.65/4*4096 )
#define	OUTPUT_SHOW_LEVEL_2		656	//(1.25/3 0.625/4*4096)	

#define	BAT_LEVEL_43_OUTPUT		661      // 1.26/3 (1.26/3.3/2*4096)
#define 	BAT_LEVEL_32_OUTPUT		645      //  1.23/3 (1.23/3.3/2*4096)
#define   BAT_LEVEL_21_OUTPUT		635	//  1.21/3(1.21/3.3/2*4096)

#define	BAT_LEVEL_43_IDLE		2048      // 1.3 (1.3/2.6*4096)
#define 	BAT_LEVEL_32_IDLE		2000      //  1.27 (1.27/2.6*4096)
#define   BAT_LEVEL_21_IDLE		1969	//  1.25 (1.25/2.6*4096)

#define BAT_ZERO_SPEC_VOLT		3938		// 2.5 (2.5/2.6*4096) 

//temperature adc
#define 	ADC_TEMP_MAX	1203    //50
#define	ADC_TEMP_MIN	3177	//-5
#endif

#if 0
#define BAT_MIN_VOLT_33_OPEN	472		//(0.3/2.6)*4096
#define BAT_MIN_VOLT_OPEN	186 			//(0.3/3.3)*4096
#define BAT_MAX_VOLT_OPEN	1055		//(1.7/3.3)*4096


#define BAT_MAX_VOLT_CLOSE 1117		//(1.8/3.3)*4096	


#define BAT_LEVEL_LOW_TO_MIDD		868			// 1.4     (1.4/3.3)*4096
#define BAT_LEVEL_MIDD_TO_HIGH		875				//  1.41	(1.41/3.3)*4096

#define CHARGING_PRE_END_VOLT		558			//(0.9/3.3)*4096
#define CHARGING_FAST_END_VOLT	900			//(1.45/3.3)*4096
#define CHARGING_FAST_MAX_VOLT	993			//(1.6/3.3)*4096

#define CHARGING_FAST_TEMP_END_VOLT	900			// (1.45/3.3)*4096
#endif

#define BAT_LEVEL_LOW	1
#define BAT_LEVEL_MIDD	2
#define BAT_LEVEL_HIGH	3


#define BAT_CHARGING_PRE_MAX_COUNT	3000	// 10min   (10*60*1000)/200
#define BAT_START_DV_COUNT	750                     // 2.5min (2.5*60*1000/500)
//#define BAT_CHARGING_FAST_MAX_COUNT		16500	// 55min (55min*60*1000/500)
//#define BAT_CHARGING_FAST_MAX_COUNT_AAA	15000	// 50min   (50*60*1000/500)

#define BAT_CHARGING_FAST_MAX_COUNT		23100	// 77min (77min*60*1000/500)    2700/2100
#define BAT_CHARGING_FAST_MAX_COUNT_AAA	15900	// 53min   (53*60*1000/500)   800/900

#define BAT_INITIAL_FULL_CHECK_COUNT		600		// 2min  (90*1000/500)
#define BAT_NEAR_FULL_MAX_COUNT			900		// 3min (3*60*1000/500)


#define TRICKLE_CHARGE_COUNT		9000	// 30min  (30*60*1000/200   )

#define FAST_SKIP_COUNT	0
#define SUP_SKIP_COUNT		0
#define PRE_SKIP_COUNT		0
#define FULL_SKIP_COUNT		5
#define SPEC_SKIP_COUNT	5
#define DUMMY_SKIP_COPUNT	0xFF

#define CHANNEL_VBAT_1	10
#define CHANNEL_VBAT_2 11
#define CHANNEL_VBAT_3	8
#define CHANNEL_VBAT_4	13


#define CHANNEL_TEMP_1		12
#define CHANNEL_TEMP_2		1

#define CHANNEL_20_RES	9
#define CHANNEL_30_RES 7

#define CHANNEL_VIN_5V	3

#define BAT_VALID_VALUE	787

#define SHOW_CHARGING_TICK	0x40
#define SHOW_OUTPUT_TICK		610		//10s  10*1000/16.384

#define LED_INITIAL_DISPLAY		61		// 1s 10000/16.384
#define LED_INITIAL_DISPLAY_END	120	// 2s 
#define LED_DISPLAY_ON			30		//500ms
#define LED_DISPLAY_OFF		120		// 2s

#define LED_DISPLAY_INTERVAL	122		// 2s 2*1000/16.384
#define MIN_BAT_LEVEL_CHANGE   54931              // 15min  15*60*1000/16.384 
#define MIN_BAT_LEVEL_CHANGE_OUTPUT	29296			  // 8min   8*60*1000/16.384

//output
#define OUTPUT_STATUS_WAIT	0
#define OUTPUT_STATUS_NORMAL	1
#define OUTPUT_STATUS_STOP	2

#define OUTPUT_CHECK_INTERVAL		31	//(500/16.384	31)	


#define ENABLE_BOOST()	P25=1
#define DISABLE_BOOST()	P25=0

void ledHandler(void);
void delay_us(u16);

u16 getAdcValue(u8 channel);	
u16 getVbatAdc(u8 channel);
u16 getAverage(u8 channel);
u16 getBatTemp(u8 batNum);
u16 getVin5V(void);

void send(u16 sData);
void sendStr(char str[]);
void sendF(u32 sData);

void LED_OFF(u8 led);
void LED_ON(u8 led);

void delay_ms(u16 nus);

void setCurrent(u8 level);
#endif
