/*
 * @Author       : Anan
 * @Date         : 2023-07-03 11:01:23
 * @LastEditors  : Anan.Huang
 * @LastEditTime : 2023-08-04 18:38:99
 * @Description  :
 */
#include "anker_probe.h"

#if ENABLED(ANKER_MAKE_API)

    #include "../../module/probe.h"
    #include "../../module/planner.h"
    #if ENABLED(ADAPT_DETACHED_NOZZLE)
        #include "../interactive/uart_nozzle_rx.h"
    #endif
    #if ENABLED(ANKER_PROBE_SET)
        #include "anker_z_offset.h"
    #endif
    #define DEBUG_OUT ENABLED(DEBUG_LEVELING_FEATURE)
    #include "../../core/debug_out.h"
    #include "anker_homing.h"
    #include "../../module/endstops.h"
    #if ENABLED(ANKER_PROBE_CONFIRM_RETRY)
        #include "../interactive/uart_nozzle_tx.h"
        #include "../../module/stepper.h"
        #include "../interactive/protocol.h"
    #endif


    Anker_probe anker_probe;

    uint8_t Anker_probe::try_times = 0; // The number of attempts for random movement XY-axis 
    uint8_t Anker_probe::randomxy_flag = false; 
    TERN_(WS1_HOMING_5X, bool Anker_probe::safely_delay = true);//Determine whether a delay before zeroing is required
    TERN_(ANKER_PROBE_CONFIRM_RETRY, bool Anker_probe::is_probing_flag = false;)

    /**
     * @brief  In order to eliminate the abnormal high-level triggering caused by hardware or hand touching the TYPE-C cable,
     *         randomly move the X/Y axis a few times before proceeding to the next probing.
     * @param  None
     * @retval None
     */
    void Anker_probe::random_movexy(const_float_t zraise) {
        // Pick a random direction, starting angle, and radius
        const int dir = (random(0, 10) > 5.0) ? -1 : 1;  // clockwise or counter clockwise
        float angle = random(0, 360);
        const float radius = random(int(5), int(0.125 * _MIN(X_BED_SIZE, Y_BED_SIZE)));

        if (WITHIN(zraise, 0.2, 2.5)){ // determine whether the Z-axis needs to be raised
            current_position.z += zraise;
            line_to_current_position(homing_feedrate(Z_AXIS));
            planner.synchronize();
        }

        const abce_pos_t test_position = planner.get_axis_positions_mm();

        // Move from leg to leg in rapid succession
        LOOP_L_N(leg, 6) {
            // Just move further along the perimeter.
            const float delta_angle = dir * (float)random(25, 45);
            angle += delta_angle;

            // Trig functions work without clamping, but just to be safe...
            while (angle > 360.0) angle -= 360.0;
            while (angle < 0.0) angle += 360.0;


            // Choose the next position as an offset to chosen test position
            const xy_pos_t noz_pos = test_position;
            xy_pos_t next_pos = {
            noz_pos.x + float(cos(RADIANS(angle))) * radius,
            noz_pos.y + float(sin(RADIANS(angle))) * radius
            };

            // For a rectangular bed just keep the probe in bounds
            LIMIT(next_pos.x, X_MIN_POS, X_MAX_POS);
            LIMIT(next_pos.y, Y_MIN_POS, Y_MAX_POS);

            do_blocking_move_to_xy(next_pos, 150); // 150MM/S
        } // n_legs loop

        const xy_pos_t current_pos = test_position; // go back
        do_blocking_move_to_xy(current_pos, 150); // 150MM/S

        if (WITHIN(zraise, 0.2, 2)){ // determine whether the Z-axis needs to be raised
            current_position.z -= zraise;
            line_to_current_position(homing_feedrate(Z_AXIS));
            planner.synchronize();
        }
    }


    #if ENABLED(EVT_HOMING_5X)
        /**
         * @brief  anker probe Home an individual linear axis
         * @param  None
         * @retval None
         */
        bool  Anker_probe::homing_move(const AxisEnum axis, const float distance, const feedRate_t fr_mm_s/*=0.0*/, const bool final_approach/*=true*/, const bool zero_axis/*=true*/) {
            DEBUG_SECTION(log_move, "do_homing_move", DEBUGGING(LEVELING));

            const feedRate_t home_fr_mm_s = fr_mm_s ?: homing_feedrate(axis);

            // Only do some things when moving towards an endstop
            const int8_t axis_home_dir = TERN0(DUAL_X_CARRIAGE, axis == X_AXIS)
                        ? TOOL_X_HOME_DIR(active_extruder) : home_dir(axis);
            const bool is_home_dir = (axis_home_dir > 0) == (distance > 0);
            
            bool no_trigge=false;
            if (is_home_dir) {
                if(safely_delay)
                {
                    if (!IS_new_nozzle_board())
                        digitalWrite(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
                    if(anker_leve_pause)
                    probe.anker_level_set_probing_paused(true,ANKER_LEVEING_DELAY_BEFORE_PROBING_TRUE);
                    else
                    probe.anker_level_set_probing_paused(true,ANKER_LEVEING_DELAY_BEFORE_PROBING);
                    if (!IS_new_nozzle_board())
                        digitalWrite(PROVE_CONTROL_PIN, PROVE_CONTROL_STATE);
                }
                TERN_(ANKER_PROBE_CONFIRM_RETRY, anker_probe.is_probing_flag = true;production_mode.rx_ack = false);
                randomxy_flag = false; 
            }

            // Get the ABC or XYZ positions in mm
            abce_pos_t target = planner.get_axis_positions_mm();

            if(zero_axis)target[axis] = 0;                         // Set the single homing axis to 0
            planner.set_machine_position_mm(target);  // Update the machine position

            TERN_(USE_Z_SENSORLESS, anker_homing.set_triger_per_ms());

            // Set delta/cartesian axes directly
            if(zero_axis)
                target[axis] = distance;                   // The move will be towards the endstop
            else
                target[axis] += distance;
            planner.buffer_segment(target OPTARG(HAS_DIST_MM_ARG, cart_dist_mm), home_fr_mm_s, active_extruder);

            if(anker_probe_home_synchronize())
            {
                no_trigge=true;
            }
            
            if (is_home_dir) {
                #if HOMING_Z_WITH_PROBE && HAS_QUIET_PROBING
                    if (axis == Z_AXIS && final_approach) probe.set_probing_paused(false);
                #endif

                TERN_(ANKER_PROBE_CONFIRM_RETRY, anker_probe.is_probing_flag = false);

                endstops.validate_homing_move(axis);
            }
            return no_trigge;
        }
    #endif

    #if ENABLED(ANKER_PROBE_DETECT_TIMES)
        /**
         * @brief  Try a Z-axis probing and check if there are any probe failures.
         * @param  None
         * @retval None
         */
        bool Anker_probe::try_to_probe(PGM_P const plbl, const_float_t z_probe_low_point, const feedRate_t fr_mm_s, const bool scheck, const float clearance) {
            // Do a first probe at the fast speed
            TERN_(ANKER_PROBE_CONFIRM_RETRY, anker_probe.is_probing_flag = true);
            production_mode.rx_ack = false;
            randomxy_flag = false;
            TERN_(USE_Z_SENSORLESS, anker_homing.set_triger_per_ms());
            const bool probe_fail = probe.probe_down_to_z(z_probe_low_point, fr_mm_s),            // No probe trigger?
                    early_fail = (scheck && current_position.z > -probe.offset.z + clearance); // Probe triggered too high?
            TERN_(ANKER_PROBE_CONFIRM_RETRY, anker_probe.is_probing_flag = false);
            if (probe_fail || early_fail || randomxy_flag) {
                if (probe_fail){
                    DEBUG_ECHOPGM_P(plbl);
                    DEBUG_ECHOLNPGM(" Probe fail! - No trigger.");
                }
                if (early_fail || randomxy_flag){ 
                    if(try_times >= 3){
                        DEBUG_ECHOPGM_P(plbl);
                        DEBUG_ECHOLNPGM(" Probe fail! - Triggered early.");
                    }else{ //  Triggered early, try random movement on X/Y axis once first, and then try again.
                        SEND_ECHO_TO_HOST("random_xy_probe=%d\r\n", try_times);
                        if(randomxy_flag){ // The probe may have descended a portion
                            random_movexy(2.0);
                        }else{
                            random_movexy(0.0); // move up and try
                        }
                        randomxy_flag = false;
                        try_times++;
                        return false;
                    }
                }
            }
            return probe_fail || early_fail;
        }

        /**
         * @brief  Multiple detections and comparison of the error between each two detections
         * @param  None
         * @retval None
         */
        bool Anker_probe::Mult_detections(const float z_probe_low_point, const float first_probe_z, const bool sanity_check/*=true*/) {
            uint8_t count_flag = 5; // The maximum number of consecutive attempts
            float first_probe = first_probe_z;
            
            try_times = 0; // Reset the number of attempts for random movement XY-axis after each successful probing.
            
            do{ // Probe and check the error between each probe
                if (try_to_probe(PSTR("SLOW"), z_probe_low_point, MMM_TO_MMS(Z_PROBE_FEEDRATE_SLOW),
                                sanity_check, Z_CLEARANCE_MULTI_PROBE) ) return true;
                const float second_probe_z = current_position.z;
                SEND_ECHO_TO_HOST("num:%d Probe Z:%3.2f diff:%3.2f %3.2f %d\r\n", (uint8_t)(5-count_flag), second_probe_z, (first_probe - second_probe_z), planner.get_axis_position_mm(Z_AXIS));
                if(ABS(first_probe - second_probe_z) < Z_PROBE_DETECTION_DEVIATION){
                    try_times = 0; // Reset the number of attempts for random movement XY-axis after each successful probing.
                    break; // OK
                }
                else
                { // try again
                    if(--count_flag){ // Remaining attempts for the loop
                        first_probe = second_probe_z;
                        do_blocking_move_to_z(current_position.z + Z_CLEARANCE_MULTI_PROBE, MMM_TO_MMS(HOMING_RISE_SPEED));
                        TERN_(ANKER_PROBE_SET, if(anker_probe_set.probe_start(anker_probe_set.leveing_value)){return true;});
                        SEND_ECHO_TO_HOST("move up and try again: %3.2f %3.2f\r\n", planner.get_axis_position_mm(Z_AXIS), planner.triggered_position_mm(_AXIS(Z)));
                    }else{ // error!!!
                        break;
                    }
                }
            }while(count_flag > 0);

            if(count_flag == 0){ // error!!!
                SEND_ECHO_TO_HOST("Mult_det: num:%d Probe Z:%3.2f %3.2f\r\n", (uint8_t)(5-count_flag), current_position.z, planner.get_axis_position_mm(Z_AXIS));
                if(TERN1(ADAPT_DETACHED_NOZZLE, IS_new_nozzle_board())) return true; // error!!!  Only try on new_nozzle_board.
            }

            return false;
        }

        /**
         * @brief  When homing, detect and compare the error between every two times multiple times.
         * @param  None
         * @retval None
         */
        int Anker_probe::homing_Mult_detections(const AxisEnum axis, const float probe_first, const float bump) {
            
            const float rebump = bump * 2;
            float probe_first_z = probe_first;

            // After random movement XY-axis, the probing distance is doubled to prevent the probe 
            // from not reaching the position after lifting due to triggering at a higher level.
            randomxy_flag = false; 

            TERN_(ANKER_PROBE_SET, if(anker_probe_set.probe_start(anker_probe_set.leveing_value)){return 0;})// error!!!

            if (axis == Z_AXIS)
            {
                uint8_t count_flag = 5; // The maximum number of consecutive attempts
                
                do{ // Probe and check the error between each probe
                    TERN_(ADAPT_DETACHED_NOZZLE, if(IS_new_nozzle_board())uart_nozzle_tx_point_type(POINT_G28, (uint8_t)(7 - count_flag)));

                    if (homing_move(axis, (randomxy_flag ? (rebump*2) : rebump), get_homing_bump_feedrate(axis), true, false)) {// timer out!!! move up and try again
                        MYSERIAL2.printf("Z homing second err= %3.4f %d\n",  planner.triggered_position_mm(_AXIS(Z)), anker_homing.triger_Take_time());
                        return 0;// error!!!
                    }
                    float probe_second =  planner.triggered_position_mm(_AXIS(Z));
                    SEND_ECHO_TO_HOST("Z homing try:%d Probe Z at:%3.2f diff:%3.2f %3.2f %d", (uint8_t)(5-count_flag), probe_second, (probe_first_z - probe_second), planner.get_axis_position_mm(Z_AXIS),anker_homing.triger_Take_time());
                    
                    // NEAR_ZERO(probe_second) //TYPE-C cable is broken, please move randomly in the XY direction and recheck.
                    if (WITHIN(probe_second, -0.01f, 0.01f)) {randomxy_flag = true;}

                    if((false == randomxy_flag) && ABS(probe_first_z - probe_second) < Z_PROBE_DETECTION_DEVIATION){
                        break; // OK!
                    }
                    else
                    {// move up and try again
                        if(--count_flag){ // Remaining attempts for the loop
                            probe_first_z = probe_second;
                            homing_move(axis, -bump, TERN(HOMING_Z_WITH_PROBE, (axis == Z_AXIS ? MMM_TO_MMS(HOMING_RISE_SPEED) : 0), 0), false, false);
                            SEND_ECHO_TO_HOST("move up and try again: %3.2f %3.2f\r\n", planner.get_axis_position_mm(Z_AXIS), planner.triggered_position_mm(_AXIS(Z)));
                            if (randomxy_flag) {// TYPE-C cable is broken, please move randomly in the XY direction and recheck.
                                const abce_pos_t target = planner.get_axis_positions_mm();
                                destination = current_position = target;
                                planner.set_machine_position_mm(target);  // Update the machine position
                                random_movexy();
                                SEND_ECHO_TO_HOST("random_xy_homing=%3.2f", current_position.z);
                            }
                            TERN_(ANKER_PROBE_SET, if(anker_probe_set.probe_start(anker_probe_set.leveing_value)){return 0;});
                        }else{ // error!!!
                            break;
                        }
                    }
                }while(count_flag > 0);

                if(count_flag == 0){ // error!!!
                    SEND_ECHO_TO_HOST("HOME_Mult_det: num:%d Probe Z:%3.2f %3.2f %d\r\n", (uint8_t)(5-count_flag), current_position.z, planner.get_axis_position_mm(Z_AXIS), IS_new_nozzle_board());
                    if(TERN1(ADAPT_DETACHED_NOZZLE, IS_new_nozzle_board())) {return 0;} // error!!! try homing again. Only try on new_nozzle_board.
                }
            }else{ // Other axis homing
                homing_move(axis, rebump, get_homing_bump_feedrate(axis), true, false);
            }

            return 1; // OK
        }
    #endif /* ANKER_PROBE_DETECT_TIMES */

    static void reset_position(const AxisEnum axis)
    {
        #if ENABLED(PROVE_CONTROL)
        if (!IS_new_nozzle_board())
        digitalWrite(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
        #endif
        set_axis_is_at_home(axis);
        sync_plan_position();
        destination[axis] = current_position[axis];
        if (axis == Z_AXIS) probe.anker_stow();
    }

  int Anker_probe::Probe_homeaxis(const AxisEnum axis) {

    int error_status = 1;
    #if EITHER(MORGAN_SCARA, MP_SCARA)
      // Only Z homing (with probe) is permitted
      if (axis != Z_AXIS) { BUZZ(100, 880); return; }
    #else
      #define _CAN_HOME_P(A) (axis == _AXIS(A) && ( \
           ENABLED(A##_SPI_SENSORLESS) \
        || TERN0(HAS_Z_AXIS, TERN0(HOMING_Z_WITH_PROBE, _AXIS(A) == Z_AXIS)) \
        || TERN0(A##_HOME_TO_MIN, A##_MIN_PIN > -1) \
        || TERN0(A##_HOME_TO_MAX, A##_MAX_PIN > -1) \
      ))
      if (LINEAR_AXIS_GANG(
           !_CAN_HOME_P(X),
        && !_CAN_HOME_P(Y),
        && !_CAN_HOME_P(Z),
        && !_CAN_HOME_P(I),
        && !_CAN_HOME_P(J),
        && !_CAN_HOME_P(K))
      ) return 2;
    #endif

    if (DEBUGGING(LEVELING)) DEBUG_ECHOLNPAIR(">>> probe_homeaxis(", AS_CHAR(AXIS_CHAR(axis)), ")");

    const int axis_home_dir = TERN0(DUAL_X_CARRIAGE, axis == X_AXIS)
                ? TOOL_X_HOME_DIR(active_extruder) : home_dir(axis);

    anker_probe.safely_delay = true;
    //
    // Homing Z with a probe? Raise Z (maybe) and deploy the Z probe.
    //
    if (TERN0(HOMING_Z_WITH_PROBE, axis == Z_AXIS && probe.anker_deploy()))
      return 2;

    // Determine if a homing bump will be done and the bumps distance
    // When homing Z with probe respect probe clearance
    const bool use_probe_bump = TERN0(0, axis == Z_AXIS && anker_home_bump_mm(axis));
    const float bump = axis_home_dir * (
      use_probe_bump ? _MAX(TERN0(0, Z_CLEARANCE_BETWEEN_PROBES), anker_home_bump_mm(axis)) : anker_home_bump_mm(axis)
    );
    //
    // Fast move towards endstop until triggered
    //
    const float move_length = 1.5f * max_length(TERN(DELTA, Z_AXIS, axis)) * axis_home_dir;
    if (DEBUGGING(LEVELING)) DEBUG_ECHOLNPAIR("probe_Home Fast: ", move_length, "mm");
    //do_homing_move(axis, move_length, 0.0, !use_probe_bump);

    TERN_(ADAPT_DETACHED_NOZZLE, if(IS_new_nozzle_board())uart_nozzle_tx_point_type(POINT_G28, 1));
    TERN_(ANKER_PROBE_SET, if(anker_probe_set.probe_start(anker_probe_set.leveing_value)){reset_position(axis);return 0;})

    if(anker_probe.homing_move(axis, move_length, MMM_TO_MMS(Z_PROBE_FEEDRATE_FAST), !use_probe_bump))
    {
      MYSERIAL2.printLine("echo: Z homing first err= %3.2f %d %d\n",  planner.triggered_position_mm(_AXIS(Z)), anker_homing.triger_Take_time(), anker_probe_set.leveing_value);
      reset_position(axis);
       return 0;
    }
    float probe_first =  planner.triggered_position_mm(_AXIS(Z));
    MYSERIAL2.printLine("echo: Z homing first=%3.2f\n", probe_first);
    #if BOTH(HOMING_Z_WITH_PROBE, BLTOUCH_SLOW_MODE)
      if (axis == Z_AXIS) bltouch.stow(); // Intermediate STOW (in LOW SPEED MODE)
    #endif
     #if ENABLED(PROVE_CONTROL)
     if (!IS_new_nozzle_board())
         digitalWrite(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
     #endif
    // If a second homing move is configured...
    if (bump) {
      // Move away from the endstop by the axis HOMING_BUMP_MM
      if (DEBUGGING(LEVELING)) DEBUG_ECHOLNPAIR("probe_Move Away: ", -bump, "mm");
       anker_probe.homing_move(axis, -bump, TERN(HOMING_Z_WITH_PROBE, (axis == Z_AXIS ? MMM_TO_MMS(HOMING_RISE_SPEED) : 0), 0), false, false);

      if(READ(Z_MIN_PROBE_PIN) == Z_MIN_PROBE_STATE)
      {
        MYSERIAL2.printf("probe: homeaxis signal error!\r\n");
        reset_position(axis);
        return 0;
      }

      #if ENABLED(DETECT_BROKEN_ENDSTOP)
        // Check for a broken endstop
        EndstopEnum es;
        switch (axis) {
          default:
          case X_AXIS: es = X_ENDSTOP; break;
          case Y_AXIS: es = Y_ENDSTOP; break;
          case Z_AXIS: es = Z_ENDSTOP; break;
          #if LINEAR_AXES >= 4
            case I_AXIS: es = I_ENDSTOP; break;
          #endif
          #if LINEAR_AXES >= 5
            case J_AXIS: es = J_ENDSTOP; break;
          #endif
          #if LINEAR_AXES >= 6
            case K_AXIS: es = K_ENDSTOP; break;
          #endif
        }
        if (TEST(endstops.state(), es)) {
          SERIAL_ECHO_MSG("Bad ", AS_CHAR(AXIS_CHAR(axis)), " Endstop?");
          kill(GET_TEXT(MSG_KILL_HOMING_FAILED));
        }
      #endif

      #if BOTH(HOMING_Z_WITH_PROBE, BLTOUCH_SLOW_MODE)
        if (axis == Z_AXIS && bltouch.deploy()) return; // Intermediate DEPLOY (in LOW SPEED MODE)
      #endif
      // Slow move towards endstop until triggered
      const float rebump = bump * 2;
      if (DEBUGGING(LEVELING)) DEBUG_ECHOLNPAIR("probe_Re-bump: ", rebump, "mm");

      #if ENABLED(ANKER_PROBE_DETECT_TIMES)
        error_status = anker_probe.homing_Mult_detections(axis, probe_first, bump);
      #else // ! ENABLED(ANKER_PROBE_DETECT_TIMES)
        anker_probe.homing_move(axis, rebump, get_homing_bump_feedrate(axis), true, false);
      #endif

      #if BOTH(HOMING_Z_WITH_PROBE, BLTOUCH)
        if (axis == Z_AXIS) bltouch.stow(); // The final STOW
      #endif
      #if ENABLED(PROVE_CONTROL)
      if (!IS_new_nozzle_board())
          digitalWrite(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
      #endif
    }
    
     MYSERIAL2.printLine("echo: CHECK HOMING----Probe Z:%3.2f\r\n", planner.triggered_position_mm(_AXIS(Z)));

    set_axis_is_at_home(axis);

    sync_plan_position();
    destination[axis] = current_position[axis];
    if (DEBUGGING(LEVELING)) DEBUG_POS("> probe_AFTER set_axis_is_at_home", current_position);

    // Put away the Z probe
    #if HOMING_Z_WITH_PROBE
       #if ENABLED(PROVE_CONTROL)
       if (!IS_new_nozzle_board())
           digitalWrite(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
       #endif
      if (axis == Z_AXIS && probe.anker_stow()) return 2;
    #endif
    
    // #if ENABLED(ANKER_ANLIGN)
    //   anker_align.run_align();
    // #endif
    if (DEBUGGING(LEVELING)) DEBUG_ECHOLNPAIR("<<< probe_homeaxis(", AS_CHAR(AXIS_CHAR(axis)), ")");
    return error_status;
  } // homeaxis()

  #if ENABLED(EVT_HOMING_5X)
    bool Anker_probe::anker_probe_home_synchronize(void)
    {
        while (planner.has_blocks_queued() || planner.cleaning_buffer_counter)
        {
            #if ENABLED(USE_Z_SENSORLESS)
                if(anker_homing.is_z_probe_no_triger())
                {
                    planner.endstop_triggered(_AXIS(Z));
                    SERIAL_ECHO(" probe timeout!!\r\n");
                    TERN_(ANKER_PROBE_CONFIRM_RETRY, stepper.run_status = STPPER_RUNNING);
                    return true; // error
                }
                TERN_(ANKER_PROBE_CONFIRM_RETRY, if(anker_probe.is_probing_flag && anker_probe.is_in_probing_state()){anker_probe.is_probing_flag = false;}) // ok
                
                // The current block has already been started to clear before the randomxy_flag is set.
                if(randomxy_flag) return false; // TX/RX: TYPE-C cable is broken, please move randomly in the XY direction and recheck.
                
            #endif
            idle();
        } // PROBE_TRIGGER_NORMAL = 6
        return false; // ok
    }
  #endif

  #if ENABLED(ANKER_PROBE_CONFIRM_RETRY)

    void Anker_probe::stop_all_steppers(const AxisEnum axis)
    {
        if(axis == Z_AXIS && endstops.z_probe_enabled && is_probing_flag)
        {// Only during the probing process can all steppers be stopped.
            stepper.run_status = STPPER_PROBE_PAUSE;
        }
    }

    bool Anker_probe::probe_pin_state_filter(void) // Add a filtering operation when obtaining the IO status.
    {
        uint8_t count = 0, state = 0;
        while(count < 50) // 50ms filter
        {
            if(READ(Z_MIN_PROBE_PIN) == Z_MIN_PROBE_STATE){state++;}
            safe_delay(1);
            count++;
        }

        if(state > 40) // 40ms/50ms=90% time trigger
            return true;
        else
            return false;
    }

    bool Anker_probe::is_in_probing_state(void)
    {
        if(IS_new_nozzle_board() && endstops.z_probe_enabled) // Only works on the new nozzle board
        {// It is in the probing state
            if(STPPER_PROBE_PAUSE == stepper.run_status) // Upon receiving the trigger signal, it is necessary to confirm whether the signal is triggered normally.
            {
                switch(production_mode.rx_type)
                {
                    case PROBE_TRIGGER_NORMAL:
                    case PROBE_TRIGGER_IDLE:
                        {
                            const bool early_terigger = anker_homing.triger_Take_time() < PROBE_TRIGGERED_EARLY;
                            stepper.run_status = STPPER_RUNNING;
                            MYSERIAL2.printLine("echo: trigger-normal=%d %d %d\n",anker_homing.triger_Take_time(), early_terigger, production_mode.rx_ack); // anan debug

                            if(early_terigger)// Check if the trigger time is too short, try to trigger again. 500ms
                            {
                                // MYSERIAL2.printLine("echo: trigger-early=%d\n",anker_homing.triger_Take_time()); // anan debug
                            }
                            if(production_mode.rx_ack)
                            {
                                production_mode.rx_ack = false;
                                // MYSERIAL2.printLine("echo: trigger-ack ok!\n"); // anan debug
                            }
                            return true; // OK
                        }
                        break;
                    case PROBE_START: // If there is a start signal, check if the triggering of the probe is successful upon timeout.
                    case PROBE_ZERO_POINT_OK :
                    case PROBE_ZERO_POINT_FAILED :
                        {
                            const millis_t ms = millis();
                            if(production_mode.rx_ack)
                            {
                                production_mode.rx_ack = false;
                                if(!probe_pin_state_filter()){ // Only when the IO is properly connected will the probing action continue to execute
                                    stepper.restart_stopped_motion();// Restart the stopped motion
                                }else if((ms - anker_homing.trigger_ms) > PROBE_ACK_TIMEROUT){ // IO disconnects and status query is received normally, but the time is out.
                                    MYSERIAL2.printLine("echo: trigger-ack out=%d %d %d %d\n",anker_homing.triger_Take_time(), production_mode.rx_timeout, anker_homing.trigger_ms, production_mode.current_diff); // anan debug
                                    if(production_mode.threshold == 0) production_mode.threshold = LEVEING_PROBE_VALUE;
                                    if((PROBE_ZERO_POINT_OK == production_mode.rx_type) && (production_mode.current_diff < (production_mode.threshold/3))){ // The returned current data is much smaller than the threshold, but the TX/RX communication is normal, continue to probing running.
                                        stepper.restart_stopped_motion();// Restart the stopped motion
                                    }else{
                                        stepper.run_status = STPPER_RUNNING; // Start clearing the current block. Assuming that abort_current_block has already been set to true.
                                        randomxy_flag = true; // TX/RX: TYPE-C cable is broken, please move randomly in the XY direction and recheck.
                                        return true; // try again
                                    }
                                }
                                MYSERIAL2.printLine("echo: trigger-ack=%d %d %d\n",anker_homing.triger_Take_time(), production_mode.rx_type, production_mode.current_diff); // anan debug
                            }
                            else if (ms - anker_homing.trigger_ms > PROBE_TIMEROUT_AGAIN)  // try after 100ms timeout
                            {
                                if(ELAPSED(ms, production_mode.rx_timeout))
                                {// The first time it times out, rx_timeout is always smaller than ms, the second time you come in, look at the set rx_timeout time
                                    production_mode.rx_ack = false;
                                    production_mode.rx_timeout = ms + PROBE_TIMEROUT_AGAIN; //try again after 100ms
                                    uart_nozzle_tx_single_data(GCP_CMD_4D_PROBE_ACK, 0);
                                    //MYSERIAL2.printLine("echo: trigger-ack set=%d %d %d\n",anker_homing.triger_Take_time(), production_mode.rx_timeout, anker_homing.trigger_ms); // anan debug
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }
            }else if(PROBE_TRIGGER_NORMAL == production_mode.rx_type || PROBE_TRIGGER_IDLE == production_mode.rx_type)
            { // When an IO exception occurs, we rely on serial communication to trigger signal feedback, but it may lag by about 10~15ms.
                stepper.endstop_triggered(ALL_AXES_ENUM); // abort_current_block = true;
                anker_homing.set_probe_triger_ms(false);
                stepper.run_status = STPPER_RUNNING;
                MYSERIAL2.printLine("echo: Level-IO exception occurs\n");
                return true; // OK
            }
        }
        return false;
    }

    bool Anker_probe::probe_start_confirm(const uint16_t value)
    {
        uint16_t timeout_ms = 0, query_count = 0;
        production_mode.rx_type = PROBE_UNKNOW; // RESET rx_type

        do{
            if(production_mode.rx_type == PROBE_UNKNOW && timeout_ms%100 == 0){
                if(timeout_ms == 800){ // The movement at the 800ms mark cannot simultaneously send a probing signal, and successfully sending a probing signal just during the movement process will cause an exception.
                    const abce_pos_t target = planner.get_axis_positions_mm();
                    destination = current_position = target;
                    planner.set_machine_position_mm(target);  // Update the machine position
                    random_movexy(2.0); // randomly move the X/Y axis
                }else{
                    uart_nozzle_tx_probe_val(value);
                    SERIAL_ECHOLNPAIR("echo: Comm-try: ", timeout_ms);
                }
            }else if(production_mode.rx_type >= PROBE_START && timeout_ms%100 == 0){ // Communication retransmission （100ms）
                uart_nozzle_tx_single_data(GCP_CMD_4D_PROBE_ACK, 0);
                query_count++;
            }
            
            if(PROBE_ZERO_POINT_OK == production_mode.rx_type || query_count > 10){ // Communication acknowledgement or  PROBE_START=query timeout=1000ms
                SERIAL_ECHOLNPAIR("echo: Comm-ack: ", timeout_ms,"ms, Type=", production_mode.rx_type," query=",query_count); // anan debug
                safe_delay(50); // Delay waiting for some processes that have not yet been fully triggered
                break;
            }
            timeout_ms += 50; // 50ms
            safe_delay(50);
        }while (timeout_ms < PROBE_START_TIMEROUT); // 2second

        if(timeout_ms >= PROBE_START_TIMEROUT){ // Timeout without receiving back-transmitted data, the RX/TX line may have been disconnected
            SERIAL_ECHOLNPAIR("echo: Comm-timeout=", timeout_ms, " query=", query_count);// anan debug
            return true; // error
        }
        return false; // OK
    }


  #endif

#endif /* ANKER_MAKE_API */
