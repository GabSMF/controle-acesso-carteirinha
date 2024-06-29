#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <SoftwareSerial.h>


String ssid = "Projeto";
String password = "2022-11-07";
const String serverName = "https://1xrlin-ip-139-82-247-136.tunnelmole.net/passaPresenca";
//const String serverName = "https://215fd557-a5b8-4fcd-aa2d-0f76fbb13625-00-4flode0l653d.kirk.replit.dev/message";
JsonDocument doc;
StaticJsonDocument<200> docPresenca;
String matriculas[5] = { "132435", "123414", "978758", "587576", "788776" };
int horarios[5];
String weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

String hora;
String arduino_says;

String horaRTC;
time_t hora_ntp;

bool inicioPrograma = false;
bool esp_tah_ativo = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "a.st1.ntp.br", -3 * 3600, 60000);

unsigned long millisNTP = millis();
unsigned long millisAtualizaRTC = millis();

const byte rxPin = 12;
const byte txPin = 14;

SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);


void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);


  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Wifi connected!");


  timeClient.begin();
  timeClient.update();
  unsigned long hora_ntp = timeClient.getEpochTime();

  mySerial.println("Conexao com ESP8266 estabelecida!");
}

void create_nested_json() {

  JsonObject obj;
  for (int i = 0; i < 5; i++) {
    obj = docPresenca.createNestedObject();
    obj["matricula"] = matriculas[i];
    obj["horario"] = hora;
    obj["turma"] = "33A";
  }
}

void loop() {

  if (millis() - millisNTP > 5000) {
    timeClient.update();
    hora_ntp = timeClient.getEpochTime();
    Serial.println("NTP Time server: " + timeClient.getFormattedTime());
    hora = timeClient.getFormattedTime();
    millisNTP = millis();
  }

  if (inicioPrograma == false){
    if(mySerial.available())
    {
      Serial.println("aqui");
      String arduino_says = mySerial.readStringUntil('\n');
      arduino_says.trim();
      if (arduino_says=="comecou programa"){
        Serial.println("entremos");
        inicioPrograma = true;
        mySerial.println(hora_ntp);
      }
    }
  }

  if(millis() - millisAtualizaRTC > 1800000) //se já se passou mais de meia hora manda o horario de novo
  {
    if(Serial.available())
    {
      mySerial.println(hora_ntp);
    }
  }


  if (Serial.available()) {
    Serial.println("aqui na Serial!");
    if (arduino_says == "comecou programa") {
      Serial.println("entremos");
      mySerial.println(hora_ntp);
    }
    //texto.trim();
    //mySerial.print(texto);
  }


  if (mySerial.available()) {
    Serial.println("Arduino mandou o doc de presencas e cadastros!");
    //horaRTC = mySerial.readStringUntil('\n');
    //texto.trim();
    //comunicação com o arduino
    StaticJsonDocument<5000> docPres;
    DeserializationError err = deserializeJson(docPres, mySerial);

    if (err == DeserializationError::Ok) {
      Serial.println(doc["presenca"].as<String>());
      Serial.println(doc["cadastro"].as<String>());
    } else {
      Serial.print("deserializeJson() returned ");
      Serial.println(err.c_str());
    }

    Serial.println("RTC time: " + horaRTC);
  }

  create_nested_json();


  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    // Ignore SSL certificate validation
    client->setInsecure();


    HTTPClient https;


    if (https.begin(*client, serverName)) {
      https.addHeader("Content-Type", "application/json");
      https.addHeader("ngrok-skip-browser-warning", "true");
      Serial.print("[HTTPS] POST...\n");
    }

    String json;


    StaticJsonDocument<200> docTurma;
    JsonObject turma;
    turma = docTurma.createNestedObject();

    String data;
    struct tm *ptm = gmtime((time_t *)&hora_ntp);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;

    String currentDate = String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear);

    turma["data"] = currentDate;
    turma["presenca"] = docPresenca;

    //serializeJson(docTurma,Serial);

    //subindo dados para o servidor
    serializeJson(docTurma, json);

    //json = "{\"key\":\"value\"}";

    String jsonPresenca;
    String jsonCadastro;

    //int httpCode1 = https.POST(jsonPresenca);
    //int httpCode2 = https.POST(jsonCadastro);

    int httpCode = https.POST(json);

    //Serial.println(httpCode);

    // httpCode will be negative on error
    //   if (httpCode > 0)
    //   {
    //     // HTTP header has been send and Server response header has been handled
    //     Serial.printf("[HTTP] POST... code: %d\n", httpCode);


    //     // file found at server
    //     if (httpCode == HTTP_CODE_OK)
    //     {
    //       const String& payload = https.getString();
    //       Serial.println("received payload:\n<<");
    //       Serial.println(payload);
    //       Serial.println(">>");
    //     }
    //   }
    //   else
    //   {
    //     Serial.printf("[HTTP] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
    //   }
    //   https.end();
    // }
    // else
    // {
    //   Serial.println("Desconnected Wifi!");
    // }
  }
}
