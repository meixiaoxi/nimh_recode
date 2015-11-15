#include "nimh.h"

extern u8 gBatStateBuf[4];
extern u8 gBatLeveL[4];
extern u8 gSysStatus;
extern u8 gOutputStatus;
extern u16 gBatVoltArray[4];
extern u32 idata gLastChangeLevelTick[4];
extern u8 idata gIsFisrtChangeLevel[4];
extern u32 shortTick;
void LED_ON(u8 led)
{
	switch(led)
	{
		case 0:
			P35=0;break;
		case 1:
			P34=0;break;
		case 2:
			#ifdef EVT_BOARD
			P36=0;break;
			#else
			P00=0;break;
			#endif
		case 3:
			P37=0;break;
		default:
			break;
	}
}

void LED_OFF(u8 led)
{
	switch(led)
	{
		case 0:
			P35=1;break;
		case 1:
			P34=1;break;
		case 2:
			#ifdef EVT_BOARD
			P36=1;break;
			#else
			P00=1;break;
			#endif
		case 3:
			P37=1;break;
		default:
			break;
	}	
}


u8 ledDisplayCount = 0, ledErrorCount =0;
u8 gLedStatus = 0, ledErrorStatus = 0;
#if 1
void ledHandler(void)
{
	u8 i;
	
	
if(gSysStatus == SYS_CHARGING_STATE)
{
	#ifndef LED_CHARGING_DISPLAY_SUPPORT
	if(gLedStatus ==0)
	{
		for(i=BT_1;i<=BT_4;i++)
		{
			if(gBatStateBuf[i] == STATE_NORMAL_CHARGING ||gBatStateBuf[i] == STATE_BATTERY_FULL)
				LED_ON(i);
		}
		ledDisplayCount++;
		if(ledDisplayCount > 100)
		{
			ledDisplayCount = 0;
			gLedStatus = 1;
		}
	}
	else
	{
		for(i=BT_1;i<=BT_4;i++)
		{
			if(gBatStateBuf[i] == STATE_NORMAL_CHARGING)
				LED_OFF(i);		
		}
		ledDisplayCount++;
		if(ledDisplayCount > 100)
		{
			ledDisplayCount = 0;
			gLedStatus = 0;
		}
	}
	#endif
	//error state

	if(ledErrorStatus == 0)
	{
		for(i=BT_1;i<=BT_4;i++)
		{
			if((gBatStateBuf[i] >= STATE_BATTERY_TYPE_ERROR) && (gBatStateBuf[i] <= STATE_ZERO_BATTERY_CHARGE_ERROR))
			{
				LED_ON(i);	
			}
		}
		ledErrorCount++;
		if(ledErrorCount>=10)
		{
			ledErrorCount = 0;
			ledErrorStatus =1;
		}
	}
	else
	{
		for(i=BT_1;i<=BT_4;i++)
		{
			if((gBatStateBuf[i]>=STATE_BATTERY_TYPE_ERROR) && (gBatStateBuf[i]<=STATE_ZERO_BATTERY_CHARGE_ERROR))
				LED_OFF(i);		
		}
		ledErrorCount++;
		if(ledErrorCount>=10)
		{
			ledErrorCount = 0;
			ledErrorStatus =0;
		}
	}
}
else
{
	#if 0
	if(gOutputStatus == OUTPUT_STATUS_NORMAL)
	{	
		/*********
		gIsFisrtChangeLevel[0]用于轮流闪开始的标志
		gIsFisrtChangeLevel[1]用于放电状态下初始四个灯亮的结束标志
		gIsFisrtChangeLevel[2]用于表示当前处于亮灯状态的灯
		gIsFisrtChangeLevel[3]用于轮流亮时亮/灭的标志

		**********/
		if(gIsFisrtChangeLevel[1])
		{
			if(gIsFisrtChangeLevel[0])
			{
				if(gIsFisrtChangeLevel[3])  //亮
				{
					if(getDiffTickFromNow(gLastChangeLevelTick[0]) > LED_DISPLAY_ON)
					{
						LED_OFF(gIsFisrtChangeLevel[2]);
						if(gIsFisrtChangeLevel[2] ==4)
							gIsFisrtChangeLevel[2] =1;
						else
							gIsFisrtChangeLevel[2]++;
						gLastChangeLevelTick[0] = getSysTick();
						gIsFisrtChangeLevel[3] = 0;
					}
				}
				else  //灭
				{
					if(getDiffTickFromNow(gLastChangeLevelTick[0]) > LED_DISPLAY_OFF)
					{
						LED_ON(gIsFisrtChangeLevel[2]);
						gLastChangeLevelTick[0] = getSysTick();
						gIsFisrtChangeLevel[3] = 1;
					}
				}
			}
		      else //第一次
		      {
				gIsFisrtChangeLevel[0] = 1;
				gIsFisrtChangeLevel[2] =1;
				gIsFisrtChangeLevel[3] = 1;
				LED_ON(1);
				gLastChangeLevelTick[0] = getSysTick();
			}
		}
		else
		{
			if(gLastChangeLevelTick[0])
			{
				if(getDiffTickFromNow(gLastChangeLevelTick[0]) > LED_INITIAL_DISPLAY)
				{
					for(i=1;i<5;i++)
						LED_OFF(i);
				}
				if(getDiffTickFromNow(gLastChangeLevelTick[0]) > LED_INITIAL_DISPLAY_END)
				{
					gIsFisrtChangeLevel[1]=1;
				}
			}
			else
			{
				for(i=1;i<5;i++)
					LED_ON(i);
				gLastChangeLevelTick[0] = getSysTick();
				if(gLastChangeLevelTick[0] == 0)
				{
					EA = 0;
					shortTick = 1;
					EA =1;
					gLastChangeLevelTick[0] = 1;
				}
			}
		}
	}
	#endif
}
	
	
}
#endif
