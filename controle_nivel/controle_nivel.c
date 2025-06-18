#include <stdio.h>               // Biblioteca padrão para entrada e saída
#include <string.h>              // Biblioteca manipular strings
#include <stdlib.h>              // funções para realizar várias operações, incluindo alocação de memória dinâmica (malloc)

#include "pico/stdlib.h"         // Biblioteca da Raspberry Pi Pico para funções padrão (GPIO, temporização, etc.)
#include "hardware/pwm.h"
#include "hardware/adc.h"        // Biblioteca da Raspberry Pi Pico para manipulação do conversor ADC
#include "pico/cyw43_arch.h"     // Biblioteca para arquitetura Wi-Fi da Pico com CYW43  
#include "lwip/pbuf.h"           // Lightweight IP stack - manipulação de buffers de pacotes de rede
#include "lwip/tcp.h"            // Lightweight IP stack - fornece funções e estruturas para trabalhar com o protocolo TCP
#include "lwip/netif.h"          // Lightweight IP stack - fornece funções e estruturas para trabalhar com interfaces de rede (netif)
#include "ws2812.pio.h"          //Manipula a matriz de LED
#include "hardware/i2c.h"        //Necessário para a comunicação com ssd1306
#include "ssd1306.h"             //inicia o ssd1306
#include "font.h"                //Fonte de palavras a serem escritas no ssd1306
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
ssd1306_t ssd;
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

void num1(uint8_t r, uint8_t g, uint8_t b){
    
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
            put_pixel(0);  // Desliga os LEDs com zero no buffer
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

void num2(uint8_t r, uint8_t g, uint8_t b){
    
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
            put_pixel(0);  // Desliga os LEDs com zero no buffer
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

void num3(uint8_t r, uint8_t g, uint8_t b){
    
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
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
    
}
// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

// Tratamento do request do usuário
void user_request(char **request);

//configurações do PWM
void pwm_init_gpio(uint gpio, uint wrap, float clkdiv) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, wrap);
    pwm_config_set_clkdiv(&config, clkdiv);
    pwm_init(slice_num, &config, true);
}
void inicia_pinos(){
    //Inicializa todos os tipos de bibliotecas stdio padrão presentes que estão ligados ao binário.
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
    pwm_set_gpio_level(buzzer1, 0);  // Inicialmente desligado
  
    pwm_init_gpio(buzzer2, PWM_WRAP, PWM_CLK_DIV);
    pwm_set_gpio_level(buzzer2, 0);  // Inicialmente desligado 
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
static void gpio_irq_handler(uint gpio, uint32_t events){
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    if(tempo_atual - ultimo_tempo > 200000 && !gpio_get(BOTAO_B)){
        // Se a interrupção é gerada pelo botão B, o estado do relé é alterado em qualquer posição abaixo do limite máximo
        if(T_x < 2.5){
            bomba = !bomba;
        }
        gpio_put(RELE, bomba);
        ultimo_tempo = tempo_atual; 
    } else if(tempo_atual - ultimo_tempo > 200000 && !gpio_get(BOTAO_A)){
        // Se a interrupção é gerada pelo botão A, os limites mínimo e máximo são resetados aos valores iniciais
        limite_max = 2.2;
        limite_min = 1.75;
        ultimo_tempo = tempo_atual;
    }
}
void nivel_tanque(){
    if(T_x >= limite_max){
        // Se o valor está no limite máximo, o relé é desligado e o led verde é aceso
        gpio_put(RELE, 0);
        gpio_put(VERDE, 1);
        gpio_put(VERMELHO, 0);
        bomba = false;
     } else if (T_x <= limite_min){
        // Se o valor está no limite mínimo, o relé é ligado e o led vermelho é aceso
        gpio_put(RELE, 1);
        gpio_put(VERDE, 0);
        gpio_put(VERMELHO, 1);
        bomba = true;
     } else if (T_x > limite_min && T_x < limite_max){
        // Em demais situações, os leds verde e vermelho são acesos e o relé não sofre alterações
        gpio_put(VERDE, 1);
        gpio_put(VERMELHO, 1);
     }
     
     if(T_x - T_xanterior > 0.01){
        num3(0, 100, 0); // Se houver crescimento no nível de água (aumento de tensão), surge uma seta verde para cima na matriz
     } else if(T_x - T_xanterior < -0.01){
        num2(100, 0, 0); // Se houver decrescimento no nível de água, surge uma seta vermelha para baixo na matriz
     } else {
        num1(100, 100, 0);// Se não houver alteração, é mostrado apenas um traço amarelo horizontal
     }

}
int main()
{
    inicia_pinos();
    
    // Ativa a função de interrupção
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    //Inicializa a arquitetura do cyw43
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

    // Configura o servidor TCP - cria novos PCBs TCP. É o primeiro passo para estabelecer uma conexão TCP.
    struct tcp_pcb *server = tcp_new();
    if (!server)
    {
        printf("Falha ao criar servidor TCP\n");
        return -1;
    }

    //vincula um PCB (Protocol Control Block) TCP a um endereço IP e porta específicos.
    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Falha ao associar servidor TCP à porta 80\n");
        return -1;
    }

    // Coloca um PCB (Protocol Control Block) TCP em modo de escuta, permitindo que ele aceite conexões de entrada.
    server = tcp_listen(server);
    bool cor = true;
    // Define uma função de callback para aceitar conexões TCP de entrada. É um passo importante na configuração de servidores TCP.
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");

    while (true) {
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

     //Chama a função que atualiza os parâmetros 
     nivel_tanque();

     ssd1306_fill(&ssd, false);

     // Retângulo principal externo
     ssd1306_rect(&ssd, 5, 5, 123, 59, cor, false);

     // Linhas horizontais para dividir em 4 seções
     ssd1306_line(&ssd, 5, 19, 123, 19, cor);  // Primeira divisão
     ssd1306_line(&ssd, 5, 33, 123, 33, cor);  // Segunda divisão
     ssd1306_line(&ssd, 5, 47, 123, 47, cor);  // Terceira divisão

     ssd1306_draw_string(&ssd, "Agua(%): ", 10, 8);
     ssd1306_draw_string(&ssd, "Max|Min: ", 10, 22);
     ssd1306_draw_string(&ssd, "Bomba: ", 10, 36);
     ssd1306_draw_string(&ssd, "Wifi: ", 10, 50);

     ssd1306_send_data(&ssd);
    }
    cyw43_arch_deinit();
    return 0;
}

/*===========Funções da atividade OIT, remodelar===============*/

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

const char *get_alert_messages() {
    static char alert[512];
    alert[0] = '\0';
        /*
    if (temp1 == true)
        strcat(alert, "<p style='color:red;'> Temperatura elevada - Ligue o ventilador!</p>");
    if (temp2 == true)
        strcat(alert, "<p style='color:orange;'> Umidade baixa - Ligue o umidificador!</p>");
    if (temp3 == true)
        strcat(alert, "<p style='color:blue;'> Umidade alta - Ligue o desumidificador!</p>");
    if (temp4 == true)
        strcat(alert, "<p style='color:purple;'> Temperatura muito baixa - Ligue o aquecedor!</p>");

    if (alert[0] == '\0')
        strcpy(alert, "<p style='color:green;'> Sistema operando normalmente.</p>");
                            */
    return alert;
}


// Tratamento do request do usuário - digite aqui
void user_request(char **request){
    
    /*if (strstr(*request, "GET /ventilador") != NULL) {
        ventilador_ligado = !ventilador_ligado;
        temp1 = !temp1;
    }
    else if (strstr(*request, "GET /aquecedor") != NULL) {
        aquecedor_ligado = !aquecedor_ligado;

    }
    else if (strstr(*request, "GET /umidificador") != NULL) {
        umidificador_ligado = !umidificador_ligado;
        temp2 = !temp2;
    }
    else if (strstr(*request, "GET /desumidificador") != NULL) {
        desumidificador_ligado = !desumidificador_ligado;

    }*/
};

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
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
    

    //adc_incremental(&temperatura, &umidade);
    // Cria a resposta HTML
    char html[2048]; 


    // Instruções html do webserver
    snprintf(html, sizeof(html),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "  <title>Embarcatech</title>\n"
    "  <style>\n"
    "    body { font-family: Arial, sans-serif; background-color: #f4f4f4; color: #333; }\n"
    "    h1 { text-align: center; }\n"
    "    .container { width: 80%%; margin: auto; overflow: hidden; }\n"
    "    .button { padding: 10px 20px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; }\n"
    "    .blue { background-color: #007BFF; color: white; }\n"
    "    .green { background-color: #28A745; color: white; }\n"
    "    .red { background-color: #DC3545; color: white; }\n"
    "    .orange { background-color: #FFC107; color: white; }\n"
    "    .status { text-align: center; font-size: 1.5em; margin: 20px 0; }\n"
    "    .alert-box { border: 2px solid #444; background: #ffe; padding: 10px; margin: 10px; }\n"
    "  </style>\n"
    "</head>\n"
    "<body>\n"
    "  <div class='container'>\n"
    "    <h1>Monitoramento Ambiental</h1>\n"
    "    <div class='alert-box'>\n"
    "      %s\n"
    "    </div>\n"
    "    <div class='status'>\n"
    "      <p>Temperatura: %.1f &deg;C</p>\n"
    "      <p>Umidade: %.1f &#37;</p>\n"
    "    </div>\n"
    "    <div style='text-align: center;'>\n"
    "      <button class='button blue' onclick='location.href=\"/ventilador\"'> Ventilador</button>\n"
    "      <button class='button blue' onclick='location.href=\"/aquecedor\"'>Aquecedor</button>\n"
    "      <button class='button green' onclick='location.href=\"/umidificador\"'>Umidificador</button>\n"
    "      <button class='button green' onclick='location.href=\"/desumidificador\"'>Desumidificador</button>\n"
    "    </div>\n"
    "  </div>\n"
    "</body>\n"
    "</html>\n");
    //get_alert_messages(), temperatura, umidade
    //);

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
