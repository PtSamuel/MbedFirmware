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
#define delim ' '

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
static BufferedSerial ping2(p13, p14); // tx, rx
static char buf[BUFSIZE];
static char bufcpy[BUFSIZE];
static size_t curlen = 0;

static I2C i2c(p28, p27);

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

// len = length of buffer WITH \r
void parse(size_t len) {
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

int main()
{   

    ThisThread::sleep_for(500ms);

    i2c.start();
    printf("Status: %d\n", i2c.write(0x27 << 1));
    i2c.stop();

    char c;
    
    i2c.start();
    printf("Status: %d\n", i2c.write((0x27 << 1) & 1));
    c = i2c.read(1);
    printf("Read: 0x%02X\n", c);
    c = i2c.read(1);
    printf("Read: 0x%02X\n", c);
    i2c.stop();

    i2c.start();
    printf("Status: %d\n", i2c.write((0x27 << 1) & 1));
    c = i2c.read(1);
    printf("Read: 0x%02X\n", c);
    c = i2c.read(1);
    printf("Read: 0x%02X\n", c);
    i2c.stop();

    i2c.start();
    printf("Status: %d\n", i2c.write((0x27 << 1) & 1));
    c = i2c.read(1);
    printf("Read: 0x%02X\n", c);
    c = i2c.read(1);
    printf("Read: 0x%02X\n", c);
    i2c.stop();

    i2c.start();
    printf("Status: %d\n", i2c.write((0x27 << 1) & 1));
    c = i2c.read(1);
    printf("Read: 0x%02X\n", c);
    c = i2c.read(1);
    printf("Read: 0x%02X\n", c);
    i2c.stop();




    // char data[4] = {0, 0, 0, 0};
    // i2c.write(0x27, data, 1);
    // i2c.read(0x27, data, 4);
    // printf("[0x%02X, 0x%02X, 0x%02X, 0x%02X]\n", data[0], data[1], data[2], data[3]);    

    // char data[4] = {0, 0, 0, 0};
    // for(int i = 0; i < 128; i++) {
    //     printf("%x Write status: %d\n", i, i2c.write(i, data, 0));
    // }

    // ThisThread::sleep_for(5ms);

    // ping2.set_baud(115200);
    // printf("Starting serial...\n");

    // char msg1[] = {0x42, 0x52, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x00, 0xa1, 0x00};
    // for(size_t i = 0; i < 12; i++)
    //     ping2.write(&msg1[i], 1);
    
    // char c;
    // while(ping2.readable()) {
    //     ping2.read(&c, 1); 
    //     printf("Receiving byte: ");
    //     printf("0x%02X\n", (int)c);
    //     printf("ping2 readable: %d\n", ping2.readable());
    // }

    // printf("Sending more...\n");

    // char msg2[] = {0x42, 0x52, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0xa0, 0x00};
    // for(size_t i = 0; i < 12; i++)
    //     ping2.write(&msg2[i], 1);
    
    // while(ping2.readable()) { 
    //     ping2.read(&c, 1); 
    //     printf("Receiving byte...\n");
    //     printf("0x%02X\n", (int)c);
    // }

    // memset(buf, 0, BUFSIZE);
    // printf("Started listening for command.\n");
    
    // throttle_1.period_us((int)PWM_PERIOD_US);
    // float period_us = (float)throttle_1.read_period_us() / FREQ_MULTIPLYER;
    // float period_s = period_us * 1e-6;
    // printf("Throttle PWD frequency: %f Hz, period: %f us\n", 1 / period_s, period_us);

    // while (1) {
    //     if(curlen == BUFSIZE)
    //         serial.read(buf + curlen - 1, 1);
    //     else
    //         serial.read(buf + curlen++, 1);
        

    //     if(buf[curlen - 1] == '\r') {
    //         std::memcpy(bufcpy, buf, curlen);
    //         parse(curlen);
    //         curlen = 0;
    //     } else if(buf[curlen - 1] == 127) {
    //         if(curlen > 1)
    //             curlen -= 2;
    //         else curlen = 0;
    //     }

    //     // printbuf();

    // }
}