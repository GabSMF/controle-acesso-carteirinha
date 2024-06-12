#include <SPI.h>
#include <MFRC522.h>
#include <SdFat.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>


#define RFID_SS_PIN 53
#define RFID_RST_PIN 48


#define SD_CS_PIN 10
#define SD_MOSI_PIN 11
#define SD_MISO_PIN 12
#define SD_SCK_PIN 13


SdFat SD;
SoftSpiDriver<SD_MISO_PIN, SD_MOSI_PIN, SD_SCK_PIN> softSpi;

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
MCUFRIEND_kbv tft;

const char fileName[] = "teste.json";

bool leituraAtiva = false; 

void setup() {
  Serial.begin(9600);


  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1); 
  tft.fillScreen(0xFFFF);

  SPI.begin();
  rfid.PCD_Init();


  if (!SD.begin(SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi))) {
    Serial.println("Falha ao inicializar o cartão SD!");

    while (1);
  }
  Serial.println("Cartão SD inicializado com sucesso.");

}

void loop() {
  if (Serial.available()) {
    char comando = Serial.read();

    if (comando == '1') {
      leituraAtiva = true;
      Serial.println("Leitura de carteirinhas iniciada.");
      
    } else if (comando == '2') {
      leituraAtiva = false;
      Serial.println("Leitura de carteirinhas parada.");
    }
  }

  if (!leituraAtiva) {
    return;
  }

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  char uidString[9] = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    char hex[3];
    snprintf(hex, sizeof(hex), "%02X", rfid.uid.uidByte[i]);
    strncat(uidString, hex, sizeof(uidString) - strlen(uidString) - 1);
  }

 
  File myFile = SD.open(fileName, FILE_READ);
  if (!myFile) {
    Serial.println("Falha ao abrir o arquivo JSON.");
    return;
  }
  
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, myFile);
  myFile.close();
  
  if (error) {
    Serial.print(F("Falha ao ler o arquivo JSON: "));
    Serial.println(error.c_str());
    return;
  }

  bool encontrado = false;
  for (JsonObject aluno : doc["alunos"].as<JsonArray>()) {
    if (strcmp(aluno["uid"], uidString) == 0) {

      Serial.print("Bem vindo, ");
      Serial.println(aluno["nome"].as<const char*>());

      encontrado = true;
      break;
    }
  }

  if (!encontrado) {
    Serial.println("Aluno não cadastrado.");
    Serial.println("Deseja cadastrar este aluno? (s/n)");

    while (!Serial.available());

    String resposta = Serial.readStringUntil('\n');

    if (resposta == "s" || resposta == "S") {
      Serial.println("Digite a senha mestra:");

      while (!Serial.available());
      String senha = Serial.readStringUntil('\n');

      if (senha == "123") {
        Serial.println("Digite o nome do aluno:");

        while (!Serial.available());

        String nome = Serial.readStringUntil('\n');
        Serial.println("Digite a matrícula do aluno:");

        while (!Serial.available());

        String matricula = Serial.readStringUntil('\n');

        JsonObject novoAluno = doc["alunos"].createNestedObject();

        novoAluno["nome"] = nome;
        novoAluno["matricula"] = matricula;
        novoAluno["uid"] = uidString;

        myFile = SD.open(fileName, FILE_WRITE | O_TRUNC);

        if (!myFile) {
          Serial.println("Falha ao abrir o arquivo JSON para escrita.");
          return;
        }

        serializeJson(doc, myFile);
        myFile.close();

        Serial.println("Aluno cadastrado com sucesso.");
        
      } else {
        Serial.println("Senha incorreta.");
      }
    }
  }

  rfid.PICC_HaltA(); // Halt PICC
}
