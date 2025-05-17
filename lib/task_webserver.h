#ifndef TASK_WEBSERVER_H
#define TASK_WEBSERVER_H

#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Config WiFi
#define WIFI_SSID "Kira_Oreo"
#define WIFI_PASS "Aaik1987"

// Estado global
volatile bool wifiConectado = false;
volatile char wifiIP[20] = "Sem Conexao";

// Prototipos
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

// --- FUNÇÕES AUXILIARES ---

void wifi_connect() {
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
}

void tcp_server_setup() {
    struct tcp_pcb *server = tcp_new();
    if (!server || tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao iniciar TCP Server\n");
        vTaskDelete(NULL);
    }
    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");
}

void send_http_response(struct tcp_pcb *tpcb, const char *body, const char *content_type) {
    char response[2048];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n\r\n"
        "%s",
        content_type, strlen(body), body);

    tcp_write(tpcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
}

void handle_request(const char *request, struct tcp_pcb *tpcb) {
    if (strstr(request, "GET /dados") != NULL) {
        char json_body[256];
        snprintf(json_body, sizeof(json_body),
            "{\"t\":%.2f,\"td\":%.1f,\"ud\":%.1f,\"e\":\"%s\",\"a\":\"%s\"}",
            temperatura, tempDHT, umiDHT,
            condicaoCritica ? "CRITICO" : "OK",
            alarmeAtivo ? "ON" : "OFF");

        send_http_response(tpcb, json_body, "application/json");
        printf("→ JSON enviado: %s\n", json_body);
        return;
    }

    if (strstr(request, "GET /a_off") != NULL) {
        desativarAlarme = true;
        printf("→ Alarme desativado via Web.\n");
    }

    char html_body[2048];
    snprintf(html_body, sizeof(html_body),
        "<!DOCTYPE html><head><title>Estufa</title>"
        "<style>body{background-color:#f0f0f0;text-align:center;padding:20px;}"
        "p{font-size:20px;color:#555;}"
        "button{background:LightGray;font-size:16px;margin:10px;padding:10px 20px;}"
        "</style></head><body>"
        "<h1>Monitoramento Estufa</h1>"
        "<p>TempPot: <span id='t'></span></p>"
        "<p>UmiDHT <span id='ud'></span></p>"
        "<p>TempDHT <span id='td'></span></p>"
        "<p>Estado: <span id='e'></span></p>"
        "<p>Alarme: <span id='a'></span></p>"
        "<button onclick=\"fetch('/a_off');\"><b>Silenciar Alarme</b></button>"
        "<script>function attDados(){fetch('/dados')"
        ".then(response=>response.json())"
        ".then(data=>{"
        "document.getElementById('t').textContent=data.t.toFixed(2)+' C';"
        "document.getElementById('td').textContent=data.td.toFixed(1)+' C';"
        "document.getElementById('ud').textContent=data.ud.toFixed(1)+'%%';"
        "document.getElementById('e').textContent=data.e;"
        "document.getElementById('a').textContent=data.a;"
        "})}setInterval(attDados, 1000);window.onload=attDados;"
        "</script></body></html>"
    );

    send_http_response(tpcb, html_body, "text/html");
    printf("→ Página HTML enviada\n");
}

// --- CALLBACKS TCP ---

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("\nRequest Recebido: %s\n", request);

    handle_request(request, tpcb);

    free(request);
    pbuf_free(p);

    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// --- TASK PRINCIPAL ---

void vWebServerTask() {
    if (cyw43_arch_init()) vTaskDelete(NULL);
    cyw43_arch_enable_sta_mode();

    wifi_connect();
    tcp_server_setup();

    TickType_t lastCheck = xTaskGetTickCount();
    const TickType_t checkInterval = pdMS_TO_TICKS(5000);

    while (1) {
        cyw43_arch_poll();

        if (xTaskGetTickCount() - lastCheck >= checkInterval) {
            if (!netif_is_link_up(netif_default)) {
                printf("Conexao perdida. Reconectando Wi-Fi...\n");
                snprintf((char *)wifiIP, sizeof(wifiIP), "Reconectando...");
                wifi_connect();
            }
            lastCheck = xTaskGetTickCount();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

#endif // TASK_WEBSERVER_H
