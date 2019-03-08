
#ifndef BB_H
#define BB_H

#include "lib/dbl.h"
#include "lib/dbp.h"
#include "lib/configl.h"
#include "lib/util.h"
#include "lib/crc.h"
#include "lib/app.h"
#include "lib/timef.h"
#include "lib/udp.h"
#include "lib/tsv.h"
#include "lib/pid.h"
#include "lib/pwm.h"
#include "lib/device/ds18b20.h"
#include "lib/acp/main.h"
#include "lib/acp/app.h"
#include "lib/acp/channel.h"
#include "lib/acp/bb.h"


#define APP_NAME bb
#define APP_NAME_STR TOSTRING(APP_NAME)


#ifdef MODE_FULL
#define CONF_DIR "/etc/controller/" APP_NAME_STR "/"
#endif
#ifndef MODE_FULL
#define CONF_DIR "./"
#endif
#define CONFIG_FILE "" CONF_DIR "config.tsv"

#define WAIT_RESP_TIMEOUT 1
#define RETRY_COUNT 3

#define SOUND_NO_UPDATE 1
#define SOUND_UPDATED 2
#define SOUND_DISABLED 3

#define STATUS_SUCCESS "SUCCESS"
#define STATUS_FAILURE "FAILURE"
#define sensorRead(item) acp_getFTS ( &(item)->input, &(item)->remote_channel.peer, (item)->remote_channel.channel_id )

#define FFO ACP_FLOAT_FORMAT_OUT
enum ProgState {
    OFF,
    INIT,
    RUN,
    WAIT_PRESENT,
    PRESENT,
    ABSENT,
    BUSY,
    IDLE,
    WAIT,
    OPENED,
    CLOSED,
    UNDEFINED,
    DISABLE,
    FAILURE
} ;

enum Kind {
    TEMPERATURE,
    HUMIDITY,
    FLY
};

DEC_FIFO_LIST(int)
DEC_FUN_FIFO_PUSH(int)
DEC_FUN_FIFO_POP(int)

typedef struct {
    int id;
    int frequency;
    int duration;
} Sound;
DEC_LIST(Sound)

typedef struct {
  int id;
    int pin;
    SoundList sound_list;
    FIFOItemList_int queue;
    struct timespec delay;
} Buzzer;

typedef struct {
    PWM pwm;
    int pin;
    double duty_cycle;
    int last_v;
    Mutex mutex;
} PWMDevice;

typedef struct {
    int pin;
    uint8_t addr[DS18B20_SCRATCHPAD_BYTE_NUM];
    int resolution;
} DS18B20Device;

typedef struct {
    PID pid;
    double goal;
    Mutex mutex;
} PIDRegulator;

typedef struct {
    int pin;
    int state;
    Ton tmr_update;
    Ton tmr_disable;
    struct timespec delay_disable;
    struct timespec interval_update;
    struct timespec installed_time;
    int max_rows;
} Presence;

typedef struct {
    struct timespec last_tm;
    struct timespec interval;
    int max_rows;
    Ton tmr;
    int state;
} TempLogger;

typedef struct {
    struct timespec interval;
    int max_rows;
    Ton tmr;
    int state;
} FlyLogger;

typedef struct {
    int pin;
    struct timespec delay;
    int count;
    Ton tmr;
    int state;
    Mutex mutex;
} FlyCounter;

typedef struct {
    PWMDevice device;
    double open_duty_cycle;
    double close_duty_cycle;
    struct timespec interval;
    Ton tmr;
    int state;
} Flyte;

typedef struct {
    PWMDevice em;
    DS18B20Device sensor;
    PIDRegulator regulator;
    
    struct timespec interval;
    Ton tmr;
    int state;
    Mutex mutex;
} Cooler;

typedef struct {
    int pin;
    int last_flyte_state;
    int state;
} Door;

typedef struct {
    int id;
    FlyCounter fly_counter;
    FlyLogger fly_logger;
    TempLogger temp_logger;
    Presence presence;

    int state;
} Hive;
DEC_LLIST ( Hive )

typedef struct {
    int id;
    HiveList hive_list;
    Flyte flyte;
    Cooler cooler;
    Door door;
    Buzzer buzzer;
} Rack;


struct channel_st {
    int id;
    Rack rack;



    int sock_fd;
    int save;
    struct timespec cycle_duration;
    Mutex mutex;
    pthread_t thread;
    struct channel_st *next;
};

typedef struct channel_st Channel;

DEC_LLIST ( Channel )

struct thread_st {
    char *id;
    pthread_t thread;
    struct timespec cycle_duration;
};
typedef struct thread_st Thread;
DEC_LIST(Thread)

extern int readSettings();

extern int initData();

extern int initApp();

extern void serverRun ( int *state, int init_state );

extern void *threadFunction ( void *arg );

extern void freeData();

extern void freeApp();

extern void exit_nicely();

#endif




