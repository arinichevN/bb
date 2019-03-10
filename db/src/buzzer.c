
#include "common.h"
static int getBuzzer_callback ( void *data, int argc, char **argv, char **azColName ) {
   Buzzer *item = data;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
         item->id = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "pin" ) ) {
         item->pin = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "delay_s" ) ) {
         item->delay.tv_sec = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "delay_ns" ) ) {
         item->delay.tv_nsec = DB_CVI;
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
#define N 6
   if ( c != N ) {
      printde ( "required %d columns but %d found\n", N, c );
      return EXIT_FAILURE;
   }
#undef N
   return EXIT_SUCCESS;
}

static int getSoundList_callback ( void *data, int argc, char **argv, char **azColName ) {
   SoundList *list = data;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
         list->item[list->length].id = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "sequence" ) ) {
         c++;
      } else if ( DB_COLUMN_IS ( "frequency" ) ) {
         list->item[list->length].frequency = DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "duration" ) ) {
         list->item[list->length].duration = DB_CVI;
         c++;
      } else {
         putsde ( "unknown column\n" );
      }
   }
#define N 4
   if ( c != N ) {
      printde ( "required %d columns but %d found\n", N, c );
      return EXIT_FAILURE;
   }

#undef N
   list->length++;
   return EXIT_SUCCESS;
}
static int checkSoundList(SoundList *list){
  int success = 1;
  FORLi{
    if(LIi.duration > 1000){
      printde("bad sound.duration where id = %d", LIi.id);
      success = 0;
    }
    if(LIi.frequency > 9999){
      printde("bad sound.frequency where id = %d", LIi.id);
      success = 0;
    }
  }
  return success;
}
int getSoundListFromDB ( SoundList *list, sqlite3 *db ) {
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
}
static int checkBuzzer ( Buzzer *item ) {
   int success = 1;
   if ( item->delay.tv_sec < 0 || item->delay.tv_nsec < 0 ) {
      printde ( "bad buzzer.delay where id = %d", item->id );
      success = 0;
   }
   if ( item->cycle_duration.tv_sec < 0 || item->cycle_duration.tv_nsec < 0 ) {
      printde ( "bad buzzer.cycle_duration where id = %d", item->id );
      success = 0;
   }
   if ( !checkPin ( item->pin ) ) {
      printde ( "bad buzzer.pin where id = %d", item->id );
      success = 0;
   }
   return success;
}
int getBuzzerByIdFromDB ( Buzzer *item, int id, sqlite3 *db) {
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "select * from buzzer where id=%d limit 1", id );
   memset ( item, 0, sizeof * item );
   if ( !db_exec ( db, q, getBuzzer_callback, item ) ) {
      putsde ( " failed\n" );
      return 0;
   }
   if(!getSoundListFromDB ( &item->sound_list, db )){
     putsde ( " failed to read sound list\n" );
     return 0;
   }
   if(!checkBuzzer(item)){
     return 0;
   }
   return 1;
}

