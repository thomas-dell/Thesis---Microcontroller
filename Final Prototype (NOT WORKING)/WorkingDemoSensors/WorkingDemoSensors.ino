#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h> // Custom Arduino Library, to allow for alteration of Gyroscope and Accelerometer Sample Rates
#include <SPI.h>

#define PAYLOAD_SIZE 160 // Multiple of 4

BLEService SensorModule("01BD1279-6A80-43C1-84D4-7E27C59ADE19"); // Bluetooth® Low Energy LED Service
BLECharacteristic SensorData("3E4C2C4F-1D2C-4ABB-98DC-F29052F1AF74", BLERead | BLENotify, PAYLOAD_SIZE, true);

// Status LED Output Pin Assignments
#define BLE_CONN 2
#define BLE_DATA_SEND 4
#define BLE_CENT_FUNC 6
#define BLE_PERI_FUNC 8

#define SAMPLE_RATE 3 // Integer value between 1 and 6 
/*
  Sample Rate Values:
  1 - 14.9 Hz
  2 - 59.5 Hz
  3 - 119 Hz
  4 - 238 Hz
  5 - 476 Hz
  6 - 952 Hz
*/

unsigned long StartReadTime = 0;
int BytesPerSample = 16;

void setup() {
  // Serial connection is unnecessary as this code runs on an embedded MCU, but is helpful for debugging.
  Serial.begin(1000000);
  // while(!Serial);

  // Set status LED pin modes to output, reset them low
  pinMode(BLE_CONN, OUTPUT);
  pinMode(BLE_DATA_SEND, OUTPUT);
  pinMode(BLE_CENT_FUNC, OUTPUT);
  pinMode(BLE_PERI_FUNC, OUTPUT);
  digitalWrite(BLE_CONN, LOW);
  digitalWrite(BLE_DATA_SEND, LOW);
  digitalWrite(BLE_CENT_FUNC, LOW);
  digitalWrite(BLE_PERI_FUNC, LOW);

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  // Set connection interval to a minimum of 7.5ms and a maximum of 400ms
  //BLE.setConnectionInterval(0x0006, 0x0140);

  // Set advertised local name and service UUID:
  BLE.setLocalName("Sensor_Module");
  BLE.setAdvertisedService(SensorModule);

  // Add characteristics to the service
  SensorModule.addCharacteristic(SensorData);

  // Add service
  BLE.addService(SensorModule);

  // Set the initial value for the characeristics:
  SensorData.writeValue("");

  // Wait for IMU module to activate
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  IMU.setSampleRate(SAMPLE_RATE); // Default 3 (119Hz)

  Serial.println("Scanning...");
}

void loop() {
  BLE.advertise();
  BLE.scanForName("Relay_Module");
  BLEDevice RelayCent = BLE.central();
  BLEDevice RelayPeri = BLE.available();

  if (RelayCent) {
    BLE.stopScan();
    Serial.print("Connected to central: ");
    Serial.println(RelayCent.address());
    if (RelayCent.connected()) {
      digitalWrite(BLE_CONN, HIGH);
      digitalWrite(BLE_CENT_FUNC, HIGH);
      ReadOperation(RelayCent);
    }

  }

  if (RelayPeri) {
    BLE.stopAdvertise();
    Serial.print("Found ");
    Serial.print(RelayPeri.address());
    Serial.print(" '");
    Serial.print(RelayPeri.localName());
    Serial.print("' ");
    Serial.print(RelayPeri.advertisedServiceUuid());
    Serial.println();
    if (RelayPeri.connected()) {
      digitalWrite(BLE_CONN, HIGH);
      digitalWrite(BLE_PERI_FUNC, HIGH);
      StimOperation(RelayPeri);
    }
  }
}

int ReadOperation(BLEDevice RelayCent) {
  byte SendData[16];
  unsigned long SensorReadTime;
  float x, y, z;
  while (RelayCent.connected()) {
    if (SensorData.subscribed()) {
      if (IMU.accelerationAvailable()) {
        //If the start time has not been set, find the time of the first read
        if (StartReadTime == 0) {
          StartReadTime = millis();
        }

        // Read from sensor when possible
        IMU.readAcceleration(x, y, z);

        // Obtain time at which sensor was read by subtracting the current time from the time of the first read
        SensorReadTime = millis() - StartReadTime;

        // Convert to m/s^2
        x = x * 9.81;
        y = y * 9.81;
        z = z * 9.81;

        // Convert floats to byte arrays
        float2Bytes(x, &SendData[0]);
        float2Bytes(y, &SendData[4]);
        float2Bytes(z, &SendData[8]);

        //Convert unsigned long to byte array
        ulong2Bytes(SensorReadTime, &SendData[12]);

        // Write complete byte array to characteristic
        SensorData.writeValue(SendData, 16);
        //digitalWrite(BLE_DATA_SEND, HIGH);
        //delay(1);
        //digitalWrite(BLE_DATA_SEND, LOW);
      }
    }
    //delay(50); // To prevent massive delays on the Control end
  }
  return 0;
}

int StimOperation(BLEDevice RelayPeri) {
  digitalWrite(BLE_CONN, HIGH);
  digitalWrite(BLE_DATA_SEND, HIGH);
  digitalWrite(BLE_CENT_FUNC, HIGH);
  digitalWrite(BLE_PERI_FUNC, HIGH);
  while (1){
   delay(1);
  }
  return 0;
}

int float2Bytes(float val, byte* bytes_array) {
  // Create union of shared memory space
  union {
    float float_variable;
    byte temp_array[4];
  } u;

  // Overite bytes of union with float variable
  u.float_variable = val;
  // Assign bytes to input array

  memcpy(bytes_array, u.temp_array, 4);
  return 0;
}

int ulong2Bytes(unsigned long val, byte* bytes_array) {
  // Shift bytes into array positions from long integer using bitwise operations
  bytes_array[0] = val & 0xFF; // 0x78
  bytes_array[1] = (val >> 8) & 0xFF; // 0x56
  bytes_array[2] = (val >> 16) & 0xFF; // 0x34
  bytes_array[3] = (val >> 24) & 0xFF; // 0x12
  return 0;
}
