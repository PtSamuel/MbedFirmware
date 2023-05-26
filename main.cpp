#include "mbed.h"
#include <string>
#include <cmath>
#include <sstream>

#define BUFSIZE 32
#define FREQ_MULTIPLYER 24.0f
#define PWM_PERIOD_US 2000.0f
#define ON_TIME_MIN_US 1100.0f
#define ON_TIME_MAX_US 1900.0f
#define ARM_ON_TIME 1500.0f

AnalogIn ThrottleX(A0);
AnalogIn ThrottleY(A1);
PwmOut led1(LED1);
PwmOut led2(LED2);
PwmOut led3(LED3);
PwmOut led4(LED4);
PwmOut throttle(D6);

static BufferedSerial serial(USBTX, USBRX); // tx, rx
static char buf[BUFSIZE];
static char bufcpy[BUFSIZE];
static size_t curlen = 0;

float map_duty_cycle(float level) { // [-1.0, 1.0]
    if (level > 1.0f || level < -1.0f) 
        return ARM_ON_TIME / PWM_PERIOD_US;
    float ON_TIME = ON_TIME_MIN_US + (level + 1) / 2 * (ON_TIME_MAX_US - ON_TIME_MIN_US);
    return ON_TIME / PWM_PERIOD_US;
}

void parse(size_t len) {
    bufcpy[len - 1] = 0;
    // printf("Command received: %s\n", bufcpy);
    if(len == 1) {
        printf("Empty command.\n");
        return;
    }
    
    std::stringstream stream(bufcpy);
    if(bufcpy[0] != '$') {
        printf("Command must begin with $.\n");
        return;
    }   

    std::string inst; float level; 
    if(stream >> inst >> level) {
        if(level > 1.0f || level < -1.0f)
            printf("Level must be between -1 and 1\n");
        else {
            if(inst == "$L1")
                led1 = level;
            else if(inst == "$L2")
                led2 = level;
            else if(inst == "$L3")
                led3 = level;
            else if(inst == "$L4")
                led4 = level;
            else if(inst == "$LT")
                throttle = map_duty_cycle(level);
            else printf("Invalid instruction\n");

            // printf("Current pulse width: %f us\n", throttle.read_pulsewidth_us() / FREQ_MULTIPLYER);
        }
    } else {
        printf("Invalid format\n");
    }
}

void printbuf() {
    printf("buf content (len = %d): ", curlen);
    for(size_t i = 0; i < curlen; i++)
        if(buf[i] == '\r')
            printf("<newline>");
        else printf("%c", buf[i]);
    printf("\n");
}

int main()
{   
    memset(buf, 0, BUFSIZE);
    printf("Started listening for command.\n");
    
    throttle.period_us((int)PWM_PERIOD_US);
    float period_us = (float)throttle.read_period_us() / FREQ_MULTIPLYER;
    float period_s = period_us * 1e-6;
    printf("Throttle PWD frequency: %f Hz, period: %f us\n", 1 / period_s, period_us);

    while (1) {
        if(curlen == BUFSIZE)
            serial.read(buf + curlen - 1, 1);
        else
            serial.read(buf + curlen++, 1);
        

        if(buf[curlen - 1] == '\r') {
            std::memcpy(bufcpy, buf, curlen);
            parse(curlen);
            curlen = 0;
        } else if(buf[curlen - 1] == 127) {
            if(curlen > 1)
                curlen -= 2;
            else curlen = 0;
        }

        // printbuf();

    }
}