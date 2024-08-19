// library https://arduino.esp8266.com/stable/package_esp8266com_index.json
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <max6675.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//#define WIFI_SSID "OldCity_Fam" 
//#define WIFI_PASSWORD "Ppopoopo12"

// #define WIFI_SSID "pengungsian"
// #define WIFI_PASSWORD "awasadabolu"

#define WIFI_SSID "reca"
#define WIFI_PASSWORD "password"

//#define WIFI_SSID "awl" 
//#define WIFI_PASSWORD "12345679"

//#define WIFI_SSID "OldCity_Fam_plus" 
//#define WIFI_PASSWORD "Ppopoopo12Zxx"

// #define WIFI_SSID "Redmi A1" 
// #define WIFI_PASSWORD "cobainkak"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDByniTMlVXLBN9puuef9n_e8fX1svB7ss"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://bara-database-default-rtdb.asia-southeast1.firebasedatabase.app/" 

// Inisialisasi objek LCD dengan alamat I2C 0x27, 16 kolom, dan 2 baris
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definisi pin untuk sensor suhu MAX6675
#define SCK D5
#define CS D8
#define SO D6

// Inisialisasi objek MAX6675
MAX6675 thermocouple(SCK, CS, SO); 

#define Relay D7 // The ESP8266 pin connected to the IN1 pin of relay module

// Define Firebase Data object.
FirebaseData fbdo;

// Define firebase authentication.
FirebaseAuth auth;

// Definee firebase configuration.
FirebaseConfig config;

// Millis variable to send/store data to firebase database.
unsigned long sendDataPrevMillis = 0;

// Boolean variable for sign in status.
bool signupOK = false;


void setup() {
  Serial.begin(115200);

  pinMode(Relay, OUTPUT);

  // Mulai LCD
  lcd.init();
  lcd.backlight();

  // Tampilkan pesan awal di LCD
  lcd.print("Connecting");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
   Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

/* Assign the api key (required) */
  config.api_key = API_KEY;

/* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
if (Firebase.signUp(&config, &auth, "", "")){
  Serial.println("Firebase sign up success");
  signupOK = true;
}else{
  Serial.printf("%s\n", config.signer.signupError.message.c_str());//menampilkan pesan kesalahan pendaftaran yang disimpan dalam objek config 
}
  config.token_status_callback = tokenStatusCallback; //untuk mengautentikasi koneksi antara aplikasi dan Firebase.
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}


void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 200 || sendDataPrevMillis == 0)){ //memperbarui waktu terakhir kali data dikirim, yang digunakan untuk menghitung waktu interval pengiriman berikutnya.
    sendDataPrevMillis = millis();

  float temperature = thermocouple.readCelsius();  

  lcd.clear(); // Bersihkan layar LCD
  lcd.setCursor(0, 0);
  lcd.print("FAN  : ");


  if (temperature < 32.00) { //membuat kondisi jika temp kurang dari sama dengan 40 maka...
    digitalWrite(Relay, LOW); //relay mati
    Serial.println("FAN OFF"); //mencetak keadaan fan off di serial print
    lcd.setCursor(7,0); //tata letak tampilan di LCD
    lcd.print("OFF"); //mencetak String "OFF" pada lcd
      
   }

  else if (temperature > 32.00) { //membuat kondisi jika temp lebih dari sama dengan 41 maka...
    digitalWrite(Relay, HIGH);  // relay hidup
    Serial.println("FAN ON"); // mencetak keadaan fan on di serial print
    lcd.setCursor(7,0); //tata letak tampilan di LCD
    lcd.print("ON"); //mencetak String "ON" pada lcd
      
    }

  // Baca suhu dari sensor MAX6675
  lcd.setCursor(0, 1);
  lcd.print("Temp : ");
  lcd.print(temperature);
  lcd.print("C");

  // Tampilkan suhu pada Serial Monitor dan beri informasi tambahan
  Serial.print("Temp: ");
  Serial.print(temperature);
  if (temperature > 100){
    Serial.println("C (GOOD)");
  } else {
    Serial.println("C (WEAK)");
  }
  // Tunggu 2 detik sebelum membaca ulang sensor
  delay(1000);

  if(temperature > 32.00){
  (Firebase.RTDB.setString(&fbdo, "ESP8266/FAN", "ON")); //rtbd(real time database) membaca dan menulis ke database 
  }

  if(temperature < 32.00){
  (Firebase.RTDB.setString(&fbdo, "ESP8266/FAN", "OFF")); //fbdo(firebase data object) menyimpan operasi database firebase
  }

  Serial.println();
  //Serial.print(fan);
  Serial.print("- successfully saved to: " + fbdo.dataPath());
  Serial.println(" (" +fbdo.dataType() + ")");


if(Firebase.RTDB.setFloat(&fbdo, "ESP8266/Temp", temperature)){
  Serial.println();Serial.print(temperature);
  Serial.print("- successfully saved to: " + fbdo.dataPath());
  Serial.println(" (" +fbdo.dataType() + ")");
}else{
  Serial.println("FAILED" +fbdo.errorReason());
}  
}
}