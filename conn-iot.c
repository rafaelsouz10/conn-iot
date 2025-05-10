// === INCLUDES ===
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"

// === DEFINIÇÕES ===
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
#define BOTAO_PIN 5
#define ADC_TEMP_X 26

// === VARIÁVEIS GLOBAIS ===
#define LIMITE_BAIXO 15.0f
#define LIMITE_ALTO 40.0f

volatile float temperatura = 0.0;
volatile bool desativarAlarme = false;
volatile bool alarmeAtivo = false;
volatile bool condicaoCritica = false;
volatile bool estadoAnteriorBotao = true;

// === TASKS LIBS ===
#include "lib/task_buzzer.h"
#include "lib/task_display.h"

// === TASKS ===
void vSensorTask() {
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(ADC_TEMP_X);

    while (1) {
        adc_select_input(1);
        uint16_t vrx_temp = adc_read();
        temperatura = (vrx_temp / 4095.0) * 50.0;

        printf("adc: %d   Temperatura: %.2f\n", vrx_temp, temperatura);

        gpio_put(LED_R_PIN, temperatura > LIMITE_ALTO);
        gpio_put(LED_B_PIN, temperatura < LIMITE_BAIXO);
        gpio_put(LED_G_PIN, temperatura >= LIMITE_BAIXO && temperatura <= LIMITE_ALTO);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vBotaoTask() {
    gpio_init(BOTAO_PIN);
    gpio_set_dir(BOTAO_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_PIN);

    while (1) {
        bool estadoAtual = gpio_get(BOTAO_PIN);
        if (!estadoAtual && estadoAnteriorBotao) {
            desativarAlarme = true;
            printf("Alarme desativado!\n");
        }
        estadoAnteriorBotao = estadoAtual;
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
