#ifndef TASK_SENSOR_H
#define TASK_SENSOR_H

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
#define ADC_TEMP_X 26

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

#endif