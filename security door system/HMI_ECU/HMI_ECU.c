/******************************************************************************
 *
 * Module:main
 *
 * File Name: HMI_ECU.c
 *
 * Description: Source file for the TIMER0 driver
 *
 * Author: Saber Mahmoud
 *
 *******************************************************************************/
#include"lcd.h"
#include"keypad.h"
#include"uart.h"
#include"HMI_ECU.h"
#include<util/delay.h>
#include<avr/io.h>
#include"common_macros.h"
uint8 password[11];
void change_password()
{
	unsigned char key;


	uint8 password_counter=1,password_counter2=6;
	boolean flag=FALSE,flag2=FALSE;
	while(TRUE)
	{
		if(flag==TRUE)
		{
			password_counter=1;
			flag=FALSE;
			break;
		}
		LCD_displayString("plz enter pass:");
		LCD_moveCursor(1,0); /* Move the cursor to the second row */
		/* Get the pressed key number, if any switch pressed for more than 500 ms it will considered more than one press */
		/* Get the pressed key number, if any switch pressed for more than 500 ms it will considered more than one press */
		while(1)
		{
			key = KEYPAD_getPressedKey();
			if(key=='=')
			{
				if(password_counter!=6)
				{
					flag=FALSE;
					LCD_clearScreen();
					LCD_moveCursor(0,2);
					LCD_displayString("choose five");
					LCD_moveCursor(1,4);
					LCD_displayString(" digits");
					_delay_ms(100);
					LCD_clearScreen();
					password_counter=1;
				}
				else
					flag=TRUE;
				break;
			}
			else if(key<=9||key>=0)
			{
				password[password_counter]=key;
				LCD_displayCharacter('*');
				password_counter++;
			}
			_delay_ms(500); /* Press time */
		}
	}
	LCD_clearScreen();
	while(TRUE)
	{
		if(flag2==TRUE)
		{flag2=FALSE;
		password_counter2=1;
		break;}
		LCD_displayString("plz re_enter the");
		LCD_moveCursor(1,0); /* Move the cursor to the second row */
		LCD_displayString("same pass:");
		/* Get the pressed key number, if any switch pressed for more than 500 ms it will considered more than one press */
		/* Get the pressed key number, if any switch pressed for more than 500 ms it will considered more than one press */
		while(1)
		{
			key = KEYPAD_getPressedKey();
			if(key=='=')
			{
				if(password_counter2!=11)
				{
					flag2=FALSE;
					LCD_clearScreen();
					LCD_moveCursor(0,2);
					LCD_displayString("choose five");
					LCD_moveCursor(1,4);
					LCD_displayString(" digits");
					password_counter2=1;
					_delay_ms(100);
					LCD_clearScreen();
				}
				else
					flag2=TRUE;
				break;
			}
			else if(key<=9||key>=0)
			{
				password[password_counter2]=key;
				LCD_displayCharacter('*');
				password_counter2++;

			}
			_delay_ms(500); /* Press time */
		}
	}
	LCD_clearScreen();
}
void enter_password()
{
	uint8 key;
	uint8 password_counter3=1;
	boolean flag3=FALSE;
	while(1)
	{
		LCD_clearScreen();
		LCD_displayString("please enter");
		LCD_moveCursor(1, 0);
		LCD_displayString(" pass:");
		key = KEYPAD_getPressedKey();
		while(1)
		{
			key = KEYPAD_getPressedKey();
			if(key=='=')
			{
				if(password_counter3!=6)
				{
					flag3=FALSE;
					LCD_clearScreen();
					LCD_moveCursor(0,2);
					LCD_displayString("choose five");
					LCD_moveCursor(1,4);
					LCD_displayString(" digits");
					_delay_ms(100);
					LCD_clearScreen();
					password_counter3=1;
				}
				else
					return;
			}
			else if(key<=9||key>=0)
			{
				password[password_counter3]=key;
				LCD_displayCharacter('*');
				password_counter3++;
			}
			_delay_ms(500); /* Press time */
		}
	}
}
void open_door()
{
	UART_sendByte(4);
	while(g_flag);
	g_flag=TRUE;
}
void run_buzzer()
{
	UART_sendByte(3);
	while(g_flag);
	g_flag=TRUE;
}
int main(void)
{
	/*initialize the LCD*/
	LCD_init();
	/*variable to hold the pressed key*/
	uint8 key;
	/*config the UART*/
	UART_ConfigType UART_config={BITS_8,Disabled,One_bit,BaudRate9600};
	/*initialize the UART*/
	UART_init(&UART_config);
	/*variable to count how many times the user entered a wrong password*/
	uint8 buzzer_counter=0;
	/*holds the number of operations the operations are opening the door and changing the password*/
	uint8 operation_number;
	/*enable global interrupts*/
	SET_BIT(SREG,7);
	while(1)
	{

		restart:
		change_password();
		for(uint8 i=0;i<11;i++)	/*sends the pass to the other MCU*/
		{
			password[0]=1;
			UART_sendByte(password[i]);
			_delay_ms(200);
		}
		while(g_flag);	/*wait for the second MCU to respond with a bite*/
		g_flag=TRUE;	/*make the flag true to use it again*/
		LCD_clearScreen();
		if(g_check)		/*after storing the pass ask user for the operation he want to do*/
		{
			LCD_displayString("+ : open Door");
			LCD_moveCursor(1,0);
			LCD_displayString("- : change pass");
			while(1)
			{
				key=KEYPAD_getPressedKey();
				if(key=='+')
				{
					operation_number=0;
					enter_password();
				}
				else if(key=='-')
				{
					operation_number=1;
					enter_password();

				}
			}
			for(uint8 i=0;i<6;i++)	/*send the pass to the second MCU to compare it with the one stored at the EEPROM*/
			{
				password[0]=2;	/*choosing check operation in the control MCU by sending first byte =2*/
				UART_sendByte(password[i]);
				_delay_ms(200);
			}
			while(g_flag);
			g_flag=TRUE;
			LCD_clearScreen();
			if(g_check)
			{
				switch (operation_number) {
				case 0:
					open_door();	/*if the operation choosed is 0 open the door*/
					break;
				case 1:
					goto restart;	/*if the operation is one start from the bigining*/
					break;
					buzzer_counter=0;
				}
			}
			else
			{

				if(buzzer_counter==3)
				{
					run_buzzer();
				}
				LCD_displayString("wrong pass");
				_delay_ms(200);
				buzzer_counter++;
				enter_password();
			}
		}
		else
		{
			LCD_displayString("NOT MATCHED");
			goto restart;
		}
	}
}
