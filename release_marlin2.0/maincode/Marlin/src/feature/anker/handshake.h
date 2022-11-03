#include "../../inc/MarlinConfig.h"

#if ENABLED(HANDSHAKE)

  #include "sys.h"

class HandShake {
public:
    void init();
    void check();
    void handshake_test();
};

extern HandShake hand_shake;
#endif

