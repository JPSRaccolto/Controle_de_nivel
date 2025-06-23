![image](https://github.com/user-attachments/assets/f2a5c9b8-6208-4723-8f46-1d74be421827)

# 👥 Projeto: Sistema de Controle de Nível de Água com Raspberry Pi Pico W

## 📑 Sumário

- [🎯 Objetivos](#-objetivos)
- [📋 Descrição do Projeto](#-descrição-do-projeto)
- [⚙️ Funcionalidades Implementadas](#️-funcionalidades-implementadas)
- [🛠️ Requisitos do Projeto](#️-requisitos-do-projeto)
- [📂 Estrutura do Código](#-estrutura-do-código)
- [🖥️ Como Compilar](#️-como-compilar)
- [🤝 Contribuições](#-contribuições)
- [📽️ Demonstração em Vídeo](#️-demonstração-em-vídeo)
- [💡 Considerações Finais](#-considerações-finais)

## 🎯 Objetivos

- Desenvolver um sistema de monitoramento e controle de nível de água utilizando a Raspberry Pi Pico W.
- Implementar controle automático de uma bomba d'água com base em limites mínimo e máximo.
- Fornecer uma interface web para monitoramento e configuração remota dos níveis.
- Apresentar dados em tempo real através de um display OLED.
- Criar um sistema de alertas visuais (LEDs RGB e Matriz de LEDs) e sonoros (Buzzer) para diferentes status do reservatório.

## 📋 Descrição do Projeto

Este projeto utiliza a Raspberry Pi Pico W para automatizar o controle de um reservatório de água. O sistema é composto pelos seguintes componentes:

- **Display OLED SSD1306:** Conectado via I2C (GPIO 14 e 15) para exibir o status.
- **Sensor de Nível:** Um divisor de tensão lido pela porta ADC (GPIO 28) mede o nível da água.
- **Controle da Bomba:** Um relé conectado ao GPIO 16 aciona a bomba.
- **Alertas Visuais:** Um LED RGB (GPIO 11, 13) e uma matriz de LEDs WS2812 (GPIO 7) indicam o estado do sistema.
- **Alertas Sonoros:** Dois buzzers (GPIO 10 e 21) emitem sons para níveis críticos.
- **Controles Manuais:** Dois botões (GPIO 5 e 6) permitem a interação do usuário.
- **Interface Web:** Um servidor web integrado permite o monitoramento e ajuste dos limites de qualquer dispositivo na mesma rede Wi-Fi.

## ⚙️ Funcionalidades Implementadas

1.  **Controle Automático da Bomba:**
    - A bomba é ativada (`RELE` ligado) quando o nível de água atinge o `limite_min`.
    - A bomba é desativada (`RELE` desligado) quando o nível de água atinge o `limite_max`.

2.  **Interface de Controle Web:**
    - Conecta-se a uma rede Wi-Fi para hospedar um servidor web.
    - Exibe o status da bomba (ON/OFF) e o nível atual do reservatório em porcentagem.
    - Permite que o usuário defina novos valores para os limites mínimo e máximo através da interface.

3.  **Controles Manuais e Interrupções:**
    - **Botão B (GPIO 6):** Inverte o estado atual da bomba (liga/desliga manualmente).
    - **Botão A (GPIO 5):** Reseta os limites mínimo e máximo para os valores padrão.

4.  **Display OLED:**
    - Exibe o status do nível em texto: "CHEIO", "BAIXO" ou "MEDIO".
    - Mostra uma barra de progresso visual do nível da água.
    - Informa o nível em porcentagem e o status da bomba ("BOMBA LIGADA" / "BOMBA DESLIG").

5.  **Sistema de Alertas Visuais:**
    - **LED RGB:** Acende em vermelho para nível baixo, verde para nível alto e ambos para nível médio.
    - **Matriz de LEDs WS2812:** Exibe uma seta verde para cima quando o nível está subindo, uma seta vermelha para baixo quando está descendo e um traço amarelo quando está estável.

6.  **Sistema de Alertas Sonoros:**
    - Emite um alerta sonoro específico quando o reservatório está cheio.
    - Emite um alerta sonoro diferente e mais insistente quando o reservatório está vazio.

## 🛠️ Requisitos do Projeto

- **Pico SDK:** Utilização das bibliotecas padrão da Raspberry Pi Pico.
- **Conectividade Wi-Fi:** Uso da biblioteca `pico_cyw43_arch` para a funcionalidade de rede.
- **lwIP:** Implementação da pilha TCP/IP para o servidor web.
- **Comunicação I2C:** Integração com o display OLED SSD1306.
- **Manipulação de PWM:** Controle dos buzzers.
- **Leitura ADC:** Monitoramento do sensor de nível.
- **Interrupções GPIO:** Detecção de eventos dos botões para controle manual.
- **PIO (Programmable I/O):** Controle da matriz de LEDs WS2812.

## 📂 Estrutura do código

├── controle_nivel.c         # Código principal do projeto
├── CMakeLists.txt           # Arquivo de configuração de compilação do projeto
├── lib/
│   ├── ssd1306.c            # Implementação do driver do display OLED
│   ├── ssd1306.h            # Header do driver do display OLED
│   ├── font.h               # Fonte utilizada no display
│   ├── ws2812.pio.h         # Código PIO para a matriz de LEDs WS2812
│   └── lwipopts.h           # Configurações da pilha de rede lwIP
└── ...                      # Demais arquivos de configuração (.vscode, etc.)

## 🖥️ Como Compilar

1.  Clone o repositório:
    ```bash
    git clone [https://github.com/jpsraccolto/controle_de_nivel.git](https://github.com/jpsraccolto/controle_de_nivel.git)
    ```
2.  Navegue até o diretório do projeto:
    ```bash
    cd controle_de_nivel/controle_nivel
    ```
3.  Configure as credenciais da sua rede Wi-Fi no arquivo `controle_nivel.c`:
    ```c
    #define WIFI_SSID "SUA_REDE_WIFI"
    #define WIFI_PASSWORD "SUA_SENHA_WIFI"
    ```
4.  Compile o projeto utilizando o VS Code com a extensão Raspberry Pi Pico e as tarefas de compilação configuradas.

## 🖥️ Método alternativo:

1.  Baixe o repositório como um arquivo ZIP.
2.  Extraia para uma pasta de fácil acesso.
3.  Utilize a extensão **Raspberry Pi Pico** dentro do VS Code para importar o projeto.
4.  Aguarde até que a configuração inicial crie o diretório `build`.
5.  Utilize o ícone de **Build** na barra de status para compilar o projeto.
6.  Coloque a Raspberry Pi Pico em modo **BOOTSEL** e utilize o ícone de **Run** para carregar o programa na placa.
7.  Interaja com os botões e acesse o IP do dispositivo (exibido no terminal serial) para explorar todas as funcionalidades.

## 🧑‍💻 Autores
- **João Pedro Soares Raccolto**,
- **Nivaldo Rodrigues da Silva Júnior**,
- **Rian de Sena Mendes**
- **Samuel Guedes Canário**

## 📝 Descrição

Sistema de controle de nível que utiliza uma Raspberry Pi Pico W para monitorar e manter o nível de água em um reservatório. O projeto integra hardware local, como um display OLED e LEDs, com uma interface web remota, oferecendo uma solução completa e flexível. O sistema reage a limites configuráveis, acionando uma bomba e emitindo alertas visuais e sonoros para notificar o usuário sobre o estado do reservatório, além de permitir controle manual e reconfiguração via botões físicos e interface web.

## 🤝 Contribuições

Este projeto foi desenvolvido por **João Pedro Soares Raccolto**,**Nivaldo Rodrigues da Silva Júnior**,
**Rian de Sena Mendes**,
e **Samuel Guedes Canário**. Contribuições são bem-vindas! Siga os passos abaixo para contribuir:

1.  Faça um Fork deste repositório.
2.  Crie uma nova branch:
    ```bash
    git checkout -b minha-feature
    ```
3.  Faça suas modificações e commit:
    ```bash
    git commit -m 'Minha nova feature'
    ```
4.  Envie suas alterações:
    ```bash
    git push origin minha-feature
    ```
5.  Abra um Pull Request.

## 📽️ Demonstração em Vídeo

- Um vídeo de demonstração do projeto pode ser disponibilizado aqui: [Link para o vídeo](https://drive.google.com/drive/folders/1JETavgzeSnB6-2uaEOxAcaWOotPkdZKv?usp=sharing)

## 💡 Considerações Finais

Este projeto demonstra a versatilidade da Raspberry Pi Pico W na criação de soluções de automação e IoT. A combinação de processamento em tempo real, controle de hardware e conectividade Wi-Fi permite o desenvolvimento de sistemas robustos e interativos.
