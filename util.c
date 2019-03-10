#include "main.h"

void freeRack ( Rack * item ) {
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
   int success = 1;
   if ( item->cycle_duration.tv_sec < 0 ) {
      fprintf ( stderr, "%s(): bad cycle_duration_sec where id = %d\n", F, item->id );
      success = 0;
   }
   if ( item->cycle_duration.tv_nsec < 0 ) {
      fprintf ( stderr, "%s(): bad cycle_duration_nsec where id = %d\n", F, item->id );
      success = 0;
   }
   return success;
}

FUN_FIFO_PUSH ( int )

FUN_FIFO_POP ( int )

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
      case OPENED:
         return "OPENED";
      case CLOSED:
         return "CLOSED";
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
int getFlyteState ( Flyte *item ) {
   if ( item->duty_cycle == item->open_duty_cycle ) {
      return OPENED;
   } else if ( item->duty_cycle == item->close_duty_cycle ) {
      return CLOSED;
   }
   return OFF;
}
void flyteOpen ( Flyte *item ) {
   if ( lockMutex ( &item->mutex ) ) {
      item->duty_cycle = item->open_duty_cycle;
      unlockMutex ( &item->mutex );
   }
}
void flyteClose ( Flyte *item ) {
   if ( lockMutex ( &item->mutex ) ) {
      item->duty_cycle = item->close_duty_cycle;
      unlockMutex ( &item->mutex );
   }
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

int pp_saveTemp ( double value, int rack_id, int hive_id, int max_rows, PGconn *db_conn ) {
   if ( max_rows <= 0 ) {
      return 0;
   }
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "select bb.save_fly(%d,%d,%.3f,%d)", rack_id, hive_id, value, max_rows );
   PGresult *r;
   if ( !dbp_exec ( &r, db_conn, q ) ) {
      putsde ( "save temp function failed\n" );
      return 0;
   }
   PQclear ( r );
   return 1;
}

int pp_saveFly ( int value, int rack_id, int hive_id, int max_rows, PGconn *db_conn ) {
   if ( max_rows <= 0 ) {
      return 0;
   }
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "select bb.save_fly(%d,%d,%d,%d)", rack_id, hive_id, value, max_rows );
   PGresult *r;
   if ( !dbp_exec ( &r, db_conn, q ) ) {
      putsde ( "save fly function failed\n" );
      return 0;
   }
   PQclear ( r );
   return 1;
}

int pp_saveInstalledTime ( struct timespec item, int rack_id, int hive_id, int max_rows, PGconn *db_conn ) {
   if ( max_rows <= 0 ) {
      return 0;
   }
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "select bb.bb.save_installed(%d,%d,%d)", rack_id, hive_id, max_rows );
   PGresult *r;
   if ( !dbp_exec ( &r, db_conn, q ) ) {
      putsde ( "save update time function failed\n" );
      return 0;
   }
   PQclear ( r );
   return 1;
}

void  saveTemp ( Logger *item, int rack_id, int hive_id, Cooler *reg,  PGconn *db_conn ) {
   switch ( item->state ) {
      case WAIT:
         if ( ton ( &item->tmr ) ) {
            item->state = BUSY;
         }
         break;
      case BUSY:
         if ( lockMutex ( &reg->mutex ) ) {
            if ( item->last_tm != reg->sensor.tm ) {
               pp_saveTemp ( reg->sensor.value, rack_id, hive_id, item->max_rows, db_conn );
               item->last_tm = reg->sensor.tm;
            }
            unlockMutex ( &reg->mutex );
         }
         item->state = WAIT;

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

void  saveFly ( Logger *item, int rack_id, int hive_id, FlyCounter *sensor,  PGconn *db_conn ) {
   switch ( item->state ) {
      case WAIT:
         if ( ton ( &item->tmr ) ) {
            item->state = BUSY;
         }
         break;
      case BUSY:
         if ( lockMutex ( &sensor->mutex ) ) {
            pp_saveFly ( sensor->count, rack_id, hive_id, item->max_rows, db_conn );
            item->state = WAIT;
            unlockMutex ( &sensor->mutex );
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

void flyCounter ( FlyCounter *item ) {
   switch ( item->state ) {
      case WAIT:
         if ( pinRead ( item->pin ) == HIGH ) {
            if ( lockMutex ( &item->mutex ) ) {
               item->count++;
               unlockMutex ( &item->mutex );
            } else {
               item->count++;
            }
            tonSetInterval ( item->delay, &item->tmr );
            tonReset ( &item->tmr );
            item->state = BUSY;
         }
         break;
      case BUSY:
         if ( tonsp ( &item->tmr ) ) {
            if ( pinRead ( item->pin ) == LOW ) {
               item->state = BUSY;
            }
         }
         break;
      case INIT:
         tonSetInterval ( item->delay, &item->tmr );
         tonReset ( &item->tmr );
         item->state = WAIT;
         break;
   }
}

void beep ( int pin, unsigned int frequency, unsigned int duration ) {
   long beepDelay = ( long ) ( 1000000 / frequency );
   long time = ( long ) ( ( duration * 1000 ) / ( beepDelay * 2 ) );
   for ( int i = 0; i < time; i++ ) {
      pinHigh ( pin );
      DELAY_US_IDLE ( beepDelay );
      pinLow ( pin );
      DELAY_US_IDLE ( beepDelay );
   }
   pinLow ( pin );
   DELAY_US_IDLE ( 20000 );
}

void playSound ( int sound_id, SoundList *list, int pin ) {
   FORLi {
      if ( LIi.id == sound_id ) {
         beep ( pin, LIi.frequency, LIi.duration );
      }
   }
}

void soundControl ( Buzzer *item ) {
   int sound_id;
   if ( int_fifo_pop ( &sound_id, &item->queue ) ) {
      playSound ( sound_id, &item->sound_list, item->pin );
   }
   DELAY_US_IDLE ( item->delay_usec );
}

void doorControl ( Door *item, Flyte *flyte ) {
   switch ( item->state ) {
      case CLOSED:
         if ( pinRead ( item->pin ) == LOW ) { //opened
            item->last_flyte_state == getFlyteState ( flyte );
            if ( item->last_flyte_state == OPENED ) {
               flyteClose ( flyte );
            }
            item->state = OPENED;
         }
         break;
      case OPENED:
         if ( pinRead ( item->pin ) == HIGH ) { //opened
            if ( item->last_flyte_state == OPENED ) {
               flyteOpen ( flyte );
            }
            item->state = CLOSED;
         }
         break;
      case INIT:
         item->last_flyte_state == OFF;
         break;
   }
}



void pwmControl ( PWMDevice *item ) {
   int v;
   if ( lockMutex ( &item->mutex ) ) {
      v = pwm_control ( &item->pwm, item->duty_cycle );
      unlockMutex ( &item->mutex );
   }
   if ( v == item->last_v ) return;
   item->last_v = v;
   switch ( v ) {
      case PWM_BUSY:
         pinHigh ( item->pin );
         break;
      case PWM_IDLE:
         pinLow ( item->pin );
         break;
   }
}

int presenceControl ( Presence *item,  int rack_id, int hive_id, Buzzer *buzzer, PGconn *db_conn ) {
   switch ( item->state ) {
      case PRESENT:
         if ( pinRead ( item->pin ) == LOW ) { //absent
            tonReset ( &item->tmr_update );
            tonReset ( &item->tmr_disable );
            item->state = WAIT_PRESENT;
         }
         break;
      case ABSENT:
         if ( pinRead ( item->pin ) == HIGH ) { //present
            item->state = PRESENT;
         }
         break;
      case WAIT_PRESENT: {
            int update = 1;
            if ( tonsp ( &item->tmr_update ) ) {
               update = 0;
               int_fifo_push ( SOUND_NO_UPDATE, &buzzer->queue );
            }
            if ( tonsp ( &item->tmr_disable ) ) {
               int_fifo_push ( SOUND_DISABLED, &buzzer->queue );
               item->state = ABSENT;
               break;
            }
            if ( pinRead ( item->pin ) == HIGH ) { //present
               if ( update ) {
                  item->installed_time = getCurrentTime();
                  pp_saveInstalledTime ( rack_id, hive_id, item->max_rows, db_conn );
                  int_fifo_push ( SOUND_UPDATED, &buzzer->queue );
               }
               item->state = PRESENT;
            }
         }
         break;
      case INIT:
         tonSetInterval ( item->interval_update, &item->tmr_update );
         tonSetInterval ( item->delay_disable, &item->tmr_disable );
         if ( pinRead ( item->pin ) == LOW ) { //absent
            item->state = ABSENT;
         } else {
            item->state = PRESENT;
         }
         break;
      case OFF:
         break;
      default:
         break;
   }
   return item->state;
}

void setPower ( PWMDevice *item, double value ) {
   if ( lockMutex ( &item->mutex ) ) {
      item->duty_cycle = value;
      unlockMutex ( &item->mutex );
   }
}

void tempControl ( Cooler *item ) {
   switch ( item->state ) {
      case RUN:
         if ( ton ( &item->tmr, item->interval ) ) {
            int r = ds18b20_get_temp ( item->sensor.pin, item->sensor.addr, &item->sensor.value );
            if ( !r ) {
               setPower ( &item->em, item->em.pwm.duty_cycle_min );
               return;
            }
            item->sensor.tm = getCurrentTime();
            double output = pid ( &item->regulator.pid, item->regulator.goal, item->sensor.value );
            setPower ( &item->em, output );
         }
         break;
      case INIT:
         tonReset ( &item->tmr );
         item->state = RUN;
         break;
      case OFF:
         break;
      default:
         break;
   }
}

int bufCatProgInfo ( Channel *item, ACPResponse *response ) {
   if ( lockMutex ( &item->mutex ) ) {
      char q[LINE_SIZE];
      char *state = getStateStr ( item->prog.state );
      int closed = 0;
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
            enabled = 0;
            break;
         default:
            enabled = 1;
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
