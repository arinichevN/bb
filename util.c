#include "main.h"

void freeChannel ( Channel * item ) {
    FREE_LIST ( item->hive_list );
    freeSocketFd ( &item->sock_fd );
    freeMutex ( &item->mutex );
    free ( item );
}

void freeChannelList ( ChannelLList * list ) {
    Channel *item = list->top, *temp;
    while ( item != NULL ) {
        temp = item;
        item = item->next;
        freeChannel ( temp );
    }
    list->top = NULL;
    list->last = NULL;
    list->length = 0;
}

int checkProg ( const Prog *item ) {
    return 1;
}

int checkChannel ( const Channel *item ) {
    int success=1;
    if ( item->cycle_duration.tv_sec < 0 ) {
        fprintf ( stderr, "%s(): bad cycle_duration_sec where id = %d\n", F, item->id );
        success= 0;
    }
    if ( item->cycle_duration.tv_nsec < 0 ) {
        fprintf ( stderr, "%s(): bad cycle_duration_nsec where id = %d\n", F, item->id );
        success= 0;
    }
    return success;
}

char * getStateStr ( int state ) {
    switch ( state ) {
    case OFF:
        return "OFF";
    case INIT:
        return "INIT";
    case RUN:
        return "RUN";
    case BUSY:
        return "BUSY";
    case IDLE:
        return "IDLE";
    case WAIT:
        return "WAIT";
    case ABSENT:
        return "ABSENT";
    case PRESENT:
        return "PRESENT";
    case DISABLE:
        return "DISABLE";
    case WAIT_PRESENT:
        return "WAIT_PRESENT";
    case UNDEFINED:
        return "UNDEFINED";
    case FAILURE:
        return "FAILURE";
    }
    return "\0";
}

int pp_clearFTS ( int id, PGconn *db_conn ) {
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "delete from log.v_real where id = %d", id );
    if ( !dbp_cmd ( db_conn, q ) ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): log function failed\n", F );
#endif
        return 0;
    }
    return 1;
}

int pp_saveFTS ( FTS *item, int rack_id, int hive_id, int kind, int max_rows, PGconn *db_conn ) {
    if ( max_rows <= 0 ) {
        return 0;
    }
    if ( item->state != 1 ) {
        return 0;
    }
    char q[LINE_SIZE];
    int id = rack_id * 100 + hive_id*10 + kind;
    snprintf ( q, sizeof q, "select log.do_real(%d,%f,%u,%ld)", id, item->value, max_rows, item->tm.tv_sec );
    PGresult *r;
    if ( !dbp_exec ( &r, db_conn, q ) ) {
        putsde ( "log function failed\n" );
        return 0;
    }
    PQclear ( r );
    return 1;
}

int pp_saveTime ( struct timespec item, int rack_id, int hive_id, int max_rows, PGconn *db_conn ) {
    if ( prog->max_rows <= 0 ) {
        return 0;
    }
    if ( item->state != 1 ) {
        return 0;
    }
    char q[LINE_SIZE];
    int id = rack_id * 10 + hive_id;
    snprintf ( q, sizeof q, "select log.do_time(%d,%ld,%u)", id, item.tv_sec, max_rows );
    PGresult *r;
    if ( !dbp_exec ( &r, db_conn, q ) ) {
        putsde ( "log function failed\n" );
        return 0;
    }
    PQclear ( r );
    return 1;
}
int readNsave ( int rack_id, int hive_id, int kind,  int max_rows, Sensor *sensor, PGconn *db_conn ) {
    switch ( sensorRead ( sensor ) ) {
    case ACP_RETURN_SUCCESS:
        pp_saveFTS ( &sensor->input, rack_id, hive_id, max_rows, db_conn )
        return 1;
    case ACP_RETURN_FAILURE:
        return 1;
    }
}
void saveTempHum ( THLogger *item, int rack_id, int hive_id,  PGconn *db_conn ) {
    switch ( item->state ) {
    case WAIT:
        if ( ton ( &item->tmr ) ) {
            item->done_hum = item->done_temp = 0;
            item->state = BUSY;
        }
        break;
    case BUSY:
        if ( !item->done_temp ) {
            switch ( sensorRead ( &item->sensor_temp ) ) {
            case ACP_RETURN_SUCCESS:
                pp_saveFTS ( &item->sensor_temp.input, rack_id, hive_id, TEMPERATURE, item->max_rows, db_conn );
                item->done_temp = 1;
                break;
            case ACP_RETURN_FAILURE:
                item->done_temp = 1;
                break;
            }
        }
        if ( !item->done_hum ) {
            switch ( sensorRead ( &item->sensor_hum ) ) {
            case ACP_RETURN_SUCCESS:
                pp_saveFTS ( &item->sensor_hum.input, rack_id, hive_id, HUMIDITY, item->max_rows, db_conn );
                item->done_hum = 1;
                break;
            case ACP_RETURN_FAILURE:
                item->done_hum = 1;
                break;
            }
        }
        if ( item->done_temp && item->done_hum ) {
            item->state = WAIT;
        }
        break;
    case INIT:
        tonSetInterval ( item->interval, &item->tmr );
        tonReset ( &item->tmr );
        item->state = WAIT;
        break;
    case OFF:
        break;
    default:
        break;
    }

}
void  saveFly ( FlyLogger *item, int rack_id, int hive_id,  PGconn *db_conn ) {
    switch ( item->state ) {
    case WAIT:
        if ( ton ( &item->tmr ) ) {
            item->state = BUSY;
        }
        break;
    case BUSY:
        switch ( sensorRead ( &item->sensor ) ) {
        case ACP_RETURN_SUCCESS:
            pp_saveFTS ( &item->sensor.input, rack_id, hive_id, FLY, item->max_rows, db_conn );
            item->state = WAIT;
            break;
        case ACP_RETURN_FAILURE:
            item->state = WAIT;
            break;
        }
        break;
    case INIT:
        tonSetInterval ( item->interval, &item->tmr );
        tonReset ( &item->tmr );
        item->state = WAIT;
        break;
    case OFF:
        break;
    default:
        break;
    }
}
void progEnable ( Prog *item ) {
    item->state=INIT;
}
void progDisable ( Prog *item ) {
    item->state=DISABLE;
}
void progStop ( Prog *item ) {
    item->state=DISABLE;;
}

int slaveSetGoal ( Slave *item, double goal ) {
    I1F1 i1f1 = {.p0 = item->remote_channel.channel_id, .p1 = goal};
    I1F1List data = {.item = &i1f1, .length = 1, .max_length=1};
    return acp_requestSendUnrequitedI1F1List ( ACP_CMD_REG_PROG_SET_GOAL, &data, &item->remote_channel.peer )  ;
}

int channelReset ( Slave *item ) {
    Peer *peer=&item->remote_channel.peer;
    int remote_channel_id=item->remote_channel.channel_id;
    I1List data = {.item = &remote_channel_id, .length = 1, .max_length=1};
    if ( !acp_requestSendUnrequitedI1List ( ACP_CMD_CHANNEL_RESET, &data, peer ) ) {
        return 0;
    }
    return 1;
}

void resetFlyte ( Prog *item, Slave *slave ) {
    time_t now;
    time ( &now );
    struct tm * nowi;
    nowi = localtime ( &now );
    struct tm * last;
    last = localtime ( &item->last_time );
    if ( last->tm_mday != nowi->tm_mday ) {
        for ( int i=0; i<slave->retry_num; i++ ) {
            channelReset ( slave );
        }
        item->last_time = now;
    }
}

void updateInstalledTime ( Presence *item, Slave *sound,  int rack_id, int hive_id, int max_rows, PGconn *db_conn ) {
    item->installed_time = getCurrentTime();
    pp_saveTime ( item->installed_time, rack_id, hive_id, max_rows, db_conn );
    beep_updated ( sound );
}
void flyteControl (Rack *rack, Slave *slave ) {

}
int presenceControl ( Presence *item, Slave *sound, int rack_id, int hive_id, PGconn *db_conn ) {
    switch ( item->state ) {
    case PRESENT:
        switch ( sensorRead ( &item->sensor ) ) {
        case ACP_RETURN_SUCCESS:
            if ( item->sensor.input.value <= 0.0 ) {//absent
                tonReset ( &item->tmr_update );
                tonReset ( &item->tmr_disable );
                item->state = WAIT_PRESENT;
            }
            break;
        case ACP_RETURN_FAILURE:
            break;
        }
        break;
    case ABSENT:
        switch ( sensorRead ( &item->sensor ) ) {
        case ACP_RETURN_SUCCESS:
            if ( item->sensor.input.value >= 1.0 ) {//present
                item->state = PRESENT;
            }
            break;
        case ACP_RETURN_FAILURE:
            break;
        }
        break;
    case WAIT_PRESENT: {
        int update = 1;
        if ( tonsp ( &item->tmr_update ) ) {
            update = 0;
            beep_no_update ( sound );
        }
        if ( tonsp ( &item->tmr_disable ) ) {
            beep_disabled ( sound );
            item->state = ABSENT;
            break;
        }
        switch ( sensorRead ( &item->sensor ) ) {
        case ACP_RETURN_SUCCESS:
            if ( item->sensor.input.value >= 1.0 ) {//present
                if ( update ) {
                    updateInstalledTime ( item, sound,  rack_id, hive_id, item->max_rows,db_conn );
                }
                item->state = PRESENT;
            }
            break;
        case ACP_RETURN_FAILURE:
            break;
        }
    }
    break;
    case INIT:
        tonSetInterval ( item->interval_update, &item->tmr_update );
        tonSetInterval ( item->delay_disable, &item->tmr_disable );
        switch ( sensorRead ( &item->sensor ) ) {
        case ACP_RETURN_SUCCESS:
            if ( item->sensor.input.value <= 0.0 ) {//absent
                item->state = ABSENT;
            } else {
                item->state = PRESENT;
            }
            break;
        case ACP_RETURN_FAILURE:
            break;
        }
        item->state = OFF;
        break;
    case OFF:
        break;
    default:
        break;
    }
    return item->state;
}
void hiveControl ( Hive *item, int rack_id, Slave *sound, PGconn *db_conn ) {
    switch ( item->state ) {
    case INIT:
        item->presence.state = item->fly_logger.state = item->th_logger.state = INIT;
        item->state=BUSY;
        break;
    case BUSY:
        saveTempHum ( &item->th_logger,rack_id, item->id, db_conn );
        saveFly ( &item->fly_logger, rack_id, item->id, db_conn );
        int pstate = presenceControl ( &item->presence, sound, rack_id, item->id, db_conn );
        if ( pstate == ABSENT ) {
            item->state = IDLE;
        }
        break;
    case IDLE: {
        int pstate = presenceControl ( &item->presence, sound, rack_id, item->id, db_conn );
        if ( pstate == PRESENT ) {
            item->fly_logger.state = item->th_logger.state = INIT;
            item->state = BUSY;
        }
        break;
    }
    case OFF:
        break;
    default:
        break;
    }
}
void rackControl ( Rack *item, PGconn *db_conn ) {
    switch ( item->state ) {
    case RUN:
        FORLISTN ( item->hive_list, i ) {
            hiveControl ( &item->hive_list.item[i], item->id, &item->slave_sound, db_conn );
        }
        doorControl ( &item->sensor_door, &item->slave_flyte );
        regControl ( &item->slave_reg );
        flyteControl(&item->slave_flyte);
        break;
    case INIT:
        FORLISTN ( item->hive_list, i ) {
            item->hive_list.item[i].state = INIT;
        }
        item->state = RUN;
        break;
    case OFF:
        break;
    default:
        break;
    }

}
void progControl ( Prog *item, Sensor *sensor_temp, Sensor *sensor_hum,Sensor *sensor_fly,Sensor *sensor_prs, Slave *reg, Slave *flyte, Slave *sound, Sensor *door ) {

}

int bufCatProgInfo ( Channel *item, ACPResponse *response ) {
    if ( lockMutex ( &item->mutex ) ) {
        char q[LINE_SIZE];
        char *state = getStateStr ( item->prog.state );
        int closed=0;
        if ( item->prog.duty_cycle == item->prog.close_duty_cycle ) closed = 1;
        snprintf ( q, sizeof q, "%d" CDS "%s" CDS FFO CDS FFO CDS FFO CDS "%d" CDS FFO CDS FFO CDS "%d" CDS "%d" CDS "%d" CDS "%d" CDS "%d" CDS "%d" RDS,
                   item->id,
                   state,
                   item->sensor_temp.input.value,
                   item->sensor_hum.input.value,
                   item->sensor_fly.input.value,
                   closed,
                   item->prog.goal,
                   item->prog.delta,
                   item->sensor_temp.input.state,
                   item->sensor_hum.input.state,
                   item->sensor_fly.input.state,
                   1,
                   1,
                   1
                 );
        unlockMutex ( &item->mutex );
        return acp_responseStrCat ( response, q );
    }
    return 0;
}

int bufCatProgEnabled ( Channel *item, ACPResponse *response ) {
    if ( lockMutex ( &item->mutex ) ) {
        char q[LINE_SIZE];
        int enabled;
        switch ( item->prog.state ) {
        case OFF:
        case FAILURE:
            enabled=0;
            break;
        default:
            enabled=1;
            break;
        }
        snprintf ( q, sizeof q, "%d" CDS "%d" RDS,
                   item->id,
                   enabled
                 );
        unlockMutex ( &item->mutex );
        return acp_responseStrCat ( response, q );
    }
    return 0;
}


void printData ( ACPResponse *response ) {
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "CONFIG_FILE: %s\n", CONFIG_FILE );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "port: %d\n", sock_port );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "db_prog_path: %s\n", db_prog_path );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "app_state: %s\n", getAppState ( app_state ) );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "PID: %d\n", getpid() );
    SEND_STR_L ( q )


}

void printHelp ( ACPResponse *response ) {
    char q[LINE_SIZE];
    SEND_STR ( "COMMAND LIST\n" )
    snprintf ( q, sizeof q, "%s\tput process into active mode; process will read configuration\n", ACP_CMD_APP_START );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tput process into standby mode; all running programs will be stopped\n", ACP_CMD_APP_STOP );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tfirst stop and then start process\n", ACP_CMD_APP_RESET );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tterminate process\n", ACP_CMD_APP_EXIT );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tget state of process; response: B - process is in active mode, I - process is in standby mode\n", ACP_CMD_APP_PING );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tget some variable's values; response will be packed into multiple packets\n", ACP_CMD_APP_PRINT );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tget this help; response will be packed into multiple packets\n", ACP_CMD_APP_HELP );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tload channel into RAM and start its execution; channel id expected\n", ACP_CMD_CHANNEL_START );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tunload channel from RAM; channel id expected\n", ACP_CMD_CHANNEL_STOP );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tunload channel from RAM, after that load it; channel id expected\n", ACP_CMD_CHANNEL_RESET );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tenable running channel; channel id expected\n", ACP_CMD_CHANNEL_ENABLE );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tdisable running channel; channel id expected\n", ACP_CMD_CHANNEL_DISABLE );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tget channel state (1-enabled, 0-disabled); channel id expected\n", ACP_CMD_CHANNEL_GET_ENABLED );
    SEND_STR ( q )
    snprintf ( q, sizeof q, "%s\tget channel info; channel id expected\n", ACP_CMD_CHANNEL_GET_INFO );
    SEND_STR_L ( q )
}
