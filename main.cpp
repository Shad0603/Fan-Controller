#include "mbed.h"
#include "TextLCD.h"
#include <cstdio>
#include <chrono>
#include <ctime>
#include <iterator>

#define MAX_SPEED 2000;

TextLCD lcd(PB_15, PB_14, PB_10, PA_8, PB_2, PB_1);

// Pin declarations
InterruptIn EncA(PA_1);
InterruptIn EncB(PA_4);
DigitalOut led_1(PA_15);
DigitalOut led(PB_7);
DigitalIn button(BUTTON1);
InterruptIn tacho(PA_0);
PwmOut fan(PB_0);

Timer tacho_t;
Timer lcd_out_t;
Timer pid_calc_t; 

// Define the control mode as open loop by default
bool closed_loop_mode = false;

// structures
struct encoder_states{
   int current_state;
   int prev_state;
};

struct encoder_states s;

const int time_period_threshold = 15000; // 15000us threshold

// fan speed variables
int RPM = 0;
int set_rpm = 0;
float set_dc = 0.0; 
int curr_rpm = 0; 
float curr_dc = 0;


int timeSinceLastPulse = 0;

const int num_samples = 10;
int samples[num_samples];
//int curr_sample = 0;
//int sum = 0;

int time_between_tacho_readings = 0;

void flip(){
    // For Closed Loop Control mode turn on the REDLED
    led_1 = 1;
    led = 0;
}

int tacho_output(){
    //pulses++; // increment no. of pulses per second
    timeSinceLastPulse = tacho_t.elapsed_time().count();
    if (timeSinceLastPulse >= time_period_threshold){
        int timeBetweenTwoPulses = timeSinceLastPulse * 2; // there are 2 pulses per revolution
        RPM = 60000000 / timeBetweenTwoPulses;
        //curr_dc = RPM / MAX_SPEED ;
    }
    // read peiod of tacho signal
    time_between_tacho_readings = lcd_out_t.elapsed_time().count();
    // reset the tacho timer
    tacho_t.reset();
    // reset lcd output timer
    lcd_out_t.reset(); 
    // -----------------Moving Average Filter implementation --------------
    // define variables for Moving Average Filter
    static int sum; 
    static int curr_sample;
    // Add the new sample to the sum and subtract the oldest sample
    sum = sum + RPM - samples[curr_sample];
    // Update the sample array with the new input
    samples[curr_sample] = RPM;
    // Increment the current sample index and wrap around if necessary
    curr_sample = (curr_sample + 1) % num_samples;
    // Calculate the moving average RPM
    curr_rpm = sum / num_samples;
    // -------------------------------------------------------------------
    curr_dc = curr_rpm / MAX_SPEED;
    return curr_rpm, curr_dc;
}


// ----------------------- Encoder Reading Function -----------------------------------
int encoder(){
    
    s.current_state = EncA;
    if (s.prev_state != s.current_state){
        if (EncB != s.current_state){
            set_rpm += 14; //when the encoder is turned clockwise, increase set_rpm by 14
            set_dc = (float)set_rpm / MAX_SPEED;
            
        }
        else {
            set_rpm -= 14; //when the encoder is turned antclockwise, decrease set_rpm by 14
            set_dc = (float)set_rpm / MAX_SPEED;
            
        }
        if (set_rpm < 600){
            set_rpm = 600;
            set_dc = (float)set_rpm / MAX_SPEED;
            
        }
        if (set_rpm >= 2000){
            set_rpm = 2000; 
            set_dc = (float)set_rpm / MAX_SPEED;
           
        }
    s.prev_state = s.current_state;
    
    }
    return set_rpm;
} 
// --------------------------------------------------------------------------------

// PID global variables
//float _error = 0;
float integral = 0;
float derivative = 0;
float prev_error = 0;

// -------------------------- PID control function  ----------------------------------
float pid_control(int curr_rpm, int set_rpm){
    pid_calc_t.start();
    // Define variables for PID control
    const float kp = 1.2; //could use 1.2 for P value??
    const float ki = 0.0;
    const float kd = 0.0;

    // Calculate error between desired and current speeds
    int _error = set_rpm - curr_rpm;

    // Read time taken for iteration in us
    int time_since_last_iteration = pid_calc_t.elapsed_time().count();

    // Calculate integral and derivative
        // dt = time_since_last_iteration;
    integral += (_error * time_since_last_iteration); 
    derivative = (_error - prev_error) / (time_since_last_iteration);

    // Calculate output value using PID control formula
    int output = set_rpm + kp * _error + ki * integral + kd * derivative;
    //printf("pid output: %d\n", output);
    float pid_dc = output / MAX_SPEED;


    // Save error for next iteration
    prev_error = _error;

    // reset pid timer
    pid_calc_t.reset();

    return pid_dc;
    
}

// ------------------------------------------------------------------------------

//float open_loop_control(float set_dc){
//    float output = desired_dc;
//    return output;
//}

// ----------------------------------------- Main Loop ---------------------------------
int main(){
    tacho_t.start();
    lcd_out_t.start();

    // set fan speed to 0% duty cycle
    fan.write(0.0f); 

    // read encoder and use encoder call back function to increase/reduce speed
    EncA.rise(&encoder);
    EncB.rise(&encoder);
    EncA.fall(&encoder);
    EncB.fall(&encoder);

    // callback function for tachometer
    tacho.fall(&tacho_output);
   

    button.mode(PullUp);

    
    while(true){
        
        // Set period of the fan
        fan.period(0.01f);   
        fan.write(set_dc);
        // For Open Loop Control mode turn on the GREEN LED
        led_1 = 0;
        led = 1;

        // Calculate output using PID control algorithm
        float pid_dc = pid_control(curr_rpm, set_rpm);

        // Button function to switch between closed loop and open loop control
        //if (!button){
            //flip();
            //closed_loop_mode = true;

        //}
    
        if (time_between_tacho_readings >= 1000){
            lcd.locate(0, 0);
            lcd.printf("True RPM:%d\n", curr_rpm);
            
        }
        
        lcd.locate(0, 1);
        lcd.printf("Set RPM:%d\n", set_rpm);

    }
    
}

