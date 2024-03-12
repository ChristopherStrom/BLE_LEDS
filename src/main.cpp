#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>
#include <FastLED.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define NUM_LEDS 24
#define DATA_PIN 17 // led data
#define LED_PIN 2   // power pin
#define DAC_PIN 4   // power pin

bool switchStateLast = LOW;
const int switchPin = 33; // PowerSwitch

BLECharacteristic *pCharacteristic;
CRGB leds[NUM_LEDS];

void R();
void G();
void Y();
void GF();
void GS();
void GreenFlash();
void rainbowChase();
void setLEDOff();
void setupLEDs();
void IO_ON();
void IO_OFF();
void checkPower();
void setupIO();


class CharacteristicCallbacks : public BLECharacteristicCallbacks
{
public:
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      if (rxValue == "R")
      {
        R();
      }
      else if (rxValue == "G")
      {
        G();
      }
      else if (rxValue == "Y")
      {
        Y();
      }
      else if (rxValue == "GF")
      {
        GF();
      }
      else if (rxValue == "GS")
      {
        GS();
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);
  setupLEDs();
  setupIO();

  BLEDevice::init("Beacon");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new CharacteristicCallbacks());

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void R()
{ // Red
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
}

void G()
{ // Green
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
}

void Y()
{ // Rainbow
  unsigned long startTime = millis(); // Store the start time
  
  // Run the rainbow effect for 3 seconds
  while (millis() - startTime < 3000) {
    rainbowChase();
    delay(50); // Adjust this delay to control the speed
  }
}

void GF()
{ // Green Flash
  setLEDOff();

  unsigned long startTime = millis();
  unsigned long currentTime = startTime;

  while (currentTime  - startTime < 3000) {
    currentTime = millis();
    GreenFlash();
    delay(100);
  }
  //Leave Green on when finished
  G();
}

void GS()
{ // Green Flash Sound
  GF();
}

void GreenFlash()
{
   for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(100);
  setLEDOff();
}

void rainbowChase() {
  static const uint8_t startHue = 0;
  static const uint8_t deltaHue = 4;
  static uint8_t pos = 0;

  // Calculate current color
  uint8_t hue = startHue + pos;

  // Fill the LEDs with the rainbow colors
  fill_rainbow(leds, NUM_LEDS, hue, deltaHue);
  FastLED.show();

  pos++;

  if (pos >= NUM_LEDS) {
    pos = 0;
  }
}

void setLEDOff()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void setupLEDs()
{
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(100); // Adjust brightness as needed
  setLEDOff();
}

void IO_ON()
{
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(DAC_PIN, HIGH);
}

void IO_OFF()
{
  digitalWrite(LED_PIN, LOW);
  digitalWrite(DAC_PIN, LOW);
  setLEDOff();
}

void checkPower()
{
  int switchState = digitalRead(switchPin);
  if (switchState == LOW && switchStateLast == HIGH)
  {
    IO_ON();
    Serial.println("Power On");
  }
  else if (switchState == HIGH && switchStateLast == LOW)
  {
    IO_OFF();
    Serial.println("Power Off");
  }
  switchStateLast = switchState;
}

void setupIO()
{
  pinMode(switchPin, INPUT_PULLUP);

  // Power pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(DAC_PIN, OUTPUT);

  IO_ON();
}

void loop()
{
  checkPower();
}
