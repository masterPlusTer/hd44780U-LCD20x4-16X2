// Pines para backlight y control
const int backlightPin = A3; // Backlight
const int rs = 9;           // RS
const int rw = 10;          // RW
const int e = 11;           // Enable
const int dataPins[8] = {2, 3, 4, 5, 6, 7, 8, 12}; // DB0-DB7

// Tamaño del display
const int columns = 16; // Número de columnas
const int rows = 2;     // Número de filas

// Control de cursor
int currentRow = 0;
int currentCol = 0;

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
    Serial.println("Escribe un valor binario (8 bits) para mostrar en el display.");
    Serial.println("Escribe 'READ' para leer todo lo que está en el display.");

    // Inicializar el display
    initializeDisplay();
    Serial.println("Display inicializado.");
}

void loop() {
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.equalsIgnoreCase("READ")) {
            readDisplay();
        } else if (input.length() == 8 && isBinary(input)) {
            byte value = binaryToByte(input);
            printToLCD(value); // usar posicionamiento automático
            Serial.print("Mostrando en el display: ");
            Serial.println(input);
        } else {
            Serial.println("Entrada no válida. Ingresa 'READ' o un valor binario (8 bits).");
        }
    }
}

// Inicializar el display
void initializeDisplay() {
    waitUntilReady();
    sendCommand(0x30);
    sendCommand(0x30);
    sendCommand(0x30);
    sendCommand(0x38); // Modo 8 bits, 2 líneas, 5x8 puntos
    sendCommand(0x0C); // Display ON, cursor OFF
    sendCommand(0x06); // Incrementar cursor automáticamente
    sendCommand(0x01); // Limpiar pantalla
    currentRow = 0;
    currentCol = 0;
}

// Nueva función: imprime en posición correcta
void printToLCD(byte value) {
    // Mueve cursor a la posición actual
    sendCommand(0x80 | getAddress(currentRow, currentCol));
    sendData(value);

    // Avanza
    currentCol++;
    if (currentCol >= columns) {
        currentCol = 0;
        currentRow++;
        if (currentRow >= rows) {
            currentRow = 0; // volver a la primera fila
        }
    }
}

// Validar si la entrada es binaria
bool isBinary(String input) {
    for (char c : input) {
        if (c != '0' && c != '1') return false;
    }
    return true;
}

// Convertir cadena binaria a byte
byte binaryToByte(String binary) {
    byte value = 0;
    for (int i = 0; i < 8; i++) {
        value <<= 1;
        if (binary[i] == '1') value |= 1;
    }
    return value;
}

// Leer contenido completo del display
void readDisplay() {
    Serial.println("Leyendo contenido completo del display:");

    for (int row = 0; row < rows; row++) {
        Serial.print("Fila ");
        Serial.print(row + 1);
        Serial.print(": ");

        for (int col = 0; col < columns; col++) {
            int address = getAddress(row, col);
            sendCommand(0x80 | address);
            byte data = readData();
            Serial.print((char)data);
        }
        Serial.println();
    }
}

// Mapear fila y columna a dirección DDRAM (16x2 real)
int getAddress(int row, int col) {
    switch (row) {
        case 0: return 0x00 + col; // Fila 1
        case 1: return 0x40 + col; // Fila 2
        default: return 0x00;
    }
}

// Leer datos del display
byte readData() {
    for (int i = 0; i < 8; i++) pinMode(dataPins[i], INPUT);

    digitalWrite(rs, HIGH);
    digitalWrite(rw, HIGH);
    digitalWrite(e, HIGH);
    delayMicroseconds(1);

    byte value = 0;
    for (int i = 0; i < 8; i++) {
        value |= (digitalRead(dataPins[i]) << i);
    }

    digitalWrite(e, LOW);
    delayMicroseconds(100);

    for (int i = 0; i < 8; i++) pinMode(dataPins[i], OUTPUT);

    return value;
}

// Enviar un comando
void sendCommand(byte command) {
    waitUntilReady();
    digitalWrite(rs, LOW);
    digitalWrite(rw, LOW);
    write8Bits(command);
    pulseEnable();
}

// Enviar datos
void sendData(byte data) {
    waitUntilReady();
    digitalWrite(rs, HIGH);
    digitalWrite(rw, LOW);
    write8Bits(data);
    pulseEnable();
}

// Escribir 8 bits
void write8Bits(byte value) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(dataPins[i], (value >> i) & 0x01);
    }
}

// Pulso Enable
void pulseEnable() {
    digitalWrite(e, HIGH);
    delayMicroseconds(1);
    digitalWrite(e, LOW);
    delayMicroseconds(100);
}

// Esperar hasta que el display esté listo
void waitUntilReady() {
    pinMode(dataPins[7], INPUT);
    digitalWrite(rs, LOW);
    digitalWrite(rw, HIGH);

    bool busy = true;
    while (busy) {
        digitalWrite(e, HIGH);
        delayMicroseconds(1);
        busy = digitalRead(dataPins[7]);
        digitalWrite(e, LOW);
        delayMicroseconds(100);
    }

    pinMode(dataPins[7], OUTPUT);
    digitalWrite(rw, LOW);
}
