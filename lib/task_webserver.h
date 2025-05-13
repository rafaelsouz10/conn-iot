#include "pico/cyw43_arch.h"     // Biblioteca para arquitetura Wi-Fi da Pico com CYW43  
#include "lwip/pbuf.h"           // Lightweight IP stack - manipulação de buffers de pacotes de rede
#include "lwip/tcp.h"            // Lightweight IP stack - fornece funções e estruturas para trabalhar com o protocolo TCP
#include "lwip/netif.h"          // Lightweight IP stack - fornece funções e estruturas para trabalhar com interfaces de rede (netif)

// Configuração WiFi
#define WIFI_SSID "AP_"
#define WIFI_PASS "congresso@"

volatile bool wifiConectado = false;
volatile char wifiIP[20] = "Sem Conexao";

// Chamada das funções
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err); // Função de callback ao aceitar conexões TCP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err); // Função de callback para processar requisições HTTP
void user_request(char **request); // Tratamento do request do usuário

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Alocação do request na memória dinámica
    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("Request: %s\n", request);

    // Tratamento de request - Controle dos LEDs
    user_request(&request);

    // Cria a resposta HTML
    char html[1024];

    // Instruções html do webserver
    snprintf(html, sizeof(html), // Formatar uma string e armazená-la em um buffer de caracteres
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
            "<title>ESTUFA - Temperatura</title>"
            "<meta http-equiv=\"refresh\" content=\"5\">"
            "<style>"
                "body{background:#b5e5fb;font-family:Arial;text-align:center;margin-top:50px;}"
                "h1{font-size:64px;margin-bottom:30px;}"
                "p{font-size:20px;color:#555;}"
                "button{background:LightGray;font-size:16px;margin:10px;padding:10px 20px;border-radius:10px;}"
                ".temperature{font-size:48px;margin-top:30px;color:#333;}"
            "</style>"
        "</head>"
        "<body>"
            "<h1>ESTUFA</h1>"
            "<h3>Monitoramento</h3>"
            "<p class=\"temperature\">Temperatura: %.2f &deg;C</p>"
            "<p><strong>Estado do Sistema:</strong> %s</p>"
            "<p><strong>Alarme:</strong> %s</p>"
            "<a href='/alarme_off'><button>Silenciar Alarme</button></a>"
        "</body>"
        "</html>",
        temperatura,
        condicaoCritica ? "CRITICO" : "OK",
        alarmeAtivo ? "LIGADO" : "DESLIGADO"
    );

    // Escreve dados para envio (mas não os envia imediatamente).
    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);

    // Envia a mensagem
    tcp_output(tpcb);

    //libera memória alocada dinamicamente
    free(request);
    
    //libera um buffer de pacote (pbuf) que foi alocado anteriormente
    pbuf_free(p);

    return ERR_OK;
}

// Tratamento do request do usuário - digite aqui
void user_request(char **request) {
    if (strstr(*request, "GET /alarme_off")){
        desativarAlarme = true;
        printf("Alarme desativado via Web.\n");
    }
}

// Inicializa o servidor
void inicializar_wifi_servidor() {
    // Tenta conectar ao Wi-Fi
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 5000)) {
        printf("Tentando conectar ao Wi-Fi...\n");
        snprintf((char *)wifiIP, sizeof(wifiIP), "Conectando...");
        wifiConectado = false;
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Conectou
    wifiConectado = true;
    snprintf((char *)wifiIP, sizeof(wifiIP), "%s", ipaddr_ntoa(&netif_default->ip_addr));
    printf("Conectado! IP: %s\n", wifiIP);

    // Inicializa servidor TCP
    struct tcp_pcb *server = tcp_new();
    if (!server || tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao iniciar TCP Server\n");
        vTaskDelete(NULL);
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");
}

// Task única para Wi-Fi, servidor e polling
void vWebServerTask() {
    if (cyw43_arch_init()) vTaskDelete(NULL);
    cyw43_arch_enable_sta_mode();

    // Primeira tentativa de conectar e iniciar servidor
    inicializar_wifi_servidor();

    TickType_t lastCheck = xTaskGetTickCount();
    const TickType_t checkInterval = pdMS_TO_TICKS(5000); // Checa a cada 5s

    while (1) {
        cyw43_arch_poll();  // Mantém Wi-Fi e TCP/IP funcionando

        // Checar estado do link
        if (xTaskGetTickCount() - lastCheck >= checkInterval) {
            if (!netif_is_link_up(netif_default)) {
                printf("Conexao perdida. Tentando reconectar...\n");
                wifiConectado = false;
                snprintf((char *)wifiIP, sizeof(wifiIP), "Reconectando...");
                inicializar_wifi_servidor();  // Reutiliza a função
            }
            lastCheck = xTaskGetTickCount();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}