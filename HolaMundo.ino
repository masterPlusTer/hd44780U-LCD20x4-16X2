// Pines para backlight y control
const int backlightPin = A3; // Backlight
const int rs = 9;          // RS
const int rw = 10;           // RW
const int e = 11;            // Enable

const int dataPins[8] = {2, 3, 4, 5, 6, 7, 8, 12}; // DB0-DB7

void setup() {
    pinMode(backlightPin, OUTPUT);
    analogWrite(backlightPin, 200); // Encender backlight
    pinMode(rs, OUTPUT);
    pinMode(rw, OUTPUT);
    pinMode(e, OUTPUT);
    for (int i = 0; i < 8; i++) {
        pinMode(dataPins[i], OUTPUT);
    }

    Serial.begin(9600);
    Serial.println("Prueba con DB7 en pin 12...");

    // Inicializar el display
    initializeDisplay();

    // Escribir texto básico
    sendCommand(0x80); // Cursor al inicio
    sendData('h');
    sendData('o');
    sendData('l');
    sendData('a');
    sendData(' ');
    sendData('m');
    sendData('u');
    sendData('n');
    sendData('d');
    sendData('o');
    sendData('!');
    sendData('!');
    sendData('!');
    sendData('!');
    sendData('!');
    sendData('!');
    sendData('!');
    Serial.println("Escritura completada.");
}

void loop() {}

// Inicializar el display
void initializeDisplay() {
    delay(50); // Espera inicial

    sendCommand(0x30); // Modo 8 bits
    delay(5);
    sendCommand(0x30); // Repetir configuración
    delayMicroseconds(150);
    sendCommand(0x30); // Última configuración
    delayMicroseconds(150);

    sendCommand(0x38); // Modo 8 bits, 2 líneas, 5x8 puntos
    sendCommand(0x0C); // Display ON, cursor OFF
    sendCommand(0x06); // Incrementar cursor automáticamente
    sendCommand(0x01); // Limpiar pantalla
    delay(2);
}

// Enviar un comando al display
void sendCommand(byte command) {
    delay(2);
    digitalWrite(rs, LOW); // RS = 0 para comando
    digitalWrite(rw, LOW); // RW = 0 para escritura
    write8Bits(command);
    pulseEnable();
}

// Enviar datos al display
void sendData(byte data) {
    delay(2);
    digitalWrite(rs, HIGH); // RS = 1 para datos
    digitalWrite(rw, LOW);  // RW = 0 para escritura
    write8Bits(data);
    pulseEnable();
}

// Escribir 8 bits en el display
void write8Bits(byte value) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(dataPins[i], (value >> i) & 0x01);
    }
}

// Generar un pulso en Enable
void pulseEnable() {
    digitalWrite(e, HIGH);
    delayMicroseconds(1);
    digitalWrite(e, LOW);
    delayMicroseconds(100);
}
