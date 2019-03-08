
#include "../../main.h"
static int getPWMDevice_callback ( void *data, int argc, char **argv, char **azColName ) {
   PWMDevice *item = data;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
         item->id = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "pin" ) ) {
         item->pin = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "resolution" ) ) {
         item->pwm.resolution.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "period_sec" ) ) {
         item->pwm.period.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "period_nsec" ) ) {
         item->pwm.period.tv_nsec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "ds_min_sec" ) ) {
         item->pwm.duty_cycle_min.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "ds_min_nsec" ) ) {
         item->pwm.duty_cycle_min.tv_nsec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "ds_max_sec" ) ) {
         item->pwm.duty_cycle_max.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "ds_max_nsec" ) ) {
         item->pwm.duty_cycle_max.tv_nsec = DB_CVI;
         c++;
      } else {
         printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
         c++;
      }
   }
#define N 9
   if ( c != N ) {
      printde ( "required %d columns but %d found\n", N, c );
      return EXIT_FAILURE;
   }
#undef N
   return EXIT_SUCCESS;
}


static int checkPWMDevice ( PWMDevice *item ) {
   int success = 1;
   if ( item->pwm.period.tv_sec == 0 && item->pwm.period.tv_nsec == 0 ) {
      printde ( "bas pwm.period where id = %d", item->id );
      success = 0;
   }
   if ( !timespeccmp ( &item->pwm.duty_cycle_min, &item->pwm.duty_cycle_max, < ) ) {
      printde ( "pwm.duty_cycle_min < pwm.duty_cycle_max expected where id = %d", item->id );
      success = 0;
   }
   if ( !timespeccmp ( &item->pwm.period, &item->pwm.duty_cycle_min, > ) ) {
      printde ( "pwm.period > pwm.pwm.duty_cycle_min expected where id = %d", LIi.id );
      success = 0;
   }
   if ( item->pwm.resolution <= 0 ) {
      printde ( "bad pwm.resolution where id = %d", LIi.id );
      success = 0;
   }
   return success;
}


int getPWMDeviceByIdFromDB ( PWMDevice *item, int id, sqlite3 *dbl, const char *db_path ) {
   int close = 0;
   sqlite3 *db = db_openAlt ( dbl, db_path, &close );
   if ( db == NULL ) {
      putsde ( " failed\n" );
      return 0;
   }
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "SELECT pd.id AS id, pd.pin AS pin, p.resolution AS resolution, p.period_sec AS period_sec, p.period_nsec AS period_nsec, p.ds_min_sec AS ds_min_sec, p.ds_min_nsec AS ds_min_nsec, p.ds_max_sec AS ds_max_sec, p.ds_max_nsec AS ds_max_nsec, FROM pwm_device pd INNER JOIN pwm p WHERE pd.id=%d LIMIT 1", id );
   memset ( item, 0, sizeof * item );
   if ( !db_exec ( db, q, getPWMDevice_callback, item ) ) {
      putsde ( " failed\n" );
      if ( close ) db_close ( db );
      return 0;
   }
   if ( !checkPWMDevice ( item ) ) {
      if ( close ) db_close ( db );
      return 0;
   }
   if ( close ) db_close ( db );
   return 1;
}

