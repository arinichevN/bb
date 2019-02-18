#include "main.h"

void freeChannel ( Channel * item ) {
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
    case DISABLE:
        return "DISABLE";
    case UNDEFINED:
        return "UNDEFINED";
    case FAILURE:
        return "FAILURE";
    }
    return "\0";
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

void resetFlyte(Prog *item, Slave *slave){
	time_t now;
   time ( &now );
   struct tm * nowi;
   nowi = localtime ( &now );
   struct tm * last;
   last = localtime ( &item->last_time );
   if(last->tm_mday != nowi->tm_mday){
		for(int i=0;i<slave->retry_num;i++){
			channelReset ( slave );
		}
		item->last_time = now;
	}
}
void progControl ( Prog *item, Sensor *sensor_temp, Sensor *sensor_hum,Sensor *sensor_fly, Slave *reg,Slave *flyte) {
    switch ( item->state ) {
    case INIT:
        tonSetInterval ( item->interval_read, &item->tmrr );
        tonReset ( &item->tmrr );
        tonSetInterval ( item->interval_set, &item->tmrs );
        tonReset ( &item->tmrs );
        item->state = RUN;
        break;
    case RUN:
        if ( ton ( &item->tmrr ) ) {
		   sensorRead ( sensor_temp ); 
		   sensorRead ( sensor_hum );  
		   sensorRead ( sensor_fly ); 
		   resetFlyte(item, flyte);
	   }  
	   if ( ton ( &item->tmrs ) ) {
		   acp_setRChannelFloat(&flyte->remote_channel, item->duty_cycle);
		   slaveSetGoal ( reg, item->goal ) ;
        }
        break;
    case DISABLE:
        item->state = OFF;
        break;
    case OFF:
        break;
    case FAILURE:
        break;
    default:
        item->state = FAILURE;
        break;
    }
}

int bufCatProgInfo ( Channel *item, ACPResponse *response ) {
    if ( lockMutex ( &item->mutex ) ) {
        char q[LINE_SIZE];
        char *state = getStateStr ( item->prog.state );
        int closed=0;
        if(item->prog.duty_cycle == item->prog.close_duty_cycle) closed = 1;
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
