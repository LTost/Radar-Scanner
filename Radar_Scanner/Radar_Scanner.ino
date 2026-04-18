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

#define MAX_DIST     150
#define SAMPLES      6

#define CX           64
#define CY           63
#define RADAR_R      60

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
	OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

Servo radar_servo;
int current_angle = 0;
int sweep_dir = 1;
int scan_data[181];

float single_ping() {
	digitalWrite(TRIG_PIN, LOW);
	delayMicroseconds(2);
	digitalWrite(TRIG_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG_PIN, LOW);
	long dur = pulseIn(ECHO_PIN, HIGH, 25000);
	if (dur == 0) return MAX_DIST + 1;
	return dur * 0.0343 / 2.0;
}

float measure_distance() {
	float samples[SAMPLES];
	for (int i = 0; i < SAMPLES; i++) {
		samples[i] = single_ping();
		delayMicroseconds(400);
	}
	// bubble sort
	for (int i = 0; i < SAMPLES - 1; i++)
		for (int j = 0; j < SAMPLES - 1 - i; j++)
			if (samples[j] > samples[j+1]) {
				float t = samples[j]; samples[j] = samples[j+1]; samples[j+1] = t;
			}
	// drop 1 low + 1 high, average middle 4
	float sum = 0;
	for (int i = 1; i < SAMPLES - 1; i++) sum += samples[i];
	return sum / (SAMPLES - 2);
}

void draw_radar() {
	display.clearDisplay();

	for (int ring = 1; ring <= 3; ring++) {
		int r = (RADAR_R * ring) / 3;
		for (int a = 0; a <= 180; a += 3) {
			float rad = a * PI / 180.0;
			int x = CX + (int)(r * cos(rad));
			int y = CY - (int)(r * sin(rad));
			if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
				display.drawPixel(x, y, WHITE);
		}
	}

	for (int a = 0; a <= 180; a += 30) {
		float rad = a * PI / 180.0;
		display.drawLine(CX, CY,
			CX + (int)(RADAR_R * cos(rad)),
			CY - (int)(RADAR_R * sin(rad)), WHITE);
	}

	display.drawLine(CX - RADAR_R, CY, CX + RADAR_R, CY, WHITE);

	float sweep_rad = current_angle * PI / 180.0;
	int sx = CX + (int)(RADAR_R * cos(sweep_rad));
	int sy = CY - (int)(RADAR_R * sin(sweep_rad));
	display.drawLine(CX, CY, sx, sy, WHITE);
	display.drawLine(CX + 1, CY, sx + 1, sy, WHITE);

	for (int a = 0; a <= 180; a++) {
		if (scan_data[a] < MAX_DIST) {
			float mapped = (float)scan_data[a] / MAX_DIST;
			int r_blip = (int)(mapped * RADAR_R);
			float rad = a * PI / 180.0;
			int bx = CX + (int)(r_blip * cos(rad));
			int by = CY - (int)(r_blip * sin(rad));
			if (bx >= 0 && bx < SCREEN_WIDTH && by >= 0 && by < SCREEN_HEIGHT)
				display.fillCircle(bx, by, 2, WHITE);
		}
	}

	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0, 57);
	display.print(current_angle);
	display.print((char)247);
	float d = scan_data[current_angle];
	String dist_str = (d >= MAX_DIST) ? "---" : (String((int)d) + "cm");
	display.setCursor(128 - (dist_str.length() * 6), 57);
	display.print(dist_str);

	display.display();
}

void setup() {
	pinMode(TRIG_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);

	radar_servo.attach(SERVO_PIN);
	radar_servo.write(0);
	delay(800);

	display.begin(SSD1306_SWITCHCAPVCC);
	display.clearDisplay();
	display.display();

	for (int i = 0; i <= 180; i++) scan_data[i] = MAX_DIST + 1;
}

void loop() {
	radar_servo.write(current_angle);
	delay(40);

	float dist = measure_distance();
	scan_data[current_angle] = (dist > MAX_DIST) ? MAX_DIST + 1 : (int)dist;

	draw_radar();

	current_angle += sweep_dir;
	if (current_angle >= 180) sweep_dir = -1;
	if (current_angle <= 0)   sweep_dir = 1;
}