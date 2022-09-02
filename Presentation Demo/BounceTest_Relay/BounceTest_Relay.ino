#include <ArduinoBLE.h>

unsigned long EndInitTime;
void setup()
{
  Serial.begin(1000000);
  while (!Serial); 
  // Requires serial monitor to run

  // initialize the Bluetooth® Low Energy hardware
  BLE.begin();

  Serial.println("Bluetooth® Low Energy Central - Acceleration information");

  // start scanning for peripherals
  BLE.scanForUuid("183E");
}

void loop()
{
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral)
  {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();
    // PeripheralFlag = 1;
    if (peripheral.localName() != "Accelerometer_BMI")
    {
      return;
    }

    // stop scanning
    BLE.stopScan();
    EndInitTime = millis();
    AccelReceive(peripheral);
  }
  // peripheral disconnected, start scanning again
  BLE.scanForUuid("183E");
  /*
  if (PeripheralFlag == 1){
      Serial.println(-1, DEC);
  }
  */
}

void AccelReceive(BLEDevice peripheral)
{
  int count = 0;
  char buff[3];
  unsigned long FinalTime;
  byte relay[16];
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect())
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes())
  {
    Serial.println("Attributes discovered");
  }
  else
  {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the Acceleration characteristic
  BLECharacteristic accelCharacteristic = peripheral.characteristic("2B3C");

  if (!accelCharacteristic)
  {
    Serial.println("Peripheral does not have an acceleration characteristic!");
    peripheral.disconnect();
    return;
  }
  else if (!accelCharacteristic.canSubscribe())
  {
    Serial.println("Acceleration characteristic is not subscribable!");
    peripheral.disconnect();
    return;
  }
  else if (!accelCharacteristic.subscribe())
  {
    Serial.println("subscription failed!");
    peripheral.disconnect();
    return;
  }
  accelCharacteristic.subscribe();
  Serial.println("Subscribed.");

  //while(count < 1000)
  while (peripheral.connected())
  {
    // while the peripheral is connected
    if (accelCharacteristic.valueUpdated())
    {
      accelCharacteristic.readValue(relay, sizeof(relay));
      count = count + 1;
      // Serial.println(sizeof(relay));
      for (int i = 0; i < sizeof(relay); i++)
      {
        sprintf(buff, "%02X", relay[i]);
        Serial.print(buff);
        if ((i + 1) % 4 == 0)
        {
          Serial.print(" ");
        }
      }
      Serial.print("\n");
    }
  }

  peripheral.disconnect();
  Serial.println("Peripheral disconnected");
  FinalTime = millis() - EndInitTime;
  Serial.println(count);
  Serial.println(FinalTime/1000);
  Serial.println((count*sizeof(relay)*8)/(FinalTime/1000));
  // Serial.println(-1, DEC);
}
