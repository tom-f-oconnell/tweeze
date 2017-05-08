
#include <Servo.h>

#define NUM_SERVOS 3
#define MAX_POS 180
#define MIN_POS 0

// this struct / union pair for easier serial was adapte
// from comments by Robin2 on Arduino forums
struct servo_update {
  char dim;     // 1 byte
  int velocity; // 2 bytes
};

union update_union {
  servo_update u;
  // sizeof is a compile-time constant
  byte arr[sizeof(servo_update)];
};

byte buff[sizeof(servo_update)];
update_union data;

int servo_pins[NUM_SERVOS] = {9, 10, 11};
Servo s[NUM_SERVOS];
int p[NUM_SERVOS];
int v[NUM_SERVOS];
int u[NUM_SERVOS];

int step_size = 5;
int period = 1;

void setup() {
  attach_all();
  for (int i = 0; i < NUM_SERVOS; i++) {
    p[i] = s[i].read();
  }
  for (int i = 0; i < NUM_SERVOS; i++) {
    v[i] = 0;
    u[i] = 0;
  }
}

boolean in_range(int p) {
  return p > MIN_POS && p < MAX_POS;
}

int sign(int i) {
  if (i < 0) {
    return -1;
  } else if (i > 0) {
    return 1;
  } else {
    return 0;
  }
}

// TODO fix 179 / 1 cases
boolean will_move(int si) {
  // step size does not matter in right operand to &&
  return sign(v[si]) != 0 && in_range(p[si] + sign(v[si]));
}

void attach_all() {
  for (int i=0;i<NUM_SERVOS;i++) {
    s[i].attach(servo_pins[i], 600, 2320);
  }
}

void move_needed() {
  for (int i=0;i<NUM_SERVOS;i++) {
    if (will_move(i)) {
      //Serial.print("moving to ");
      //Serial.println(p[i] + step_size * sign(v[i]));

      u[i] = step_size * sign(v[i]);
      s[i].write(p[i] + u[i]);
    }
  }
}

void update_positions() {
  for (int i=0;i<NUM_SERVOS;i++) {
    p[i] = s[i].read();
  }
}

void loop() {

  move_needed();
  for (int i=0;i<NUM_SERVOS;i++) {
    v[i] = -1;
  }
  delay(1000);
  update_positions();

  move_needed();
  for (int i=0;i<NUM_SERVOS;i++) {
    v[i] = -1;
  }
  delay(1000);
  update_positions();
}
