# Sistema de Docas

O sistema controla:

- chegada de navios;
- prioridade dos navios;
- ocupação das docas;
- descarga de contêineres;
- armazenamento no pátio;
- retirada das cargas por caminhões.

O projeto utiliza recursos do FreeRTOS, como tarefas, filas, mutex e semáforos.

## Componentes

- ESP32;
- 3 LEDs;
- 3 botões;
- resistores;
- protoboard e jumpers.

## LEDs

- Verde: funcionamento normal;
- Amarelo: sistema em alerta;
- Vermelho: sistema em estresse.

## Botões

- GPIO 4: ativa o modo de estresse;
- GPIO 5: ativa o modo de alívio;
- GPIO 12: retorna ao modo normal.

## Como executar

1. Abra o projeto no VS Code com PlatformIO;
2. conecte o ESP32;
3. execute `Build`;
4. execute `Upload`;
5. abra o monitor serial em `115200 baud`.

O monitor serial mostra o estado das docas, do pátio, dos navios e dos caminhões.

## Link do video 
https://youtu.be/-LAcc64w0q0
