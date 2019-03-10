#include "main.h"

int app_state = APP_INIT;

TSVresult config_tsv = TSVRESULT_INITIALIZER;
char *db_prog_path;
char *db_log_path;

PGconn *db_conn_log = NULL;
int sock_port = -1;
int sock_fd = -1;

Peer peer_client = {.fd = &sock_fd, .addr_size = sizeof peer_client.addr};
Mutex channel_list_mutex = MUTEX_INITIALIZER;
Mutex db_mutex = MUTEX_INITIALIZER;

Rack rack;

Thread threads[] =
{
  {.id = "slow", .cycle_duration = {.tv_sec=0, .tv_nsec=0}},
  {.id = "sound", .cycle_duration = {.tv_sec=0, .tv_nsec=0}},
  {.id = "cooler", .cycle_duration = {.tv_sec=0, .tv_nsec=0}},
  {.id = "flyte", .cycle_duration = {.tv_sec=0, .tv_nsec=0}},
  {.id = "fly", .cycle_duration = {.tv_sec=0, .tv_nsec=0}}
};

#include "util.c"
#include "db/src/common.h"

int readSettings ( TSVresult* r, const char *data_path, int *port_id, int *rack_id, char **db_prog_path, char **db_log_path )
{
  if ( !TSVinit ( r, data_path ) )
    {
      return 0;
    }
  char *_id = TSVgetvalues ( r, 0, "rack_id" );
  char *_db_prog_path = TSVgetvalues ( r, 0, "db_prog_path" );
  char *_db_log_path = TSVgetvalues ( r, 0, "db_log_path" );
  if ( TSVnullreturned ( r ) )
    {
      return 0;
    }
  *id = _id;
  *db_prog_path = _db_prog_path;
  *db_log_path = _db_log_path;
  return 1;
}

int initApp()
{
  if ( !readSettings ( &config_tsv, CONFIG_FILE, &peer_id, &db_prog_path, &db_log_path ) )
    {
      putsde ( "failed to read settings\n" );
      return 0;
    }
  if ( !PQisthreadsafe() )
    {
      putsde ( "libpq is not thread-safe\n" );
      return 0;
    }
  if ( !dbp_wait ( db_log_path ) )
    {
      putsde ( "failed to ping database\n" );
      return 0;
    }
  if ( !initMutex ( &channel_list_mutex ) )
    {
      putsde ( "failed to initialize channel mutex\n" );
      return 0;
    }
  if ( !getPort ( &sock_port, rack.id, NULL, db_prog_path ) )
    {
      putsde ( "failed to read port\n" );
      return 0;
    }
  if ( !initServer ( &sock_fd, sock_port ) )
    {
      putsde ( "failed to initialize udp server\n" );
      return 0;
    }
  return 1;
}

int initData()
{
  if ( !initRack ( &rack, db_prog_path ) )
    {
      freeRack ( &rack );
      return 0;
    }
    size_t n = sizeof ( threads ) /sizeof ( threads[0] );
    if(!initThreads(threads, n))
  
  for ( size_t i=0; i < n; i++ )
    {
if ( !createMThread ( &threads[i].thread, &coolerThreadFunction, NULL ) )
    {

    }
    }
  
  if ( !createMThread ( &flyte_thread.thread, &flyteThreadFunction, NULL ) )
    {
      STOP_THREAD ( cooler_thread.thread );
    }
  if ( !createMThread ( &fly_thread.thread, &flyThreadFunction, NULL ) )
    {

    }
  if ( !createMThread ( &sound_thread.thread, &soundThreadFunction, NULL ) )
    {

    }
  if ( !createMThread ( &slow_thread.thread, &slowThreadFunction, NULL ) )
    {

    }
  return 1;
}

void serverRun ( int *state, int init_state )
{
  SERVER_HEADER
  SERVER_APP_ACTIONS
  if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_STOP ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
        if ( item != NULL )
          {
            if ( lockMutex ( &item->mutex ) )
              {
                progStop ( &item->prog );
                unlockMutex ( &item->mutex );
              }
            deleteChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
          }
      }
      return;
    }
  else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_START ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        addChannelById ( i1l.item[i], &channel_list, &channel_list_mutex, NULL, db_prog_path );
      }
      return;
    }
  else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_RESET ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
        if ( item != NULL )
          {
            if ( lockMutex ( &item->mutex ) )
              {
                progStop ( &item->prog );
                unlockMutex ( &item->mutex );
              }
            deleteChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
          }
      }
      FORLISTN ( i1l, i )
      {
        addChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
      }
      return;
    }
  else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_ENABLE ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
        if ( item != NULL )
          {
            if ( lockMutex ( &item->mutex ) )
              {
                progEnable ( &item->prog );
                if ( item->save ) db_saveTableFieldInt ( "channel", "enable", item->id, 1, NULL, db_prog_path );
                unlockMutex ( &item->mutex );
              }
          }
      }
      return;
    }
  else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_DISABLE ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
        if ( item != NULL )
          {
            if ( lockMutex ( &item->mutex ) )
              {
                progDisable ( &item->prog );
                if ( item->save ) db_saveTableFieldInt ( "channel", "enable", item->id, 0, NULL, db_prog_path );
                unlockMutex ( &item->mutex );
              }
          }
      }
      return;
    }
  else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_GET_INFO ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
        if ( item != NULL )
          {
            if ( !bufCatProgInfo ( item, &response ) )
              {
                return;
              }
          }
      }
    }
  else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_GET_ENABLED ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
        if ( item != NULL )
          {
            if ( !bufCatProgEnabled ( item, &response ) )
              {
                return;
              }
          }
      }
    }
  else if ( ACP_CMD_IS ( ACP_CMD_BB_CHANNEL_PROG_SAVE_GOAL ) )
    {
      SERVER_GET_I1F1LIST_FROM_REQUEST
      FORLISTN ( i1f1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1f1l.item[i].p0 )
        if ( item != NULL )
          {
            if ( lockMutex ( &item->mutex ) )
              {
                if ( item->save ) db_saveTableFieldFloat ( "prog", "goal", item->prog.id, i1f1l.item[i].p1, NULL, db_prog_path );
                item->prog.goal = i1f1l.item[i].p1;
                unlockMutex ( &item->mutex );
              }
          }
      }
      return;
    }
  else if ( ACP_CMD_IS ( ACP_CMD_BB_CHANNEL_PROG_SAVE_DELTA ) )
    {
      SERVER_GET_I1F1LIST_FROM_REQUEST
      FORLISTN ( i1f1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1f1l.item[i].p0 )
        if ( item != NULL )
          {
            if ( lockMutex ( &item->mutex ) )
              {
                if ( item->save ) db_saveTableFieldFloat ( "prog", "delta", item->prog.id, i1f1l.item[i].p1, NULL, db_prog_path );
                item->prog.delta = i1f1l.item[i].p1;
                unlockMutex ( &item->mutex );
              }
          }
      }
      return;
    }
  else if ( ACP_CMD_IS ( ACP_CMD_BB_CHANNEL_CLOSE ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
        if ( item != NULL )
          {
            if ( lockMutex ( &item->mutex ) )
              {
                item->prog.duty_cycle = item->prog.close_duty_cycle;
                if ( item->save ) db_saveTableFieldFloat ( "prog", "duty_cycle", item->prog.id, item->prog.duty_cycle, NULL, db_prog_path );
                unlockMutex ( &item->mutex );
              }
          }
      }
      return;
    }
  else if ( ACP_CMD_IS ( ACP_CMD_BB_CHANNEL_OPEN ) )
    {
      SERVER_GET_I1LIST_FROM_REQUEST
      FORLISTN ( i1l, i )
      {
        Channel *item;
        LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
        if ( item != NULL )
          {
            if ( lockMutex ( &item->mutex ) )
              {
                item->prog.duty_cycle = item->prog.open_duty_cycle;
                if ( item->save ) db_saveTableFieldFloat ( "prog", "duty_cycle", item->prog.id, item->prog.duty_cycle, NULL, db_prog_path );
                unlockMutex ( &item->mutex );
              }
          }
      }
      return;
    }
  acp_responseSend ( &response, &peer_client );
}


void cleanup_handler ( void *arg )
{
  Thread *item = arg;
  printf ( "cleaning up thread %d\n", item->id );
}

void *slowThreadFunction ( void *arg )
{
  Tread *item = arg;
  printdo ( "thread for channel with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
  pthread_cleanup_push ( cleanup_handler, item );
#endif
  while ( 1 )
    {
      struct timespec t1 = getCurrentTime();
      int old_state;
      if ( threadCancelDisable ( &old_state ) )
        {
          FORLISTN ( rack.hive_list, i )
          {
            Hive *hive = &rack.hive_list.item[i];
            flyCounter ( &hive->fly_counter );
            presenceControl ( &hive->presence, rack.id, hive->id, &rack.buzzer, db_conn_log );
            saveFly ( &hive->fly_logger, rack.id, hive->id, &hive->fly_counter, db_conn_log );
            saveTemp ( &hive->temp_logger, rack.id, hive->id, &rack.cooler, db_conn_log );
          }
          doorControl ( &rack.door, &rack.flyte );
          tempControl ( &rack.cooler );
          threadSetCancelState ( old_state );
        }
      delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
  pthread_cleanup_pop ( 1 );
#endif
}

void *soundThreadFunction ( void *arg )
{
  Tread *item = arg;
  printdo ( "thread for channel with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
  pthread_cleanup_push ( cleanup_handler, item );
#endif
  while ( 1 )
    {
      int old_state;
      if ( threadCancelDisable ( &old_state ) )
        {
          soundControl ( &rack.buzzer );
          threadSetCancelState ( old_state );
        }
    }
#ifdef MODE_DEBUG
  pthread_cleanup_pop ( 1 );
#endif
}

void *flyteThreadFunction ( void *arg )
{
  Tread *item = arg;
  printdo ( "thread for channel with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
  pthread_cleanup_push ( cleanup_handler, item );
#endif
  while ( 1 )
    {
      struct timespec t1 = getCurrentTime();
      int old_state;
      if ( threadCancelDisable ( &old_state ) )
        {
          pwmControl ( &rack.flyte.device );
          threadSetCancelState ( old_state );
        }
      delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
  pthread_cleanup_pop ( 1 );
#endif
}

void *coolerThreadFunction ( void *arg )
{
  Tread *item = arg;
  printdo ( "thread for channel with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
  pthread_cleanup_push ( cleanup_handler, item );
#endif
  while ( 1 )
    {
      struct timespec t1 = getCurrentTime();
      int old_state;
      if ( threadCancelDisable ( &old_state ) )
        {
          pwmControl ( &rack.cooler.em );
          threadSetCancelState ( old_state );
        }
      delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
  pthread_cleanup_pop ( 1 );
#endif
}

void *flyThreadFunction ( void *arg )
{
  Tread *item = arg;
  printdo ( "thread for channel with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
  pthread_cleanup_push ( cleanup_handler, item );
#endif
  while ( 1 )
    {
      struct timespec t1 = getCurrentTime();
      int old_state;
      if ( threadCancelDisable ( &old_state ) )
        {
          FORLISTN ( rack.hive_list, i )
          {
            Hive *hive = &rack.hive_list.item[i];
            flyCounter ( &hive->fly_counter );
          }
          threadSetCancelState ( old_state );
        }
      delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
  pthread_cleanup_pop ( 1 );
#endif
}

void freeData()
{
  STOP_ALL_CHANNEL_THREADS ( &channel_list );
  freeChannelList ( &channel_list );
}

void freeApp()
{
  freeData();
  freeSocketFd ( &sock_fd );
  freeMutex ( &channel_list_mutex );
  TSVclear ( &config_tsv );
}

void exit_nicely ( )
{
  freeApp();
  putsdo ( "\nexiting now...\n" );
  exit ( EXIT_SUCCESS );
}

int main ( int argc, char** argv )
{
#ifndef MODE_DEBUG
  daemon ( 0, 0 );
#endif
  conSig ( &exit_nicely );
  if ( mlockall ( MCL_CURRENT | MCL_FUTURE ) == -1 )
    {
      perrorl ( "mlockall()" );
    }
  int data_initialized = 0;
  while ( 1 )
    {
#ifdef MODE_DEBUG
      printf ( "%s(): %s %d\n", F, getAppState ( app_state ), data_initialized );
#endif
      switch ( app_state )
        {
        case APP_INIT:
          if ( !initApp() )
            {
              return ( EXIT_FAILURE );
            }
          app_state = APP_INIT_DATA;
          break;
        case APP_INIT_DATA:
          data_initialized = initData();
          app_state = APP_RUN;
          delayUsIdle ( 1000000 );
          break;
        case APP_RUN:
          serverRun ( &app_state, data_initialized );
          break;
        case APP_STOP:
          freeData();
          data_initialized = 0;
          app_state = APP_RUN;
          break;
        case APP_RESET:
          freeApp();
          delayUsIdle ( 1000000 );
          data_initialized = 0;
          app_state = APP_INIT;
          break;
        case APP_EXIT:
          exit_nicely();
          break;
        default:
          freeApp();
          putsde ( "unknown application state\n" );
          return ( EXIT_FAILURE );
        }
    }
  freeApp();
  putsde ( "unexpected while break\n" );
  return ( EXIT_FAILURE );
}
