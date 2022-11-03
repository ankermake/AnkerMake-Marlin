/*
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime : 
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"
#if ENABLED(ANKER_ANLIGN)

    class Anker_Align {
    public:
      static xy_pos_t xy[NUM_Z_STEPPER_DRIVERS];
      static float z1_value,z2_value;
      //static uint8_t anker_is_leveing;
      void init(void);
      void reset(void);
      void run_align(void);  //Run after double Z reset
      void auto_align(void); //Automatically detect two points on the heated bed to calculate the distance
      void add_z_value(float value,uint8_t choose);
      void add_z1_value(float value);
      void add_z2_value(float value);
      void add_z1_value_no_save(float value);
      void add_z2_value_no_save(float value);
    };
   extern Anker_Align anker_align;
#endif

#if ENABLED(ANKER_ANLIGN_ONLY_Z)

   class Anker_Align_Only_Z {
    public:
      static xy_pos_t xy;
      void init();
      void auto_run();
   };
  extern Anker_Align_Only_Z anker_align_only_z;
#endif
