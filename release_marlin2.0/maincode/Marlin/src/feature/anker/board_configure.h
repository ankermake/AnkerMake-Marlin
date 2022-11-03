#include "../../inc/MarlinConfig.h"

#if ENABLED(BOARD_CONFIGURE)
  #include "sys.h"


class Board_Configure {
public:
    u16 adc1,adc2;
    u16 board_info=0;
    char board_version[20];
    char board_chip[20];
    void init(void);
    u16 Get_Adc(u8 ch);
    u16 Get_Adc_Average(u8 ch,u8 times);

};

extern Board_Configure board_configure;

#endif
