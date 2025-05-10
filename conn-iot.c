// === INCLUDES ===
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"

// === DEFINIÇÕES ===
#define BOTAO_PIN 5

// === VARIÁVEIS GLOBAIS ===
#define LIMITE_BAIXO 15.0f
#define LIMITE_ALTO 40.0f
volatile float temperatura = 0.0;
volatile bool desativarAlarme = false;
volatile bool alarmeAtivo = false;
volatile bool condicaoCritica = false;

// === TASKS LIBS ===
#include "lib/task_buzzer.h"
#include "lib/task_display.h"
#include "lib/task_sensor.h"
#include "lib/task_botao.h"

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events){
    reset_usb_boot(0, 0);
}

// === FUNÇÃO PRINCIPAL ===
int main() {
    stdio_init_all();

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B
    
    xTaskCreate(vSensorTask, "Sensor", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vAlarmeTask, "Alarme", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBotaoTask, "Botao", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Display", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();

    return 0;
}
