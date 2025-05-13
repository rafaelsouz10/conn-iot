#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <string.h> 
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lib/bootsel_btn.h"

// VARIÁVEIS GLOBAIS
#define LIMITE_BAIXO 15.0f
#define LIMITE_ALTO 40.0f
volatile float temperatura = 0.0;
volatile bool desativarAlarme = false;
volatile bool alarmeAtivo = false;
volatile bool condicaoCritica = false;

// TASKS LIBS
#include "lib/task_buzzer.h"
#include "lib/task_sensor.h"
#include "lib/task_botao.h"
#include "lib/task_webserver.h"
#include "lib/task_display.h"

// FUNÇÃO PRINCIPAL
int main() {
    stdio_init_all();
    
    bootsel_btn_callback(); // Para ser utilizado o modo BOOTSEL com botão B
    
    xTaskCreate(vSensorTask, "Sensor", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vAlarmeTask, "Alarme", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBotaoTask, "Botao", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vWebServerTask, "Web", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY+3, NULL);

    vTaskStartScheduler();

    return 0;
}
