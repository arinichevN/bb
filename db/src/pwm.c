
#include "common.h"
static int getData_callback ( void *data, int argc, char **argv, char **azColName ) {
   PWM *item = data;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "resolution" ) ) {
         item->resolution.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "period_sec" ) ) {
         item->period.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "period_nsec" ) ) {
         item->period.tv_nsec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "ds_min_sec" ) ) {
         item->duty_cycle_min.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "ds_min_nsec" ) ) {
         item->duty_cycle_min.tv_nsec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "ds_max_sec" ) ) {
         item->duty_cycle_max.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "ds_max_nsec" ) ) {
         item->duty_cycle_max.tv_nsec = DB_CVI;
         c++;
      } else {
         printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
         c++;
      }
   }
#define N 7
   if ( c != N ) {
      printde ( "required %d columns but %d found\n", N, c );
      return EXIT_FAILURE;
   }
#undef N
   return EXIT_SUCCESS;
}


static int checkData ( PWM *item ) {
   int success = 1;
   if ( item->period.tv_sec == 0 && item->period.tv_nsec == 0 ) {
      printde ( "bad pwm.period\n" );
      success = 0;
   }
   if ( !timespeccmp ( &item->duty_cycle_min, &item->duty_cycle_max, < ) ) {
      printde ( "pwm.duty_cycle_min < pwm.duty_cycle_max expected\n" );
      success = 0;
   }
   if ( !timespeccmp ( &item->period, &item->duty_cycle_min, > ) ) {
      printde ( "pwm.period > pwm.pwm.duty_cycle_min expected\n" );
      success = 0;
   }
   if ( item->resolution <= 0 ) {
      printde ( "bad pwm.resolution\n" );
      success = 0;
   }
   return success;
}


int getPWMByIdFromDB ( PWM *item, int id, sqlite3 *db ) {
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "SELECT * FROM pwm WHERE id=%d LIMIT 1", id );
   memset ( item, 0, sizeof * item );
   if ( !db_exec ( db, q, getData_callback, item ) ) {
      putsde ( " failed\n" );
      return 0;
   }
   if ( !checkData ( item ) ) {
      return 0;
   }
   return 1;
}

