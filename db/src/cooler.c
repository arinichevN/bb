
#include "common.h"

static int getData_callback ( void *data, int argc, char **argv, char **azColName ) {
   struct ds {void *p1; void *p2;} *d;
   d = data;
   Cooler *item = d->p1;
   sqlite3 *db = d->p2;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
         item->id = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "pin" ) ) {
         item->em.pin = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "pwm_id" ) ) {
         if ( getPWMByIdFromDB ( &item->em.pwm, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "ds18b20_id" ) ) {
         if ( getDS18B20ByIdFromDB ( &item->sensor, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "pid_id" ) ) {
         if ( getPIDByIdFromDB ( &item->regulator->pid, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "goal" ) ) {
         item->regulator.goal = DB_CVF;
         c++;
      } else if ( DB_COLUMN_IS ( "interval_s" ) ) {
         item->interval.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "interval_ns" ) ) {
         item->interval.tv_nsec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "cycle_duration_s" ) ) {
         item->cycle_duration.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "cycle_duration_ns" ) ) {
         item->cycle_duration.tv_nsec = DB_CVI;
         c++;
      } else {
         printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
         c++;
      }
   }
#define N 10
   if ( c != N ) {
      printde ( "required %d columns but %d found\n", N, c );
      return EXIT_FAILURE;
   }
#undef N
   return EXIT_SUCCESS;
}

static int checkData ( Cooler *item ) {
   int success = 1;
   if ( item->cycle_duration.tv_sec < 0 || item->cycle_duration.tv_nsec < 0 ) {
      printde ( "bad cooler.cycle_duration where id = %d", item->id );
      success = 0;
   }
   if ( item->interval.tv_sec < 0 || item->interval.tv_nsec < 0 ) {
      printde ( "bad cooler.cycle_duration where id = %d", item->id );
      success = 0;
   }
   if ( !checkPin ( item->em.pin ) ) {
      printde ( "bad cooler.pin where id = %d", item->id );
      success = 0;
   }
   return success;
}

int getCoolerByIdFromDB ( Cooler *item, int id, sqlite3 *db ) {
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "SELECT * FROM cooler WHERE id=%d LIMIT 1", id );
   memset ( item, 0, sizeof * item );
   struct ds {void *p1; void *p2;} data = {.p1 = item, .p2 = db};
   if ( !db_exec ( db, q, getData_callback, &data ) ) {
      putsde ( " failed\n" );
      return 0;
   }
   if ( !checkData ( item ) ) {
      return 0;
   }
   return 1;
}

