
#include <Servo.h>

#define NUM_SERVOS 3
#define MAX_POS 2000
#define MIN_POS 1000

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
// below 20 didn't seem to be working
int period = 30;

void setup() {
  Serial.begin(57600);

  // TODO way to not change position on attach?
  attach_all();

  // TODO maybe remove (particularly if not using attach_all before this
  // get initial servo positions
  for (int i = 0; i < NUM_SERVOS; i++) {
    p[i] = s[i].read();
  }
  detach_all();
  
  for (int i = 0; i < NUM_SERVOS; i++) {
    v[i] = 0;
    u[i] = 0;
  }
}

boolean in_range(int p) {
  return p >= MIN_POS && p <= MAX_POS;
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
      //Serial.print("moving to ");
      //Serial.println(p[i] + step_size * sign(v[i]));

      u[i] = step_size * sign(v[i]);

      int new_p = p[i] + u[i];
      if (MAX_POS < new_p) {
        new_p = MAX_POS;
        u[i] = new_p - p[i];
        
      } else if (MIN_POS > new_p) {
        new_p = MIN_POS;
        u[i] = new_p - p[i];
      }
      
      s[i].writeMicroseconds(new_p);
    }
  }
}

void update_positions() {
  for (int i=0;i<NUM_SERVOS;i++) {
    p[i] += u[i];
    u[i] = 0;
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

void print_will_move() {
  Serial.print("will_move:");
  for (int i=0;i<NUM_SERVOS;i++) {
    Serial.print(will_move(i));
    if (i != NUM_SERVOS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("");
}

void print_moves() {
  Serial.print("new positions:");
  for (int i=0;i<NUM_SERVOS;i++) {
    u[i] = step_size * sign(v[i]);

    int new_p = p[i] + u[i];
    if (MAX_POS < new_p) {
      new_p = MAX_POS;
      u[i] = new_p - p[i];
      
    } else if (MIN_POS > new_p) {
      new_p = MIN_POS;
      u[i] = new_p - p[i];
    }
    
    Serial.print(new_p);
    if (i != NUM_SERVOS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("");
}

// TODO also detach servos if not in active serial communication?
void loop() {
  attach_needed();
  move_needed();
  delay(period);
  update_positions();
  detach_all();
}

void update_control() {
  if (Serial.available() < sizeof(servo_update)) {
    /*
    Serial.print("only ");
    Serial.print(Serial.available());
    Serial.println(" bytes available");
    */
    return;
  } else if (Serial.available() % sizeof(servo_update) != 0) {
    // TODO this might actually be causing problems
    while (Serial.available() > 0) {
      Serial.read();
    }
    Serial.flush();
  }

  for (byte i=0;i < sizeof(servo_update);i++) {
    buff[i] = Serial.read();
  }
  // TODO crc?
  for (byte i=0;i < sizeof(servo_update);i++) {
    data.arr[i] = buff[i];
  }
  v[data.u.dim] = data.u.velocity;
  Serial.flush();
}

// this function name has special importance in the Arduino environment
// it is an interrupt triggered by receipt of serial data
void serialEvent() {
  //Serial.println("serialEvent");
  update_control();
  print_moves();
}
