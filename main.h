
#ifndef BB_H
#define BB_H

#include "lib/dbl.h"
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

#define STATUS_SUCCESS "SUCCESS"
#define STATUS_FAILURE "FAILURE"
#define sensorRead(item) acp_getFTS ( &(item)->input, &(item)->remote_channel.peer, (item)->remote_channel.channel_id )

#define FFO ACP_FLOAT_FORMAT_OUT
enum ProgState {
    OFF,
    INIT,
    RUN,
    UNDEFINED,
    DISABLE,
    FAILURE
} ;

typedef struct {
    RChannel remote_channel;
    FTS input;
} Sensor;

typedef struct {
    RChannel remote_channel;
    int retry_num;
} Slave;

typedef struct {
    int id;
    struct timespec interval_read;
    struct timespec interval_set;
    int state;
    Ton tmrr;
    Ton tmrs;
    
    double goal;
    double delta;
    double open_duty_cycle;
    double close_duty_cycle;
    
    double duty_cycle;
    time_t last_time;

} Prog;

struct channel_st {
    int id;
    Prog prog;
    Sensor sensor_temp;
    Sensor sensor_hum;
    Sensor sensor_fly;
    Slave reg;
    Slave flyte;
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




