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
    Serial.println("Comandos disponibles:");
    Serial.println("- Escribe un valor binario (8 bits) para mostrar en el display");
    Serial.println("- 'READ': Leer todo lo que está en el display");
    Serial.println("- 'CLEAR': Limpiar el display");
    Serial.println("- 'CUSTOM,index,pattern': Crear carácter personalizado (5x8)");
    Serial.println("  (index: 0-7, pattern: 8 valores binarios de 5 bits)");
    Serial.println("  Ejemplo: CUSTOM,0,01110,10001,10001,11111,10001,10001,10001,00000");
    Serial.println("- 'SHOWCUSTOM,index': Mostrar carácter personalizado");

    // Inicializar el display
    initializeDisplay();
    Serial.println("Display inicializado.");
}

void loop() {
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        input.toUpperCase();

        if (input.equalsIgnoreCase("READ")) {
            readDisplay();
        } else if (input.equalsIgnoreCase("CLEAR")) {
            clearDisplay();
            Serial.println("Display limpiado.");
        } else if (input.startsWith("CUSTOM,")) {
            processCustomCharCommand(input);
        } else if (input.startsWith("SHOWCUSTOM,")) {
            processShowCustomCommand(input);
        } else if (input.length() == 8 && isBinary(input)) {
            byte value = binaryToByte(input);
            printToLCD(value);
            Serial.print("Mostrando en el display: ");
            Serial.println(input);
        } else {
            Serial.println("Entrada no válida. Ingresa 'READ', 'CLEAR', 'CUSTOM,index,pattern', 'SHOWCUSTOM,index' o un valor binario (8 bits).");
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

// Imprimir en posición correcta
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

// Limpiar display
void clearDisplay() {
    sendCommand(0x01); // Comando para limpiar display
    currentRow = 0;
    currentCol = 0;
    delay(2); // Esperar a que se complete el comando
}

// Procesar comando para crear carácter personalizado (5x8)
void processCustomCharCommand(String input) {
    // Formato: CUSTOM,index,pattern1,pattern2,...,pattern8
    input = input.substring(7); // Remover "CUSTOM,"
    
    int firstComma = input.indexOf(',');
    if (firstComma == -1) {
        Serial.println("Formato incorrecto. Usa: CUSTOM,index,pattern1,pattern2,...,pattern8");
        return;
    }
    
    int index = input.substring(0, firstComma).toInt();
    if (index < 0 || index > 7) {
        Serial.println("Índice debe estar entre 0 y 7");
        return;
    }
    
    String patterns = input.substring(firstComma + 1);
    byte charPattern[8];
    int patternCount = 0;
    
    while (patterns.length() > 0 && patternCount < 8) {
        int commaPos = patterns.indexOf(',');
        String patternStr;
        
        if (commaPos == -1) {
            patternStr = patterns;
            patterns = "";
        } else {
            patternStr = patterns.substring(0, commaPos);
            patterns = patterns.substring(commaPos + 1);
        }
        
        patternStr.trim();
        
        // Validar que sea binario de máximo 5 bits
        if (patternStr.length() <= 5 && isBinary5Bit(patternStr)) {
            // Convertir a byte (solo los 5 bits menos significativos)
            charPattern[patternCount] = binary5ToByte(patternStr);
            patternCount++;
        } else {
            Serial.println("Patrón no válido (máx 5 bits): " + patternStr);
            return;
        }
    }
    
    if (patternCount != 8) {
        Serial.println("Se requieren exactamente 8 patrones (filas)");
        return;
    }
    
    createCustomChar(index, charPattern);
    Serial.print("Carácter personalizado creado en índice ");
    Serial.println(index);
}

// Procesar comando para mostrar carácter personalizado
void processShowCustomCommand(String input) {
    // Formato: SHOWCUSTOM,index
    input = input.substring(11); // Remover "SHOWCUSTOM,"
    
    int index = input.toInt();
    if (index < 0 || index > 7) {
        Serial.println("Índice debe estar entre 0 y 7");
        return;
    }
    
    // Los caracteres personalizados se muestran con los códigos 0-7
    printToLCD(index);
    Serial.print("Mostrando carácter personalizado ");
    Serial.println(index);
}

// Crear carácter personalizado en CGRAM (5x8)
void createCustomChar(int index, byte pattern[]) {
    // Establecer dirección en CGRAM (0x40 + index * 8)
    sendCommand(0x40 | (index << 3));
    
    // Escribir los 8 patrones del carácter (5 bits cada uno)
    for (int i = 0; i < 8; i++) {
        sendData(pattern[i]);
    }
    
    // Volver a DDRAM para escritura normal
    sendCommand(0x80);
}

// Validar si la entrada es binaria (8 bits)
bool isBinary(String input) {
    for (char c : input) {
        if (c != '0' && c != '1') return false;
    }
    return true;
}

// Validar si la entrada es binaria de máximo 5 bits
bool isBinary5Bit(String input) {
    if (input.length() > 5) return false;
    for (char c : input) {
        if (c != '0' && c != '1') return false;
    }
    return true;
}

// Convertir cadena binaria de 5 bits a byte
byte binary5ToByte(String binary) {
    byte value = 0;
    int length = binary.length();
    
    // Asegurar que solo usamos los 5 bits menos significativos
    for (int i = 0; i < length; i++) {
        value <<= 1;
        if (binary[i] == '1') value |= 1;
    }
    
    return value;
}

// Convertir cadena binaria a byte (8 bits)
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
            
            // Mostrar carácter personalizado o normal
            if (data < 8) {
                Serial.print("[CUSTOM");
                Serial.print(data);
                Serial.print("]");
            } else {
                Serial.print((char)data);
            }
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
