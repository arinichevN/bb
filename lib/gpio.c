
#include "gpio.h"

#ifdef GPIO_MUTEX
#define GPIO_DIR "gpio/mutex/"
#else
#define GPIO_DIR "gpio/"
#endif

#ifdef CPU_ALLWINNER_A20
//#include "allwinner_4.c"
#include GPIO_DIR "allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H3
//#include "allwinner_2.c"
#include GPIO_DIR "allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H2P
//#include "allwinner_2.c"
#include GPIO_DIR "allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H5
#include GPIO_DIR "allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_R40
#include GPIO_DIR "allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A33
#include GPIO_DIR "allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A31s
#include GPIO_DIR "allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A64
#include GPIO_DIR "allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A83T
#include GPIO_DIR "allwinner_2.c"
#else
#ifdef CPU_ROCKCHIP_RK3288
#include GPIO_DIR "rk3288.c"
#else
//debugging mode (for machine with no GPIO)
#include GPIO_DIR "all.c"
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
