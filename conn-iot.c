// === INCLUDES ===
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"

// === DEFINIÇÕES ===
#define LIMITE_BAIXO 18.0f
#define LIMITE_ALTO 32.0f

#define ADC_TEMP_X 26

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
#define BOTAO_PIN 5

// === VARIÁVEIS GLOBAIS ===
volatile float temperatura = 0.0;
volatile float umidade = 0.0;
volatile bool desativarAlarme = false;

// === TASKS LIBS ===
#include "lib/task_buzzer.h"

// === TASKS ===
void vSensorTask() {
    while (1) {
        adc_select_input(0);
        uint16_t raw_temp = adc_read();
        temperatura = 10.0 + ((raw_temp / 4095.0f) * 40.0f);

        adc_select_input(1);
        uint16_t raw_umid = adc_read();
        umidade = 30.0 + ((raw_umid / 4095.0f) * 70.0f);

        gpio_put(LED_R_PIN, temperatura > LIMITE_ALTO);
        gpio_put(LED_B_PIN, temperatura < LIMITE_BAIXO);
        gpio_put(LED_G_PIN, temperatura >= LIMITE_BAIXO && temperatura <= LIMITE_ALTO);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vBotaoTask() {
    bool estadoAnterior = true;
    while (1) {
        bool estadoAtual = gpio_get(BOTAO_PIN);
        if (!estadoAtual && estadoAnterior) {
            desativarAlarme = true;
            printf("Alarme desativado pelo botao.\n");
        }
        estadoAnterior = estadoAtual;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events){
    reset_usb_boot(0, 0);
}

// === FUNÇÃO PRINCIPAL ===
int main() {

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B
    
    stdio_init_all();

    adc_init();
    adc_gpio_init(ADC_TEMP_X);

    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    gpio_init(BOTAO_PIN);
    gpio_set_dir(BOTAO_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_PIN);

    xTaskCreate(vSensorTask, "Sensor", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vAlarmeTask, "Alarme", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBotaoTask, "Botao", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    while (true);
    return 0;
}
