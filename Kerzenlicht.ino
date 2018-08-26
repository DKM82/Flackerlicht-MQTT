//Bibliotheken
  #include<ESP8266WiFi.h>
  #include<PubSubClient.h>

//Variablendefinitionen
  //WiFi
  const char*     ssid          =   "...";                        //WLAN SSID
  const char*     password      =   "...";                        //WLAN Passwort

  //MQTT
  const char*     mqtt_server   =   "...";                        //IP-Adresse des MQTT Brokers
  unsigned int    mqtt_port     =   1884;                         //Port des MQTT Brokers
  const char*     mqtt_user     =   "...";                        //Benutzername des MQTT-Brokers
  const char*     mqtt_pass     =   "...";                        //Passwort des MQTT Passwortes

  String          mqtt_name     =   "Flackerlicht";               //Name des Gerätes beim MQTT Brokers
  String          mqtt_topic    =   String(mqtt_name + "/data");  
  unsigned int    zustand       =   0;

//Definitionen LED-Pin's
  int             ledPin1       =   4;    //D2 - Rot
  int             ledPin2       =   5;    //D1 - Gelb
  int             ledPin3       =   12;   //D6 - Gelb
  int             ledPin4       =   14;   //D5 - Gelb
  #define intLED D0  //D0 - interne LED
  
//Sonstiges
  WiFiClient espClient;
  PubSubClient client(espClient);

void setup() {
  pinMode (ledPin1, OUTPUT);
  pinMode (ledPin2, OUTPUT);
  pinMode (ledPin3, OUTPUT);
  pinMode (ledPin4, OUTPUT);
  pinMode (intLED, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  zustand = 0;
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Verbinde mit SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi verbunden");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  char msg[length+1];
  for (int i = 0; i< length; i++) {
    msg[i] = (char)payload[i];
  }

  msg[length] = '\0';

  if (String(topic) + "/" + String(msg) == "/Flackerlicht/data/Zustand/false") {
    zustand = 0;
    client.publish(String("/" + mqtt_name + "/data/Info").c_str(), "Licht aus");
    client.publish(String("/" + mqtt_name + "/data/Flackern").c_str(), "false");
  }
  else if (String(topic) + "/" + String(msg) == "/Flackerlicht/data/Zustand/true") {
    zustand = 1;
    client.publish(String("/" + mqtt_name + "/data/Info").c_str(), "Licht ein");
  }
  else if (String(topic) + "/" + String(msg) == "/Flackerlicht/data/Flackern/true") {
    zustand = 2;
    client.publish(String("/" + mqtt_name + "/data/Info").c_str(), "Flackern ein");
    client.publish(String("/" + mqtt_name + "/data/Zustand").c_str(), "true");
  }
  else if (String(topic) + "/" + String(msg) == "/Flackerlicht/data/Flackern/false") {
    zustand = 1;
    client.publish(String("/" + mqtt_name + "/data/Zustand").c_str(), "true");
    client.publish(String("/" + mqtt_name + "/data/Info").c_str(), "Flackern aus");
    //  client.publish(String("/" + mqtt_name + "/data/Flackern").c_str(), "false");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Verbindungsaufbau mit MQTT-Server...");
    if (!client.connect(mqtt_name.c_str(), mqtt_user, mqtt_pass)) {
      Serial.print("fehlgeschlagen, rc=");
      Serial.print(client.state());
      Serial.println(" Wiederholung in 5 Sekunden");
      delay(5000);
    }
  }
  client.subscribe(String("/" + mqtt_name + "/data/Zustand").c_str());
  client.subscribe(String("/" + mqtt_name + "/data/Flackern").c_str());
  Serial.println("MQTT verbunden");
}

void Licht() {
  switch(zustand) {
    case 0:
      Licht_aus();
      break;

    case 1:
      Licht_an();
      break;

    case 2:
      Licht_Flackern_an();
      break;
  }
}

void Licht_Flackern_an() {
  digitalWrite(intLED, HIGH);
  analogWrite(ledPin1, random(30,70));
  analogWrite(ledPin2, random(50,150));
  analogWrite(ledPin3, random(50,150));
  analogWrite(ledPin4, random(50,150));
  delay(random(80, 130));
}

void Licht_an() {
  digitalWrite(intLED, HIGH);
  analogWrite(ledPin1, LOW);
  analogWrite(ledPin2, 150);
  analogWrite(ledPin3, 150);
  analogWrite(ledPin4, 150);  
}

void Licht_aus() {
  digitalWrite(intLED, LOW);
  analogWrite(ledPin1, LOW);
  analogWrite(ledPin2, LOW);
  analogWrite(ledPin3, LOW);
  analogWrite(ledPin4, LOW);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Licht();
  delay(5);
}




