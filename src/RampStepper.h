// ensure this library description is only included once
#ifndef RampStepper_h
#define RampStepper_h

// library interface description
class RampStepper {
  public:
    // constructors:
    RampStepper(int number_of_steps, int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4);

    // speed setter method:
    void setSpeed(long whatSpeed);

    // ramp methods:
    void setRamp(bool ramp); // 0 no Ramp, 1 Start & End Ramp
    void setRamp(bool startRamp, bool stopRamp); // [start_ramp, stop_ramp]
    void resetPosition();

    // mover method:
    void step(int number_of_steps);
    void toStep(int target_step, bool uzs);

  private:
    void stepMotor(int this_step);
    void calculateDelay(int steps_left, int steps_to_move);

    int direction;            // Direction of rotation
    unsigned long step_delay; // delay between steps, in ms, based on speed
    long starting_speed;
    long target_speed;
    bool start_ramp;
    bool stop_ramp;
    int position;
    int transmission;
    int number_of_steps;      // total number of steps this motor can take
    int pin_count;            // how many pins are in use.
    int step_number;          // which step the motor is on

    // motor pin numbers:
    int motor_pin_1;
    int motor_pin_2;
    int motor_pin_3;
    int motor_pin_4;

    unsigned long last_step_time; // time stamp in us of when the last step was taken
};

#endif