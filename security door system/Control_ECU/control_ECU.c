/******************************************************************************
 *
 * Module: timer0
 *
 * File Name: timer_pwm.c
 *
 * Description: Source file for the TIMER0 driver
 *
 * Author: Saber Mahmoud
 *
 *******************************************************************************/
#include"dc_motor.h"
#include"buzzer.h"
#include"external_eeprom.h"
#include"timer_pwm.h"
#include"timer1_compare_normal.h"
#include"twi.h"
#include"uart.h"
#include"common_macros.h"
#include"std_types.h"
#include<avr/io.h>
#include"control_ECU.h"
#include<util/delay.h>
int main(void)
{
	/*config the uart to send info between the MCUs*/
	UART_ConfigType UART_config={BITS_8,Disabled,One_bit,BaudRate9600};
	/*config the I2C*/
	TWI_ConfigType TWI_config={0b00000010,400000};
	/*initialize the buzzer*/
	Buzzer_init();
	/*initialize the motor*/
	DcMotor_Init();
	/*enable the global interrupts*/
	SET_BIT(SREG,7);
	/*initialize the uart*/
	UART_init(&UART_config);
	/*initialize the I2C*/
	TWI_init(&TWI_config);
	/*variable  to store the data read from the EEBROM */
	uint8 readed_data;
	/*g_password[0] is initially trash as we do not half operation yet*/
	g_PASSWORD[0]=TRASH_BYTE;
	while(1)
	{
		if(g_PASSWORD[0]==1)  /*if the first byte is 1 check if the two passwords are the same and insert the password in the EEPROM if true*/
		{
			while(flag!=11);	/*wait for the first mcu to send the password*/
			boolean check=TRUE;	/*to send byte back tells the other MCU if the passwords are matched*/
			for(uint8 i=FIRST_DIGIT;i<6;i++)
			{
				if(g_PASSWORD[i]!=g_PASSWORD[5+i])		/*compare between the two passwords*/
				{
					check=FALSE;
					break;
				}
			}
			UART_sendByte(check);	/*send byte to tell the HMI if the passwords are matched or not*/
			if(check)
			{for(uint8 i=0;i<5;++i)
			{
				EEPROM_writeByte(0x0311+i,g_PASSWORD[i]); 	/*writes the password in the EEPROM*/
			}
			}
			flag=0;
		}

		else if(g_PASSWORD[0]==2)		/*if the first bite is 2 compare the password with the password in EEPROM */
		{
			while(flag!=6);
			boolean confirm=TRUE;	/*variable we will send it to the other MCU to confirm the password is equal to the pass stored in EEPROM or not*/
			for(uint8 i=FIRST_DIGIT;i<PASSWORD_DIGITS;i++)
			{
				EEPROM_readByte((PASSWORD_ADDRESS+i), &readed_data);
				if(readed_data!=g_PASSWORD[i])
				{
					confirm=FALSE;
					break;
				}
			}
			UART_sendByte(confirm);
			flag=0;
		}
		else if(g_PASSWORD[0]==3)	/*if first byte in the password array is 3 we run the buzzer*/
		{
			Buzzer_on();
			_delay_ms(60000);
			Buzzer_off();
			UART_sendByte(1);
			flag=0;
		}
		else if(g_PASSWORD[0]==4)	/*if first byte in the password array is 4 we run the dc_MOTOR*/
		{
			DcMotor_Rotate(CW, FullSpeed);
			_delay_ms(15000);
			DcMotor_Rotate(CW, ZeroSpeed);
			_delay_ms(3000);
			DcMotor_Rotate(A_CW, FullSpeed);
			_delay_ms(15000);
			DcMotor_Rotate(CW, ZeroSpeed);
			UART_sendByte(1);
			flag=0;
		}
		g_PASSWORD[0]=TRASH_BYTE;
	}
}
