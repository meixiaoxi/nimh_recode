#include "MC96F8316.h"
#include "nimh.h"

u8 gPreChargingBatPos = BT_NULL;
u32 shortTick=0;

u8 gSysStatus;
u8 gOutputStatus;

u8  isFromOutput = 0;
				
u8 gBatStateBuf[4] = {0,0,0,0};

u16 gBatVoltArray[4]={0,0,0,0};

u8 gChargeSkipCount[] = {0,0,0,0};

u16 preVoltData[4] ={0,0,0,0};

u8 gErrorCount[4] = {0,0,0,0};

u16 gPreOutPutCurrent = 0;

u8 skipCount = 0;
u16 isPwmOn = 0;

u8 dropCount[4] = {0,0,0,0};	
u8 fitCount[4] = {0,0,0,0};

u8  gSmallModeCount[4] = {0,0,0,0};
u32 idata gNearFullTimeTick[4] = {0,0,0,0};
u32 idata gChargingTimeTick[4] ={0,0,0,0};
//u32 idata gLastChangeLevelTick[4]= {0,0,0,0};
//u8   idata gIsFisrtChangeLevel[4] = {0,0,0,0};

u8 idata gCurrentLevel[2] = {CURRENT_LEVEL_3,CURRENT_LEVEL_3};
u8 idata gCurrentNow = 1;
u8 idata gIsInTwoState= 0;
u8 idata gNowTwoBuf[2];
u8 gPreCurrent = 1;



u8 RestTime[4] = {0,0,0,0};
u8 gIsChargingBatPos=BT_NULL;
u8 gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
u8 gChargeChildStatus[4] = {0,0,0,0};
u16 gChargeCurrent_2;
u16 gChargeCurrent;
u8 gBatType[4] = {0,0,0,0};
u8 gDelayCount =0;
u8 gChargeCount =0;
u8 gDetectRemovePos=0;
u8 gHasBat = 0;
u8 gIsInTempProtect[4] = {0,0,0,0};
u8 tryUpDnCurrent[2] = {UP_CURRENT,UP_CURRENT};
u8 tryCount =0;

extern u8 ledDisplayCount;
extern u8 gLedStatus;
void FindTwoBattery()
{
	u8 batNum,fitNum = 0;
	u8 temp;
	if(gIsInTwoState == 0)
	{
		for(batNum=BT_1; batNum<=BT_4;batNum++)
		{
			if(gBatStateBuf[batNum] == STATE_NORMAL_CHARGING)// CHARGE_STATE_ERROR | BAT_TYPE_ERROR | CHARGE_STATE_FULL
			{
				if(gChargingTimeTick[batNum] < BAT_START_DV_COUNT)
				{
					gIsInTwoState = 0;
					return;
				}
				if(gBatVoltArray[batNum]>= BAT_START_FAST_CHARGE)
				{
				if(fitNum == 2)
				{
					if(gBatVoltArray[batNum] > gBatVoltArray[gNowTwoBuf[1]])	
					{
						gNowTwoBuf[0] = gNowTwoBuf[1];
						gNowTwoBuf[1] = batNum;
					}
					else if(gBatVoltArray[batNum] > gBatVoltArray[gNowTwoBuf[0]])
					{
						gNowTwoBuf[0] = batNum;
					}
				}
				else
				{
					if(fitNum)
					{
						if(gBatVoltArray[batNum] < gBatVoltArray[gNowTwoBuf[0]])
						{
							temp = gNowTwoBuf[0];
							gNowTwoBuf[0] = batNum;
							gNowTwoBuf[1] = temp;
						}
						else
							gNowTwoBuf[1] = batNum;
					}
					else
						gNowTwoBuf[0] = batNum;
					fitNum++;
				}
				}
			}
		}
	}

	temp = BT_NULL;
	
	if(fitNum)
	{
		if(fitNum == 1)
		{
			for(batNum=BT_1; batNum<=BT_4;batNum++)
			{
				if(gNowTwoBuf[0] == batNum)
				{	
					if(batNum == BT_4 && fitNum == 1)
					{
						if(temp != BT_NULL)
							gNowTwoBuf[1] = temp;
						else
							gNowTwoBuf[1] = BT_3;
					}
					continue;
				}
				if(gBatStateBuf[batNum] == STATE_NORMAL_CHARGING)// CHARGE_STATE_ERROR | BAT_TYPE_ERROR | CHARGE_STATE_FULL
				{
					if(fitNum == 1)
					{
						gNowTwoBuf[1] = batNum;
						fitNum =2;
					}
					else
					{
						if(gBatVoltArray[batNum] > gBatVoltArray[gNowTwoBuf[1]])
							gNowTwoBuf[1] = batNum;
					}
				}
				if(gBatStateBuf[batNum] == STATE_BATTERY_FULL)
				{
					temp = batNum;
				}
			}
		}
		gIsInTwoState =1;
	}
}
#if 1
void outputHandler()
{
	u8 cur_detect_pos,isVbatOk = 1;
	u16 temp_min;
do
{
	//if(gOutputStatus != OUTPUT_STATUS_STOP)
	{
		gDelayCount++;
		if(gDelayCount>=5)
		{
			gDelayCount=0;
			gErrorCount[0] = 0;
		//	if(gOutputStatus == OUTPUT_STATUS_WAIT)
		//		gBatStateBuf[0] = 1;
			if(gOutputStatus == OUTPUT_STATUS_NORMAL)
			{
				#if 0
				gChargeCurrent_2 = getAverage(CHANNEL_20_RES);  // 放电电流

				temp_min = MIN_VBAT_OUPUT - (3*gChargeCurrent_2) ;
				if(temp_min < 289 || gChargingTimeTick[0] < 900)  //0.7   
					temp_min = 289;
				#endif
				temp_min = getAverage(CHANNEL_20_RES);

				gErrorCount[2]++;   // 采样次数
				
				if(gErrorCount[0])  // 第一次
				{
					if(temp_min < gPreOutPutCurrent)
					{
						if((gPreOutPutCurrent - temp_min) > 10)
							gErrorCount[1]++;
					}
					else
					{
						if((temp_min -gPreOutPutCurrent) > 10)
							gErrorCount[1]++;
					}
					if(gErrorCount[2] >= 0xFF)
					{
						if(gErrorCount[1] >= 200)   // NG 次数
						{
							DISABLE_BOOST();
							delay_ms(100);
							ENABLE_BOOST();
						}
						gErrorCount[2] = 0;
						gErrorCount[1] = 0;
						gErrorCount[0] = 0;
					}
					gPreOutPutCurrent = temp_min;
				}
				else
				{
					gPreOutPutCurrent = temp_min;
				}
				
				temp_min = MIN_VBAT_OUPUT;
			}
			else
				temp_min = MIN_VBAT_OUTPUT_IDLE;

			

			preVoltData[BT_1] = getAverage(CHANNEL_VBAT_1);
			preVoltData[BT_2] = getAverage(CHANNEL_VBAT_2);
			preVoltData[BT_3] = getAverage(CHANNEL_VBAT_3);
			preVoltData[BT_4] = getAverage(CHANNEL_VBAT_4);
			
			if(preVoltData[BT_1] < BAT_MIN_VOLT_OPEN || preVoltData[BT_2] < BAT_MIN_VOLT_OPEN || preVoltData[BT_3] < BAT_MIN_VOLT_OPEN || preVoltData[BT_4] < BAT_MIN_VOLT_OPEN)				   
			{
				if(gOutputStatus ==OUTPUT_STATUS_STOP)
				{
					gOutputStatus = OUTPUT_STATUS_WAIT;
				}
				else if(gOutputStatus == OUTPUT_STATUS_NORMAL)
					isVbatOk = 0;
				LED_OFF(BT_1);LED_OFF(BT_2);LED_OFF(BT_3);LED_OFF(BT_4);
				gBatStateBuf[0] = 1;
			}

			#ifdef EVT_BOARD
			preVoltData[BT_3] = preVoltData[BT_3] /3;
			preVoltData[BT_4] = preVoltData[BT_4] /3;
			#else
			preVoltData[BT_4] = preVoltData[BT_4] /3;
			#endif

			if(preVoltData[BT_1] < MIN_VBAT_1)   // 3.5V   10V
			{
				isVbatOk = 0;
			}
			for(cur_detect_pos = BT_1; cur_detect_pos <BT_4; cur_detect_pos++)
			{
				if(preVoltData[cur_detect_pos+1] > preVoltData[cur_detect_pos])
					gChargeCurrent = 0;
				else
					gChargeCurrent = preVoltData[cur_detect_pos] -preVoltData[cur_detect_pos+1];

				if(gOutputStatus == OUTPUT_STATUS_WAIT)
				{
					if(gChargeCurrent >= BAT_VOLT_ALK_MIN)
						gIsInTempProtect[cur_detect_pos]  = 1;  // alkline
				}
				#if 0
				else if(gOutputStatus == OUTPUT_STATUS_NORMAL)
				{
					if(gIsInTempProtect[cur_detect_pos] == 1)
						temp_min = MIN_VBAT_OUTPUT_ALK;
				}
				#endif
				if(gChargeCurrent < temp_min)
				{
					
					if(gOutputStatus == OUTPUT_STATUS_WAIT)
					{
						isVbatOk = 0;
						if(gChargeCurrent < MIN_OUTPUT_DISPLAY_VOLT)
							gBatStateBuf[0] = 1;
					}
					else if(gOutputStatus == OUTPUT_STATUS_NORMAL)
					{
						gChargeSkipCount[cur_detect_pos]++;
						if(gChargeSkipCount[cur_detect_pos] >=60)
							isVbatOk = 0;
					}
					else
					{
						isVbatOk = 0;
					}
				}
				else
					gChargeSkipCount[cur_detect_pos] = 0;
			}

			if(gOutputStatus == OUTPUT_STATUS_WAIT)
			{
				if(preVoltData[BT_4] >= BAT_VOLT_ALK_MIN)
						gIsInTempProtect[BT_4]  = 1;  // alkline
			}
			#if 0
			else if(gOutputStatus == OUTPUT_STATUS_NORMAL)
			{
				if(gIsInTempProtect[BT_4] == 1)
					temp_min = MIN_VBAT_OUTPUT_ALK;
			}
			#endif
			
			if(preVoltData[BT_4] < temp_min)
			{
				if(gOutputStatus == OUTPUT_STATUS_WAIT)
				{
					isVbatOk = 0;
					if(preVoltData[BT_4] < MIN_OUTPUT_DISPLAY_VOLT)
						gBatStateBuf[0] = 1;
				}
				else if(gOutputStatus == OUTPUT_STATUS_NORMAL)
				{
					gChargeSkipCount[BT_4]++;
					if(gChargeSkipCount[BT_4] >=60)
						isVbatOk = 0;
				}
				else
					isVbatOk = 0;
			}
			else
				gChargeSkipCount[BT_4] = 0;
			
			if(skipCount == 0)  //温度正常
			{
				temp_min = getBatTemp(BT_1);
				if(temp_min < ADC_TEMP_OUTPUT_MAX || temp_min > ADC_TEMP_OUTPUT_MIN)
				{
					skipCount = 1;
				}
				else
				{
					temp_min = getBatTemp(BT_3);
					if(temp_min < ADC_TEMP_OUTPUT_MAX || temp_min > ADC_TEMP_OUTPUT_MIN)
					{
						skipCount = 1;
					}
				}
			}
			else
			{
				temp_min = getBatTemp(BT_1);
				if(temp_min > ADC_TEMP_OUTPUT_MAX_RECOVERY&& temp_min < ADC_TEMP_OUTPUT_MIN_RECOVERY)
				{
					temp_min = getBatTemp(BT_3);
					if(temp_min > ADC_TEMP_OUTPUT_MAX_RECOVERY && temp_min < ADC_TEMP_OUTPUT_MIN_RECOVERY)
					{
						skipCount = 0;
					}
				}
			}

				if(isVbatOk== 1 && skipCount == 0)
				{
					if(gOutputStatus == OUTPUT_STATUS_WAIT)
					{
						gOutputStatus = OUTPUT_STATUS_NORMAL;
						gBatStateBuf[0] = 0;
						gBatStateBuf[1] = 0;
						gBatStateBuf[2] = 0;
						gBatStateBuf[3] = 0;
						gIsInTwoState = 0;
						ENABLE_BOOST();
						gChargingTimeTick[0] = 0;
						//updateBatLevel(gBatVoltArray[1][0],gCount+1);
					}
					gChargingTimeTick[0]++;
				}
				else
				{
					#if 0
					if(skipCount == 0 && gOutputStatus == OUTPUT_STATUS_NORMAL)   //  temperature ok
					{
						if(gChargingTimeTick[0] < 10)
						{
							gChargingTimeTick[0]++;
							return;
						}
					}
					#endif
					
					if(gOutputStatus == OUTPUT_STATUS_NORMAL)
						gOutputStatus = OUTPUT_STATUS_STOP;

					gIsInTempProtect[0] = 0;
					gIsInTempProtect[1] = 0;					
					gIsInTempProtect[2] = 0;
					gIsInTempProtect[3] = 0;
					
					//gChargingTimeTick[0] = 0;
					if(gBatStateBuf[0])
					{
						LED_OFF(BT_1),LED_OFF(BT_2),LED_OFF(BT_3),LED_OFF(BT_4);
					}
					DISABLE_BOOST();
				}				
		}		
	}
}while(0);
}
#endif
void PwmControl(unsigned char status)
{
	if(status == PWM_OFF)
	{	
		isPwmOn =0;

	switch(gIsChargingBatPos)
	{
		case 0:
			P30 =0;
			break;
		case 1:
			P31 =0;
			break;
		case 2:
			P33 =0;
			break;
		case 3:
			P23 =0;
			break;
		default:
			break;
	}
	}
	else
	{
	switch(gIsChargingBatPos)
	{
		case 0:
			P30 =1;
			break;
		case 1:
			P31 =1;
			break;
		case 2:
			P33 =1;
			break;
		case 3:
			P23 =1;
			break;
		default:
			break;
	}		
	}
}

void removeBat(u8 batNum)
{
	gChargeSkipCount[batNum] = 0;
	gChargingTimeTick[batNum] = 0;
	gBatStateBuf[batNum] =STATE_DEAD_BATTERY;
	dropCount[batNum] = 0;
	fitCount[batNum] = 0;
	preVoltData[batNum] = 0;
	gNearFullTimeTick[batNum] = 0;
	RestTime[batNum] = 0;
	LED_OFF(batNum);
	gBatType[batNum] = 0;
	gIsInTempProtect[batNum] = 0;
	gErrorCount[batNum] = 0;
	//PB &= 0xF0;   //close current pwm channel
	if(batNum == gIsChargingBatPos)
		PwmControl(PWM_OFF);

	if(batNum == gPreChargingBatPos)
		gPreChargingBatPos = BT_NULL;
	gIsInTwoState = 0;
}

void removeAllBat()
{
	u8 i;

	for(i=0;i<4;i++)
	{
	gChargeSkipCount[i] = 0;
	gChargingTimeTick[i] = 0;
	gBatStateBuf[i] =STATE_DEAD_BATTERY;
	dropCount[i] = 0;
	fitCount[i] = 0;
	preVoltData[i] = 0;
	gNearFullTimeTick[i] = 0;
	LED_OFF(i);
	RestTime[i] = 0;
	gBatType[i] = 0;
	gIsInTempProtect[i] = 0;
	gErrorCount[i] = 0;
	}

	gCurrentLevel[0] = CURRENT_LEVEL_3;
	gCurrentLevel[1] = CURRENT_LEVEL_3;
	tryUpDnCurrent[0] = UP_CURRENT;
	tryUpDnCurrent[1] = UP_CURRENT;

	setCurrent(CURRENT_LEVEL_1);
	
	//PWM
	P30 =0;
	P31 =0;
	P33 =0;
	P23 =0;

	isPwmOn =0; 
	
	gIsChargingBatPos = BT_NULL;
	gPreChargingBatPos = BT_NULL;
	gIsInTwoState = 0;
}
void StatusCheck()
{
		if(gSysStatus != (GET_SYS_STATUS()))
		{
			if(gSysStatus == SYS_CHARGING_STATE)
			{
				if(gSysStatus != (GET_SYS_STATUS()))
				{
					gSysStatus = SYS_DISCHARGE_STATE;
					CHANGE_TO_OUTPUT();
					gOutputStatus = OUTPUT_STATUS_WAIT;				
					//remove all bat
					removeAllBat();
					isFromOutput = 0;
					gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
					gDelayCount = 50;
					skipCount = 0;
					gHasBat = 0;
					gChargeCount = 0;
				}
			}
			else
			{
				DISABLE_BOOST();
				gSysStatus = SYS_CHARGING_STATE;
				CHANGE_TO_INPUT();
				isFromOutput = 1;
				removeAllBat();
				gDelayCount = 0;

				ledDisplayCount = 0;
				gLedStatus = 1;
				gChargeCount = 0;
			}
		}	
}




//void batRemoveHandler()
//{
//	if(gBatStateBuf[] != STATE_DEAD_BATTERY && gBatStateBuf[] != STATE_CHARGING_DEAD_ERROR)
//}



#define	BAT_AAA_TYPE	1
void StatusChange(u8 batNum, u8 statusNow)
{
	if(batNum == gPreChargingBatPos)
		gPreChargingBatPos = BT_NULL;
	if(statusNow == STATE_DEAD_BATTERY)
	{
		removeBat(batNum);
	}
	else		
		gBatStateBuf[batNum] = statusNow;

	gIsInTwoState = 0;
}

#ifdef DVT_BOARD
void batTypeDetect()
{
	u8 i;
	u16 temp_1,temp_2,max_1,max_2,min_1,min_2;

	temp_1 = getAdcValue(CHANNEL_20_RES);
	temp_2 = getAdcValue(CHANNEL_30_RES);
	gChargeCurrent = temp_1;
	gChargeCurrent_2 = temp_2;
	max_1 =temp_1;
	min_1 = temp_1;
	max_2 = temp_2;
	min_2 = temp_2;
	for(i=0;i<9;i++)
	{
		//delay_us(100);
		ClrWdt();
		 temp_1 = getAdcValue(CHANNEL_20_RES);
		 temp_2 = getAdcValue(CHANNEL_30_RES);
	 	if(temp_1 > max_1)
	 	{
			max_1 = temp_1;
	 	}

		 if(temp_1 < min_1)
		{
			min_1 = temp_1;
	 	}
		if(temp_2 > max_2)
	 	{
			max_2 = temp_2;
	 	}

		 if(temp_2< min_2)
		{
			min_2 = temp_2;
	 	}
	 	gChargeCurrent += temp_1;
		gChargeCurrent_2 += temp_2;
	}
	gChargeCurrent = (gChargeCurrent-max_1-min_1) >>3;
	gChargeCurrent_2 = (gChargeCurrent_2-max_2-min_2)>>3;
}
#endif

void batteryDetect(u8 batNum,u16 tempV)
{
	#ifdef DVT_BOARD
	batTypeDetect();
	if(gChargeCurrent_2 > gChargeCurrent)
	{
		if((gChargeCurrent_2-gChargeCurrent) > 7)
			gBatType[gIsChargingBatPos]= BAT_AAA_TYPE;
	}
	#endif
	if(batNum == BT_4|| gBatType[batNum] == BAT_AAA_TYPE)
		isPwmOn = BAT_MAX_VOLT_CLOSE_CHANNEL_4;
	else
		isPwmOn = BAT_MAX_VOLT_CLOSE;
					
	if(tempV>isPwmOn || gChargeCurrent_2 <3  )
	{
		StatusChange(batNum, STATE_BATTERY_TYPE_ERROR);
		PwmControl(PWM_OFF);
		return;
	}

	PwmControl(PWM_OFF);
	
}

void PreCharge(u8 batNum)
{
	u16 tempT;

		gChargingTimeTick[batNum]++;
		
		if(gChargingTimeTick[batNum] > BAT_CHARGING_PRE_MAX_COUNT)
		{
			StatusChange(batNum,STATE_BATTERY_TYPE_ERROR);
		}
		else
		{ 
			gBatVoltArray[batNum] = getVbatAdc(batNum);
			if(gBatVoltArray[batNum] >= CHARGING_PRE_END_VOLT )
			{				
				gChargingTimeTick[batNum] = 0;
				StatusChange(batNum,STATE_NORMAL_CHARGING);
				gChargeChildStatus[batNum] = CHARGE_STATE_FAST;
			}
			else
			{
				tempT = getBatTemp(batNum);
				if(tempT < ADC_TEMP_MAX || tempT > ADC_TEMP_MIN)
				{
					StatusChange(batNum,STATE_BATTERY_TEMPERATURE_ERROR);
					gChargingTimeTick[batNum] = 0;
				}
			}
				
		}
}

void FastCharge(u8 batNum)
{
	u16 tempV,tempT;
	u32 overTimer = BAT_CHARGING_FAST_MAX_COUNT;

	tempV = getVbatAdc(batNum);

	if(gBatType[batNum] != BAT_AAA_TYPE)
	{
	if(gPreCurrent== CURRENT_LEVEL_1)	
	{
		gChargingTimeTick[batNum]++;
		if(tempV > BAT_VOLT_NEAR_FULL)
			gNearFullTimeTick[batNum]++;
		else if(gNearFullTimeTick[batNum] < 50)
			gNearFullTimeTick[batNum] = 0;
	}
	else if(gPreCurrent== CURRENT_LEVEL_2)
	{
		gSmallModeCount[batNum]++;
		if(gSmallModeCount[batNum] >= 7)
		{
			gChargingTimeTick[batNum] = gChargingTimeTick[batNum]+3;
			gSmallModeCount[batNum] = 0;
			if(tempV > BAT_VOLT_NEAR_FULL)
				gNearFullTimeTick[batNum] = gNearFullTimeTick[batNum] + 3;
			else if(gNearFullTimeTick[batNum] < 50)
				gNearFullTimeTick[batNum] = 0;
		}
	}
	else
	{
		gSmallModeCount[batNum]++;
		if(gSmallModeCount[batNum] >= 21)
		{
			gChargingTimeTick[batNum] = gChargingTimeTick[batNum]+4;
			gSmallModeCount[batNum] = 0;
			if(tempV > BAT_VOLT_NEAR_FULL)
				gNearFullTimeTick[batNum] = gNearFullTimeTick[batNum] + 4;
			else if(gNearFullTimeTick[batNum] < 50)
				gNearFullTimeTick[batNum] = 0;
		}
	}
	}
	else
	{
	if(gPreCurrent== CURRENT_LEVEL_1)	
	{
		gChargingTimeTick[batNum]++;
		if(tempV > BAT_VOLT_NEAR_FULL)
			gNearFullTimeTick[batNum]++;
		else if(gNearFullTimeTick[batNum] < 50)
			gNearFullTimeTick[batNum] = 0;
	}
	else if(gPreCurrent== CURRENT_LEVEL_2)
	{
		gSmallModeCount[batNum]++;
		if(gSmallModeCount[batNum] >= 29)
		{
			gChargingTimeTick[batNum] = gChargingTimeTick[batNum]+12;
			gSmallModeCount[batNum] = 0;
			if(tempV > BAT_VOLT_NEAR_FULL)
				gNearFullTimeTick[batNum] = gNearFullTimeTick[batNum] + 12;
			else if(gNearFullTimeTick[batNum] < 50)
				gNearFullTimeTick[batNum] = 0;
		}
	}
	else
	{
		gSmallModeCount[batNum]++;
		if(gSmallModeCount[batNum] >= 29)
		{
			gChargingTimeTick[batNum] = gChargingTimeTick[batNum]+5;
			gSmallModeCount[batNum] = 0;
			if(tempV > BAT_VOLT_NEAR_FULL)
				gNearFullTimeTick[batNum] = gNearFullTimeTick[batNum] + 5;
			else if(gNearFullTimeTick[batNum] < 50)
				gNearFullTimeTick[batNum] = 0;
		}
	}		
	}
		
	tempT = getBatTemp(batNum);

	if(gChargingTimeTick[batNum] > BAT_START_DV_COUNT)  //hod-off time, in this period, we do NOT detect -dv
	{
			if(gBatType[batNum] == BAT_AAA_TYPE)
				overTimer = BAT_CHARGING_FAST_MAX_COUNT_AAA;
			if(gChargingTimeTick[batNum] > overTimer || gNearFullTimeTick[batNum] > BAT_NEAR_FULL_MAX_COUNT)
			{
					StatusChange(batNum,STATE_BATTERY_FULL);	
					gChargingTimeTick[batNum] = 0;
			}

			if(tempV > gBatVoltArray[batNum])
			{
				gBatVoltArray[batNum] = tempV;
				dropCount[batNum] = 0;
			}
			else
			{
				if((gBatVoltArray[batNum] - tempV) > ADC_DV_VOLT)
				{
					if(tempV < DV_ENABLE_MIN_VOLT)
						return;
					dropCount[batNum]++;
					if(dropCount[batNum] >3)
					{
						StatusChange(batNum,STATE_BATTERY_FULL);
						gChargingTimeTick[batNum] = 0;
						return;
					}
				}
			}
			 if(tempT < ADC_TEMP_MAX || tempT > ADC_TEMP_MIN)
			 {
			 	StatusChange(batNum, STATE_BATTERY_TEMPERATURE_ERROR);
			 }
			 else if(tempT < ADC_TEMP_DOWN_CURRENT)
			 {
			 	gIsInTempProtect[batNum] = 1;
			 }
			 if(gIsInTempProtect[batNum])
			 {
			 	if(tempT > ADC_TEMP_MAX_RECOVERY)
					gIsInTempProtect[batNum] = 0;
					
			 }
	}
	else
	{
		gBatVoltArray[batNum] = tempV;

		if(tempV > BAT_INITIAL_VOLT_FULL && gChargingTimeTick[batNum] > BAT_INITIAL_FULL_CHECK_COUNT)
		{
			StatusChange(batNum,STATE_BATTERY_FULL);	
			gChargingTimeTick[batNum] = 0;
		}
		
		if(tempT < ADC_TEMP_MAX || tempT > ADC_TEMP_MIN )
		{
			StatusChange(batNum,STATE_BATTERY_TEMPERATURE_ERROR);
		}
	}

}


//CUR_CTL	P32
//CUR_CTL2	P36
// 0 input  1 output
void setCurrent(u8 level)
{
	if(gCurrentNow == level)
		return;
	
	if(level == CURRENT_LEVEL_1) //输入高阻     2A 900mA
	{
		P3IO &= 0xBB;  //输入
	}
	else if(level == CURRENT_LEVEL_2) //CUR_CTL输出低 CUR_CTL2高阻  900mA  360mA
	{
		P3IO |= 0x04;  //cur_ctl输出
		P32 = 0;
		
		P3IO &= 0xBF; //cur_ctl2 输入
	}
	else if(level == CURRENT_LEVEL_3)  //  200mA   90mA 
	{
		P3IO &= 0xFB;   //cur_ctl输入	

		P3IO |= 0x40;
		P36 = 0;
	}
	gCurrentNow = level;
}

u8 EnterTrickleCharge()
{
	u8 batNum,fitCount = 0;;

	if(gChargingTimeTick[gIsChargingBatPos] > TRICKLE_CHARGE_COUNT)
		return 0;

	if(gIsInTwoState)
	{
		if(gNowTwoBuf[0] == gIsChargingBatPos || gNowTwoBuf[1] == gIsChargingBatPos)
			return 1;
		else
			return 0;
	}

	for(batNum = BT_1; batNum <= BT_4; batNum++)
	{
		if(batNum == gIsChargingBatPos)
			continue;
		if(gBatStateBuf[batNum] == STATE_NORMAL_CHARGING)
		{
			fitCount++;
			if(fitCount > 1)
				return 0;
		}
	}

	return 1;
}
void PreChargeBatHandler()
{
	if(RestTime[gPreChargingBatPos] > MIN_DETECT_PRE_BATTERY && isPwmOn == 0)
	{
		switch(gChargeChildStatus[gPreChargingBatPos])
		{
			case CHARGE_STATE_FAST:
				FastCharge(gPreChargingBatPos);break;
			case CHARGE_STATE_PRE:
				PreCharge(gPreChargingBatPos);break;
			default:
				break;
		}
		gPreChargingBatPos = BT_NULL;
	}	
}
void chargeHandler(void)
{
	u16 tempT,tempV,temp_2;
	u8 battery_state = gBatStateBuf[gIsChargingBatPos];
	static u8 chargingTime = 0;
	u8 chargeCurrent = 0,temp_3;

	if(gChargingStatus == SYS_CHARGE_WAIT_TO_PICK_BATTERY)
	{
		if(battery_state == STATE_DEAD_BATTERY)
		{
			chargingTime = CHARGING_TIME_10MS;
			chargeCurrent = CURRENT_LEVEL_2;
		}
		else if(battery_state == STATE_BATTERY_DETECT)
		{
			chargingTime = CHARGING_TIME_200MS;
			chargeCurrent = CURRENT_LEVEL_1;
		}
		else if(battery_state == STATE_NORMAL_CHARGING)
		{
			chargingTime = CHARGING_TIME_200MS;
			if(gBatType[gIsChargingBatPos] == BAT_AAA_TYPE)
				temp_3 = 0;
			else
				temp_3 = 1;
			if(gCurrentLevel[temp_3] == CURRENT_LEVEL_1)
				chargeCurrent = CURRENT_LEVEL_1;
			else if(gCurrentLevel[temp_3]  == CURRENT_LEVEL_2)
				chargeCurrent = CURRENT_LEVEL_2;
			else
					chargeCurrent = CURRENT_LEVEL_3;
			if(gIsInTempProtect[gIsChargingBatPos])
			{
				if(chargeCurrent != CURRENT_LEVEL_3)
					chargeCurrent++;
			}
			if(gChargeChildStatus[gIsChargingBatPos] == CHARGE_STATE_PRE)
				chargeCurrent = CURRENT_LEVEL_3;
		}
		else if(battery_state == STATE_BATTERY_TYPE_ERROR)
		{
			if(gIsChargingBatPos == BT_1)
				chargingTime = CHARGING_TIME_10MS;
			else
				chargingTime = CHARGING_TIME_0MS;
			chargeCurrent = CURRENT_LEVEL_3;
		}
		else if(battery_state == STATE_BATTERY_TEMPERATURE_ERROR)
		{
			if(gIsChargingBatPos == BT_1)
				chargingTime = CHARGING_TIME_10MS;
			else
				chargingTime = CHARGING_TIME_0MS;
			chargeCurrent = CURRENT_LEVEL_3;
		}
		else if(battery_state == STATE_BATTERY_FULL)
		{
			if(EnterTrickleCharge())
			{
				chargingTime = CHARGING_TIME_200MS;
			}
			else if(gIsChargingBatPos == BT_1)
			{
				chargingTime = CHARGING_TIME_10MS;
			}
			else
				chargingTime = CHARGING_TIME_0MS;
			chargeCurrent = CURRENT_LEVEL_3;
		}
		else
			chargingTime =0;

		if(chargingTime != 0)
		{
			if(battery_state == STATE_DEAD_BATTERY)
			{
				gHasBat = 0;
				tempV = getVbatAdc(gIsChargingBatPos);

				if(tempV >= BAT_MAX_VOLT_OPEN && gIsChargingBatPos != BT_1)
				{
					StatusChange(gIsChargingBatPos, STATE_BATTERY_TYPE_ERROR);
					return;
				}
				else if(tempV >= BAT_HAS_BATTERY_VOLT)
				{
					gHasBat = 1;
				}
				if(gIsChargingBatPos == BT_1)
					gHasBat = 0;
			}
			isPwmOn = 1;
			#ifdef DVT_BOARD
			setCurrent(chargeCurrent);
			#endif
			PwmControl(PWM_ON);
			gChargingStatus = SYS_CHARGE_IS_CHARGING;
			gDelayCount =0;
			gChargeCount =0;
		}
		else
		{
			tempV = getVbatAdc(gIsChargingBatPos);
			if(battery_state == STATE_BATTERY_TEMPERATURE_ERROR)
			{
				tempT = getBatTemp(gIsChargingBatPos);
				if(tempV < BAT_MIN_VOLT_OPEN)
					StatusChange(gIsChargingBatPos,STATE_DEAD_BATTERY);
				else if(tempT > ADC_TEMP_MAX_RECOVERY && tempT < ADC_TEMP_MIN_RECOVERY)
					StatusChange(gIsChargingBatPos,STATE_NORMAL_CHARGING);
			}
			else if(battery_state == STATE_BATTERY_TYPE_ERROR)
			{
				if(tempV < BAT_MIN_VOLT_OPEN)
					StatusChange(gIsChargingBatPos,STATE_DEAD_BATTERY);
			}
		}
	}
	else if(gChargingStatus == SYS_CHARGE_IS_CHARGING)
	{
		if(chargingTime != 0)
		{
			gChargeCount++;
			if(gChargeCount >= chargingTime)  //充电时间到
			{
				if(battery_state == STATE_DEAD_BATTERY)
				{	
					tempV = getVbatAdc(gIsChargingBatPos);
					PwmControl(PWM_OFF);
					if(tempV < BAT_ZERO_SPEC_VOLT)
						StatusChange(gIsChargingBatPos,STATE_BATTERY_DETECT);
					else if(gHasBat)
					{
						StatusChange(gIsChargingBatPos,STATE_BATTERY_TYPE_ERROR);
					}
					gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
				}
				else if(battery_state == STATE_BATTERY_DETECT)
				{
					if(isPwmOn)
					{
						tempV = getVbatAdc(gIsChargingBatPos);
						batteryDetect(gIsChargingBatPos,tempV);
						if(gBatStateBuf[gIsChargingBatPos] == STATE_BATTERY_TYPE_ERROR)
						{
							gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
						}
						gDelayCount= 0;
					}
					else
					{
						gDelayCount++;
						if(gDelayCount >=3)
						{
							tempV = getVbatAdc(gIsChargingBatPos);
							tempT = getBatTemp(gIsChargingBatPos);
							gBatVoltArray[gIsChargingBatPos] = tempV;
							StatusChange(gIsChargingBatPos, STATE_NORMAL_CHARGING);
							gCurrentLevel[0] = CURRENT_LEVEL_3;
							gCurrentLevel[1] = CURRENT_LEVEL_3;
							tryUpDnCurrent[0] = UP_CURRENT;
							tryUpDnCurrent[1] = UP_CURRENT;
							tryCount =0;
							if(tempV< CHARGING_PRE_END_VOLT )
								gChargeChildStatus[gIsChargingBatPos] =  CHARGE_STATE_PRE;
							else
								gChargeChildStatus[gIsChargingBatPos] = CHARGE_STATE_FAST;

							if(tempT < ADC_TEMP_MAX || tempT > ADC_TEMP_MIN)
							{
								StatusChange( gIsChargingBatPos,STATE_BATTERY_TEMPERATURE_ERROR);
							}
							gChargingTimeTick[gIsChargingBatPos] = 0;
							gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
						}
					}
						
				}
				else if(battery_state == STATE_NORMAL_CHARGING)
				{
					tempV = getVbatAdc(gIsChargingBatPos);
					if(tempV > BAT_ZERO_SPEC_VOLT)
					{
						StatusChange(gIsChargingBatPos, STATE_DEAD_BATTERY);
						PwmControl(PWM_OFF);
						gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
						return;
					}
					
					if(gDelayCount == 0)
					{
						if(gIsChargingBatPos == BT_4 || gBatType[gIsChargingBatPos] == BAT_AAA_TYPE)
						{
						#ifdef DVT_BOARD
							if(gCurrentNow== CURRENT_LEVEL_1)
								temp_2 = BAT_MAX_VOLT_CLOSE_CHANNEL_4;
							else if(gCurrentNow == CURRENT_LEVEL_2)
								temp_2 = BAT_MAX_CLOSE_CHANNEL_4_LEVEL_2;
							else 
								temp_2 = BAT_MAX_CLOSE_CHANNEL_4_LEVEL_3;
						#else
							temp_2 = BAT_MAX_VOLT_CLOSE_CHANNEL_4;
						#endif	
						}
						else
						{
						#ifdef DVT_BOARD
							if(gCurrentNow == CURRENT_LEVEL_1)
								temp_2 = BAT_MAX_VOLT_CLOSE;
							else if(gCurrentNow == CURRENT_LEVEL_2)
								temp_2 = BAT_MAX_CLOS_LEVEL_2;
							else
								temp_2 = BAT_MAX_CLOS_LEVEL_3;
						#else
							temp_2 = BAT_MAX_VOLT_CLOSE;
						#endif	
						}
						if(tempV > temp_2)
						{
							gErrorCount[gIsChargingBatPos]++;
							if(gErrorCount[gIsChargingBatPos] >=3)
							{
								PwmControl(PWM_OFF);
								StatusChange(gIsChargingBatPos,STATE_BATTERY_TYPE_ERROR);
								gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
								return;
							}
						}
						#ifdef DVT_BOARD
						if(gBatType[gIsChargingBatPos] == BAT_AAA_TYPE)
							temp_3 = 0;
						else
							temp_3 = 1;
							
						temp_2 = getAverage(CHANNEL_VIN_5V);
						if(temp_2 < VIN_5V_MINUM)
						{
							temp_2 = getAverage(CHANNEL_VIN_5V);
							if(temp_2 < VIN_5V_MINUM)
							{
								tryCount++;
								//if(tryCount > 7)
								{
									if(gCurrentLevel[temp_3] < CURRENT_LEVEL_3)
										gCurrentLevel[temp_3]++;
									tryUpDnCurrent[temp_3] = DN_CURRENT;
									tryCount = 0;
								}
							}
						}
						else
						{
							if(tryUpDnCurrent[temp_3] == UP_CURRENT)
							{
								if(gCurrentLevel[temp_3] > CURRENT_LEVEL_1)
								{
									gCurrentLevel[temp_3]--;
								}
							}
							tryCount = 0;
						}
						#endif
					}
					PwmControl(PWM_OFF);
					if(gPreChargingBatPos != BT_NULL)  //wait for Pre battery detect finish
					{
						gDelayCount = 1;
						return;
					}
					
					gPreChargingBatPos = gIsChargingBatPos;
					gPreCurrent = gCurrentNow;
					gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
				}
				else if(battery_state == STATE_BATTERY_TEMPERATURE_ERROR)   //only for BT_1
				{
					tempV = getVbatAdc(gIsChargingBatPos);
					tempT = getBatTemp(gIsChargingBatPos);
					PwmControl(PWM_OFF);
					if(tempV > BAT_ZERO_SPEC_VOLT)
					{
						StatusChange(gIsChargingBatPos, STATE_DEAD_BATTERY);
					}
					else if(tempT > ADC_TEMP_MAX_RECOVERY && tempT < ADC_TEMP_MIN_RECOVERY)
						StatusChange(gIsChargingBatPos,STATE_NORMAL_CHARGING);
					gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
				}
				else if(battery_state == STATE_BATTERY_TYPE_ERROR)
				{
					tempV = getVbatAdc(gIsChargingBatPos);
					PwmControl(PWM_OFF);
					if(tempV > BAT_ZERO_SPEC_VOLT)
					{
						StatusChange(gIsChargingBatPos, STATE_DEAD_BATTERY);
					}
					gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
				}
				else if(battery_state == STATE_BATTERY_FULL)
				{
					if(chargingTime == CHARGING_TIME_10MS)   // only for BT_1
					{
						tempV = getVbatAdc(BT_1);
						PwmControl(PWM_OFF);
						if(tempV > BAT_ZERO_SPEC_VOLT)
						{
							StatusChange(gIsChargingBatPos, STATE_DEAD_BATTERY);
						}
						PwmControl(PWM_OFF);
						gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
					}
					else
					{
						tempV = getVbatAdc(gIsChargingBatPos);
						PwmControl(PWM_OFF);
						if(tempV > BAT_ZERO_SPEC_VOLT)
						{
							StatusChange(gIsChargingBatPos, STATE_DEAD_BATTERY);
							gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
							return;
						}
						gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
						gChargingTimeTick[gIsChargingBatPos]++;
					}
				}
				else
				{
					PwmControl(PWM_OFF);
					gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
				}
			}
		}
	}
}

void addRestTime()
{
u8 batNum;	
			for(batNum = BT_1; batNum<=BT_4; batNum++)
			{
				if(((batNum == gIsChargingBatPos) && (isPwmOn == 1)) || gBatStateBuf[batNum] == STATE_DEAD_BATTERY)
					continue;
				
				RestTime[batNum]++;
			}
}

void PickBattery()
{
		FindTwoBattery();

		toNextBattery();

		if(gBatStateBuf[gIsChargingBatPos] == STATE_NORMAL_CHARGING || gBatStateBuf[gIsChargingBatPos] == STATE_BATTERY_FULL)
		{
			if(gIsInTwoState)
			{
				while(gNowTwoBuf[0] != gIsChargingBatPos && gNowTwoBuf[1] != gIsChargingBatPos)
				{
					if(gBatStateBuf[gIsChargingBatPos] != STATE_NORMAL_CHARGING  &&	gBatStateBuf[gIsChargingBatPos] != STATE_BATTERY_FULL)
						return;
					if(gIsChargingBatPos == BT_1)  //we can't detect whether BT_1 exist by zero volt 
						break;
					toNextBattery();
				}
			}
			
			if(RestTime[gIsChargingBatPos] >= 18)
				RestTime[gIsChargingBatPos] = 0;
			else
				toNextBattery();
		}
}



void btRemove()
{
	u16 tempV;
	u8 batNum;

	for(batNum = BT_1; batNum<= BT_4; batNum++)
	{
		if(gBatStateBuf[batNum] != STATE_DEAD_BATTERY)
		{	
			tempV = getVbatAdc(batNum);

			if(tempV < BAT_MIN_VOLT_OPEN_SPE || tempV > BAT_ZERO_SPEC_VOLT) //0 or BAT_ZERO_SPEC_VOLT for the charing battery
			{
				StatusChange(batNum,STATE_DEAD_BATTERY);
				if(batNum == gIsChargingBatPos)
				{
					gChargingStatus = SYS_CHARGE_WAIT_TO_PICK_BATTERY;
				}
			}
		}
	}

}



//工厂测试
void factoryTest()
{
	u8 testlevel = 1;
	gDetectRemovePos = 1;

	gSysStatus = GET_SYS_STATUS();
	while(gSysStatus != SYS_CHARGING_STATE)
	{
		gSysStatus = GET_SYS_STATUS();
		ClrWdt();
	}
	
do{
	gBatVoltArray[0] = getAverage(CHANNEL_VIN_5V);
	if(gBatVoltArray[0] > VIN_5V_TEST_MAX || gBatVoltArray[0] < VIN_5V_TEST_MIN)
		break;
	
	gDetectRemovePos++;

	gBatVoltArray[0] = getBatTemp(BT_1);
	if(gBatVoltArray[0] > 2172  || gBatVoltArray[0]  < 1923)  // 1.65 +/- 0.1V
		break;
	gDetectRemovePos++;
	gBatVoltArray[0] = getBatTemp(BT_3);
	if(gBatVoltArray[0] > 2172  || gBatVoltArray[0]  < 1923)
		break;
	gDetectRemovePos++;

// 2.8V ok?
	gIsChargingBatPos = BT_1;
	P0IO |= 0x04;  //set chg_dischg to output
	P02 = 1;   //output 0 to lwo mos
	PwmControl(PWM_ON);
	delay_ms(10);

	gBatVoltArray[0] = getVbatAdc(BT_1);
	PwmControl(PWM_OFF);
	delay_ms(10);
	
	P0IO &= 0xFB;  //set chg_dischg to input
	gIsChargingBatPos = 1;
	
	if(gBatVoltArray[0] > TEST_VOLT_YUNFANG_MAX|| gBatVoltArray[0] < TEST_VOLT_YUNFANG_MIN)  // 2.8 +/- 0.2
		break;
	gDetectRemovePos++;
	
	for(gIsChargingBatPos=BT_1; gIsChargingBatPos <= BT_4; gIsChargingBatPos++)
	{
		if(GET_FACTORY_STATUS() == 0)
		{
			while(1)
			{
			}
		}
		ClrWdt();

		if(gIsChargingBatPos == BT_4)
			gHasBat = CHANNEL_30_RES;
		else
			gHasBat = CHANNEL_20_RES;
		
		#if 0
		gBatVoltArray[gIsChargingBatPos] = getVbatAdc(gIsChargingBatPos);
		if(gBatVoltArray[gIsChargingBatPos] >1923 || gBatVoltArray[gIsChargingBatPos] < 1799 )
		{
			fitCount[gIsChargingBatPos] =1;
			continue;
		}
		//gDetectRemovePos++;	
		#endif

			testlevel = CURRENT_LEVEL_1;
			do{
				setCurrent(testlevel);
				delay_ms(10);
				gBatVoltArray[gIsChargingBatPos] = getVbatAdc(gIsChargingBatPos);
				if(gChargeCurrent_2 > 7) // we haven't open MOS, so should not have current
					break;
				PwmControl(PWM_ON);
				delay_ms(200);
				if(testlevel == 1)   //  1.9 - 2.3A     700 -1000   20 /30
				{
					if(gIsChargingBatPos == BT_4)
					{
						preVoltData[0] = 947;      // 0.763  0.7 + 0.7*90ohm
						preVoltData[1] = 1352;    // 1.09          1 + 1*90ohm
					}
					else
					{
						preVoltData[0] = 2482;    // 2V   1.9+ 1.9*90ohm
						preVoltData[1] = 3103;    // 2.5V  2.3 + 2.3*90ohm
					}
					gBatVoltArray[gIsChargingBatPos] = getVbatAdc(gIsChargingBatPos);
					if(gBatVoltArray[gIsChargingBatPos] >preVoltData[1] || gBatVoltArray[gIsChargingBatPos] < preVoltData[0] )  // 电池通道ADC
					{
						PwmControl(PWM_OFF);
						fitCount[gIsChargingBatPos] =1;
						break;
					}
					gBatVoltArray[gIsChargingBatPos] = getAverage(gHasBat);  //CHANNEL_20_RES or CHANNEL_30_RES
					PwmControl(PWM_OFF);
					if(gIsChargingBatPos == BT_4)
					{
						preVoltData[0] = CURRENT_MAX_LEVEL_1_AAA;
						preVoltData[1] = CURRENT_MIN_LEVEL_1_AAA;
					}
					else
					{
						preVoltData[0] = CURRENT_MAX_LEVEL_1;
						preVoltData[1] = CURRENT_MIN_LEVEL_1;
					}
					if(gBatVoltArray[gIsChargingBatPos] >preVoltData[0]|| gBatVoltArray[gIsChargingBatPos] < preVoltData[1] )  //电流是否合理
					{
						fitCount[gIsChargingBatPos] =1;
						break;
					}
					if(gIsChargingBatPos == BT_2 || gIsChargingBatPos == BT_3)
						break;
				}
				else if(testlevel == 2)   //  600 - 1000     200 - 500
				{
					gBatVoltArray[gIsChargingBatPos] = getAverage(gHasBat);
					PwmControl(PWM_OFF);
					if(gIsChargingBatPos == BT_4)
					{
						preVoltData[0] = CURRENT_MAX_LEVEL_2_AAA;
						preVoltData[1] = CURRENT_MIN_LEVEL_2_AAA;
					}
					else
					{
						preVoltData[0] = CURRENT_MAX_LEVEL_2;
						preVoltData[1] = CURRENT_MIN_LEVEL_2;
					}
					if(gBatVoltArray[gIsChargingBatPos] >preVoltData[0] || gBatVoltArray[gIsChargingBatPos] < preVoltData[1] )   // 电流是否合理i
					{
						fitCount[gIsChargingBatPos] =1;
						break;
					}
				}
				else   // 100 - 600   50 -300
				{
					gBatVoltArray[gIsChargingBatPos] = getAverage(gHasBat);
					PwmControl(PWM_OFF);
					if(gIsChargingBatPos == BT_4)
					{
						preVoltData[0] = CURRENT_MAX_LEVEL_3_AAA;
						preVoltData[1] = CURRENT_MIN_LEVEL_3_AAA;
					}
					else
					{
						preVoltData[0] = CURRENT_MAX_LEVEL_3;
						preVoltData[1] = CURRENT_MIN_LEVEL_3;
					}
					if(gBatVoltArray[gIsChargingBatPos] >preVoltData[0] || gBatVoltArray[gIsChargingBatPos] < preVoltData[1] )  // 电流是否合理i
					{
						fitCount[gIsChargingBatPos] =1;
						break;
					}
				}
				testlevel++;
			}while(testlevel<= CURRENT_LEVEL_3);
	}
}while(0);

	while(1)
	{
		if(GET_FACTORY_STATUS() == 0)
		{
			while(1)
			{
			}
		}
		if(gSysStatus != (GET_SYS_STATUS()))
		{
			LED_OFF(BT_1);LED_OFF(BT_2);LED_OFF(BT_3);LED_OFF(BT_4);
		}
		gSysStatus = GET_SYS_STATUS();
		if(gSysStatus == SYS_CHARGING_STATE)
		{
			for(gIsChargingBatPos=BT_1; gIsChargingBatPos <= BT_4; gIsChargingBatPos++)
			{
				if(skipCount)
				{
					if(fitCount[gIsChargingBatPos] !=0 || gDetectRemovePos <=4)  // >3 means vin5V and NTC and yunfang 2.8V all ok
					{
						LED_ON(gIsChargingBatPos);
					}
					else
					{
						LED_OFF(gIsChargingBatPos);
					}
				}
				else
				{
					LED_ON(gIsChargingBatPos);
				}
			}
			delay_ms(150);
			if(skipCount)
				skipCount=0;
			else
				skipCount=1;
		}
		else
		{
			for(gIsChargingBatPos=BT_1; gIsChargingBatPos <= BT_4; gIsChargingBatPos++)
			{
				if(gDetectRemovePos <= 4)
				{
					LED_ON(gDetectRemovePos -1);;
				}
				else
				{
					if(fitCount[gIsChargingBatPos] == 1)
						LED_ON(gIsChargingBatPos);
				}
			}
		}
		ClrWdt();
	}
		
}

void InitConfig()
{

   #ifdef EVT_BOARD
					  //7     6      5      4             3               2                 1             0
       				  //-     -      -      -      Vin5V_DET   V2+_H_CTL     NTC2     iout_cc         
    P0IO    = 0xF0;         // out     out    out     out        input              input           input       input                 (0:input   1:output)
    P0OD    = 0x00;        // -      pp     pp      pp            PP                PP               pp            pp                    (0:push-pull   1:open-drain)
    P0PU    = 0x70;         // -      on      on       on           off                off             off           off                  (0:disable      1:enable)               
    P0        = 0x00;	        // -      -       -         -              0              0                0               0
    P03DB   = 0x00;       // 0       0      0       0               0              0                 0              0
    P0FSR   = 0x13;       // 0      0      0       1               0              0                   1            1

                                    //-     V4+_DET    NTC1   V2+_DET   V1+_DET   GND_ALL   V3+_DET   CHG_DISCHG
    P1IO    = 0x80;         // out      input        input        input         input          input           input             inut
    P1OD    = 0x00;        // pp        PP           PP           pp             PP             PP             PP                 pp
    P1PU    = 0x80;        // on        off          off           off            off             off            off                 off
    P1	  = 0x00;        // 00000000
    P12DB   = 0x00;       // 00000000
    P1FSRH  = 0x2A;      // 00       10            10            10
     P1FSRL  = 0x54;	 //                                                      010              10            10                 0 
    
                                    //-    -  boost_en     v1+_h_ctl   pwm4   -         -      -
    P2IO    = 0x4F;         //-   out     input               input         out    out     out     out
    P2OD    = 0x00;         // -   PP      PP              PP             PP      PP      pp     pp
    P2PU    = 0x47;         // -    on     off              off             off     on      on     on
    P2	  = 0x00;		    // -      -      1      1      -      -      -      -
    P2FSR   = 0x00;	   //             00000000

                                     //led4    led3    led1    led2    pwm3    RST    pwm2   pwm1
    P3IO    = 0xFF;         // out       out      out       out      out       out       out       out
    P3OD    = 0x00;        // PP         PP      PP         PP       PP         PP       PP       PP
    P3PU    = 0x00;         // off       off      off        off       off       off       off       off
    P3	   = 0xF0;	//00000000
    P3FSR   = 0x00;		  // 0        0         0          0         0         0          0        0	
    #else  //DVT BOARD
    					  //7     6      5      4             3               2                 1             0
       				  //-     -      -      -      Vin5V_DET   CHG_DISCHG    NTC2     led3         
    P0IO    = 0xF1;         // out     out    out     out        input              input           input       out                 (0:input   1:output)
    P0OD    = 0x00;        // -      pp     pp      pp            PP                PP               pp            pp                    (0:push-pull   1:open-drain)
    P0PU    = 0x70;         // -      on      on       on           off                off             off           off                  (0:disable      1:enable)               
    P0        = 0x01;	        // -      -       -         -              0              0                0               1
    P03DB   = 0x00;       // 0       0      0       0               0              0                 0              0
    P0FSR   = 0x12;       // 0      0      0       1               0              0                   1            0

                                    //-     V4+_DET    NTC1   V2+_DET   V1+_DET   GND_ALL   V3+_DET   GND_ALL2
    P1IO    = 0x80;         // out      input        input        input         input          input           input             inut
    P1OD    = 0x00;        // pp        PP           PP           pp             PP             PP             PP                 pp
    P1PU    = 0x80;        // on        off          off           off            off             off            off                 off
    P1	  = 0x00;        // 00000000
    P12DB   = 0x00;       // 00000000
    P1FSRH  = 0x2A;      // 00       10            10            10
     P1FSRL  = 0x55;	 //                                                      010              10            10                 1 
    
                                    //-    -  boost_en     v1+_h_ctl   pwm4   -         -      -
    P2IO    = 0x4F;         //-   out     out               input         out    out     out     out
    P2OD    = 0x00;         // -   PP      PP              PP             PP      PP      pp     pp
    P2PU    = 0x47;         // -    on     off              off             off     on      on     on
    P2	  = 0x00;		    // -      -      1      1      -      -      -      -
    P2FSR   = 0x00;	   //             00000000

                                     //led4    cur_ctl2    led1    led2    pwm3    CUR_CTL    pwm2   pwm1
    P3IO    = 0xBB;         // out       input        out       out      out         input       out       out
    P3OD    = 0x00;        // PP         PP      PP         PP       PP         PP       PP       PP
    P3PU    = 0x00;         // off       off      off        off       off       off       off       off
    P3	   = 0xB0;	//10110000
    P3FSR   = 0x00;		  // 0        0         0          0         0         0          0        0	
    #endif
}

void main()
{
	u8 cur_detect_pos=1;
	OSCCR = 0x20;		// internal OSC 8MHz
	BITCR = 0x4E;		// BIT 16.384ms

	delay_ms(16);

	LVRCR  = 0x02;                      // builtin reset 2.75V set, LVRCR.0=0 enable !!!

	InitConfig();

	WDTDR = 0xFF;
	ClrWdt();
	
	//timer0   as system tick
	T0DR = 38;
	T0CR = 0x8D;

	//ADC
	ADCCRH = 0x07;
	ADCCRL  = 0x90;

	//interrupt
	IE = 0x00;
	IE1 = 0x00;
	IE2 = 0x02;    //enable timer0 overflow 
	IE3 = 0x00;
	EIFLAG0 = 0;
	EIFLAG1 = 0;
	IIFLAG = 0;

	IE |= (1<<7);    //global interrupt

	isFromOutput = 0;

	if(GET_FACTORY_STATUS())
	{
		factoryTest();
	}
	else
	{
		P2IO |= (1<<5);
	}
	gSysStatus =  GET_SYS_STATUS();
	if(gSysStatus == SYS_DISCHARGE_STATE)
	{
		gOutputStatus = OUTPUT_STATUS_WAIT;
		gDelayCount = 50;
		gHasBat = 0;
		CHANGE_TO_OUTPUT();
	}
	delay_ms(100);
	
	while(1)
	{
		shortTick =0;
		StatusCheck();
		
		if(gSysStatus == SYS_CHARGING_STATE)
		{
			addRestTime();
			btRemove();

			if(gChargingStatus == SYS_CHARGE_WAIT_TO_PICK_BATTERY)
				PickBattery();

			chargeHandler();

			if(gPreChargingBatPos < BT_NULL)
				PreChargeBatHandler();
		}
		else    //output handler
		{
			outputHandler();
		}

		ledHandler();

		ClrWdt();
		
		do{
			ClrWdt();
			}while(shortTick == 0);

	}
	
	
}



void T0_Int_Handler(void) interrupt 13
{
	shortTick = 1;
}
