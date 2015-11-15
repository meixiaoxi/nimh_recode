#include "MC96F8316.h"
#include "nimh.h"

u32 idata gUpdateDebanceTick[4];
u8 gPreChargingBatPos = BT_NULL;
u32 shortTick=0;

u8 gSysStatus;
u8 gOutputStatus;

u8  isFromOutput = 0;

//type_error charge_error  pre  fast sup  trick  charging(on_off)  is_detect(电池检测) bat_state(valid)
//the first byte is a dummy						
u8 gBatStateBuf[4] = {0,0,0,0};

u16 gBatVoltArray[4]={0,0,0,0};

u8 gChargeSkipCount[] = {0,0,0,0};    //控制PWM周期

u16 preVoltData[4] ={0,0,0,0};


u8 skipCount;
u16 isPwmOn = 0;

u8 dropCount[4] = {0,0,0,0};	
u8 fitCount[4] = {0,0,0,0};

u32 idata gNearFullTimeTick[4] = {0,0,0,0};
u32 idata gChargingTimeTick[4] ={0,0,0,0};
u32 idata gLastChangeLevelTick[4]= {0,0,0,0};
u8   idata gIsFisrtChangeLevel[4] = {0,0,0,0};

u8 idata gCurrentLevel[2];
u8 idata gIsInTwoState= 0;
u8 idata gNowTwoBuf[2];



u8 TotalTime[4] = {0,0,0,0};
u8 gIsChargingBatPos=BT_1;
u8 gChargingStatus = SYS_CHARGING_STATUS_DETECT;
u8 gChargeChildStatus[4] = {0,0,0,0};
u16 gChargeCurrent_2;
u16 gChargeCurrent;
u8 gBatType[4] = {0,0,0,0};
u8 gDelayCount =0;
u8 gChargeCount =0;
u8 gDetectRemovePos=0;

void FindTwoBattery()
{
	u8 batNum,fitNum = 0;
	u8 temp;
	if(gIsInTwoState == 0)
	{
		for(batNum=BT_1; batNum<=BT_4;batNum++)
		{
			if(gBatStateBuf[batNum] == STATE_NORMAL_CHARGING && gBatVoltArray[batNum]>= BAT_START_FAST_CHARGE)// CHARGE_STATE_ERROR | BAT_TYPE_ERROR | CHARGE_STATE_FULL
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
	if(fitNum)
	{
		if(fitNum == 1)
		{
			for(batNum=BT_1; batNum<=BT_4;batNum++)
			{
				if(gNowTwoBuf[0] == batNum)
				{	
					if(batNum == BT_4 && fitNum == 1)
						gNowTwoBuf[1] = batNum-1;
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
			}
		}
		gIsInTwoState =1;
	}
}
#if 0
void outputHandler()
{
	u8 cur_detect_pos;

do
{
	if(gOutputStatus != OUTPUT_STATUS_STOP)
	{
		if(/*gOutputStatus == OUTPUT_STATUS_WAIT  ||*/ getDiffTickFromNow(ChargingTimeTick) > OUTPUT_CHECK_INTERVAL)
		{
			ChargingTimeTick = getSysTick();

			preVoltData[0] = getVbatAdc(1);
			if(gOutputStatus == OUTPUT_STATUS_WAIT)
			{
				gBatVoltArray[1] = 0xFFFF;
				gCount = 1;
				for(cur_detect_pos = 1; cur_detect_pos < 4; cur_detect_pos++)
				{
					preVoltData[cur_detect_pos] = getVbatAdc(cur_detect_pos+1);

					if(cur_detect_pos == 2 || cur_detect_pos == 3)
						preVoltData[cur_detect_pos] = preVoltData[cur_detect_pos]/3;
					
					gBatVoltArray[0] = preVoltData[cur_detect_pos-1] -preVoltData[cur_detect_pos];
					if(gBatVoltArray[0] < gBatVoltArray[1])
					{
						gCount = cur_detect_pos;
						gBatVoltArray[1] = gBatVoltArray[0];
					}
					if(preVoltData[cur_detect_pos] > preVoltData[cur_detect_pos-1] ||( gBatVoltArray[0]<MIN_VBAT_OUTPUT_IDLE))
						break;	
				}
				if(cur_detect_pos == 4)
				{
					gOutputStatus = OUTPUT_STATUS_NORMAL;
					ENABLE_BOOST();
					//updateBatLevel(gBatVoltArray[1][0],gCount+1);
				}
				else
				{
					gOutputStatus = OUTPUT_STATUS_WAIT;
					LED_OFF(1),LED_OFF(2),LED_OFF(3),LED_OFF(4);
					DISABLE_BOOST();
				}				
			}
			else
			{
				for(cur_detect_pos = 1; cur_detect_pos < 4; cur_detect_pos++)
				{
					preVoltData[cur_detect_pos] = getVbatAdc(cur_detect_pos+1);

					if(cur_detect_pos == 2 || cur_detect_pos == 3)
						preVoltData[cur_detect_pos] = preVoltData[cur_detect_pos]/3;
					
					gBatVoltArray[0] = preVoltData[cur_detect_pos-1] -preVoltData[cur_detect_pos];
					
					if(cur_detect_pos == gCount)
						gBatVoltArray[1] = gBatVoltArray[0];
					
					if(preVoltData[cur_detect_pos] > preVoltData[cur_detect_pos-1] ||( gBatVoltArray[0] < MIN_VBAT_OUPUT))
						break;	
				}
				if(cur_detect_pos != 4)
				{
					gOutputStatus = OUTPUT_STATUS_WAIT;
					LED_OFF(1),LED_OFF(2),LED_OFF(3),LED_OFF(4);
					DISABLE_BOOST();
				}
				else if(getDiffTickFromNow(gUpdateDebanceTick[gCount]) > MIN_BAT_LEVEL_UPDATE_INTERVAL)
				{
				}
				//	updateBatLevel(gBatVoltArray[1][0],gCount+1);
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
	gIsFisrtChangeLevel[batNum] = 0;
	gLastChangeLevelTick[batNum] = 0;
	gNearFullTimeTick[batNum] = 0;
	TotalTime[batNum] = 0;
	LED_OFF(batNum);
	//PB &= 0xF0;   //close current pwm channel
	PwmControl(PWM_OFF);

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
	gIsFisrtChangeLevel[i] = 0;
	gLastChangeLevelTick[i] = 0;
	gNearFullTimeTick[i] = 0;
	LED_OFF(i);
	TotalTime[i] = 0;
	}

	
	//PWM
	P30 =0;
	P31 =0;
	P33 =0;
	P23 =0;

	isPwmOn =0; 
	
	gIsChargingBatPos = BT_1;
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
					gChargingStatus = SYS_CHARGING_STATUS_DETECT;
				}
			}
			else
			{
				DISABLE_BOOST();
				gSysStatus = SYS_CHARGING_STATE;
				CHANGE_TO_INPUT();
				isFromOutput = 1;
				removeAllBat();
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
					
	if(tempV>isPwmOn || gChargeCurrent <3  )
	{
		StatusChange(batNum, STATE_BATTERY_TYPE_ERROR);
		PwmControl(PWM_OFF);
		return;
	}

	 #ifdef DVT_BOARD
	tempV = getAdcValue(CHANNEL_VIN_5V);
	if(tempV < VIN_5V_MINUM)
	{
		if(gBatType[batNum] == BAT_AAA_TYPE)
			gCurrentLevel[0] = 1;
		else
			gCurrentLevel[1] = 1;
	}
	else
	{
		if(gBatType[batNum] == BAT_AAA_TYPE)
			gCurrentLevel[0] = 0;
		else
			gCurrentLevel[1] = 0;

	}
	#endif
	PwmControl(PWM_OFF);
	
}

void PreCharge(u8 batNum)
{
	u16 tempT;

		gChargingTimeTick[batNum]++;
		
		if(gChargingTimeTick[batNum] > BAT_CHARGING_PRE_MAX_COUNT)
		{
			StatusChange(batNum,STATE_ZERO_BATTERY_CHARGE_ERROR);
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
					StatusChange(batNum,STATE_ZERO_BATTERY_TEMPERATURE_ERROR);
					gChargingTimeTick[batNum] = 0;
				}
			}
				
		}
}

void FastCharge(u8 batNum)
{
	u16 tempV,tempT;
	u32 overTimer = BAT_CHARGING_FAST_MAX_COUNT;

	gChargingTimeTick[batNum]++;
	tempT = getBatTemp(batNum);

	if(gChargingTimeTick[batNum] > BAT_START_DV_COUNT)  //hod-off time, in this period, we do NOT detect -dv
	{
		tempV = getVbatAdc(batNum);
		#if 0
		if(preVoltData[batNum])
		{
			tempV = ((preVoltData[batNum]<<2) + tempV)/5;
			preVoltData[batNum] = tempV;
		}
		else
		{
			preVoltData[batNum] = tempV;
		}
		#endif
		
		if(tempV > BAT_VOLT_NEAR_FULL)
			gNearFullTimeTick[batNum]++;

			if(gBatType[batNum] == BAT_AAA_TYPE)
				overTimer = BAT_CHARGING_FAST_MAX_COUNT_AAA;
			if(tempV >= CHARGING_FAST_MAX_VOLT || gChargingTimeTick[batNum] > BAT_CHARGING_FAST_MAX_COUNT || tempT < ADC_TEMP_MAX || tempT > ADC_TEMP_MIN || gNearFullTimeTick[batNum] > BAT_NEAR_FULL_MAX_COUNT)
			{
				if((tempT < ADC_TEMP_MAX  && tempV <CHARGING_FAST_TEMP_END_VOLT) || tempT > ADC_TEMP_MIN)   //过温
				{
					//电压不足
					StatusChange(batNum,STATE_BATTERY_TEMPERATURE_ERROR);

					preVoltData[batNum] = 0;
					gIsInTwoState = 0;
					return;
				}
				fitCount[batNum]++;
				if(fitCount[batNum] > 3)
				{
					 //timer or maxVolt
					StatusChange(batNum,STATE_BATTERY_FULL);	
					gChargingTimeTick[batNum] = 0;
				}
					return;
			}
			fitCount[batNum] = 0;

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
					}
				}
			}
	}
	else
	{
		tempV = getVbatAdc(batNum);
		if(tempV > BAT_VOLT_NEAR_FULL)
			gNearFullTimeTick[batNum]++;
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

void chargeHandler(void)
{
	u16 tempT,tempV;
	u8 battery_state = gBatStateBuf[gIsChargingBatPos];
	static u8 chargingTime = 0;
	u8 chargingCurrent = 0;
	if(gChargingStatus == SYS_CHARGING_STATUS_DETECT)
	{
		if(battery_state == STATE_DEAD_BATTERY)
		{
			chargingTime = CHARGING_TIME_10MS;
		}
		else if(battery_state == STATE_BATTERY_DETECT)
		{
			chargingTime = CHARGING_TIME_500MS;
		}
		else if(battery_state == STATE_NORMAL_CHARGING)
		{
			chargingTime = CHARGING_TIME_500MS;
		}
		else if(battery_state == STATE_ZERO_BATTERY_TEMPERATURE_ERROR || battery_state == STATE_ZERO_BATTERY_CHARGE_ERROR)
		{
			chargingTime = CHARGING_TIME_10MS;
		}
		else if(battery_state == STATE_BATTERY_TYPE_ERROR)
		{
			chargingTime = CHARGING_TIME_0MS;
		}
		else if(battery_state == STATE_BATTERY_TEMPERATURE_ERROR)
		{
			chargingTime = CHARGING_TIME_0MS;
		}
		else if(battery_state == STATE_BATTERY_FULL)
		{

			chargingTime = CHARGING_TIME_0MS;
		}
		else
			chargingTime =0;

		if(chargingTime != 0)
		{
			if(battery_state == STATE_DEAD_BATTERY)
			{
				tempV = getVbatAdc(gIsChargingBatPos);
				if(tempV >= BAT_MAX_VOLT_OPEN)
				{
					StatusChange(gIsChargingBatPos, STATE_BATTERY_TYPE_ERROR);
					return;
				}
			}
			isPwmOn = 1;
			PwmControl(PWM_ON);
			gChargingStatus = SYS_CHARGING_STATUS_NORMAL;
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
				else if(tempT > ADC_TEMP_MAX && tempT < ADC_TEMP_MIN)
					StatusChange(gIsChargingBatPos,STATE_NORMAL_CHARGING);
			}
			else if(battery_state == STATE_BATTERY_TYPE_ERROR)
			{
				if(tempV < BAT_MIN_VOLT_OPEN)
					StatusChange(gIsChargingBatPos,STATE_DEAD_BATTERY);
			}
		}
	}
	else if(gChargingStatus == SYS_CHARGING_STATUS_NORMAL)
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
					gChargingStatus = SYS_CHARGING_STATUS_DETECT;
				}
				else if(battery_state == STATE_BATTERY_DETECT)
				{
					if(isPwmOn)
					{
						tempV = getVbatAdc(gIsChargingBatPos);
						batteryDetect(gIsChargingBatPos,tempV);
						if(gBatStateBuf[gIsChargingBatPos] == STATE_BATTERY_TYPE_ERROR)
						{
							gChargingStatus = SYS_CHARGING_STATUS_DETECT;
						}
						gDelayCount= 0;
					}
					else
					{
						gDelayCount++;
						if(gDelayCount >=3)
						{
							tempV = getVbatAdc(gIsChargingBatPos);
							gBatVoltArray[gIsChargingBatPos] = tempV;
							StatusChange(gIsChargingBatPos, STATE_NORMAL_CHARGING);
							if(tempV< CHARGING_PRE_END_VOLT )
								gChargeChildStatus[gIsChargingBatPos] =  CHARGE_STATE_PRE;
							else
								gChargeChildStatus[gIsChargingBatPos] = CHARGE_STATE_FAST;

							if(tempT < ADC_TEMP_MAX || tempT > ADC_TEMP_MIN)
							{
								if(gChargeChildStatus[gIsChargingBatPos] == CHARGE_STATE_PRE)
									StatusChange(gIsChargingBatPos,STATE_ZERO_BATTERY_TEMPERATURE_ERROR);
								else
									StatusChange( gIsChargingBatPos,STATE_BATTERY_TEMPERATURE_ERROR);
							}
							gChargingTimeTick[gIsChargingBatPos] = 0;
							gChargingStatus = SYS_CHARGING_STATUS_DETECT;
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
						gChargingStatus = SYS_CHARGING_STATUS_DETECT;
						return;
					}
					PwmControl(PWM_OFF);
					gDelayCount++;
					if(gDelayCount < 2)
						return;
					if(gIsChargingBatPos<=BT_4)
					{
						switch(gChargeChildStatus[gIsChargingBatPos])
						{
							case CHARGE_STATE_FAST:
								FastCharge(gIsChargingBatPos);break;
							case CHARGE_STATE_PRE:
								PreCharge(gIsChargingBatPos);break;
							default:
								break;
						}
					}
					//gPreChargingBatPos = gIsChargingBatPos;
					gChargingStatus = SYS_CHARGING_STATUS_DETECT;
				}
				else if(battery_state == STATE_ZERO_BATTERY_TEMPERATURE_ERROR)
				{
					tempV = getVbatAdc(gIsChargingBatPos);
					if(tempV > BAT_ZERO_SPEC_VOLT)
					{
						StatusChange(gIsChargingBatPos, STATE_DEAD_BATTERY);
						PwmControl(PWM_OFF);
						gChargingStatus = SYS_CHARGING_STATUS_DETECT;
						return;
					}
					PwmControl(PWM_OFF);
					tempT = getBatTemp(gIsChargingBatPos);
					if(tempT > ADC_TEMP_MAX && tempT < ADC_TEMP_MIN)
						StatusChange(gIsChargingBatPos, STATE_NORMAL_CHARGING);
					gChargingStatus = SYS_CHARGING_STATUS_DETECT;
				}
				else if(battery_state == STATE_BATTERY_FULL)
				{
					PwmControl(PWM_OFF);
					gChargingStatus = SYS_CHARGING_STATUS_DETECT;
				}
			}
		}
	}
}

void PickBattery()
{
	u8 batNum;


		FindTwoBattery();
	
		if(gIsChargingBatPos >= BT_4)
			gIsChargingBatPos = BT_1;
		else
			gIsChargingBatPos++;

		if(gBatStateBuf[gIsChargingBatPos] == STATE_NORMAL_CHARGING)
		{
			if(gIsInTwoState)
			{
				while(gNowTwoBuf[0] != gIsChargingBatPos && gNowTwoBuf[1] != gIsChargingBatPos)
				{
					if(gIsChargingBatPos >= BT_4)
						gIsChargingBatPos = BT_1;
					else
						gIsChargingBatPos++;
				}
			}
			for(batNum = BT_1; batNum<=BT_4; batNum++)
			{
				if(batNum == gIsChargingBatPos)
					continue;
				if(gIsInTwoState)
				{
					if(gBatStateBuf[batNum] == STATE_NORMAL_CHARGING)
					{
						if(batNum != gNowTwoBuf[0] && batNum != gNowTwoBuf[1])
							continue;
					}
				}
				//else
				{
					switch(gBatStateBuf[batNum])
					{
						case STATE_DEAD_BATTERY:
						case STATE_ZERO_BATTERY_TEMPERATURE_ERROR:
								TotalTime[gIsChargingBatPos] = TotalTime[gIsChargingBatPos]+2;
								break;
						case STATE_NORMAL_CHARGING:
						case STATE_BATTERY_DETECT:
								TotalTime[gIsChargingBatPos] = TotalTime[gIsChargingBatPos]+50;
								break;
						default:
								TotalTime[gIsChargingBatPos] = TotalTime[gIsChargingBatPos] +1;
								break;
					}
				}
				if(TotalTime[gIsChargingBatPos] >= 50)
					break;
			}
			if(TotalTime[gIsChargingBatPos] < 48)
			{
				if(gIsChargingBatPos >= BT_4)
					gIsChargingBatPos = BT_1;
				else
					gIsChargingBatPos++;
			}
			else
				TotalTime[gIsChargingBatPos] = 1;
		}
}



void btRemove()
{
	u16 tempV;

	if(gIsChargingBatPos != gDetectRemovePos)
	{
		if(gBatStateBuf[gDetectRemovePos] >= STATE_NORMAL_CHARGING && gBatStateBuf[gDetectRemovePos] <=STATE_BATTERY_TEMPERATURE_ERROR)
		{	
			if(gChargeChildStatus[gDetectRemovePos] == CHARGE_STATE_FAST)
			{
				tempV = getVbatAdc(gDetectRemovePos);
				if(tempV < BAT_REMOVE_VOLT)
				{
					StatusChange(gDetectRemovePos,STATE_DEAD_BATTERY);
				}
			}
		}
	}
	if(gDetectRemovePos >= BT_4)
		gDetectRemovePos = BT_1;
	else
		gDetectRemovePos++;
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
    P0IO    = 0xF5;         // out     out    out     out        input              out           input       out                 (0:input   1:output)
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

	LVRCR  = 0x02;                      // builtin reset 2.00V set, LVRCR.0=0 enable !!!

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
		//factoryTest();
	}
	else
	{
		P2IO |= (1<<5);
	}
	gSysStatus =  GET_SYS_STATUS();
	if(gSysStatus == SYS_DISCHARGE_STATE)
	{
		gOutputStatus = OUTPUT_STATUS_WAIT;
		CHANGE_TO_OUTPUT();
	}
	
	while(1)
	{
		shortTick =0;
		StatusCheck();
		
		if(gSysStatus == SYS_CHARGING_STATE)
		{
			btRemove();

			if(gChargingStatus != SYS_CHARGING_STATUS_NORMAL)
				PickBattery();

			chargeHandler();
		}
		else    //output handler
		{
			ClrWdt();
		}	//outputHandler();

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
