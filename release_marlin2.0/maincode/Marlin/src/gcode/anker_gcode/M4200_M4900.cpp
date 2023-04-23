#include "../gcode.h"
#include "../../MarlinCore.h"
#include "../../module/planner.h"
#include "../../module/settings.h"

#if ENABLED(ANKER_MAKE_API)

// Apply limits to a variable and give a warning if the value was out of range
inline void limit_and_warn(float &val, const uint8_t axis, PGM_P const setting_name, const xyze_float_t &max_limit) {
  const uint8_t lim_axis = TERN_(HAS_EXTRUDERS, axis > E_AXIS ? E_AXIS :) axis;
  const float before = val;
  LIMIT(val, 0.1, max_limit[lim_axis]);
  if (before != val) {
    SERIAL_CHAR(AXIS_CHAR(lim_axis));
    SERIAL_ECHOPGM(" Max ");
    SERIAL_ECHOPGM_P(setting_name);
    SERIAL_ECHOLNPAIR(" limited to ", val);
  }
}
/**
 * M201: Set max acceleration in units/s^2 for print moves (M201 X1000 Y1000)
 *
 *       With multiple extruders use T to specify which one.
 */
void GcodeSuite::M4201() {

  const int8_t target_extruder = get_target_extruder_from_command();
  if (target_extruder < 0) return;

  LOOP_LOGICAL_AXES(i) {
    if (parser.seenval(axis_codes[i])) {
      const uint8_t axis = TERN(HAS_EXTRUDERS, (i == E_AXIS ? uint8_t(E_AXIS_N(target_extruder)) : i), i);
      float inMaxAccelMMS2 = parser.value_axis_units((AxisEnum)axis);
      constexpr xyze_float_t max_accel_edit = LA_V1_MAX_ACCEL_EDIT_VALUES;
      const xyze_float_t &max_acc_edit_scaled = max_accel_edit;
      limit_and_warn(inMaxAccelMMS2, axis, PSTR("Acceleration"), max_acc_edit_scaled);
      planner.settings.max_acceleration_mm_per_s2[axis] = inMaxAccelMMS2;
      // Update steps per s2 to agree with the units per s2 (since they are used in the planner)
      planner.reset_acceleration_rates();
    }
  }
}

/**
 * M203: Set maximum feedrate that your machine can sustain (M203 X200 Y200 Z300 E10000) in units/sec
 *
 *       With multiple extruders use T to specify which one.
 */
void GcodeSuite::M4203() {

  const int8_t target_extruder = get_target_extruder_from_command();
  if (target_extruder < 0) return;

  LOOP_LOGICAL_AXES(i)
    if (parser.seenval(axis_codes[i])) {
      const uint8_t axis = TERN(HAS_EXTRUDERS, (i == E_AXIS ? uint8_t(E_AXIS_N(target_extruder)) : i), i);
      float inMaxFeedrateMMS = parser.value_axis_units((AxisEnum)axis);
      constexpr xyze_float_t max_fr_edit = LA_V1_MAX_FEEDRATE_EDIT_VALUES;
      const xyze_float_t &max_fr_edit_scaled = max_fr_edit;
      limit_and_warn(inMaxFeedrateMMS, axis, PSTR("Feedrate"), max_fr_edit_scaled);
      planner.settings.max_feedrate_mm_s[axis] = inMaxFeedrateMMS;
    }
}

/**
 * M204: Set Accelerations in units/sec^2 (M204 P1200 R3000 T3000)
 *
 *    P = Printing moves
 *    R = Retract only (no X, Y, Z) moves
 *    T = Travel (non printing) moves
 */
void GcodeSuite::M4204() {

  if (!parser.seen("PRST")) {
    SERIAL_ECHOPAIR("Acceleration: P", planner.settings.acceleration);
    SERIAL_ECHOPAIR(" R", planner.settings.retract_acceleration);
    SERIAL_ECHOLNPAIR_P(SP_T_STR, planner.settings.travel_acceleration);
  }
  else {
    //planner.synchronize();
    // 'S' for legacy compatibility. Should NOT BE USED for new development
      if (parser.seenval('S')) planner.settings.travel_acceleration = planner.settings.acceleration = parser.value_linear_units();
      if (parser.seenval('P')) planner.settings.acceleration = parser.value_linear_units();
      if (parser.seenval('R')) planner.settings.retract_acceleration = parser.value_linear_units();
      if (parser.seenval('T')) planner.settings.travel_acceleration = parser.value_linear_units();
  }
}
static void set_max_jerk(const AxisEnum axis, float inMaxJerkMMS) {
  constexpr xyze_float_t max_jerk_edit = LA_V1_MAX_JERK_EDIT_VALUES;
  limit_and_warn(inMaxJerkMMS, axis, PSTR("Jerk"), max_jerk_edit);
  planner.max_jerk[axis] = inMaxJerkMMS;
}

/**
 * M205: Set Advanced Settings
 *
 *    B = Min Segment Time (µs)
 *    S = Min Feed Rate (units/s)
 *    T = Min Travel Feed Rate (units/s)
 *    X = Max X Jerk (units/sec^2)
 *    Y = Max Y Jerk (units/sec^2)
 *    Z = Max Z Jerk (units/sec^2)
 *    E = Max E Jerk (units/sec^2)
 *    J = Junction Deviation (mm) (If not using CLASSIC_JERK)
 */
void GcodeSuite::M4205() {
  if (!parser.seen("BST" TERN_(HAS_JUNCTION_DEVIATION, "J") TERN_(HAS_CLASSIC_JERK, "XYZE"))) return;

  //planner.synchronize();
  if (parser.seenval('B')) planner.settings.min_segment_time_us = parser.value_ulong();
  if (parser.seenval('S')) planner.settings.min_feedrate_mm_s = parser.value_linear_units();
  if (parser.seenval('T')) planner.settings.min_travel_feedrate_mm_s = parser.value_linear_units();
  #if HAS_JUNCTION_DEVIATION
    #if HAS_CLASSIC_JERK && (AXIS4_NAME == 'J' || AXIS5_NAME == 'J' || AXIS6_NAME == 'J')
      #error "Can't set_max_jerk for 'J' axis because 'J' is used for Junction Deviation."
    #endif
    if (parser.seenval('J')) {
      const float junc_dev = parser.value_linear_units();
      if (WITHIN(junc_dev, 0.01f, 0.3f)) {
        planner.junction_deviation_mm = junc_dev;
        TERN_(LIN_ADVANCE, planner.recalculate_max_e_jerk());
      }
      else
        SERIAL_ERROR_MSG("?J out of range (0.01 to 0.3)");
    }
  #endif
  #if HAS_CLASSIC_JERK
    bool seenZ = false;
    LOGICAL_AXIS_CODE(
      if (parser.seenval('E')) set_max_jerk(E_AXIS, parser.value_linear_units()),
      if (parser.seenval('X')) set_max_jerk(X_AXIS, parser.value_linear_units()),
      if (parser.seenval('Y')) set_max_jerk(Y_AXIS, parser.value_linear_units()),
      if ((seenZ = parser.seenval('Z'))) set_max_jerk(Z_AXIS, parser.value_linear_units()),
      if (parser.seenval(AXIS4_NAME)) set_max_jerk(I_AXIS, parser.value_linear_units()),
      if (parser.seenval(AXIS5_NAME)) set_max_jerk(J_AXIS, parser.value_linear_units()),
      if (parser.seenval(AXIS6_NAME)) set_max_jerk(K_AXIS, parser.value_linear_units())
    );
    #if HAS_MESH && DISABLED(LIMITED_JERK_EDITING)
      if (seenZ && planner.max_jerk.z <= 0.1f)
        SERIAL_ECHOLNPGM("WARNING! Low Z Jerk may lead to unwanted pauses.");
    #endif
  #endif // HAS_CLASSIC_JERK
}
/**
 * M4899: Set Advanced Settings
 *
 * T: 0=Version 1.0 1=Version 1.5
 */
void GcodeSuite::M4899() {
    #if DISTINCT_AXES < 4
      #error"M4899() is required with DISTINCT_AXES >= 4."
    #endif

    if (!parser.seen("T")){
      SERIAL_ECHOPAIR("LIN_ADV_version: T", planner.LIN_ADV_version_change);
      return;
    }

    if (parser.seenval('T')){
      uint8_t version = parser.value_byte();

      planner.synchronize();          // Wait for planner moves to finish!

      switch(version)
      {
        case LIN_ADV_VERSION_0:
          {
            DISABLE_ISRS();
            planner.LIN_ADV_version_change = LIN_ADV_VERSION_0;
            planner.settings.travel_acceleration = planner.settings.acceleration = DEFAULT_ACCELERATION;
            planner.settings.retract_acceleration = DEFAULT_RETRACT_ACCELERATION;
            set_max_jerk(X_AXIS, DEFAULT_XJERK);
            set_max_jerk(Y_AXIS, DEFAULT_YJERK);
            set_max_jerk(Z_AXIS, DEFAULT_ZJERK);
            set_max_jerk(E_AXIS, DEFAULT_EJERK);
            const feedRate_t feedrate[DISTINCT_AXES] = DEFAULT_MAX_FEEDRATE;
            planner.settings.max_feedrate_mm_s[X_AXIS] = feedrate[X_AXIS];
            planner.settings.max_feedrate_mm_s[Y_AXIS] = feedrate[Y_AXIS];
            planner.settings.max_feedrate_mm_s[Z_AXIS] = feedrate[Z_AXIS];
            planner.settings.max_feedrate_mm_s[E_AXIS] = feedrate[E_AXIS];
            const uint32_t max_accel[DISTINCT_AXES] = MAX_ACCEL_EDIT_VALUES;
            planner.settings.max_acceleration_mm_per_s2[X_AXIS] = max_accel[X_AXIS];
            planner.settings.max_acceleration_mm_per_s2[Y_AXIS] = max_accel[Y_AXIS];
            planner.settings.max_acceleration_mm_per_s2[Z_AXIS] = max_accel[Z_AXIS];
            planner.settings.max_acceleration_mm_per_s2[E_AXIS] = max_accel[E_AXIS];
            planner.reset_acceleration_rates();
            //planner.extruder_advance_K[E_INDEX_N(0)] = LIN_ADVANCE_K;
            ENABLE_ISRS();
          }
          break; // old version

        case LIN_ADV_VERSION_1: // read from flash
          {
            planner.LIN_ADV_version_change = LIN_ADV_VERSION_1;
            settings.load();
          }break;

        case LIN_ADV_VERSION_2: // new version only new K lin_adv
          {
            DISABLE_ISRS();
            planner.LIN_ADV_version_change = LIN_ADV_VERSION_2;
            planner.settings.travel_acceleration = planner.settings.acceleration = LA_V1_DEFAULT_ACCELERATION;
            planner.settings.retract_acceleration = LA_V1_DEFAULT_RETRACT_ACCELERATION;
            set_max_jerk(X_AXIS, DEFAULT_XJERK);
            set_max_jerk(Y_AXIS, DEFAULT_YJERK);
            set_max_jerk(Z_AXIS, DEFAULT_ZJERK);
            set_max_jerk(E_AXIS, LA_V1_DEFAULT_EJERK);
            const feedRate_t feedrate[DISTINCT_AXES] = LA_V1_DEFAULT_MAX_FEEDRATE;
            planner.settings.max_feedrate_mm_s[X_AXIS] = feedrate[X_AXIS];
            planner.settings.max_feedrate_mm_s[Y_AXIS] = feedrate[Y_AXIS];
            planner.settings.max_feedrate_mm_s[Z_AXIS] = feedrate[Z_AXIS];
            planner.settings.max_feedrate_mm_s[E_AXIS] = feedrate[E_AXIS];
            const uint32_t max_accel[DISTINCT_AXES] = LA_V1_MAX_ACCEL_EDIT_VALUES;
            planner.settings.max_acceleration_mm_per_s2[X_AXIS] = max_accel[X_AXIS];
            planner.settings.max_acceleration_mm_per_s2[Y_AXIS] = max_accel[Y_AXIS];
            planner.settings.max_acceleration_mm_per_s2[Z_AXIS] = max_accel[Z_AXIS];
            planner.settings.max_acceleration_mm_per_s2[E_AXIS] = max_accel[E_AXIS];
            planner.reset_acceleration_rates();
            //planner.extruder_advance_K[E_INDEX_N(0)] = LIN_ADVANCE_K;
            ENABLE_ISRS();
          }
          break; // new version

        case LIN_ADV_VERSION_3: // new version Scurve + new K lin_adv
          {
            DISABLE_ISRS();
            planner.LIN_ADV_version_change = LIN_ADV_VERSION_3;
            planner.settings.travel_acceleration = planner.settings.acceleration = LA_V1_DEFAULT_ACCELERATION;
            planner.settings.retract_acceleration = LA_V1_DEFAULT_RETRACT_ACCELERATION;
            set_max_jerk(X_AXIS, DEFAULT_XJERK);
            set_max_jerk(Y_AXIS, DEFAULT_YJERK);
            set_max_jerk(Z_AXIS, DEFAULT_ZJERK);
            set_max_jerk(E_AXIS, LA_V1_DEFAULT_EJERK);
            const feedRate_t feedrate[DISTINCT_AXES] = LA_V1_DEFAULT_MAX_FEEDRATE;
            planner.settings.max_feedrate_mm_s[X_AXIS] = feedrate[X_AXIS];
            planner.settings.max_feedrate_mm_s[Y_AXIS] = feedrate[Y_AXIS];
            planner.settings.max_feedrate_mm_s[Z_AXIS] = feedrate[Z_AXIS];
            planner.settings.max_feedrate_mm_s[E_AXIS] = feedrate[E_AXIS];
            const uint32_t max_accel[DISTINCT_AXES] = LA_V1_MAX_ACCEL_EDIT_VALUES;
            planner.settings.max_acceleration_mm_per_s2[X_AXIS] = max_accel[X_AXIS];
            planner.settings.max_acceleration_mm_per_s2[Y_AXIS] = max_accel[Y_AXIS];
            planner.settings.max_acceleration_mm_per_s2[Z_AXIS] = max_accel[Z_AXIS];
            planner.settings.max_acceleration_mm_per_s2[E_AXIS] = max_accel[E_AXIS];
            planner.reset_acceleration_rates();
            //planner.extruder_advance_K[E_INDEX_N(0)] = LIN_ADVANCE_K;
            ENABLE_ISRS();
          }break;

        default: break;
      }
      MYSERIAL2.printLine("LIN_ADV_version = %d\r\n", planner.LIN_ADV_version_change);
    }
}

void GcodeSuite::M4900() {
    GcodeSuite::M900();
}

#endif
