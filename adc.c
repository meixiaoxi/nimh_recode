#include "nimh.h"
//u8 tempStr1[] = "temp1:";
//u8 tempStr2[] = "temp2:";

extern u16 gChargeCurrent;
u16 getAdcValue(u8 channel)
{
	
	u16 temp = 0xffff;
	
	short value; 
	char  rAdc_Offset;


	// read adc offset
#pragma asm
	mov A, #0			;
	mov DPTR, #3FFFh	;
	movc A, @A+DPTR		;
#pragma endasm

	rAdc_Offset = ACC;		//; offset value


	ADCCRH = 0x07;
	ADCCRL = 0x80;           	         // VDD reference
	
	ADCCRL |= channel;

	ADCCRL |= (1<<6);
	
	while((ADCCRL&0x10) != 0x10)
	{
		temp--;
		ClrWdt();
		if(temp == 0)
			break;
	}


	value = (ADCDR & 0xfff);	

	// adjust adc value
	if(rAdc_Offset >= -3 && rAdc_Offset <= 7)
	{
	//	rAdc_Offset += 2;
		if(value > rAdc_Offset)
		{
			value -= rAdc_Offset;
		}
		else
		{
			value = 0;
		}
	}	

	ADCCRL = 0x80;
	
	return value;
}

u16 getAverage(u8 channel)
{
	u8 i;
	u16 temp,max,min,ret;


	temp = getAdcValue(channel);
	ret= temp;
	max =temp;
	min = temp;
	for(i=0;i<9;i++)
	{
		//delay_us(100);
		 temp = getAdcValue(channel);
	 	if(temp > max)
	 	{
			max = temp;
	 	}

		 if(temp < min)
		{
			min = temp;
	 	}
	 	ret += temp;
	}
	

	return (ret - max - min)>>3;
}

u16 getBatTemp(u8 batNum)
{
	u16 tempT;



	if(batNum <BT_3)
	{
		tempT = getAverage(CHANNEL_TEMP_1);
	}
	else
	{
		tempT = getAverage(CHANNEL_TEMP_2);
	}

	return tempT;
}

u16 getVbatAdc(u8 channel)
{
	u16 tempV;


	switch(channel)
	{
		case 0:
			channel = CHANNEL_VBAT_1;break;
		case 1:
			channel = CHANNEL_VBAT_2;break;
		case 2:
			channel = CHANNEL_VBAT_3;break;
		case 3:
			channel = CHANNEL_VBAT_4;break;
		default:
			break;
	}


	gChargeCurrent= getAverage(CHANNEL_20_RES);
	tempV = getAverage(channel);

//	sendStr(tempStr1);
//	send(gChargeCurrent);
//	sendStr(tempStr2);
//	send(temp2);

	if(tempV < gChargeCurrent)
	{
		gChargeCurrent = 0;
		return 0;
	}
	return (tempV-gChargeCurrent);
}
