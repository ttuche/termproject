#include <stdio.h>
#include <stdlib.h>
#include <pigpiod_if2.h>
#include <unistd.h>
#include <termios.h>


#define TRIG_PINN0 13 // 13 Left
#define ECHO_PINN0 16 // 16

#define TRIG_PINN1 19 // 19 Front
#define ECHO_PINN1 20 // 20

#define TRIG_PINN2 26 // 26 Right 
#define ECHO_PINN2 21 // 21


#define PINN0 23 // 23  pwm0
#define PINN1 24 // 24  pwm1

#define DUTYCYCLE(x,range) x/(float)range*100


#define INPUT1 5 
#define INPUT2 6 
#define INPUT3 17 
#define INPUT4 27 



int i=0;
int pi;
int default_range = 100;

int range1; // left
int range2; // right
int f_left = 40;
int f_right = 38;

float distance1;   //left
float distance2;   //forward
float distance3;   //right

uint32_t start_tick_[3], dist_tick_[3];

void trigger(void);
void cb_func_echo0(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_echo1(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_echo2(int pi, unsigned gpio, unsigned level, uint32_t tick);
void forward(void);
void backward(void);
void left(void);
void right(void);
void stop(void);



int main(void) 
{ 

        if((pi = pigpio_start(NULL, NULL)) < 0){ 
                fprintf(stderr, "%s\n", pigpio_error(pi));
                    exit(-1); 
        } 

        set_PWM_range(pi, PINN0, default_range); 
        set_PWM_range(pi, PINN1, default_range); 
            
      

        //모터 pin 설정
        set_mode(pi, INPUT1, PI_OUTPUT);   set_mode(pi, INPUT2, PI_OUTPUT); 
        set_mode(pi, INPUT3, PI_OUTPUT);   set_mode(pi, INPUT4, PI_OUTPUT);
       
        gpio_write(pi, INPUT1, PI_LOW);     gpio_write(pi, INPUT2, PI_LOW);
        gpio_write(pi, INPUT3, PI_LOW);     gpio_write(pi, INPUT4, PI_LOW);
        time_sleep(1);
        //센서 설정
        set_mode(pi, TRIG_PINN0, PI_OUTPUT);  set_mode(pi, ECHO_PINN0, PI_INPUT);
        set_mode(pi, TRIG_PINN1, PI_OUTPUT);  set_mode(pi, ECHO_PINN1, PI_INPUT);
        set_mode(pi, TRIG_PINN2, PI_OUTPUT);  set_mode(pi, ECHO_PINN2, PI_INPUT);


        callback(pi, ECHO_PINN0, EITHER_EDGE, cb_func_echo0); 
        gpio_write(pi, TRIG_PINN0, PI_OFF);

        callback(pi, ECHO_PINN1, EITHER_EDGE, cb_func_echo1);
        gpio_write(pi, TRIG_PINN1, PI_OFF);
        
        callback(pi, ECHO_PINN2, EITHER_EDGE, cb_func_echo2);
        gpio_write(pi, TRIG_PINN2, PI_OFF);
        time_sleep(1);     // delay 1 second
        printf("Raspberry Pi HC-SR04 UltraSonic sensor\n" );
  
       
       // set_PWM_dutycycle(pi, PINN0, 0); 
       // set_PWM_dutycycle(pi, PINN1, 0);      
       // usleep();

        while(1){
        for(i = 0; i < 3; i++)
        {
             start_tick_[i] = dist_tick_[i]= 0;
        }
        gpio_trigger(pi, TRIG_PINN0, 10, PI_HIGH);
        gpio_trigger(pi, TRIG_PINN1, 10, PI_HIGH); 
        gpio_trigger(pi, TRIG_PINN2, 10, PI_HIGH); 
        time_sleep(0.05);

        //left센서
        if(dist_tick_[0] && start_tick_[0]){
            distance1 = dist_tick_[0] / 1000000. * 340 / 2 * 100;
            if(distance1 < 2 || distance1 > 400)
                distance1 = -1;             

        }
        else
              distance1 = -2;
    

        //forward센서
        if(dist_tick_[1] && start_tick_[1]){
            distance2 = dist_tick_[1] / 1000000. * 340 / 2 * 100;      
            if(distance2 < 2 || distance2 > 400)
                
                 distance2=-1;
        }                          
        else        
             distance2 = -2;                      
        
        //right센서
        if(dist_tick_[2] && start_tick_[2]){
            distance3 = dist_tick_[2] / 1000000. * 340 / 2 * 100;
             if(distance3 < 2 || distance3 > 400)
               
                distance3 = - 1;
        }
        else
             distance3 = -2;
            
       printf("left:%6dus, %6.1f cm forward:%6dus, %6.1fcm right:%6dus, %6.1fcm\n", dist_tick_[0],distance1, dist_tick_[1], distance2, dist_tick_[2], distance3);
          
      
      float dis=7.5;
  //  float dis2=8;
      float error_L = distance1 - dis; 
      float error_R = distance3 - dis;

      if(distance2 < 10)    
      {
         stop();
 //       time_sleep(0.1);
         if(error_R>0)
             left();
         else if(error_L >0)
             right();
      }
    else
    {   
        forward();
    
       if( error_R>2.5)
           set_PWM_dutycycle(pi,PINN1,39);
       if( error_L>2.5)
           set_PWM_dutycycle(pi,PINN1,37);
    }
        }            
      pigpio_stop(pi);
      return 0;
}




void forward(void)
{
       set_PWM_dutycycle(pi, PINN0,f_right); 
       set_PWM_dutycycle(pi, PINN1,f_left); 
        
        gpio_write(pi, INPUT1, PI_HIGH);    
        gpio_write(pi, INPUT2, PI_LOW); 
        gpio_write(pi, INPUT3, PI_HIGH);    
        gpio_write(pi, INPUT4, PI_LOW); 
}

void backward(void)
{
        set_PWM_dutycycle(pi, PINN0, 30); 
        set_PWM_dutycycle(pi, PINN1, 30);

        gpio_write(pi, INPUT1, PI_LOW);    
        gpio_write(pi, INPUT2, PI_HIGH); 
        gpio_write(pi, INPUT3, PI_LOW);    
        gpio_write(pi, INPUT4, PI_HIGH); 
}

void left(void)
{
        set_PWM_dutycycle(pi, PINN0, 35); 
        set_PWM_dutycycle(pi, PINN1, 35);

        gpio_write(pi, INPUT1, PI_LOW);    
        gpio_write(pi, INPUT2, PI_HIGH); 
        gpio_write(pi, INPUT3, PI_HIGH);    
        gpio_write(pi, INPUT4, PI_LOW); 
}

void right(void)
{
        set_PWM_dutycycle(pi, PINN0, 25); 
        set_PWM_dutycycle(pi, PINN1, 25);

        gpio_write(pi, INPUT1, PI_HIGH);    
        gpio_write(pi, INPUT2, PI_LOW); 
        gpio_write(pi, INPUT3, PI_LOW);    
        gpio_write(pi, INPUT4, PI_HIGH); 
}

void stop(void)
{
       set_PWM_dutycycle(pi, PINN0, 0); 
       set_PWM_dutycycle(pi, PINN1, 0);

       gpio_write(pi, INPUT1, PI_LOW);
       gpio_write(pi, INPUT2, PI_LOW); 
       gpio_write(pi, INPUT3, PI_LOW);    
       gpio_write(pi, INPUT4, PI_LOW); 
}


void cb_func_echo0(int pi, unsigned gpio, unsigned level, uint32_t tick)                
{ 
       if(level == PI_HIGH)
       
             start_tick_[0] = tick; 
       
       else if(level == PI_LOW)
       
             dist_tick_[0] = tick - start_tick_[0];
       
}


void cb_func_echo1(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
         if(level == PI_HIGH)
                  start_tick_[1] = tick; 
         else if(level == PI_LOW)
                   dist_tick_[1] = tick - start_tick_[1];
}

void cb_func_echo2(int pi, unsigned gpio, unsigned level, uint32_t tick) 
{ 
        if(level == PI_HIGH)
                  start_tick_[2] = tick; 
        else if(level == PI_LOW)
                  dist_tick_[2] = tick - start_tick_[2];
}


