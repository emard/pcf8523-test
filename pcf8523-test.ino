/* RTC test
 * Battery required: CR1225 3V
 * PCB v1.7 needs changes: D11=RED LED, R4=4.7k
 * To enter sleep mode, D18 (GREEN LED) should be OFF
 * D18 is OFF by default when FT231x usb-serial is not enumerated
 * Either plug FPGA into 5V USB charger
 * or reprogram FT231x to drive D18 OFF: :
 * ftx_prog --cbus 3 drive_0
 * to restore default D18 state:
 * ftx_prob --cbus 3 sleep
 */

#include <SoftwareWire.h>
SoftwareWire Wire(32+29,32+28);

#define I2CADDR B1101000

void regwrite(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(I2CADDR);
  Wire.write(byte(reg));
  Wire.write(byte(value));
  Wire.endTransmission();  
}

int regread(uint8_t reg, int8_t n)
{
  Wire.beginTransmission(I2CADDR);
  Wire.write(byte(reg));
  Wire.endTransmission();

  Wire.requestFrom(I2CADDR, n);
  if(Wire.available()) // slave may send less than requested
    return Wire.read();
  return -1;  
}

void setup() {
  Wire.setClock(100000L); // 100000-400000
  Wire.begin();        // join i2c bus (address optional for master)
  // Serial.begin(115200);  // start serial for output

  // resonator capacity 12.5pF, alarm interrupt
  regwrite(0x00, B10000010);

  // disable CLKOUT, short (pulsed interrupts)
  regwrite(0x0F, B11111000);

  // clear all interrupts
  regwrite(0x01, B00000000);
  regwrite(0x02, B10000000); // use battery backup during power off

  #if 1
  // set clock
  regwrite(0x09, 0x17); // year
  regwrite(0x08, 0x12); // month
  regwrite(0x06, 0x31); // day
  regwrite(0x05, 0x23); // hour
  regwrite(0x04, 0x58); // minute
  regwrite(0x03, 0x00); // second
  #endif

  #if 1
  // set alarm (in 2 minutes)
  // to disable each: |0x80
  regwrite(0x0D, 0x80); // weekdeay disabled
  regwrite(0x0C, 0x01); // day
  regwrite(0x0B, 0x00); // hour
  regwrite(0x0A, 0x00); // minute
  #endif

  // clear all interrupts
  regwrite(0x01, B00000000);
  regwrite(0x02, B10000000);
}

void loop() {
  Serial.print("20");
  Serial.print(regread(0x09,1) & 0xFF, HEX);
  Serial.print("-");
  Serial.print(regread(0x08,1) & 0x1F, HEX);
  Serial.print("-");
  Serial.print(regread(0x06,1) & 0x3F, HEX);
  Serial.print(" ");
  Serial.print(regread(0x05,1) & 0x3F, HEX);
  Serial.print(":");
  Serial.print(regread(0x04,1) & 0x7F, HEX);
  Serial.print(":");
  Serial.print(regread(0x03,1) & 0x7F, HEX);
  Serial.print(" ALM ");
  Serial.print(regread(0x0C,1) & 0x3F, HEX);
  Serial.print(" ");
  Serial.print(regread(0x0B,1) & 0x3F, HEX);
  Serial.print(":");
  Serial.print(regread(0x0A,1) & 0x7F, HEX);
  Serial.println(".");
  static int shut = 0;
  pinMode(32+21, OUTPUT); // connect +21 to SHUTDOWN to test software power off
  digitalWrite(32+21, shut);
  digitalWrite(15, shut);
  shut ^= 1;
  delay(2000);
}
