
#include "mavlink/common/mavlink.h"        // Mavlink interface
#include "mavlink/common/mavlink_msg_obstacle_distance.h"
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>


SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData;
int imageResolution = 0; //Used to pretty print output
int imageWidth = 0; //Used to pretty print output


unsigned long previousMillis = 0;
const long interval = 100;

int lidarAngle = 0;
int messageAngle = 0;
uint16_t distances[72];
int target = 0;
unsigned char data_buffer[4] = {0};
int adjusted = 0;
int distance = 0;
int range = 0;
unsigned char CS;
uint8_t Index;
byte received;
char serial_buffer[15];

int ledState = LOW;

int pos = 1;          // servo position
int dir = 1;          // servo moving direction: +1/-1
int val;              // LiDAR measured value
#define stepAng           1      // step angle
#define numStep           64        // = 180/stepAng

void setup()
{
  Serial2.begin(1500000); // FC
  Serial.begin(500000);
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C


  memset(distances, UINT16_MAX, sizeof(distances)); // Filling the distances array with UINT16_MAX
  Serial.println("Initializing sensor board. This can take up to 10s. Please wait.");
  if (myImager.begin() == false)
  {
    Serial.println(F("Sensor not found - check your wiring. Freezing"));
    while (1) ;
  }

  myImager.setResolution(8 * 8); //Enable all 64 pads

  imageResolution = myImager.getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
  imageWidth = sqrt(imageResolution); //Calculate printing width

  myImager.startRanging();

  memset(distances, UINT16_MAX, sizeof(distances)); // Filling the distances array with UINT16_MAX
}
int16_t Dist = 0;    // Distance to object in centimeters
void loop()
{


  
  if (myImager.isDataReady() == true)
  {
    if (myImager.getRangingData(&measurementData)) //Read distance data into array
    {

      //The ST library returns the data transposed from zone mapping shown in datasheet
      //Pretty-print data with increasing y, decreasing x to reflect reality
      for (int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth)
      {

        for (int x = imageWidth - 1 ; x >= 0 ; x--)
        {
          Serial.print("\t");
          Serial.print(measurementData.distance_mm[x + y]);
          distances[x + y] = (measurementData.distance_mm[(((x + y) / 8) + 1)]);
          command_mavlink();
          Serial.println();
        }
        Serial.println();
      }
    }
  }
}
  command_lidar();
  command_servo();
  // command_led();
  //command_print();

}


void command_servo() {
  pos += dir;
  if (pos == 64)
  {
    dir = -1;
  }

  else if (pos == 1)
  {
    dir = 1;
  }

}

void command_lidar() {


  void command_mavlink() {


    int sysid = 1;
    //< The component sending the message.
    int compid = 196;
    uint64_t time_usec = 0;
    uint8_t sensor_type = 0;
    distances[pos] = Dist - 2.0f; //UINT16_MAX gets updated with actual distance values
    uint8_t increment = 1;
    uint16_t min_distance = 10;
    uint16_t max_distance = 650;
    float increment_f = 0;
    float angle_offset = 0;
    uint8_t frame = 12;
    uint8_t system_type = MAV_TYPE_GENERIC;
    uint8_t autopilot_type = MAV_AUTOPILOT_INVALID;
    uint8_t system_mode = MAV_MODE_PREFLIGHT; ///< Booting up
    uint32_t custom_mode = 30;                 ///< Custom mode, can be defined by user/adopter
    uint8_t system_state = MAV_STATE_STANDBY; ///< System ready for flight

    // Initialize the required buffers
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    int type = MAV_TYPE_GROUND_ROVER;
    // Pack the message

    mavlink_msg_obstacle_distance_pack(sysid, compid, &msg, time_usec, sensor_type, distances, increment, min_distance, max_distance, increment_f, angle_offset, frame);
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    Serial1.write(buf, len);


    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;


      mavlink_msg_heartbeat_pack(1, 196, &msg, type, autopilot_type, system_mode, custom_mode, system_state);
      len = mavlink_msg_to_send_buffer(buf, &msg);
      Serial1.write(buf, len);
    }

  }

  void command_led() {

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(16, ledState);
  }

  void command_print() {

    // Serial.print("range: ");
    //  Serial.print(sensor.ranging_data.range_mm);
    //  Serial.print("angle: ");
    // Serial.print(pos * stepAng);
    //  Serial.println();
  }
