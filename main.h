
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
#include "lib/acp/main.h"
#include "lib/acp/app.h"
#include "lib/acp/channel.h"
#include "lib/acp/bb.h"
#include "lib/acp/regulator.h"


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

#define beep_no_update(slave) beep(slave, 200, 5000)
#define beep_updated(slave) beep(slave, 500, 5000)
#define beep_disabled(slave) beep(slave, 100, 5000);beep(slave, 100, 5000)

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
    UNDEFINED,
    DISABLE,
    FAILURE
} ;

enum Kind {
    TEMPERATURE,
    HUMIDITY,
    FLY
};

typedef struct {
    RChannel remote_channel;
    FTS input;
} Sensor;

typedef struct {
    RChannel remote_channel;
    int retry_num;
} Slave;

typedef struct {
    Sensor sensor;
    int state;
    Ton tmr_update;
    Ton tmr_disable;
    struct timespec delay_disable;
    struct timespec interval_update;
    struct timespec installed_time;
    int max_rows;
} Presence;

typedef struct {
    Sensor sensor_temp;
    Sensor sensor_hum;
    int max_rows;
    struct timespec interval;
    int done_temp;
    int done_hum;
    Ton tmr;
    int state;
} THLogger;

typedef struct {
    Sensor sensor;
    double saved_fly;
    struct timespec interval;
    int max_rows;
    Ton tmr;
    int state;
} FlyLogger;

typedef struct {
    int id;
    THLogger th_logger;
    FlyLogger fly_logger;
    Presence presence;

    int state;
} Hive;
DEC_LLIST ( Hive )

typedef struct {
    double duty_cycle;
    Slave slave;
    double open_duty_cycle;
    double close_duty_cycle;
    struct timespec interval;
    Ton tmr;
    int state;
} FlyteCtl;

typedef struct {
    double goal;
    Slave slave;
    struct timespec interval;
    Ton tmr;
    int state;
} RegCtl;

typedef struct {
    double goal;
    Sensor sensor;
    int state;
} DoorCtl;

typedef struct {
    int id;
    FlyteCtl flyte;
    RegCtl reg;
    DoorCtl door;
    double goal;

    HiveList hive_list;
    Slave slave_reg;
    Slave slave_sound;
    Sensor sensor_door;


    int state;
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

extern int readSettings();

extern int initData();

extern int initApp();

extern void serverRun ( int *state, int init_state );

extern void *threadFunction ( void *arg );

extern void freeData();

extern void freeApp();

extern void exit_nicely();

#endif




