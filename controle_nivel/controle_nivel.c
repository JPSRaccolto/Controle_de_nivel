#include <stdio.h>  // Biblioteca padrão para entrada e saída
#include <string.h> // Biblioteca manipular strings
#include <stdlib.h> // funções para realizar várias operações, incluindo alocação de memória dinâmica (malloc)

#include "pico/stdlib.h" // Biblioteca da Raspberry Pi Pico para funções padrão (GPIO, temporização, etc.)
#include "hardware/pwm.h"
#include "hardware/adc.h"    // Biblioteca da Raspberry Pi Pico para manipulação do conversor ADC
#include "pico/cyw43_arch.h" // Biblioteca para arquitetura Wi-Fi da Pico com CYW43
#include "lwip/pbuf.h"       // Lightweight IP stack - manipulação de buffers de pacotes de rede
#include "lwip/tcp.h"        // Lightweight IP stack - fornece funções e estruturas para trabalhar com o protocolo TCP
#include "lwip/netif.h"      // Lightweight IP stack - fornece funções e estruturas para trabalhar com interfaces de rede (netif)
#include "ws2812.pio.h"      //Manipula a matriz de LED
#include "hardware/i2c.h"    //Necessário para a comunicação com ssd1306
#include "ssd1306.h"         //inicia o ssd1306
#include "font.h"            //Fonte de palavras a serem escritas no ssd1306
#include <math.h>

#define WIFI_SSID "Galaxy S20 FE 5G"
#define WIFI_PASSWORD "abcd9700"


#define BOTAO_A 5
#define BOTAO_B 6
#define AZUL 12
#define VERDE 11
#define VERMELHO 13
#define RELE 16
#define PWM_WRAP 4095
#define PWM_CLK_DIV 30.52f
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO_I2C 0x3C
#define buzzer1 10
#define buzzer2 21
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define DEADZONE 170
#define ADC_PIN 28 // GPIO para o voltímetro

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float T_x = 0.0;           // Tensão no potenciômetro
float T_xanterior = 0.0;   // Valor passado da tensão no potenciômetro
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
float limite_min = 1.75;
float limite_max = 2.2;
static volatile uint32_t ultimo_tempo = 0;
bool bomba = false;
bool alerta_cheio_emitido;
bool alerta_vazio_emitido;

ssd1306_t ssd;

float volume_atual = 0.0;

void atualizar_display_nivel(float tensao, float min, float max); 
// Inicializa o PWM no pino do buzzer
void pwm_init_buzzer(uint pin);

// Toca um beep com frequência e duração especificadas
void beep(uint pin, uint frequency, uint duration_ms);

// Emite alerta de "reservatório cheio"
void alerta_reservatorio_cheio(uint pin);

// Emite alerta de "reservatório vazio"
void alerta_reservatorio_vazio(uint pin);

const char HTML_BODY[] =
    "<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'><title>Projeto Bomba D'Água</title>"
    "<style>"
    "body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; background-color: #f4f4f4; }"
    ".container { text-align: center; padding: 30px; background-color: #fff; border-radius: 20px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.4); width: 450px; }"
    "h1 { margin-bottom: 30px; font-family: Arial, sans-serif; color: #000000; font-size: 1.8rem; }"
    ".icones { display: flex; justify-content: center; gap: 20px; margin-bottom: 20px; }"
    //"#estado, #quantidade { width: 140px; height: 140px; display: flex; flex-direction: column; justify-content: center; align-items: center; text-align: center; border-radius: 15px; font-size: 2.5rem; color: white; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.4); }"
    //"#estado-val, #quantidade_val { color: white; font-size: 2.5rem; font-family: Arial, sans-serif; }"
    //"#estado { background-color: green; font-size: 2rem; }"
    //"#estado.off { background-color: red; }"
    //"#quantidade { background-color: #0138ff; font-size: 2rem; }"
    // ".titulo-icone { font-size: 1.2rem; font-weight: bold; color: white; margin-bottom: 10px; }"
    // "#minimo, #maximo { background-color: #eeeeee; padding: 12px 20px; border-radius: 10px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); margin-top: 15px; font-size: 1.4rem; font-family: Arial, sans-serif; color: #333; font-weight: bold; }"
    // "label { font-size: 1.2rem; color: #333; font-weight: bold; display: block; margin-bottom: 10px; }"
    // ".slider-container { display: flex; flex-direction: column; align-items: center; width: 100%; margin-top: 20px; }"
    // "input[type='range'] { width: 80%; margin-top: 10px; -webkit-appearance: none; appearance: none; height: 8px; background: #ddd; border-radius: 5px; outline: none; transition: background 0.3s; }"
    // "input[type='range']::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 20px; height: 20px; background: #4CAF50; border-radius: 50%; cursor: pointer; }"
    // "input[type='range']:hover { background: #ccc; }"
    // "input[type='range']:active::-webkit-slider-thumb { background: #45a049; }"
    // "span { font-size: 1.2rem; font-weight: normal; margin-left: 10px; }"
    "</style>"

    "<script>"
    "function sendCommandMinimun(cmd) { fetch('/minimun/' + cmd); }"
    "function sendCommandMaximun(cmd) { fetch('/maximun/' + cmd); }"
    "document.addEventListener('DOMContentLoaded', function () {"
    "  const minimoSlider = document.getElementById('minimo');"
    "  const maximoSlider = document.getElementById('maximo');"
    "  minimoSlider.addEventListener('input', function () {"
    "    const valor = this.value;"
    "    document.getElementById('minimo-val').textContent = valor + '%';"
    "    sendCommandMinimun(valor);"
    "  });"
    "  maximoSlider.addEventListener('input', function () {"
    "    const valor = this.value;"
    "    document.getElementById('maximo-val').textContent = valor + '%';"
    "    sendCommandMaximun(valor);"
    "  });"
    "});"
    "function atualizar() {"
    "  fetch('/estado')"
    "    .then(res => res.json())"
    "    .then(data => {"
    "      const estadoEl = document.getElementById('estado');"
    "      const estadoVal = document.getElementById('estado-val');"
    "      if (data.estado) {"
    "        estadoVal.innerText = 'ON';"
    "        estadoEl.classList.remove('off');"
    "      } else {"
    "        estadoVal.innerText = 'OFF';"
    "        estadoEl.classList.add('off');"
    "      }"
    "      document.getElementById('quantidade-val').innerText = data.quantidade;"
    "    });"
    "}"
    "setInterval(atualizar, 1000);"
    "</script></head>"

    "<body>"
    "<div class='container'>"
    "  <h1>Sistema Bomba D'Água</h1>"
    "  <div class='icones'>"
    "    <div id='estado'>"
    "      <div class='titulo-icone'>Bomba</div>"
    "      <label for='state'><span id='estado-val'>ON</span></label>"
    "    </div>"
    "    <div id='quantidade'>"
    "      <div class='titulo-icone'>Nível da Água</div>"
    "      <label for='quantity'><span id='quantidade-val'>--%</span></label>"
    "    </div>"
    "  </div>"
    "  <div class='slider-container'>"
    "    <label for='minimo'>Mínimo: <span id='minimo-val'>0%</span></label>"
    "    <input type='range' id='minimo' min='0' max='100' value='0'>"
    "  </div>"
    "  <div class='slider-container'>"
    "    <label for='maximo'>Máximo: <span id='maximo-val'>100%</span></label>"
    "    <input type='range' id='maximo' min='0' max='100' value='100'>"
    "  </div>"
    "</div>"
    "</body></html>";

struct http_state
{
    char response[4096];
    size_t len;
    size_t sent;
};

static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;
    if (hs->sent >= hs->len)
    {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}

static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs)
    {
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    hs->sent = 0;

    if (strstr(req, "GET /minimun/"))
    {
        int valor = atoi(strstr(req, "/minimun/") + strlen("/minimun/"));
        float nova_tensao = (valor / 100.0f) * ADC_VREF;

        if (nova_tensao < limite_max)
        {
            limite_min = nova_tensao;
        }

        const char *msg = "Minimo atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(msg), msg);
    }
    else if (strstr(req, "GET /maximun/"))
    {
        int valor = atoi(strstr(req, "/maximun/") + strlen("/maximun/"));
        float nova_tensao = (valor / 100.0f) * ADC_VREF;

        if (nova_tensao > limite_min)
        {
            limite_max = nova_tensao;
        }

        const char *msg = "Maximo atualizado";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(msg), msg);
    }
    else if (strstr(req, "GET /estado"))
    {
        volume_atual = ((T_x * 100) / 3.3);

        char json_payload[96];
        int json_len = snprintf(json_payload, sizeof(json_payload),
                                "{\"estado\":%d,\"quantidade\":%d}\r\n",
                                bomba, volume_atual);

        printf("[DEBUG] JSON: %s\n", json_payload);
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           json_len, json_payload);
    }
    else
    {
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(HTML_BODY), HTML_BODY);
    }

    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);
    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    pbuf_free(p);
    return ERR_OK;
}

static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

static void start_http_server(void)
{
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("Erro ao criar PCB TCP\n");
        return;
    }
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

int i = 0;

// Desenho do número 1
double desenho1[25] =
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.2, 0.2, 0.2, 0.2, 0.2,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0};

void num1(uint8_t r, uint8_t g, uint8_t b)
{

    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);
    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (desenho1[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}

// Desenho do número 2
double desenho2[25] =
    {0.0, 0.0, 0.2, 0.0, 0.0,
     0.0, 0.2, 0.2, 0.2, 0.0,
     0.2, 0.0, 0.2, 0.0, 0.2,
     0.0, 0.0, 0.2, 0.0, 0.0,
     0.0, 0.0, 0.2, 0.0, 0.0};

void num2(uint8_t r, uint8_t g, uint8_t b)
{

    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);
    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (desenho2[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}

// Desenho do número 3
double desenho3[25] =
    {0.0, 0.0, 0.2, 0.0, 0.0,
     0.0, 0.0, 0.2, 0.0, 0.0,
     0.2, 0.0, 0.2, 0.0, 0.2,
     0.0, 0.2, 0.2, 0.2, 0.0,
     0.0, 0.0, 0.2, 0.0, 0.0};

void num3(uint8_t r, uint8_t g, uint8_t b)
{

    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);
    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (desenho3[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}

// configurações do PWM
void pwm_init_gpio(uint gpio, uint wrap, float clkdiv)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, wrap);
    pwm_config_set_clkdiv(&config, clkdiv);
    pwm_init(slice_num, &config, true);
}
void inicia_pinos()
{
    // Inicializa todos os tipos de bibliotecas stdio padrão presentes que estão ligados ao binário.
    stdio_init_all();
    sleep_ms(50);
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO_I2C, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    adc_init();
    adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica
    pwm_init_gpio(buzzer1, PWM_WRAP, PWM_CLK_DIV);
    pwm_set_gpio_level(buzzer1, 0); // Inicialmente desligado

    pwm_init_gpio(buzzer2, PWM_WRAP, PWM_CLK_DIV);
    pwm_set_gpio_level(buzzer2, 0); // Inicialmente desligado
    gpio_init(VERDE);
    gpio_set_dir(VERDE, GPIO_OUT);
    gpio_init(AZUL);
    gpio_set_dir(AZUL, GPIO_OUT);
    gpio_init(VERMELHO);
    gpio_set_dir(VERMELHO, GPIO_OUT);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_init(RELE);
    gpio_set_dir(RELE, GPIO_OUT);
    gpio_put(RELE, 0);
}

// Função para administrar a interrupção gerada pelos botões
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    if (tempo_atual - ultimo_tempo > 200000 && !gpio_get(BOTAO_B))
    {
        // Se a interrupção é gerada pelo botão B, o estado do relé é alterado em qualquer posição abaixo do limite máximo
        if (T_x < 2.5)
        {
            bomba = !bomba;
        }
        gpio_put(RELE, bomba);
        ultimo_tempo = tempo_atual;
    }
    else if (tempo_atual - ultimo_tempo > 200000 && !gpio_get(BOTAO_A))
    {
        // Se a interrupção é gerada pelo botão A, os limites mínimo e máximo são resetados aos valores iniciais
        limite_max = 2.2;
        limite_min = 1.75;
        ultimo_tempo = tempo_atual;
    }
}

void nivel_tanque()
{
    if (T_x >= limite_max)
    {
        // Se o valor está no limite máximo, o relé é desligado e o led verde é aceso
        gpio_put(RELE, 0);
        gpio_put(VERDE, 1);
        gpio_put(VERMELHO, 0);
        bomba = false;
        
        if (!alerta_cheio_emitido) {
            alerta_reservatorio_cheio(buzzer1);
            alerta_cheio_emitido = true;
            alerta_vazio_emitido = false;  // Reseta o outro alerta
        }
    }
    else if (T_x <= limite_min)
    {
        // Se o valor está no limite mínimo, o relé é ligado e o led vermelho é aceso
        gpio_put(RELE, 1);
        gpio_put(VERDE, 0);
        gpio_put(VERMELHO, 1);
        bomba = true;
        
        if (!alerta_vazio_emitido) {
            alerta_reservatorio_vazio(buzzer2);
            alerta_vazio_emitido = true;
            alerta_cheio_emitido = false; // Reseta o outro alerta
        }
    }
    else if (T_x > limite_min && T_x < limite_max)
    {
        // Em demais situações, os leds verde e vermelho são acesos e o relé não sofre alterações
        gpio_put(VERDE, 1);
        gpio_put(VERMELHO, 1);
    }

    if (T_x - T_xanterior > 0.01)
    {
        num3(0, 100, 0); // Se houver crescimento no nível de água (aumento de tensão), surge uma seta verde para cima na matriz
    }
    else if (T_x - T_xanterior < -0.01)
    {
        num2(100, 0, 0); // Se houver decrescimento no nível de água, surge uma seta vermelha para baixo na matriz
    }
    else
    {
        num1(100, 100, 0); // Se não houver alteração, é mostrado apenas um traço amarelo horizontal
    }
}
int main()
{
    inicia_pinos();


    // Ativa a função de interrupção
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicializa a arquitetura do cyw43
    while (cyw43_arch_init())
    {
        printf("Falha ao inicializar Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }

    // Ativa o Wi-Fi no modo Station, de modo a que possam ser feitas ligações a outros pontos de acesso Wi-Fi.
    cyw43_arch_enable_sta_mode();

    // Conectar à rede WiFI - fazer um loop até que esteja conectado
    printf("Conectando ao Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000))
    {
        printf("Falha ao conectar ao Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }
    printf("Conectado ao Wi-Fi\n");

    // Caso seja a interface de rede padrão - imprimir o IP do dispositivo.
    if (netif_default)
    {
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }

    bool cor = true;
    start_http_server();

    while (true)
    {
        cyw43_arch_poll(); // Necessário para manter o Wi-Fi ativo
        sleep_ms(1000);
        adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

        float soma = 0.0f;
        // São somadas 100 amostras do valor do ADC e se calcula uma média dessas amostras
        for (int i = 0; i < 100; i++)
        {
            soma += adc_read();
            sleep_ms(1);
        }
        float media = soma / 100.0f;

        // Atualiza o valor de T_xanterior antes de calcular um valor mais recente
        T_xanterior = T_x;
        // Fórmula para fornecer a tensão lida no divisor de tensão que atualiza o valor de T_x
        T_x = media * ADC_VREF / ADC_RESOLUTION;

        // Chama a função que atualiza os parâmetros

        nivel_tanque();
        atualizar_display_nivel(T_x, limite_min, limite_max);

    }
    cyw43_arch_deinit();

    atualizar_display_nivel(T_x, limite_min, limite_max);

    return 0;
}


void atualizar_display_nivel(float tensao, float min, float max) {
    char nivel_texto[20];

    // Determina o texto do nível de água
    if (tensao >= max - 0.05) {
        strcpy(nivel_texto, "NIVEL: CHEIO");
    } else if (tensao <= min + 0.05) {
        strcpy(nivel_texto, "NIVEL: BAIXO");
    } else {
        strcpy(nivel_texto, "NIVEL: MEDIO");
    }

    // Limpa o display
    ssd1306_fill(&ssd, false);

    // Escreve o nível
    ssd1306_draw_string(&ssd, nivel_texto, 5, 5);

    // Barra de nível
    int barra_x = 5;
    int barra_y = 20;
    int barra_largura = 100;
    int barra_altura = 15;

    // Retângulo da barra
    ssd1306_rect(&ssd, barra_y, barra_x, barra_largura, barra_altura, true, false);

    // Calcula preenchimento
    float porcentagem = (tensao - min) / (max - min);
    if (porcentagem > 1.0f) porcentagem = 1.0f;
    if (porcentagem < 0.0f) porcentagem = 0.0f;

    int largura_preenchida = (int)(porcentagem * (barra_largura - 2));
    for (int i = 0; i < largura_preenchida; i++) {
        for (int j = 0; j < barra_altura - 2; j++) {
            ssd1306_pixel(&ssd, barra_x + 1 + i, barra_y + 1 + j, true);
        }
    }

    // Mostra tensão
    char perc_str[20];
    sprintf(perc_str, "Nivel: %d%%", (int)(porcentagem * 100));
    ssd1306_draw_string(&ssd, perc_str, 5, 40);

    // Mostra status da bomba
    if (bomba) {
        ssd1306_draw_string(&ssd, "BOMBA LIGADA", 5, 52);
    } else {
        ssd1306_draw_string(&ssd, "BOMBA DESLIG", 5, 52);
    }

    // Atualiza o display
    ssd1306_send_data(&ssd);
}

//Inicializa o PWM no pino do buzzer
void pwm_init_buzzer(uint pin) {
    // Configura o pino como saída PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obtém o número do slice PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configura o PWM com as configurações padrão
    pwm_config config = pwm_get_default_config();
    pwm_init(slice_num, &config, true);

    // Inicia o PWM no nível baixo (0)
    pwm_set_gpio_level(pin, 0);
}

// Função que emite um som (beep) com uma frequência e duração especificadas
void beep(uint pin, uint frequency, uint duration_ms) {
    // Obtém o número do slice PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Calcula a frequência do clock do sistema
    uint32_t clock_freq = clock_get_hz(clk_sys);
    // Calcula o divisor de clock necessário
    float divider = (float)clock_freq / (float)(frequency * 4096);
    // Define o divisor de clock para o slice PWM
    pwm_set_clkdiv(slice_num, divider);

    // Calcula o valor de 'top' baseado na frequência desejada
    uint32_t top = clock_freq / (frequency * divider) - 1;

    // Configura o valor de 'top' no PWM e ajusta o duty cycle para 50% (ativo)
    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(pin, top / 2);

    // Aguarda pelo tempo de duração do beep
    sleep_ms(duration_ms);

    // Desativa o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa de 50ms entre os beeps
    sleep_ms(50);
}

// Alerta para reservatório cheio: dois beeps suaves
void alerta_reservatorio_cheio(uint pin) {
    beep(pin, 600, 150);  // Beep médio-suave
    sleep_ms(100);
    beep(pin, 800, 150);  // Beep um pouco mais agudo
}

// Alerta para reservatório vazio: cinco beeps agudos e rápidos
void alerta_reservatorio_vazio(uint pin) {
    for (int i = 0; i < 5; i++) {
        beep(pin, 1000, 200);  // Beep agudo e curto
        sleep_ms(100);         // Pausa curta entre os beeps
    }
}