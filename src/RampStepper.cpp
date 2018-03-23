#include "Arduino.h"
#include "RampStepper.h"

/*
 *   constructor for four-pin version
 *   Sets which wires should control the motor.
 */
RampStepper::RampStepper(int number_of_steps, int motor_pin_1, int motor_pin_2, int motor_pin_3, int motor_pin_4)
{
  this->step_number = 0;    // which step the motor is on
  this->direction = 0;      // motor direction
  this->last_step_time = 0; // time stamp in us of the last step taken
  this->starting_speed = 150;
  this->position = 0;
  this->transmission = 32;
  this->start_ramp = true;
  this->stop_ramp = true;
  this->number_of_steps = number_of_steps; // total number of steps for this motor

  // Arduino pins for the motor control connection:
  this->motor_pin_1 = motor_pin_1;
  this->motor_pin_2 = motor_pin_2;
  this->motor_pin_3 = motor_pin_3;
  this->motor_pin_4 = motor_pin_4;
  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1, OUTPUT);
  pinMode(this->motor_pin_2, OUTPUT);
  pinMode(this->motor_pin_3, OUTPUT);
  pinMode(this->motor_pin_4, OUTPUT);
  // pin_count is used by the stepMotor() method:
  this->pin_count = 4;
}

void RampStepper::setSpeed(long whatSpeed)
{
  this->target_speed = whatSpeed;
}

void RampStepper::setRamp(bool ramp) {
    this->start_ramp = ramp;
    this->stop_ramp = ramp;
}
void RampStepper::setRamp(bool startRamp, bool stopRamp) {
    this->start_ramp = startRamp;
    this->stop_ramp = stopRamp;
}

void RampStepper::resetPosition() {
    this->position = 0;
}
/*
 * Calculates the delay between steps
 */
void RampStepper::calculateDelay(int steps_left, int steps_to_move)
{
    long calculatedSpeed = this->target_speed;
    // speed up
    if(steps_left >= (steps_to_move*3)/4 && this->start_ramp) {
        calculatedSpeed = map(steps_to_move-steps_left, 0, steps_to_move/4, this->starting_speed, this->target_speed);
    }
    // slow down
    if(steps_left <= steps_to_move/4 && this->stop_ramp) {
        calculatedSpeed = map(steps_left, steps_to_move/4, 0, this->target_speed, this->starting_speed);
    }
    this->step_delay = 60L * 1000L * 1000L / this->number_of_steps / calculatedSpeed;
}

void RampStepper::toStep(int target_step, bool uzs) {

  int stepsBetween;
  if(uzs) {
    stepsBetween = target_step - this->position;
    if(stepsBetween < 0) {
      stepsBetween += (this->number_of_steps * this->transmission);
    }
  } else {
    stepsBetween = this->position - target_step;
    if(stepsBetween < 0) {
      stepsBetween += (this->number_of_steps * this->transmission);
    }
    stepsBetween = -stepsBetween;
  }

  this->step(stepsBetween);
}

/*
 * Moves the motor steps_to_move steps.  If the number is negative,
 * the motor moves in the reverse direction.
 */
void RampStepper::step(int steps_to_move)
{
  int steps_left = abs(steps_to_move);  // how many steps to take

  // determine direction based on whether steps_to_mode is + or -:
  if (steps_to_move > 0) { this->direction = 1; }
  if (steps_to_move < 0) { this->direction = 0; }

  // decrement the number of steps, moving one step each time:
  while (steps_left > 0)
  {
    calculateDelay(steps_left, abs(steps_to_move));
    unsigned long now = micros();
    // move only if the appropriate delay has passed:
    if (now - this->last_step_time >= this->step_delay)
    {
      // get the timeStamp of when you stepped:
      this->last_step_time = now;
      // increment or decrement the step number,
      // depending on direction:
      if (this->direction == 1)
      {
        this->step_number++;
        this->position++;
        if (this->step_number == this->number_of_steps) {
          this->step_number = 0;
        }
        if (this->position == this->number_of_steps * this->transmission) {
          this->position = 0;
        }
      }
      else
      {
        if (this->step_number == 0) {
          this->step_number = this->number_of_steps;
        }
        if (this->position == 0) {
          this->position = this->number_of_steps * this->transmission;
        }
        this->step_number--;
        this->position--;
      }
      // decrement the steps left:
      steps_left--;
      stepMotor(this->step_number % 4);
    }
  }
}
/*
 * Moves the motor forward or backwards.
 */
void RampStepper::stepMotor(int thisStep)
{
  if (this->pin_count == 4) {
    switch (thisStep) {
      case 0:  // 1010
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, LOW);
      break;
      case 1:  // 0110
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, LOW);
      break;
      case 2:  //0101
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, HIGH);
      break;
      case 3:  //1001
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, HIGH);
      break;
    }
  }
}