
#include "common.h"
static int getData_callback ( void *data, int argc, char **argv, char **azColName ) {
   PID *item = data;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
         c++;
      } else if ( DB_COLUMN_IS ( "kp" ) ) {
         item->kp = DB_CVF;
         c++;
      } else if ( DB_COLUMN_IS ( "ki" ) ) {
         item->ki = DB_CVF;
         c++;
      } else if ( DB_COLUMN_IS ( "kd" ) ) {
         item->kd = DB_CVF;
         c++;
      } else if ( DB_COLUMN_IS ( "output_min" ) ) {
         item->output_min = DB_CVF;
         c++;
      } else if ( DB_COLUMN_IS ( "output_max" ) ) {
         item->output_max = DB_CVF;
         c++;
      } else {
         printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
         c++;
      }
   }
#define N 6
   if ( c != N ) {
      printde ( "required %d columns but %d found\n", N, c );
      return EXIT_FAILURE;
   }
#undef N
   return EXIT_SUCCESS;
}


static int checkData ( PID *item ) {
   int success = 1;
   return success;
}


int getPIDByIdFromDB ( PID *item, int id, sqlite3 *db ) {
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "SELECT * FROM pid WHERE id=%d LIMIT 1", id );
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

