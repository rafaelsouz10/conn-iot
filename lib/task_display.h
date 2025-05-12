#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include "hardware/i2c.h"
#include "ssd1306/ssd1306.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

bool cor = true;    // Define a cor do display (preto/branco)
ssd1306_t ssd;    // Inicializa a estrutura do display

void display_init(){
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); //Inicializa o display
    ssd1306_config(&ssd); //Configura o display
    ssd1306_send_data(&ssd); //Envia os dados para o display

    //O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// TASK DO DISPLAY
void vDisplayTask(){
    display_init();

    const TickType_t intervaloPiscar = pdMS_TO_TICKS(300);
    TickType_t proximoToggle = xTaskGetTickCount();

    bool mostrar_alerta = true;

    while (true) {
        // Alterna o estado de piscar sem delay bloqueante
        if (xTaskGetTickCount() >= proximoToggle) {
            mostrar_alerta = !mostrar_alerta;
            proximoToggle = xTaskGetTickCount() + intervaloPiscar;
        }

         ssd1306_fill(&ssd, 0); // limpa o display

        // Cabeçalho
        ssd1306_draw_string(&ssd, "ESTUFA", 35, 5);

        // Temperatura
        char buffer_temp[32];
        snprintf(buffer_temp, sizeof(buffer_temp), "Temp: %.2f C", temperatura);
        ssd1306_draw_string(&ssd, buffer_temp, 5, 20);

        // Estado Critico
        char buffer_critico[32];
        snprintf(buffer_critico, sizeof(buffer_critico), "Estado: %s", condicaoCritica ? "CRITICO" : "OK");
        ssd1306_draw_string(&ssd, buffer_critico, 5, 35);

        // Alarme
        char buffer_alarme[32];
        snprintf(buffer_alarme, sizeof(buffer_alarme), "Alarme: %s", alarmeAtivo  ? "ON" : "OFF");
        ssd1306_draw_string(&ssd, buffer_alarme, 5, 45);

        // Se estiver em estado crítico, pisca um retângulo no canto
        if (condicaoCritica && mostrar_alerta) {
            ssd1306_rect(&ssd, 0, 0, 128, 64, true, false); // borda da tela
        }

        // Status Wi-Fi
        ssd1306_draw_string(&ssd, (const char *)wifiIP, 5, 55);

        // Envia os dados para o display
        ssd1306_send_data(&ssd);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif