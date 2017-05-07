
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

int step_size = 1;
int period = 1;

void setup() {
  Serial.begin(9600);

  attach_all();

  // TODO maybe remove (particularly if not using attach_all before this
  // get initial servo positions
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

boolean will_move(int si) {
  // step size does not matter in right operand to &&
  return sign(v[si]) != 0 && in_range(p[si] + sign(v[si]));
}

void attach_all() {
  for (int i=0;i<NUM_SERVOS;i++) {
    s[i].attach(servo_pins[i], 600, 2320);
  }
}

void attach_needed() {
  for (int i=0;i<NUM_SERVOS;i++) {
    // don't attach if we don't need to move
    if (will_move(i)) {
      // https://github.com/tardate/LittleArduinoProjects/tree/master/playground/ServoTest
      // has some evidence that (under some circumstances) these parameters yield better stability
      // for the TowerPro SG90 I was using
      s[i].attach(servo_pins[i], 600, 2320);
    }
  }
}

void move_needed() {
  for (int i=0;i<NUM_SERVOS;i++) {
    if (will_move(i)) {
      Serial.print("moving to ");
      Serial.println(p[i] + step_size * sign(v[i]));

      u[i] = step_size * sign(v[i]);
      s[i].write(p[i] + u[i]);
    }
  }
}

void update_positions() {
  for (int i=0;i<NUM_SERVOS;i++) {
    /*
    // TODO intermittently (or always if not costly) check servo position against expected update
    // and make period minimum such that expectation is not violated
    p[i] += u[i];
    u[i] = 0;

    if (!in_range(p[i])) {
      p[i] = s[i].read();
    }
    */
    p[i] = s[i].read();
  }
}

void detach_all() {
  for (int i=0;i<NUM_SERVOS;i++) {
    s[i].detach();
  }
}

void print_positions() {
  Serial.print("p:");
  for (int i=0;i<NUM_SERVOS;i++) {
    Serial.print(p[i]);
    if (i != NUM_SERVOS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("");
}
    
void print_rates() {
  Serial.print("v:");
  for (int i=0;i<NUM_SERVOS;i++) {
    Serial.print(v[i]);
    if (i != NUM_SERVOS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("");
}

// TODO also detach servos if not in active serial communication?
void loop() {
  Serial.println("loop");
  // TODO still detach servos if they are at bounds / v=0? would need to handle retachment then
  
  print_positions();
  print_rates();
  Serial.println("");

  // TODO problems if this gets interrupted? tolerable performance if i disable interrupts?
  // (non-issue because serialEvent happens only at end of loop. not interrupt based.)
  
  //attach_needed();
  move_needed();

  unsigned long start_time = millis();
  unsigned long end_time = start_time + period;
  while (millis() < end_time) {
    /*
    if (interrupted) {
      // starts loop() from the beginning again
      return;
    }
    */
    ;
  }
  
  update_positions();
  //detach_all();
}

// TODO if period isn't limiting serialEvent response time now. what is?

// this function name has special importance in the Arduino environment
// it is an interrupt triggered by receipt of serial data
void serialEvent() {
  if (Serial.available() < sizeof(servo_update)) {
    return;
  }

  for (byte i=0;i < sizeof(servo_update);i++) {
    buff[i] = Serial.read();
  }
  // TODO crc?
  for (byte i=0;i < sizeof(servo_update);i++) {
    data.arr[i] = buff[i];
  }
  v[data.u.dim] = data.u.velocity;
}
