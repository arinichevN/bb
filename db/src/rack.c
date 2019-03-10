
#include "common.h"

static int getData_callback ( void *data, int argc, char **argv, char **azColName ) {
   struct ds {void *p1; void *p2;} *d;
   d = data;
   Rack *item = d->p1;
   sqlite3 *db = d->p2;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
         item->id = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "flyte_id" ) ) {
         if ( getFlyteByIdFromDB ( &item->flyte, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "cooler_id" ) ) {
         if ( getCoolerByIdFromDB ( &item->cooler, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "buzzer_id" ) ) {
         if ( getBuzzerByIdFromDB ( &item->buzzer, DB_CVI, db ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "door_pin" ) ) {
         item->door.pin = DB_CVI;
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

static int checkData ( Rack *item ) {
   int success = 1;
   if ( item->cycle_duration.tv_sec < 0 || item->cycle_duration.tv_nsec < 0 ) {
      printde ( "bad rack.cycle_duration where id = %d", item->id );
      success = 0;
   }
   if ( item->interval.tv_sec < 0 || item->interval.tv_nsec < 0 ) {
      printde ( "bad rack.cycle_duration where id = %d", item->id );
      success = 0;
   }
   if ( !checkPin ( item->door.pin ) ) {
      printde ( "bad rack.door.pin where id = %d", item->id );
      success = 0;
   }
   return success;
}

int getRackByIdFromDB ( Rack *item, int id, sqlite3 *dbl, const char *db_path ) {
   int close = 0;
   sqlite3 *db = db_openAlt ( dbl, db_path, &close );
   if ( db == NULL ) {
      putsde ( " failed\n" );
      return 0;
   }
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "SELECT * FROM rack WHERE id=%d LIMIT 1", id );
   memset ( item, 0, sizeof * item );
   struct ds {void *p1; void *p2;} data = {.p1 = item, .p2 = db};
   if ( !db_exec ( db, q, getData_callback, &data ) ) {
      putsde ( " failed\n" );
      if ( close ) db_close ( db );
      return 0;
   }
   if(!getHiveListByIdFromDB ( &item->hive_list, item->id, db )){
     if ( close ) db_close ( db );
      return 0;
   }
   if ( close ) db_close ( db );
   if ( !checkData ( item ) ) {
      return 0;
   }
   return 1;
}

