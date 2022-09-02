#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h> // Custom Arduino Library, to allow for alteration of Gyroscope and Accelerometer Sample Rates

BLEService accelService("183E"); // Bluetooth® Low Energy LED Service
BLECharacteristic accelCharacteristic("2B3C", BLERead | BLENotify, 16, true);

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

unsigned long EndInitTime = 0;

void setup() {
  // Serial connection is unnecessary as this code runs on an embedded MCU, but is helpful for debugging.
  Serial.begin(9600);
  // while(!Serial);

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  
  // BLE.setConnectionInterval(0x0006, 0x0140);
  
  // set advertised local name and service UUID:
  BLE.setLocalName("Accelerometer_BMI");
  BLE.setAdvertisedService(accelService);

  // add the characteristic to the service
  accelService.addCharacteristic(accelCharacteristic);

  // add service
  BLE.addService(accelService);

  // set the initial value for the characeristic:
  accelCharacteristic.writeValue("");

  // start advertising
  BLE.advertise();
  Serial.println("BLE Acceleration Peripheral");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  IMU.setSampleRate(SAMPLE_RATE); // Default 3 (119Hz)

  Serial.print("Accelerometer sample rate = ");
  Serial.println(IMU.accelerationSampleRate());
}

void loop() {
  //static unsigned long EndInitTime = millis();
  byte SendData[16];
  unsigned long SensorReadTime;
  float x, y, z;

  BLEDevice central = BLE.central();

  if (central) {
    /*
      Serial.print("Connected to central: ");
      // print the central's MAC address:
      Serial.println(central.address());
    */

    while (central.connected()) {
      if (accelCharacteristic.subscribed()) {
        if (IMU.accelerationAvailable()) {
          //If the start time has not been set, find the time of the first read
          if (EndInitTime == 0) {
            EndInitTime = millis();
          }
          
          // Read from sensor when possible
          IMU.readAcceleration(x, y, z);

          // Obtain time at which sensor was read by subtracting the current time from the time of the first read
          SensorReadTime = millis() - EndInitTime;

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

          /*
            Serial.print(x);
            Serial.print(' ');
            Serial.print(y);
            Serial.print(' ');
            Serial.print(z);
            Serial.print(' ');
            Serial.println(SensorReadTime);

            for (int i = 0; i < sizeof(SendData); i++){
            Serial.print(SendData[i],HEX);
            Serial.print(" ");
            }
            Serial.print("\n");
          */

          // Write complete byte array to characteristic
          accelCharacteristic.writeValue(SendData, 16);
        }
      }
    }
    /*
      // when the central disconnects, print it out:
      Serial.print(F("Disconnected from central: "));
      Serial.println(central.address());
    */

  }
  // Loop runs faster than sample rate of sensor -> Data sent as it arrives.
  // delay() can be added to slower sample rates to reduce polling.
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
