# TempoCidad
Repositório de código para o projeto TempoCidad desenvolvido na Softex no projeto de Extensão Tecnológica

# CONEXÕES E PORTAS 


### SENSOR DHT11


Conecte pino 1 do sensor DTH11( o mais a esquerda) um 3V

Conecte pino 2 do sensor ao pino de dados definido na ESP, no caso do programa é a porta D2 

Conecte pino 4 do sensor, o pino GND, ao GND da placa 

Conecte o resistor de 10K entre pino 1, o mais a esquerda, e o pino 4 o mais a direita


### SENSOR DE VELOCIDADE LMR393

Não conecte o pino A0. 

Concecte um extremo um fio jumper ao D0 e o outro a porta D5

Conecte um extremo do fio jumper ao pino GND e a outro extremo a um GND 

Conecte um extremo do fio jumper ao pino VCC e a outro extremo a 3V 

Para exemplificar seguem as imagens abaixo 





https://arduino.esp8266.com/stable/package_esp8266com_index.json

https://dl.espressif.com/dl/package_esp32_index.json




Em seguida, a etapa de instalação da placa ilhada com a ESP32 

Segue a foto do circuito na parte traseira da placa 



Em seguida, a parte da frente da placa 


Atente para as configurações usadas em cada pino dos sensores e porta da esp 32 demonstradas abaixo. 

### Anemometro


Uma vez que, para o anemometro conecte o pino D0 na porta D15 da ESP32 e 
que no código a D15 é referenciada como 15 

### Sensor DHT11 

Por sua vez, para o sensor DHT11 conecte o pino D0 na porta D5 que no código é referenciada como 5



