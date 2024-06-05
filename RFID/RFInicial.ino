#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 7

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// Struct para armazenar informações dos alunos
struct Aluno {
  char nome[32];
  char matricula[8];
  char uid[5];
};

// Vetor fixo de alunos
Aluno alunos[] = {
  {"Joao Silva", "20220101", "04A3"},
  {"Maria Souza", "20220102", "A1B2"},
  {"Carlos Lima", "20220103", "B5C6"}
};

int numAlunos = sizeof(alunos) / sizeof(alunos[0]);

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init RC522
}

void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been read
  if (!rfid.PICC_ReadCardSerial())
    return;

  // Convert the UID to a string
  char uidString[5] = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    char hex[3];
    snprintf(hex, sizeof(hex), "%02X", rfid.uid.uidByte[i]);
    strncat(uidString, hex, sizeof(uidString) - strlen(uidString) - 1);
  }

  // Verifica se o UID está na lista de alunos
  bool encontrado = false;
  for (int i = 0; i < numAlunos; i++) {
    if (strncmp(alunos[i].uid, uidString, sizeof(alunos[i].uid)) == 0) {
      Serial.print("Bem vindo, ");
      Serial.println(alunos[i].nome);
      encontrado = true;
      break;
    }
  }

  if (!encontrado) {
    Serial.println("Aluno não cadastrado.");
  }

  rfid.PICC_HaltA(); // Halt PICC
}

// Routine to dump a byte array as hex values to Serial. 
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
