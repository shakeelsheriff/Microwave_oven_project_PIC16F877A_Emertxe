/*
 * File:   main.c
 * Author: Shakeel
 *
 * Created on 21 September, 2023, 10:25 AM
 */


#include "main.h"
#pragma config WDTE = OFF //Watchdog timer disabled

unsigned char sec=0, min=0, flag =0;
int operation_flag = POWER_ON_SCREEN;   // power on screen
static void init_config(void) {
    //Write your initialization code here
    init_clcd();
    init_matrix_keypad();
    
    //RC2 as output
    FAN_DDR = OFF;
    FAN= OFF;// turn off fan
    
    //initialization of timer2 module
    init_timer2();
    
    PEIE = 1;
    GIE = 1;
    
    BUZZER = OFF;
    BUZZER_DDR = OFF;
    
    DOOR_DDR = 1;//Input
    
}

void main(void) {
    init_config(); //Calling initializing function
    
    //Variable Declaration
    unsigned char key;
    int reset_flag;
    while (1) {
        key = read_matrix_keypad(STATE);
        
        // Check the status of the door
        if (DOOR == OPEN) {
            // The door is open, take appropriate action
            door_status();
        }
         // Rest of existing code for handling keypad input and operations
        if(operation_flag == MENU_DISPLAY_SCREEN)
        {
            //SW 1 pressed 
        if(key == 1)//Micro mode
        {
            operation_flag = MICRO_MODE;
            reset_flag  = RESET_MODE;
            clear_screen();
            clcd_print(" power = 900W ",LINE2(0));
            __delay_ms(2000); //2sec
            clear_screen();

        }
        else if(key == 2)//Grill
        {
            operation_flag = GRILL_MODE;
            reset_flag = RESET_MODE;
            clear_screen();
        }
        else if(key == 3)//Convection
        {
            operation_flag = CONVECTION_MODE;
            reset_flag = RESET_MODE;
            clear_screen();
        }
        else if(key == 4)//Start Mode
        {
            sec = 30;
            min = 0;
            FAN = ON;
            TMR2ON = ON;
            clear_screen();
            operation_flag = TIME_DISPLAY;
        }
        }
         else if(operation_flag == TIME_DISPLAY)
        {
            if(key == 4) // START/RESUME
            {
                sec = sec + 30;
                if(sec > 59)
                {
                    min++;
                    sec = sec -60;
                }
            }
            else if(key == 5) // PAUSE
            {
                operation_flag = PAUSE;
            }
            else if(key == 6) // RESUME
            {
                operation_flag = STOP;
                clear_screen();
            }
        }
        else if(operation_flag == PAUSE)
        {
            if (key ==4) // RESUME
            {
                FAN = ON;
                TMR2ON = ON;
                operation_flag = TIME_DISPLAY;
            }
        }
        
        
        switch(operation_flag)
        {
            case POWER_ON_SCREEN:
                
                power_on_screen();
                operation_flag = MENU_DISPLAY_SCREEN; //menu display screen
                clear_screen();
                break;
            case MENU_DISPLAY_SCREEN:
                menu_display_screen();
                break;
            case GRILL_MODE:
                set_time(key,reset_flag);
                break;
            case MICRO_MODE:
                set_time(key,reset_flag);
                break;
            case CONVECTION_MODE:
                
                if(flag == 0)
                {
                    set_temp(key,reset_flag);
                    if(flag == 1)
                    {
                        clear_screen();
                        reset_flag = RESET_MODE;
                        continue;
                    }
                }
                else if(flag == 1)
                {
                    set_time(key, reset_flag);
                }
                break;
            case TIME_DISPLAY:
                time_display_screen();
                break;
            case PAUSE:
                FAN = OFF;
                TMR2ON = OFF;
                break;
            case STOP:
                FAN = OFF;
                TMR2ON = OFF;
                operation_flag = MENU_DISPLAY_SCREEN;
                break;
        }
        reset_flag = RESET_NOTHING;
    }
}
void door_status(void)
{
    if(DOOR == OPEN)
    {
        TMR2ON = OFF;
        FAN = OFF;
        BUZZER = ON;
        clear_screen();
        clcd_print("DOOR is Open",LINE2(0));
        clcd_print("Please Close",LINE3(0));
        //Wait till Door Closed
        while(DOOR == OPEN) //RB3 =1
        {
            ;
        }
        BUZZER = OFF;
        TMR2ON = ON;
        FAN = ON;
        clear_screen();
    }
}
void time_display_screen(void)
{
    door_status();
    /*Line1 display*/
    clcd_print("TIME = ",LINE1(0));
    //print min and sec
    //min
    clcd_putch(min/10 + '0', LINE1(9));
    clcd_putch(min%10 + '0', LINE1(10));
    clcd_putch(':', LINE1(11));
    //sec
    clcd_putch(sec/10 + '0', LINE1(12));
    clcd_putch(sec%10 + '0', LINE1(13));
    
    //print options
    clcd_print(" 4.Start/Resume",LINE2(0));
    clcd_print(" 5.Pause",LINE3(0));
    clcd_print(" 6.Stop",LINE4(0));
    
    if(sec == 0 && min == 0)
    {
        clear_screen();
        clcd_print(" Time Up! ", LINE2(0));
        BUZZER = ON;
        __delay_ms(3000); // 3 sec
        clear_screen();
        BUZZER = OFF;
        FAN = OFF;
        TMR2ON = OFF;
        operation_flag = MENU_DISPLAY_SCREEN;
    }
}

void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}
void power_on_screen(void)
{
    unsigned char i;
    //line1
    for(i=0; i<16; i++)
    {
        clcd_putch(BAR, LINE1(i)); //i=0
        __delay_ms(100);
    }
    clcd_print("  Powering ON  ", LINE2(0));
    clcd_print(" Microwave Oven ", LINE3(0));
    //line4
    for(i=0; i<16; i++)
    {
        clcd_putch(BAR, LINE4(i)); //i=0
        __delay_ms(100);
    }

        //Delay after transition from power ON Screen
    __delay_ms(3000);
}

void menu_display_screen(void)
{
    clcd_print("1.Micro",LINE1(0)); //Line1
    clcd_print("2.Grill",LINE2(0));//Line2
    clcd_print("3.Convection",LINE3(0));//Line3
    clcd_print("4.Start",LINE4(0));//Line4
}

void set_temp(unsigned char key, int reset_flag)
{
    static unsigned char key_count, blink, temp;
    static int wait;
    if(reset_flag == RESET_MODE)
    {
        key_count = 0;
        wait = 0;
        blink = 0;
        temp = 0;
        key = ALL_RELEASED;
        clcd_print("SET TEMP ( C) ", LINE1(0));
        clcd_putch(DEGREE, LINE1(10));
        clcd_print("TEMP = ", LINE2(0));
        clcd_print("(LIMIT : 180 C) ", LINE3(0));
        clcd_putch(DEGREE, LINE3(12));
        clcd_print("*:CLEAR  #:ENTER",LINE4(0));
    }
       if((key!='*')&& (key != '#')&& (key!= ALL_RELEASED))
    {
        key_count++;
        if(key_count <=3)// reading number of sec
        {
        temp = temp * 10 + key;
        }
    }
    else if(key == '*')
    {
        temp=0;
        key_count=0;
    }
    else if(key == '#')
    {
        if(temp > 180 || temp==0)
        {  
            clear_screen();
            clcd_print("  Temp. Invalid  ", LINE2(0));
            __delay_ms(2000);
            clear_screen();
            operation_flag = MENU_DISPLAY_SCREEN;   
        }
        else if(temp <= 180 && temp!=0)
        {
        clear_screen();
       //Pre-heating
        clcd_print("  Pre-Heating  ", LINE1(0));
        clcd_print(" Time Rem.= ", LINE3(0));
        FAN = ON;
        TMR2ON = 1;
        sec = 180;
        while(sec!=0)
        {
        // to enter Time
        clcd_putch((sec/100) + '0', LINE3(11));
        clcd_putch((sec/10)%10 + '0', LINE3(12));
        clcd_putch((sec%10) + '0', LINE3(13));
        }
        if(sec == 0)
        {
           flag = 1;
           clear_screen();
           clcd_print("  Set the Time  ", LINE2(0));
           BUZZER = ON;
           __delay_ms(3000); // 3 sec
           clear_screen();
           FAN = OFF;
           BUZZER = OFF;
           TMR2ON = 0;
            //Set time screen exactly like grill mode
        }
        }
    }
    if(wait++ ==15)
    {
        wait =0;
        blink =!blink;
        // Printing Temp on set temp screen
        // to enter Temperature
        clcd_putch((temp/100) + '0', LINE2(8));
        clcd_putch((temp/10)%10 + '0', LINE2(9));
        clcd_putch((temp%10) + '0', LINE2(10));
    }
    if(blink)
    {
        clcd_print("   ", LINE2(8));         
    }
    
}
void set_time(unsigned char key, int reset_flag)
{
    static unsigned char key_count, blink_pos,blink;
    static int wait;
    if(reset_flag == RESET_MODE)
    {
        key_count = 0;
        sec =0;
        min =0;
        blink_pos =0;
        wait =0;
        blink=0;
        key =ALL_RELEASED;
        clcd_print("SET TIME (MM:SS)", LINE1(0));
        clcd_print("TIME- ", LINE2(0));
        clcd_print("*:CLEAR  #:ENTER",LINE4(0));
    }
    
    
    if((key!='*')&& (key != '#')&& (key!= ALL_RELEASED))
    {
        key_count++;
        if(key_count <=2)// reading number of sec
        {
        sec = sec * 10 + key;
        blink_pos=0;
        }
        else if(key_count >2 && key_count <= 4)
        {
            min = min*10 +key;
            blink_pos=1;
        }
    }
    else if(key == '*')
    {
        if(blink_pos == 0)
        {
            sec = 0;
            key_count =0;
        }
        else if(blink_pos == 1)
        {
            min = 0;
            key_count = 2;
        }
    }
    else if(key == '#')
    {
       flag = 0;
       clear_screen();
       operation_flag = TIME_DISPLAY;
       FAN = ON; //turn on fan
       /*Switching on Timer2*/
       TMR2ON = ON;
    }
    if(wait++ ==15)
    {
        wait =0;
        blink =!blink;
        //min
    clcd_putch(min/10 + '0', LINE2(6));
    clcd_putch(min%10 + '0', LINE2(7));
    clcd_putch(':', LINE2(8));
    //sec
    clcd_putch(sec/10 + '0', LINE2(9));
    clcd_putch(sec%10 + '0', LINE2(10));
    }
    if(blink)
    {
        switch(blink_pos)
        {
            case 0: //sec
                clcd_print("  ", LINE2(9));
                break;
            case 1://min
                clcd_print("  ", LINE2(6));
                break;
            
        }
        

    }
    
    
    
    
    
}