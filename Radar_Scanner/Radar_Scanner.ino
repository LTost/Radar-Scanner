#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_CLK   18
#define OLED_MOSI  23
#define OLED_CS    5
#define OLED_DC    2
#define OLED_RST   4

#define TRIG_PIN   13
#define ECHO_PIN   12
#define SERVO_PIN  14

#define MAX_DIST   100  // cm, beyond this = nothing detected
#define RADAR_R    28   // radius of radar arc on screen
#define CX         36   // arc center X (left side to leave room)
#define CY         56   // arc center Y (bottom)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
	OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

Servo radar_servo;

int current_angle = 0;
int sweep_dir = 1;

// Stores last detected distance per degree (0-180)
int scan_data[181];

float measure_distance() {
	digitalWrite(TRIG_PIN, LOW);
	delayMicroseconds(2);
	digitalWrite(TRIG_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG_PIN, LOW);

	long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
	if (duration == 0) return MAX_DIST + 1;
	return duration * 0.0343 / 2.0;
}

void draw_radar() {
	display.clearDisplay();

	// Draw arc lines (quarter circles)
	for (int r = RADAR_R / 3; r <= RADAR_R; r += RADAR_R / 3) {
		for (int a = 0; a <= 180; a += 2) {
			float rad = a * PI / 180.0;
			int x = CX + r * cos(rad);
			int y = CY - r * sin(rad);
			display.drawPixel(x, y, WHITE);
		}
	}

	// Draw base line
	display.drawLine(CX - RADAR_R, CY, CX + RADAR_R, CY, WHITE);

	// Draw center cross lines (30/60/90/120/150 deg)
	int guide_angles[] = {30, 60, 90, 120, 150};
	for (int a : guide_angles) {
		float rad = a * PI / 180.0;
		display.drawLine(CX, CY,
			CX + RADAR_R * cos(rad),
			CY - RADAR_R * sin(rad), WHITE);
	}

	// Draw sweep line
	float sweep_rad = current_angle * PI / 180.0;
	display.drawLine(CX, CY,
		CX + RADAR_R * cos(sweep_rad),
		CY - RADAR_R * sin(sweep_rad), WHITE);

	// Draw detected blips
	for (int a = 0; a <= 180; a++) {
		if (scan_data[a] < MAX_DIST) {
			float mapped = (float)scan_data[a] / MAX_DIST;
			int r_blip = (int)(mapped * RADAR_R);
			float rad = a * PI / 180.0;
			int bx = CX + r_blip * cos(rad);
			int by = CY - r_blip * sin(rad);
			display.fillCircle(bx, by, 2, WHITE);
		}
	}

	// Distance readout on right side
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(72, 0);
	display.print(current_angle);
	display.print((char)247); // degree symbol
	display.setCursor(72, 12);
	float d = scan_data[current_angle];
	if (d >= MAX_DIST) display.print("---");
	else { display.print((int)d); display.print("cm"); }

	display.display();
}

void setup() {
	pinMode(TRIG_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);

	radar_servo.attach(SERVO_PIN);
	radar_servo.write(0);
	delay(1000);

	display.begin(SSD1306_SWITCHCAPVCC);
	display.clearDisplay();
	display.display();

	for (int i = 0; i <= 180; i++) scan_data[i] = MAX_DIST + 1;
}

void loop() {
	radar_servo.write(current_angle);
	delay(20); // let servo settle

	float dist = measure_distance();

	if (dist <= 10 || dist > MAX_DIST) {
			scan_data[current_angle] = MAX_DIST + 1; // treat as no detection
	} else {
			scan_data[current_angle] = (int)dist;
	}

	draw_radar();

	current_angle += sweep_dir;
	if (current_angle >= 180) sweep_dir = -1;
	if (current_angle <= 0)   sweep_dir = 180;
}