/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueue_Btn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;


void btn_callback(uint gpio, uint32_t events){
    if (events == 0x4) { // fall edge
        xQueueSend(xQueue_Btn, &gpio, 0);
    }
}


void btn_task(void* p) {
    // GPIO BUTTON PINS SETUP
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R,GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y,GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    // CALLBACKS
    gpio_set_irq_enabled_with_callback(BTN_PIN_R,GPIO_IRQ_EDGE_FALL,true,&btn_callback);
    gpio_set_irq_enabled_with_callback(BTN_PIN_Y,GPIO_IRQ_EDGE_FALL,true,&btn_callback);


    int btn = 0;
    while (true) {
        if (xQueueReceive(xQueue_Btn,&btn,pdMS_TO_TICKS(100))){
            printf("%d\n",btn);
            if (btn==BTN_PIN_R) xSemaphoreGiveFromISR(xSemaphoreLedR,0);
            if (btn==BTN_PIN_Y) xSemaphoreGiveFromISR(xSemaphoreLedY,0);
        }
    }
}

void led_r_task(void* p){
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R,GPIO_OUT);

    int led_state = 0;

    while(true){
        if (xSemaphoreTake(xSemaphoreLedR,pdMS_TO_TICKS(100)) == pdTRUE){
            led_state = !led_state;
            // if(led_state){
            //     gpio_put(LED_PIN_R, 0);
            // }
        }

        if (led_state){
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

    }
}

void led_y_task(void* p){
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y,GPIO_OUT);

    int led_state = 0;

    while(true){
        if (xSemaphoreTake(xSemaphoreLedY,pdMS_TO_TICKS(100)) == pdTRUE){
            led_state = !led_state;
            // if(led_state){
            //     gpio_put(LED_PIN_Y, 0);
            // }
        }

        if (led_state){
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

    }
}



int main() {
    stdio_init_all();

    // BUTTON TASK
    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    
    // LED TASKS
    xTaskCreate(led_r_task, "LED_R_TASK", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Y_TASK", 256, NULL, 1, NULL);

    // SEMAPHORES  
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    // QUEUE
    xQueue_Btn = xQueueCreate(32,sizeof(uint));

    vTaskStartScheduler();

    while(1){}

    return 0;
}