
#include "common.h"
static int getData_callback ( void *data, int argc, char **argv, char **azColName ) {
   Presence *item = data;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
         item->id = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "pin" ) ) {
         item->pin = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "delay_disable_s" ) ) {
         item->delay_disable.tv_sec = DB_CVI;
         item->delay_disable.tv_nsec = 0;
         c++;
      } else if ( DB_COLUMN_IS ( "interval_update_s" ) ) {
         item->interval_update.tv_sec = DB_CVI;
         item->interval_update.tv_nsec = 0;
         c++;
      } else if ( DB_COLUMN_IS ( "max_rows" ) ) {
         item->max_rows = DB_CVI;
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

static int checkData ( Presence * item ) {
   int success = 1;
   if ( !checkPin ( item->pin ) ) {
      printde ( "bad presence.pin where id = %d", item->id );
      success = 0;
   }
   if ( item->delay_disable.tv_sec < 0 || item->delay_disable.tv_nsec < 0 ) {
      printde ( "bad presence.delay_disable where id = %d", item->id );
      success = 0;
   }
   if ( item->interval_update.tv_sec < 0 || item->interval_update.tv_nsec < 0 ) {
      printde ( "bad presence.interval_update where id = %d", item->id );
      success = 0;
   }
   if ( item->max_rows < 0 ) {
      printde ( "bad presence.max_rows where id = %d", item->id );
      success = 0;
   }
   return success;
}

int getPresenceByIdFromDB ( Presence * item, int id, sqlite3 * db ) {
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "select * from presence where id=%d limit 1", id );
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

