#include "nimh.h"

extern u8 gBatStateBuf[4];
extern u8 gSysStatus;
extern u8 gOutputStatus;
extern u16 gBatVoltArray[4];
extern u8 idata gIsInTwoState;
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

	gIsInTwoState++;
	
	if(gBatStateBuf[0] ==0)
	{
		if(gIsInTwoState > 100)
		{
			LED_OFF(BT_1);LED_OFF(BT_2);LED_OFF(BT_3);LED_OFF(BT_4);
			gBatStateBuf[0] = 1;
			gIsInTwoState = 0;
		}
		else
		{
			LED_ON(BT_1);LED_ON(BT_2);LED_ON(BT_3);LED_ON(BT_4);
		}
		return;
	}

	#if 1
	if(gOutputStatus == OUTPUT_STATUS_NORMAL)
	{	
		/*********
		gIsFisrtChangeLevel[1]用于轮流闪开始的标志
		gIsFisrtChangeLevel[2]用于表示当前处于亮灯状态的灯
		gIsFisrtChangeLevel[3]用于轮流亮时亮/灭的标志

		**********/
			if(gBatStateBuf[1])
			{
				if(gBatStateBuf[3])  //亮
				{
					if(gIsInTwoState > 50)
					{
						LED_OFF(gBatStateBuf[2]);
						if(gBatStateBuf[2] ==BT_4)
							gBatStateBuf[2] =BT_1;
						else
							gBatStateBuf[2]++;
						gIsInTwoState = 0;
						gBatStateBuf[3] = 0;
					}
				}
				else  //灭
				{
					if(gIsInTwoState > 200)
					{
						LED_ON(gBatStateBuf[2]);
						gBatStateBuf[3] = 1;
						gIsInTwoState = 0;
					}
				}
			}
		      else //第一次
		      {
				gBatStateBuf[1] = 1;
				gBatStateBuf[2] =BT_1;
				gBatStateBuf[3] = 1;
				LED_ON(BT_1);
			}
	}
	#endif
}
	
	
}
#endif
