
#include "common.h"

static int getData_callback ( void *data, int argc, char **argv, char **azColName ) {
   struct ds {void *p1; void *p2;} *d;
   d = data;
   HiveList *list = d->p1;
   sqlite3 *db = d->p2;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "rack_id" ) ) {
         c++;
      } else if ( DB_COLUMN_IS ( "id" ) ) {
         list->item[list->length].id = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "fly_counter_id" ) ) {
         if ( getFlyCounterByIdFromDB ( &list->item[list->length].fly_counter, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "fly_logger_id" ) ) {
         if ( getLoggerByIdFromDB ( &list->item[list->length].fly_logger, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "temp_logger_id" ) ) {
         if ( getLoggerByIdFromDB ( &list->item[list->length].temp_logger, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "presence_id" ) ) {
         if ( getPresenceByIdFromDB ( &list->item[list->length].presence, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
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
   list->length++;
   return EXIT_SUCCESS;
}

static int checkData ( HiveList *list ) {
   int success = 1;
   FORLi{
     ;
   }
   return success;
}

int getRackHiveListByIdFromDB ( HiveList *list, int rack_id, sqlite3 *db ) {
   RESET_LIST ( list )
   int n = 0;
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "SELECT count(*) FROM rack_hive WHERE rack_id=%d", rack_id );
   db_getInt ( &n, db, q );
   if ( n <= 0 ) {
      return 1;
   }
   ALLOC_LIST ( list, n )
   if ( list->max_length != n ) {
      putsde ( "failed to allocate memory\n" );
      return 0;
   }
   snprintf ( q, sizeof q, "SELECT * FROM rack_hive WHERE rack_id=%d ORDER BY id", rack_id );
   if ( !db_exec ( db, q, getData_callback, list ) ) {
      putsde ( "failed\n" );
      FREE_LIST ( list );
      return 0;
   }
   if ( !checkData ( list ) ) {
      FREE_LIST ( list );
      return 0;
   }
   return 1;
}

