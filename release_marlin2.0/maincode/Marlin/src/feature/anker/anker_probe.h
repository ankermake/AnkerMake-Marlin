/*
 * @Author       : Anan
 * @Date         : 2023-07-03 11:01:23
 * @LastEditors  : Anan.Huang
 * @LastEditTime : 2023-07-03 11:01:23
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if ENABLED(ANKER_MAKE_API)


    class Anker_probe {
        public:
            TERN_(WS1_HOMING_5X, static bool safely_delay;)//Determine whether a delay before zeroing is required
            static void random_movexy(const_float_t zraise = 0.0);
            static bool try_to_probe(PGM_P const plbl, const_float_t z_probe_low_point, const feedRate_t fr_mm_s, const bool scheck, const float clearance);
            static bool Mult_detections(const float z_probe_low_point, const float first_probe_z, const bool sanity_check=true);
            static int homing_Mult_detections(const AxisEnum axis, const float probe_first, const float bump);
            static bool homing_move(const AxisEnum axis, const float distance, const feedRate_t fr_mm_s=0.0, const bool final_approach=true, const bool zero_axis=true);
            static int Probe_homeaxis(const AxisEnum axis);
            #if ENABLED(ANKER_PROBE_CONFIRM_RETRY)
            static bool is_probing_flag;
            static void stop_all_steppers(const AxisEnum axis);
            static bool probe_pin_state_filter(void);
            static bool is_in_probing_state(void);
            bool probe_start_confirm(const uint16_t value);
            #endif
            static bool anker_probe_home_synchronize(void); // Block until all buffered steps are executed / cleaned
        private:
            static uint8_t try_times; // The number of attempts for random movement XY-axis
            static uint8_t randomxy_flag; 
    };
    
    extern Anker_probe anker_probe;

#endif /* ANKER_MAKE_API */
