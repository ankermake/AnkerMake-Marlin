#include "../../inc/MarlinConfig.h"
#if ENABLED(ANKER_MAKE_API)
    #if ENABLED(ANKER_LOG_DEBUG)
        void anker_check_block_buf(void);
        void set_anker_debug_flag(int value);
        int get_anker_debug_flag();
    #endif
#endif
