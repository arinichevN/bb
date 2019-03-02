#include "main.h"

int app_state = APP_INIT;

TSVresult config_tsv = TSVRESULT_INITIALIZER;
char *db_prog_path;
char *db_log_path;
char *peer_id;
__time_t socket_timeout=0;

int sock_port = -1;
int sock_fd = -1;

Peer peer_client = {.fd = &sock_fd, .addr_size = sizeof peer_client.addr};
Mutex channel_list_mutex = MUTEX_INITIALIZER;
Mutex db_mutex = MUTEX_INITIALIZER;

ChannelLList channel_list = LLIST_INITIALIZER;

#include "util.c"
#include "db.c"

int readSettings ( TSVresult* r, const char *data_path, char **peer_id, char **db_prog_path, char **db_log_path, __time_t * socket_timeout) {
    if ( !TSVinit ( r, data_path ) ) {
        return 0;
    }
    char *_peer_id = TSVgetvalues ( r, 0, "peer_id" );
    char *_db_prog_path = TSVgetvalues ( r, 0, "db_prog_path" );
    char *_db_log_path = TSVgetvalues ( r, 0, "db_log_path" );
    int _socket_timeout = TSVgetis ( r, 0, "socket_timeout" );
    if ( TSVnullreturned ( r ) ) {
        return 0;
    }
    *peer_id = _peer_id;
    *db_prog_path = _db_prog_path;
    *db_log_path = _db_log_path;
    *socket_timeout = _socket_timeout;
    return 1;
}

int initApp() {
    if ( !readSettings ( &config_tsv, CONFIG_FILE, &peer_id, &db_prog_path, &db_log_path, &socket_timeout) ) {
        putsde ( "failed to read settings\n" );
        return 0;
    }
    if ( !PQisthreadsafe() ) {
        putsde ( "libpq is not thread-safe\n" );
        return 0;
    }
    if ( !dbp_wait ( db_log_path ) ) {
        putsde ( "failed to ping database\n" );
        return 0;
    }
    if ( !initMutex ( &channel_list_mutex ) ) {
        putsde ( "failed to initialize channel mutex\n" );
        return 0;
    }
    if ( !config_getPort ( &sock_port, peer_id, NULL, db_prog_path ) ) {
        putsde ( "failed to read port\n" );
        return 0;
    }
    if ( !initServer ( &sock_fd, sock_port ) ) {
        putsde ( "failed to initialize udp server\n" );
        return 0;
    }
    return 1;
}

int initData() {
    if ( !loadActiveChannel ( &channel_list,&channel_list_mutex,NULL, db_prog_path ) ) {
        freeChannelList ( &channel_list );
        return 0;
    }
    return 1;
}

void serverRun ( int *state, int init_state ) {
    SERVER_HEADER
    SERVER_APP_ACTIONS
    if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_STOP ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( lockMutex ( &item->mutex ) ) {
                    progStop ( &item->prog );
                    unlockMutex ( &item->mutex );
                }
                deleteChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
            }
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_START ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            addChannelById ( i1l.item[i], &channel_list, &channel_list_mutex, NULL, db_prog_path );
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_RESET ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( lockMutex ( &item->mutex ) ) {
                    progStop ( &item->prog );
                    unlockMutex ( &item->mutex );
                }
                deleteChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
            }
        }
        FORLISTN ( i1l, i ) {
            addChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_ENABLE ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( lockMutex ( &item->mutex ) ) {
                    progEnable ( &item->prog );
                    if ( item->save ) db_saveTableFieldInt ( "channel", "enable", item->id, 1, NULL, db_prog_path );
                    unlockMutex ( &item->mutex );
                }
            }
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_DISABLE ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( lockMutex ( &item->mutex ) ) {
                    progDisable ( &item->prog );
                    if ( item->save ) db_saveTableFieldInt ( "channel", "enable", item->id, 0, NULL, db_prog_path );
                    unlockMutex ( &item->mutex );
                }
            }
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_GET_INFO ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( !bufCatProgInfo ( item, &response ) ) {
                    return;
                }
            }
        }
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_GET_ENABLED ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( !bufCatProgEnabled ( item, &response ) ) {
                    return;
                }
            }
        }
    } else if ( ACP_CMD_IS ( ACP_CMD_BB_CHANNEL_PROG_SAVE_GOAL ) ) {
        SERVER_GET_I1F1LIST_FROM_REQUEST
        FORLISTN ( i1f1l, i ) {
			Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1f1l.item[i].p0 )
            if ( item != NULL ) {
				if(lockMutex(&item->mutex)){
		            if(item->save) db_saveTableFieldFloat ( "prog", "goal", item->prog.id, i1f1l.item[i].p1, NULL, db_prog_path );
		            item->prog.goal = i1f1l.item[i].p1;
		            unlockMutex(&item->mutex);
				}
			}
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_BB_CHANNEL_PROG_SAVE_DELTA ) ) {
        SERVER_GET_I1F1LIST_FROM_REQUEST
        FORLISTN ( i1f1l, i ) {
			Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1f1l.item[i].p0 )
            if ( item != NULL ) {
				if(lockMutex(&item->mutex)){
		           if(item->save) db_saveTableFieldFloat ( "prog", "delta", item->prog.id, i1f1l.item[i].p1, NULL, db_prog_path );
		            item->prog.delta = i1f1l.item[i].p1;
		            unlockMutex(&item->mutex);
				}
			}
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_BB_CHANNEL_CLOSE ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
			Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
				if(lockMutex(&item->mutex)){
					item->prog.duty_cycle = item->prog.close_duty_cycle;
		            if(item->save) db_saveTableFieldFloat ( "prog", "duty_cycle", item->prog.id, item->prog.duty_cycle, NULL, db_prog_path );
		            unlockMutex(&item->mutex);
				}
			}
        }
        return;
    }else if ( ACP_CMD_IS ( ACP_CMD_BB_CHANNEL_OPEN ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
			Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
				if(lockMutex(&item->mutex)){
					item->prog.duty_cycle = item->prog.open_duty_cycle;
		            if(item->save) db_saveTableFieldFloat ( "prog", "duty_cycle", item->prog.id, item->prog.duty_cycle, NULL, db_prog_path );
		            unlockMutex(&item->mutex);
				}
			}
        }
        return;
    }
    acp_responseSend ( &response, &peer_client );
}



void cleanup_handler ( void *arg ) {
    Channel *item = arg;
    printf ( "cleaning up thread %d\n", item->prog.id );
}

void *threadFunction ( void *arg ) {
    Channel *item = arg;
    printdo ( "thread for channel with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
    pthread_cleanup_push ( cleanup_handler, item );
#endif
    while ( 1 ) {
        struct timespec t1 = getCurrentTime();
        int old_state;
        if ( threadCancelDisable ( &old_state ) ) {
            if ( lockMutex ( &item->mutex ) ) {
                progControl ( &item->prog, &item->sensor_temp, &item->sensor_hum,  &item->sensor_fly,  &item->reg,  &item->flyte );
#ifdef MODE_DEBUG
                char *state = getStateStr ( item->prog.state );
                printf ( "channel_id=%d state=%s\n", item->id, state );
#endif
                unlockMutex ( &item->mutex );
            }
            threadSetCancelState ( old_state );
        }
        delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
    pthread_cleanup_pop ( 1 );
#endif
}

void freeData() {
    STOP_ALL_CHANNEL_THREADS ( &channel_list );
    freeChannelList ( &channel_list );
}

void freeApp() {
    freeData();
    freeSocketFd ( &sock_fd );
    freeMutex ( &channel_list_mutex );
    TSVclear ( &config_tsv );
}

void exit_nicely ( ) {
    freeApp();
    putsdo ( "\nexiting now...\n" );
    exit ( EXIT_SUCCESS );
}

int main ( int argc, char** argv ) {
#ifndef MODE_DEBUG
    daemon ( 0, 0 );
#endif
    conSig ( &exit_nicely );
    if ( mlockall ( MCL_CURRENT | MCL_FUTURE ) == -1 ) {
        perrorl ( "mlockall()" );
    }
    int data_initialized = 0;
    while ( 1 ) {
#ifdef MODE_DEBUG
        printf ( "%s(): %s %d\n", F, getAppState ( app_state ), data_initialized );
#endif
        switch ( app_state ) {
        case APP_INIT:
            if ( !initApp() ) {
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
