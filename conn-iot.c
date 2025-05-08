#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"

#include "lwip/tcp.h"
#include "lwip/netif.h"

// ====== Credenciais Wi-Fi ======
#define WIFI_SSID "AP"
#define WIFI_PASSWORD "congresso@"

// ====== Prototipagem ======
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

// ====== Função principal ======
int main() {
    stdio_init_all();

    // Inicializa Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi (CYW43)\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha ao conectar. Tentando novamente...\n");
        sleep_ms(5000);
    }
    printf("Conectado!\n");

    // Exibe IP no terminal
    if (netif_default) {
        printf("IP local: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }

    // Configura servidor TCP
    struct tcp_pcb *server = tcp_new();
    if (!server) {
        printf("Erro ao criar PCB do servidor\n");
        return -1;
    }

    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao associar o servidor à porta 80\n");
        return -1;
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    printf("Servidor ouvindo na porta 80...\n");

    // Loop principal
    while (true) {
        cyw43_arch_poll(); // necessário para manter Wi-Fi ativo
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}

// ====== Aceitação de conexões ======
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// ====== Tratamento inicial da requisição (placeholder) ======
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Apenas descarta o conteúdo por enquanto
    pbuf_free(p);
    return ERR_OK;
}
