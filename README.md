# Estufa Inteligente com Monitoramento e Alarme via Webserver

---

## Descrição do Projeto
Desenvolver um sistema embarcado para monitoramento da temperatura em uma estufa utilizando a Raspberry Pi Pico W. O sistema deve exibir os dados localmente (display OLED), sinalizar condições críticas com LED e buzzer, e disponibilizar essas informações via servidor web acessível por dispositivos conectados à rede Wi-Fi.

---

## Requisitos

- **Microcontrolador**: Raspberry Pi Pico ou Raspberry Pi Pico W (opcional)
- **Placa de Desenvolvimento:** BitDogLab (opcional).
- **Conta Criada no Wokwi Simulator**: [Wokwi](https://wokwi.com/).
- **Editor de Código**: Visual Studio Code (VS Code).
- **SDK do Raspberry Pi Pico** configurado no sistema.
- Ferramentas de build: **CMake** e **Ninja**.

---

## Instruções de Uso

### 1. Clone o Repositório

Clone o repositório para o seu computador:
```bash
git clone https://github.com/rafaelsouz10/conn-iot.git
cd conn-iot
code .
```
---

### 2. Instale as Dependências

Certifique-se de que o SDK do Raspberry Pi Pico está configurado corretamente no VS Code. As extensões recomendadas são:

- **C/C++** (Microsoft).
- **CMake Tools**.
- **Wokwi Simulator**.
- **Raspberry Pi Pico**.

---

### 3. Configure o VS Code

Abra o projeto no Visual Studio Code e siga as etapas abaixo:

1. Certifique-se de que as extensões mencionadas anteriormente estão instaladas.
2. No terminal do VS Code, compile o código clicando em "Compile Project" na interface da extensão do Raspberry Pi Pico.
3. O processo gerará o arquivo `.uf2` necessário para a execução no hardware real.

---

### 4. Teste no Simulador Wokwi Integrado ao VS Code

Após ter configurado o VS Code conforme descrito no item 3, siga os passos abaixo para simular o projeto:

1. Abra o arquivo `diagram.json` no Visual Studio Code.
2. Clique no botão "Play" disponível na interface.
3. Divirta-se interagindo com os componentes disponíveis no simulador integrado!

---

### 5. Teste no Hardware Real

#### Utilizando a Placa de Desenvolvimento BitDogLab com Raspberry Pi Pico W:

1. Conecte a placa ao computador no modo BOTSEEL:
   - Pressione o botão **BOOTSEL** (localizado na parte de trás da placa de desenvolvimento) e, em seguida, o botão **RESET** (localizado na frente da placa).
   - Após alguns segundos, solte o botão **RESET** e, logo depois, solte o botão **BOOTSEL**.
   - A placa entrará no modo de programação.

2. Compile o projeto no VS Code utilizando a extensão do [Raspberry Pi Pico W](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico):
   - Clique em **Compile Project**.

3. Execute o projeto clicando em **Run Project USB**, localizado abaixo do botão **Compile Project**.

---

### 🔌 6. Conexões e Esquemático
Abaixo está o mapeamento de conexões entre os componentes e a Raspberry Pi Pico W:

| **Componentes**        | **Pino Conectado (GPIO)** |
|------------------------|---------------------------|
| Display SSD1306 (SDA)  | GPIO 14                   |
| Display SSD1306 (SCL)  | GPIO 15                   |
| Matriz LED RGB         | GPIO 7                    |
| LED RGB Vermelho       | GPIO 13                   |
| LED RGB Verde          | GPIO 11                   |
| Buzzer                 | GPIO 21                   |
| Botão A                | GPIO 5                    |
| Botão B                | GPIO 6                    |
| DHT22                  | GPIO 8                    |


#### 🛠️ Hardware Utilizado
- **Microcontrolador Raspberry Pi Pico W**
- **Display OLED SSD1306 (I2C)**
- **Botão A**
- **Buzzer**
- **LED RGB**
- **Wi-Fi (CYW43439)**
- **DHT22**

---


### 7. Funcionamento do Sistema

#### 📌 Funcionalidades

**O sistema realiza a leitura de temperatura de forma simulada (potenciômetro) para que seja possível perceber a variação do parâmetro simulado e o acionamento do alarme e também de forma real (sensor DHT22).**

Os valores lidos pelo **potenciômetro** são comparados com limites estabelecidos. Com base nisso:

- **LEDs** sinalizam o estado do ambiente:
   - **Led verde: temperatura ideal**
   - **Led azul: frio demais**
   - **Led vermelho: quente demais**

- O **buzzer** é ativado em **situações críticas**;

- O **display OLED** exibe as informações em **tempo real**;

- Um **servidor web** permite o **monitoramento remoto** e o silenciamento do alarme via **interface HTML**.

O **usuário** pode acessar a **página do Webserver** pela rede Wi-Fi para verificar a **temperatura simulada, os parâmetros lidos pelo DHT22, o estado do sistema e desativar o alarme**.

---

### 8. Vídeos Demonstrativo

**Click [AQUI](https://drive.google.com/file/d/1ExtOO20hji8XOA8F3JFLqy2kIRu8l-Y-/view?usp=sharing) para acessar o link do Vídeo Ensaio**

Obs: Caso queira ver o vídeo referente a atividade 1 (parcial) do projeto:

Click [AQUI](https://drive.google.com/file/d/1GTvek5WsZRavbE1PbcevZN9vSysE3bh8/view?usp=sharing) para acessar o link do Vídeo Ensaio Parcial
