
#include "common.h"
static int getData_callback ( void *data, int argc, char **argv, char **azColName ) {
   DS18B20Device *item = data;
   int c = 0;
   DB_FOREACH_COLUMN {
      if ( DB_COLUMN_IS ( "id" ) ) {
	item->id= DB_CVI;
         c++;
      } else if ( DB_COLUMN_IS ( "pin" ) ) {
         item->pin = DB_CVF;
         c++;
      } else if ( DB_COLUMN_IS ( "address" ) ) {
         if ( !ds18b20_parse_address ( item->addr, DB_COLUMN_VALUE ) ) {
            return EXIT_FAILURE;
         }
         c++;
      } else if ( DB_COLUMN_IS ( "resolution" ) ) {
         item->resolution = DB_CVI;
         c++;
      } else {
         printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
         c++;
      }
   }
#define N 4
   if ( c != N ) {
      printde ( "required %d columns but %d found\n", N, c );
      return EXIT_FAILURE;
   }
#undef N
   return EXIT_SUCCESS;
}


static int checkData ( DS18B20Device *item ) {
   int success = 1;
   if ( ! ( item->resolution == 9 || item->resolution == 10 || item->resolution == 11 || item->resolution == 12 ) ) {
      printde ( "bad ds18b20.resolution where id = %d", item->id );
      success = 0;
   }
   if(!checkPin(item->pin)){
     printde ( "bad ds18b20.pin where id = %d", item->id );
      success = 0;
   }
   return success;
}


int getDS18B20ByIdFromDB ( DS18B20Device *item, int id, sqlite3 *db ) {
   char q[LINE_SIZE];
   snprintf ( q, sizeof q, "SELECT * FROM ds18b20 WHERE id=%d LIMIT 1", id );
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

