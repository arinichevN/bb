
#include "common.h"
static int getFC_callback ( void *data, int argc, char **argv, char **azColName ) {
   FlyCounter *item = data;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
         item->id = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "pin" ) ) {
         item->pin = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "delay_ms" ) ) {
         int ms = DB_CVI;
         item->delay.tv_sec = ms / 1000;
         item->delay.tv_nsec = ms % 1000 * 1000000;
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
#define N 5
   if ( c != N ) {
      printde ( "required %d columns but %d found\n", N, c );
      return EXIT_FAILURE;
   }
#undef N
   return EXIT_SUCCESS;
}

static int checkFlyCounter ( FlyCounter *item ) {
   int success = 1;
   if ( item->delay.tv_sec < 0 || item->delay.tv_nsec < 0 ) {
      printde ( "bad fly_counter.delay where id = %d", item->id );
      success = 0;
   }
   if ( item->cycle_duration.tv_sec < 0 || item->cycle_duration.tv_nsec < 0 ) {
      printde ( "bad fly_counter.cycle_duration where id = %d", item->id );
      success = 0;
   }
   if ( !checkPin ( item->pin ) ) {
      printde ( "bad fly_counter.pin where id = %d", item->id );
      success = 0;
   }
   return success;
}

int getFlyCounterByIdFromDB ( FlyCounter *item, int id, sqlite3 *db ) {
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "select * from fly_counter where id=%d limit 1", id );
   memset ( item, 0, sizeof * item );
   if ( !db_exec ( db, q, getFC_callback, item ) ) {
      putsde ( " failed\n" );
      return 0;
   }
   if(!checkFlyCounter(item)){
     return 0;
   }
   return 1;
}

