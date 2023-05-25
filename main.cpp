#include "mbed.h"
#include <string>
#include <cmath>
#include <sstream>

#define BUFSIZE 32
#define FREQ_MULTIPLYER 24

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
        // printf("inst: %s, level: %f\n", inst.c_str(), level);
        if(level > 1.0f || level < 0.0f)
            printf("Level must be between 0 and 1\n");
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
                throttle = level;
            else printf("Invalid instruction\n");
            printf("current pulse width: %d us\n", throttle.read_pulsewidth_us() / FREQ_MULTIPLYER);
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
    
    throttle.period_us(2000);
    float period_us = throttle.read_period_us() / FREQ_MULTIPLYER;
    float period_s = period_us  * 1e-6;
    printf("Throttle PWD frequency: %f Hz, period: %f us\n", 1 / period_s, period_us);

    while (1) {
        if(curlen == BUFSIZE)
            serial.read(buf + curlen - 1, 1);
        else
            serial.read(buf + curlen++, 1);
        

        if(buf[curlen - 1] == '\r') {
            // printf("new line detected\n");
            // printf("\n"); // new line from buf content line
            std::memcpy(bufcpy, buf, curlen);
            parse(curlen);
            curlen = 0;
        } else if(buf[curlen - 1] == 127) { // backspace
            if(curlen > 1)
                curlen -= 2;
            else curlen = 0;
        }

        printbuf();

        // size_t buflen = std::strlen(buf);
        // if(buflen > 0)
        //     printf("Receiving: %s\n", buf);
        // ThisThread::sleep_for(100ms);
    }
}

/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

// #include "mbed.h"

// static BufferedSerial pc(USBTX, USBRX); // tx, rx
// PwmOut led(LED1);

// float brightness = 0.0;

// int main()
// {
//     char msg[] = "Press 'u' to turn LED1 brightness up, 'd' to turn it down\n";
//     char *c = new char[1];
//     pc.write(msg, sizeof(msg));

//     while (1) {
//         pc.read(c, sizeof(c));
//         pc.write(c, sizeof(c));
//         if ((*c == 'u') && (brightness < 0.5)) {
//             brightness += 0.01;
//             led = brightness;
//         }
//         if ((*c == 'd') && (brightness > 0.0)) {
//             brightness -= 0.01;
//             led = brightness;
//         }
//     }

// }