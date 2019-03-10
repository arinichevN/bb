
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

int getRackHiveListByIdFromDB ( HiveList *list, int rack_id, sqlite3 *db ) {
  RESET_LIST ( list )
    int n = 0;
    char *qn = "select count(*) FROM sound";
    db_getInt ( &n, db, qn );
    if ( n <= 0 ) {
        return 1;
    }
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
        putsde ( "failed to allocate memory\n" );
        return 0;
    }
    char *q = "SELECT sound.id AS id, sound.sequence AS sequence, note.frequency AS frequency, sound.duration AS duration FROM sound INNER JOIN note ON sound.note_id = note.id ORDER BY id, sequence";
    if ( !db_exec ( db, q, getSoundList_callback, list ) ) {
        putsde ( "failed\n" );
        FREE_LIST ( list );
        return 0;
    }
    if ( !checkSoundList ( list ) ) {
        FREE_LIST ( list );
        return 0;
    }
    return 1;
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "SELECT * FROM rack_hive WHERE rack_id=%d LIMIT 1", rack_id );
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

