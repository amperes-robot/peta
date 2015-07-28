#include "course.h"
#include "isr.h"
#include "io.h"
#include "menu.h"
#include "motion.h"
#include "pid.h"
#include <avr/pgmspace.h>

namespace course
{
  uint8_t pet_id;
  
  namespace
  {
    enum { REV_TURN_THETA = 150, REV_BACK_THETA = -140, REV_DEAD_BEGIN = 400, REV_DEAD_END = 600 };
    enum { ARM_LO_THRESH = -28, ARM_HI_THRESH = -5, ARM_MID_THRESH = -15 };
    enum { PAR_REVERSE_THETA = 0, PAR_ENTRY_THETA = 135, PAR_BACK_LEFT_THETA = -80, PAR_BACK_RIGHT_THETA = -80 };
    enum { SQAURE_MAX_FRONT_CORRECTION= 30, SQAURE_MAX_BACK_CORRECTION = -30};
    // cooldowns determine amount of time that must be waited before side qrd can trigger again
    
    const PROGMEM uint16_t COOLDOWNS[] = { 0, 1000, 1000, 5000 };
    
    pid::DigitalController dcontroller(0, 0, 0);
    pid::Controller acontroller(0, 0, 0);
    
    uint8_t retry_count;
    uint8_t state;
    
    void begin_tick()
    {
      pet_id = 0;
      control::set_mode(&follow_mode);
    }
    
    /**
     * FOLLOW
     */
    void follow_begin()
    {
      io::lcd.home();
      io::lcd.clear();
      io::lcd.print(TO_FSTR(strings::follow));
      
      io::Timer::start();
      state = 0;
      
      dcontroller.reset();
      dcontroller.gain_p = menu::flw_gain_p.value();
      dcontroller.gain_i = menu::flw_gain_i.value();
      dcontroller.gain_d = menu::flw_gain_d.value();
    }
    void follow_tick()
    {
      enum { HOLD_AMT = 1 };
      // HOLD is for debouncer (number of loops that the switch must be activated in a row)
      
      if (menu::stop_falling())
      {
        control::set_mode(&menu::main_mode);
        return;
      }
      
      bool qrd = max(io::Analog::qrd_side_left.read(), io::Analog::qrd_side_right.read()) > menu::flw_thresh_side.value();
      // side trigger, detects if left or right qrd reads line
      
      
      if (qrd && io::Timer::time() > (uint16_t) pgm_read_word(COOLDOWNS + pet_id))
        // qrd is triggered and the cooldown has ended
      {
        state++;
      }
      else
      {
        state = 0; // debouncer failed
      }
      
      if (state > HOLD_AMT)
      {
        control::set_mode(&square_mode); // following has ended
        return;
      }
      
      int8_t in = pid::follow_value_digital(); // read sensors
      
      dcontroller.in(in);
      int16_t out = dcontroller.out();
      
      motion::vel(menu::flw_vel.value());
      motion::dir(out);
      
      io::delay_ms(10);
    }
    
    /**
     * REVERSE FOLLOW
     */
    void reverse_follow_begin()
    {
      dcontroller.reset();
      dcontroller.gain_p = menu::flw_gain_p.value();
      dcontroller.gain_i = menu::flw_gain_i.value();
      dcontroller.gain_d = menu::flw_gain_d.value();
      
      io::lcd.home();
      io::lcd.clear();
      io::lcd.print(TO_FSTR(strings::follow));
      
      state = 0;
    }
    void reverse_follow_tick()
    {
      // here we want to back up, turn and find the tape, and follow in the opposite direction
      enum
      {
        REV_BACK_BEGIN = 0,
        REV_BACK,
        REV_TURN_BEGIN,
        REV_TURN,
        REV_FOLLOW_BEGIN,
        REV_FOLLOW
      };
      
      if (menu::stop_falling()) // cancel
      {
        control::set_mode(&menu::main_mode);
        return;
      }
      
      switch (state)
      {
        case REV_BACK_BEGIN: // start moving backwards
        {
          motion::left.speed(-MEDIUM_SPEED);
          motion::right.speed(-MEDIUM_SPEED);
          motion::right_theta = 0;
          state++;
          // fall through
        }
        case REV_BACK:
        {
          if (motion::right_theta < REV_BACK_THETA)
          {
            state = REV_TURN_BEGIN;
          }
          break;
        }
        case REV_TURN_BEGIN: // pivot on the spot CCW
        {
          motion::left.speed(-MEDIUM_SPEED);
          motion::right.speed(MEDIUM_SPEED);
          motion::right_theta = 0;
          state++;
          // fall through
        }
        case REV_TURN: // must have turned for THETA && left sensor has found tape
        {
          if (motion::right_theta > REV_TURN_THETA && io::Analog::qrd_tape_left.read() > menu::flw_thresh_left.value())
          {
            state = REV_FOLLOW_BEGIN;
          }
          break;
        }
        case REV_FOLLOW_BEGIN:
        {
          state++;
          io::Timer::start();
          // fall through
        }
        case REV_FOLLOW:
        {
          io::lcd.clear();
          io::lcd.home();
          io::lcd.print(io::Timer::time());
          
          if (io::Timer::time() > menu::rev_dead_begin.value() * 10 && io::Timer::time() < menu::rev_dead_end.value() * 10)
            // in dead zone, ignore the QRD and drive straight forward, force a left turn by setting the PID controller's
            // recovery value to the negative value
          {
            motion::dir(0);
            pid::digital_recovery = -((int8_t) menu::flw_drecover.value());
            break;
          }
          
          int8_t in = pid::follow_value_digital();
          
          dcontroller.in(in);
          int16_t out = dcontroller.out();
          
          motion::vel(menu::flw_vel.value());
          motion::dir(out);
          break;
        }
      }
      
      motion::update_enc();
      io::delay_ms(10);
    }
    
    /**
     * RETRIEVAL (dropping arm)
     */
    void retrieve_begin()
    {
      motion::left.halt();
      motion::right.halt();
      
      io::lcd.clear();
      io::lcd.home();
      io::lcd.print(TO_FSTR(strings::retrieval));
      
      motion::update_enc(); // clear accumulator
      motion::arm_theta = 0;
      
      retry_count = 0;
      state = 0;
    }
    
    void retrieve_tick()
    {
      int8_t drop_thresh;
      if (pet_id < 4)
      {
        drop_thresh = ARM_LO_THRESH;
      }
      else
      {
        drop_thresh = ARM_MID_THRESH;
      }
      
      enum { N_RETRIES = 2 };
      enum
      {
        DROPPING_BEGIN = 0,
        DROPPING,
        BRAKE_BEGIN,
        BRAKE,
        LIFTING_BEGIN,
        LIFTING,
        RETRY_BEGIN,
        RETRY,
        ZERO_BEGIN,
        ZERO,
        DONE_BEGIN,
        DONE
      };
      
      if (menu::stop_falling())
      {
        control::set_mode(&menu::main_mode);
        return;
      }
      
      switch (state) // dropping arm
      {
        case DROPPING_BEGIN: // drop the arm
        {
          io::lcd.setCursor(0, 1);
          io::lcd.print("drp");
          motion::arm.speed(-MEDIUM_SPEED);
          io::Timer::start();
          state++;
          // fall through
        }
        case DROPPING:
        {
          if (motion::arm_theta < drop_thresh || io::Timer::time() > 1000 /*|| io::Digital::switch_upper.read()*/)
            // the arm is down or the microswitch has been activated or timeout
          {
            state = BRAKE_BEGIN;
          }
          break;
        }
        case BRAKE_BEGIN: // brake
        {
          io::lcd.setCursor(0, 1);
          io::lcd.print("brk");
          motion::arm.halt();
          state++;
          io::Timer::start();
          // fall through
        }
        case BRAKE: // wait for arm to slow to halt
        {
          if (io::Timer::time() > 400)
          {
            state = LIFTING_BEGIN;
          }
          break;
        }
        case LIFTING_BEGIN: // lift
        {
          io::lcd.setCursor(0, 1);
          io::lcd.print("lft");
          motion::arm.speed(MEDIUM_SPEED - 15);
          state++;
          io::Timer::start();
          // fall through
        }
        case LIFTING:
        {
          if (motion::arm_theta > ARM_HI_THRESH /* || !io::Digital::switch_upper.read()*/)
            // wait until pet is detached or the arm is up
          {
            state = ZERO_BEGIN;
          }
          else if (io::Timer::time() > 2000) // timeout
          {
            if (retry_count < N_RETRIES)
            {
              state = RETRY_BEGIN;
            }
            else
            {
              state = ZERO_BEGIN;
            }
          }
          break;
        }
        case RETRY_BEGIN: // move down and try again
        {
          io::lcd.setCursor(0, 1);
          io::lcd.print("rty");
          retry_count++;
          state++;
          motion::arm.speed(-SLOW_SPEED);
          // fall through
        }
        case RETRY:
        {
          if (motion::arm_theta < ARM_LO_THRESH)
          {
            state = BRAKE_BEGIN;
          }
          break;
        }
        case ZERO_BEGIN: // zero the arm by ramming it into the hardstop
        {
          io::lcd.setCursor(0, 1);
          io::lcd.print("zro");
          
          motion::arm.speed(MEDIUM_SPEED);
          io::Timer::start();
          state++;
          // fall through
        }
        case ZERO:
        {
          if (io::Timer::time() > 500)
          {
            state = DONE_BEGIN;
          }
          break;
        }
        case DONE_BEGIN:
        {
          io::lcd.setCursor(0, 1);
          io::lcd.print("dne");
          
          motion::arm.halt();
          state++;
          // fall through
        }
        case DONE:
        {
          control::set_mode(&recover_mode);
          break;
        }
      }
      
      motion::update_enc();
      io::delay_ms(10);
    }
    void beacon_homing_begin()
    {
      io::Timer::start();
      io::lcd.clear();
      io::lcd.home();
      io::lcd.print(TO_FSTR(strings::home));
      
      acontroller.reset();
      acontroller.gain_p = menu::home_gain_p.value();
      acontroller.gain_i = menu::home_gain_i.value();
      acontroller.gain_d = menu::home_gain_d.value();
      
      motion::update_enc();
      motion::left_theta = 0;
      motion::right_theta = 0;
    }
    
    void beacon_homing_tick()
    {
      if (menu::stop_falling())
      {
        control::set_mode(&menu::main_mode);
        return;
      }
      
      uint16_t left = io::Analog::pd_left.read();
      uint16_t right = io::Analog::pd_right.read();
      
      io::lcd.clear();
      
      if (pet_id == 4)
      {
        if ((motion::left_theta + motion::right_theta) / 2 > (int16_t) menu::beacon_theta.value()) // move a fixed amount forward
        {
          control::set_mode(&retrieve_mode);
          return;
        }
      }
      else if (pet_id == 5)
      {
        if (io::Timer::time() > 1100)
        {
          motion::excavator.halt();
        }
        else
        {
          motion::excavator.speed(-70);
        }
        
        if (io::Digital::switch_front.read())
        {
          control::set_mode(&adjust_mode);
          return;
        }
      }
      else if (pet_id == 6)
      {
        if (io::Analog::qrd_side_right.read() > menu::flw_thresh_side.value())
        {
          control::set_mode(&reverse_follow_mode);
          return;
        }
      }
      
      int16_t in = ((int32_t) right - left) * 50 / (left + right);
      
      io::lcd.print(in);
      
      acontroller.in(in);
      int16_t out = acontroller.out();
      
      io::lcd.setCursor(0, 1);
      io::lcd.print(out);
      
      if (pet_id == 6)
      {
        motion::vel(-((int16_t) menu::home_vel.value()));
        motion::dir(-out);
      }
      else
      {
        motion::vel(menu::home_vel.value());
        motion::dir(out);
      }
      
      io::delay_ms(50);
    }
    
    void parallel_park_begin()
    {
      io::lcd.clear();
      io::lcd.home();
      io::lcd.print(TO_FSTR(strings::ppark));
      
      state = 0;
    }
    
    void parallel_park_tick()
    {
      enum
      {
        REVERSE_BEGIN = 0,
        REVERSE,
        ENTRY_BEGIN,
        ENTRY,
        BACK_LEFT_BEGIN,
        BACK_LEFT,
        BACK_RIGHT_BEGIN,
        BACK_RIGHT
      };
      
      if (menu::stop_falling())
      {
        control::set_mode(&menu::main_mode);
        return;
      }
      
      switch (state)
      {
        case REVERSE_BEGIN:
        {
          state++;
          motion::left.speed(-MEDIUM_SPEED);
          motion::right.speed(-MEDIUM_SPEED);
          motion::left_theta = 0;
          motion::right_theta = 0;
          // fall through
        }
        case REVERSE:
        {
          if ((motion::right_theta + motion::left_theta) / 2 < PAR_REVERSE_THETA)
          {
            state = ENTRY_BEGIN;
          }
          break;
        }
        case ENTRY_BEGIN:
        {
          state++;
          motion::left.speed(-MEDIUM_SPEED);
          motion::right.speed(MEDIUM_SPEED);
          motion::right_theta = 0;
          // fall through
        }
        case ENTRY: // go in
        {
          if (motion::right_theta > PAR_ENTRY_THETA)
          {
            state = BACK_LEFT_BEGIN;
          }
          break;
        }
        case BACK_LEFT_BEGIN:
        {
          state++;
          motion::left.speed(-SLOW_SPEED);
          motion::right.halt();
          motion::left_theta = 0;
          // fall through
        }
        case BACK_LEFT:
        {
          if (motion::left_theta < PAR_BACK_LEFT_THETA)
          {
            state = BACK_RIGHT_BEGIN;
          }
          break;
        }
        case BACK_RIGHT_BEGIN:
        {
          state++;
          motion::left.halt();
          motion::right.speed(-SLOW_SPEED);
          motion::right_theta = 0;
          // fall through
        }
        case BACK_RIGHT:
        {
          if (motion::right_theta < PAR_BACK_RIGHT_THETA)
          {
            control::set_mode(&rubble_excavation_mode);
          }
          break;
        }
      }
      
      io::delay_ms(10);
      motion::update_enc();
    }
    
    void rubble_excavation_begin()
    {
      io::lcd.clear();
      io::lcd.home();
      io::lcd.print("exc");
      state = 0;
      motion::left.halt();
      motion::right.halt();
    }
    
    void rubble_excavation_tick()
    {
      enum
      {
        LOWER_BEGIN = 0,
        LOWER,
        RAISE_BEGIN,
        RAISE,
        BRAKE_BEGIN,
        BRAKE
      };
      
      if (menu::stop_falling())
      {
        control::set_mode(&menu::main_mode);
        return;
      }
      
      switch (state)
      {
        case LOWER_BEGIN:
        {
          state++;
          motion::right.halt();
          io::Timer::start();
          motion::excavator.speed(-70);
          // fall through
        }
        case LOWER:
        {
          if (io::Timer::time() > 600)
          {
            state = RAISE_BEGIN;
          }
          break;
        }
        case RAISE_BEGIN:
        {
          state++;
          io::Timer::start();
          motion::excavator.speed(200);
          // fall through
        }
        case BRAKE:
        {
          if (io::Timer::time() > 400)
          {
            motion::excavator.halt();
            control::set_mode(&recover_mode);
            return;
          }
          break;
        }
      }
      
      motion::update_enc();
      io::delay_ms(10);
    }
    
    void square_begin()
    {
      io::lcd.clear();
      io::lcd.home();
      io::lcd.print("square");
      state = 0;
      motion::left.halt();
      motion::right.halt();
    }
    
    void square_tick()
    {
      if (menu::stop_falling())
      {
        control::set_mode(&menu::main_mode);
        return;
      }
      
      enum
      {
        BEGIN = 0,
        LEFT_FORWARD_BEGIN,
        LEFT_FORWARD,
        LEFT_BACKWARD_BEGIN,
        LEFT_BACKWARD,
        RIGHT_FORWARD_BEGIN,
        RIGHT_FORWARD,
        RIGHT_BACKWARD_BEGIN,
        RIGHT_BACKWARD
      };
      
      bool qrd_left = io::Analog::qrd_side_left.read() > menu::flw_thresh_side.value();
      bool qrd_right = io::Analog::qrd_side_right.read() > menu::flw_thresh_side.value();
      
      switch (state)
      {
        case BEGIN:
        {
          if (!qrd_left)
          {
            state = LEFT_FORWARD_BEGIN;
          }
          else if (!qrd_right)
          {
            state = RIGHT_FORWARD_BEGIN;
          }
          else
          {
            control::set_mode(&adjust_mode);
          }
          break;
        }
        case LEFT_FORWARD_BEGIN:
        {
            state++;
            motion::left.speed(SLOW_SPEED);
            motion::left_theta = 0;
        }
        case LEFT_FORWARD:
        {
          if (qrd_left)
          {
            control::set_mode(&adjust_mode);
          }
          if (motion::left_theta > SQAURE_MAX_FRONT_CORRECTION)
          {
            state++;
          }
          break;
        }
        case LEFT_BACKWARD_BEGIN: 
    				{
              state++;
              motion::left.speed(-SLOW_SPEED);
              motion::left_theta = 0;
            }
        case LEFT_BACKWARD:
        {
          if (qrd_left || motion::left_theta < SQAURE_MAX_BACK_CORRECTION)
          {
            control::set_mode(&adjust_mode);
          }
          break;
        }
        case RIGHT_FORWARD_BEGIN: 
        {

              state++;
              motion::right.speed(SLOW_SPEED);
              motion::right_theta = 0;
        }
        case RIGHT_FORWARD:
        {
          if (qrd_right)
          {
            control::set_mode(&adjust_mode);
          }
          if (motion::right_theta > SQAURE_MAX_FRONT_CORRECTION)
          {
            state++;
          }
          break;
        }
        case RIGHT_BACKWARD_BEGIN: 
    				{
              state++;
              motion::right.speed(-SLOW_SPEED);
              motion::right_theta = 0;
            }
        case RIGHT_BACKWARD:
        {
          if (qrd_right || motion::right_theta < SQAURE_MAX_BACK_CORRECTION)
          {
            control::set_mode(&adjust_mode);
          }
          break;
        }
      }
      motion::update_enc();
      
    }
    
  }
  
		
  
  const control::Mode begin_mode
  {
    &control::nop,
    &begin_tick,
    &control::nop
  };
  
  const control::Mode follow_mode
  {
    &follow_begin,
    &follow_tick,
    &control::nop
  };
  
  const control::Mode retrieve_mode
  {
    &retrieve_begin,
    &retrieve_tick,
    &control::nop
  };
  
  const control::Mode beacon_homing_mode
  {
    &beacon_homing_begin,
    &beacon_homing_tick,
    &control::nop
  };
  
  const control::Mode parallel_park_mode
  {
    &parallel_park_begin,
    &parallel_park_tick,
    &control::nop
  };
  
  const control::Mode reverse_follow_mode
  {
    &reverse_follow_begin,
    &reverse_follow_tick,
    &control::nop
  };
  
  const control::Mode rubble_excavation_mode
  {
    &rubble_excavation_begin,
    &rubble_excavation_tick,
    &control::nop
  };
  
  const control::Mode return_mode
  {
    &control::nop,
    &control::nop,
    &control::nop
  };
  
  const control::Mode square_mode
  {
    &square_begin,
    &square_tick,
    &control::nop
  };
}
