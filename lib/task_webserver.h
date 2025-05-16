#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Config WiFi
#define WIFI_SSID "AP_"
#define WIFI_PASS "congresso@"

volatile bool wifiConectado = false;
volatile char wifiIP[20] = "Sem Conexao";

// Prototipos
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

// Accept callback
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// Request Handler
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("\nRequest: %s\n", request);

    // Endpoint /dados → JSON com Connection: close
    if (strstr(request, "GET /dados") != NULL) {
        char json_body[128];
        snprintf(json_body, sizeof(json_body),
            "{\"temperatura\":%.2f,\"estado\":\"%s\",\"alarme\":\"%s\"}",
            temperatura,
            condicaoCritica ? "CRITICO" : "OK",
            alarmeAtivo ? "LIGADO" : "DESLIGADO");

        char json_response[256];
        snprintf(json_response, sizeof(json_response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            strlen(json_body),
            json_body);

        printf("→ JSON enviado: %s\n", json_response);

        tcp_write(tpcb, json_response, strlen(json_response), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);

        free(request);
        pbuf_free(p);
        return ERR_OK;
    }

    // Endpoint /alarme_off → desativa alarme
    if (strstr(request, "GET /alarme_off")) {
        desativarAlarme = true;
        printf("→ Alarme desativado via Web.\n");
    }

    // Página principal com fetch() e Connection: close
    char html_body[1024];
    snprintf(html_body, sizeof(html_body),
        "<!DOCTYPE html>"
        "<html lang='pt-BR'>"
        "<head><meta charset='UTF-8'><title>Estufa</title>"
        "<style>body{background:#b5e5fb;font-family:Arial;text-align:center;margin-top:50px;}</style>"
        "</head>"
        "<body>"
        "<h1>Monitoramento da Estufa</h1>"
        "<p>Temperatura: <span id='temp'>---</span></p>"
        "<p>Estado: <span id='estado'>---</span></p>"
        "<p>Alarme: <span id='alarme'>---</span></p>"
        "<button onclick=\"fetch('/alarme_off').then(() => console.log('Alarme silenciado'));\"><b>Silenciar Alarme</b></button>"
        "<script>"
        "function atualizarDados() {"
        " fetch('/dados')"
        "  .then(response => response.json())"
        "  .then(data => {"
        "    console.log('Dados recebidos:', data);"
        "    document.getElementById('temp').textContent = data.temperatura.toFixed(2) + ' °C';"
        "    document.getElementById('estado').textContent = data.estado;"
        "    document.getElementById('alarme').textContent = data.alarme;"
        "  })"
        "  .catch(err => console.error('Erro no fetch:', err));"
        "}"
        "setInterval(atualizarDados, 2000);"
        "window.onload = atualizarDados;"
        "</script>"
        "</body></html>"
    );

    char html_response[1500];
    snprintf(html_response, sizeof(html_response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n\r\n"
        "%s",
        strlen(html_body),
        html_body);

    printf("→ Enviando página HTML\n");

    tcp_write(tpcb, html_response, strlen(html_response), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    free(request);
    pbuf_free(p);

    return ERR_OK;
}

// Inicializa WiFi + Servidor
void inicializar_wifi_servidor() {
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 5000)) {
        printf("Tentando conectar ao Wi-Fi...\n");
        snprintf((char *)wifiIP, sizeof(wifiIP), "Conectando...");
        wifiConectado = false;
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    wifiConectado = true;
    ip4_addr_t ip = cyw43_state.netif[CYW43_ITF_STA].ip_addr;
    snprintf((char *)wifiIP, sizeof(wifiIP), "%s", ip4addr_ntoa(&ip));
    printf("Conectado! IP: %s\n", wifiIP);

    struct tcp_pcb *server = tcp_new();
    if (!server || tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao iniciar TCP Server\n");
        vTaskDelete(NULL);
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");
}

// Task principal
void vWebServerTask() {
    if (cyw43_arch_init()) vTaskDelete(NULL);
    cyw43_arch_enable_sta_mode();

    inicializar_wifi_servidor();

    TickType_t lastCheck = xTaskGetTickCount();
    const TickType_t checkInterval = pdMS_TO_TICKS(5000);

    while (1) {
        cyw43_arch_poll();

        if (xTaskGetTickCount() - lastCheck >= checkInterval) {
            if (!netif_is_link_up(netif_default)) {
                printf("Conexao perdida. Tentando reconectar...\n");
                wifiConectado = false;
                snprintf((char *)wifiIP, sizeof(wifiIP), "Reconectando...");
                inicializar_wifi_servidor();
            }
            lastCheck = xTaskGetTickCount();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
