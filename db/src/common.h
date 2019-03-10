
#ifndef BB_DB_COMMON_H
#define BB_DB_COMMON_H

#include "../../main.h"

extern int getBuzzerByIdFromDB ( Buzzer *item, int id, sqlite3 *dbl, const char *db_path );
extern int getFlyCounterByIdFromDB ( FlyCounter *item, int id, sqlite3 *dbl, const char *db_path );
extern int getLoggerByIdFromDB ( Logger *item, int id, sqlite3 *dbl, const char *db_path );
extern int getPresenceByIdFromDB ( Presence * item, int id, sqlite3 * dbl, const char * db_path );
extern int getPWMByIdFromDB ( PWM *item, int id, sqlite3 *db );
extern int getFlyteByIdFromDB ( Flyte *item, int id, sqlite3 *dbl, const char *db_path );
extern int getCoolerByIdFromDB ( Cooler *item, int id, sqlite3 *dbl, const char *db_path );
extern int getDS18B20ByIdFromDB ( DS18B20Device *item, int id, sqlite3 *db );
extern int getPIDByIdFromDB ( PID *item, int id, sqlite3 *db );
extern int getRackHiveListByIdFromDB ( HiveList *item, int rack_id, sqlite3 *db );
extern int getRackByIdFromDB ( Rack *item, int id, sqlite3 *dbl, const char *db_path );

#endif




