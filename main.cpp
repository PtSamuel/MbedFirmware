#include "mbed.h"
#include <chrono>
#include <string>
#include <cmath>
#include <sstream>

#define BUFSIZE 32
#define FREQ_MULTIPLYER 24.0f
#define PWM_PERIOD_US 2000.0f
#define ON_TIME_MIN_US 1100.0f
#define ON_TIME_MAX_US 1900.0f
#define ARM_ON_TIME 1500.0f
#define delim ' '
#define ALTITUDE_RESPONSE_LENGTH 15

PwmOut led_1(LED1);
PwmOut led_2(LED2);
PwmOut led_3(LED3);
PwmOut led_4(LED4);
PwmOut throttle_1(p26);
PwmOut throttle_2(p25);
PwmOut throttle_3(D4);
PwmOut throttle_4(D5);
PwmOut throttle_5(D6);
PwmOut throttle_6(D7);

/**
    $LT x1, x2, x3, x4, x5, x6
    $LED1 x
    $L 
 */

static BufferedSerial serial(USBTX, USBRX); // tx, rx
static BufferedSerial ping2(p28, p27, 115200); // tx, rx
static char buf[BUFSIZE];
static char bufcpy[BUFSIZE];
static size_t curlen = 0;

// static I2C i2c(p28, p27);

float map_duty_cycle(float level) { // [-1.0, 1.0]
    if (level > 1.0f || level < -1.0f) 
        -ARM_ON_TIME / PWM_PERIOD_US;
    float ON_TIME = ON_TIME_MIN_US + (level + 1) / 2 * (ON_TIME_MAX_US - ON_TIME_MIN_US);
    return ON_TIME / PWM_PERIOD_US;
}

void led_controller(PwmOut& led, std::stringstream& stream) {
    float level;
    if(stream >> level) {
        if(level > 1.0f || level < -1.0f)
            printf("Level must be between -1 and 1\n");
        else
            led = level;
    } else {
        printf("Invalid level.\n");
    }
}

void throttle_controller(std::stringstream& stream) {
    float t1, t2, t3, t4, t5, t6;
    if(stream >> t1 >> t2 >> t3 >> t4 >> t5 >> t6) {
        throttle_1 = map_duty_cycle(t1);
        throttle_2 = map_duty_cycle(t2);
        throttle_3 = map_duty_cycle(t3);
        throttle_4 = map_duty_cycle(t4);
        throttle_5 = map_duty_cycle(t5);
        throttle_6 = map_duty_cycle(t6);
    } else printf("Conversion failure.\n");
}

static inline unsigned long now_ms() {
    auto time = std::chrono::time_point_cast<std::chrono::milliseconds>(Kernel::Clock::now());
    unsigned long time_ms = time.time_since_epoch().count(); 
    return time_ms;
}

static bool measure_altitude(int *depth, float *confidence) {
    char query[] = {0x42, 0x52, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0xbb, 0x04, 0x5b, 0x01};
    for(size_t i = 0; i < 12; i++)
        ping2.write(&query[i], 1);

    char response[ALTITUDE_RESPONSE_LENGTH];

    for(size_t i = 0; i < ALTITUDE_RESPONSE_LENGTH; i++) {
        unsigned long start = now_ms();
        while(!ping2.readable()) {
            if(now_ms() - start > 500) {
                printf("Cannot receive next byte...\n");
                return false;
            }
        }
        ping2.read(response + i, 1);
    }

    *depth = *((int*)(response + 8));
    *confidence = (float)(response[12]) / 256.0f;

    return true;
}

static void report_altitude() {
    int depth; float confidence;
    if(measure_altitude(&depth, &confidence))
        printf("[%lu] depth: %d mm, confidence: %f\n", now_ms(), depth, confidence);
}

// len = length of buffer WITH \r
static void parse(size_t len) {
    bufcpy[len - 1] = 0;
    if(len == 1) {
        printf("Empty command.\n");
        return;
    } else printf("Command received: %s\n", bufcpy);
    
    std::stringstream stream(bufcpy);
    string command;
    float level;

    if(stream >> command) {
        if(command == "$L1")
            led_controller(led_1, stream);
        else if(command == "$L2")
            led_controller(led_2, stream);
        else if(command == "$L3")
            led_controller(led_3, stream);
        else if(command == "$L4")
            led_controller(led_4, stream);
        else if(command == "$LT")
            throttle_controller(stream);
        else if(command == "$ALT")
            report_altitude();
        else printf("Unrecognized command: %s\n", command.c_str());
    } else printf("Invalid command\n");
}

void printbuf() {
    printf("buf content (len = %d): ", curlen);
    for(size_t i = 0; i < curlen; i++)
        if(buf[i] == '\r')
            printf("<newline>");
        else printf("%c", buf[i]);
    printf("\n");
}

uint16_t checksum(char* arr, size_t len) {
    uint16_t sum = 0;
    for(size_t i = 0; i < len; i++)
        sum += arr[i];
    return sum;
}

void pwd_info() {
    printf("Started listening for command.\n");
    throttle_1.period_us((int)PWM_PERIOD_US);
    float period_us = (float)throttle_1.read_period_us() / FREQ_MULTIPLYER;
    float period_s = period_us * 1e-6;
    printf("Throttle PWD frequency: %f Hz, period: %f us\n", 1 / period_s, period_us);
}

int main()
{   
    memset(buf, 0, BUFSIZE);
    
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