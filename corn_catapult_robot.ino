#include <Servo.h>
#include <NewPing.h>

#define TRIGGER_PIN  2  // Pin for the ultrasonic sensor trigger
#define ECHO_PIN     3  // Pin for the ultrasonic sensor echo
#define MAX_DISTANCE 150  // Maximum detection distance (in cm)

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Creating objects to control the servos
Servo tensionServo;
Servo shutterServo;
Servo baseServo;
Servo feedServo;

// Initial positions for the servos
const int tensionInitialPos = 121;
const int tensionFireDelay = 700;

const int shutterInitialPos = 179;
const int shutterFirePos = 105;
const int shutterFireDelay = 1500;

const int baseStartPos = 105;  // Starting position for the base
const int baseEndPos = 170;    // Ending position for the base
const int baseFirePos = 90;    // Position for firing
const int baseReturnPos = 105; // Position for returning the base
const int scanDelay = 10;      // Delay for servo movement (in ms)
const int pauseDelay = 300;    // Pause at position (in ms)
const int stopDelay = 1500;    // Delay for stopping (in ms)
const int pauseBeforeFire = 200;  // Pause before firing sequence
const int delayAfterTensionReturn = 700;  // Delay after tension returns before closing the shutter

const int feedInitialPos = 115;  // Initial position for feeding
const int feedOpenPos = 130;     // Open position for feeding
const int feedClosePos = 118;    // Closed position for feeding
const int feedCycleDelay = 1000;  // Delay for the full feeding cycle (in ms)

const int fixedTensionAngle = 18;  // Fixed tension angle

const int validObjectDistance = 100; // Maximum distance for object detection
const int minStableReadings = 2; // Minimum number of consecutive stable readings from the ultrasonic sensor

void setup() {
  Serial.begin(9600);  // Initializing serial port for debugging

  // Attach servos to their respective pins
  tensionServo.attach(10);  // Attach tension servo to pin 10
  shutterServo.attach(11);  // Attach shutter servo to pin 11
  baseServo.attach(6);      // Attach base servo to pin 6
  feedServo.attach(9);      // Attach feed servo to pin 9

  // Set initial positions
  tensionServo.write(tensionInitialPos);
  shutterServo.write(shutterInitialPos);
  baseServo.write(baseStartPos);
  feedServo.write(feedInitialPos);

  delay(2000);  // Give the servos time to reach their initial positions
}

void loop() {
  scanAndFire(baseStartPos, baseEndPos); // Scan forward
  delay(pauseDelay);  // Pause
  scanAndFire(baseEndPos, baseStartPos);  // Scan backward
}

// Function for smooth scanning and firing
void scanAndFire(int startAngle, int endAngle) {
  int step = (startAngle < endAngle) ? 1 : -1;  // Determine movement direction
  int stableReadingsCount = 0;  // Counter for stable readings

  for (int angle = startAngle; angle != endAngle + step; angle += step) {
    baseServo.write(angle);
    delay(scanDelay);  // Delay for smooth movement

    unsigned int distance = sonar.ping_cm();
    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print(" Distance: ");
    Serial.println(distance);

    // If the object is within the valid range (and the readings are stable)
    if (distance > 0 && distance <= validObjectDistance) {
      stableReadingsCount++;  // Increment the counter for stable readings

      // If two consecutive stable readings are detected, stop the base
      if (stableReadingsCount >= minStableReadings) {
        baseServo.write(angle);  // Stop the base at the current angle
        delay(stopDelay);  // Delay for stopping
        fireSequence();  // Start the firing sequence

        // Move the base to the feeding position
        baseServo.write(baseFirePos);
        delay(1000);  // Give time to set the position

        // Start the feeding cycle
        feedCycle();

        // Return the base to the starting position for a new scan
        baseServo.write(baseReturnPos);
        delay(2000);  // Give time for the base to return

        break;  // Stop scanning after firing
      }
    } else {
      stableReadingsCount = 0;  // Reset the counter if the readings are unstable
    }
  }
}

void fireSequence() {
  Serial.println("Starting fire sequence...");
  delay(pauseBeforeFire);  // Pause before starting the firing sequence

  tensionServo.write(fixedTensionAngle);  // Set the tension to a fixed angle of 18 degrees
  delay(tensionFireDelay);  // Wait for the tension to reach its position
  
  shutterServo.write(shutterFirePos);  // Open the shutter
  delay(shutterFireDelay);  // Delay for shutter opening
  
  tensionServo.write(tensionInitialPos);  // Return the tension
  delay(delayAfterTensionReturn);  // Pause after tension returns

  shutterServo.write(shutterInitialPos);  // Close the shutter
  delay(100);  // Delay for shutter return

  Serial.println("Fire sequence completed.");
}

void feedCycle() {
  Serial.println("Starting feed cycle...");
  feedServo.write(feedOpenPos);  // Open the feed
  delay(feedCycleDelay / 2);  // Delay for opening
  feedServo.write(feedClosePos);  // Close the feed
  delay(feedCycleDelay / 2);  // Delay for closing
  Serial.println("Feed cycle completed.");
}
