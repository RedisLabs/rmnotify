#include "keyspacenote.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "../deps/hiredis/adapters/libevent.h"

#ifdef _EVENT_HAVE_PTHREADS
int evthread_use_pthreads(void);
#define EVTHREAD_USE_PTHREADS_IMPLEMENTED
#endif

void _connectSync(keyspaceNotifier *notifier) {
    const char *hostname = "127.0.0.1";
    int port = 6379;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    
    // Open connection to local redis.
    notifier->c = redisConnectWithTimeout(hostname, port, timeout);
    if (notifier->c == NULL || notifier->c->err) {
        if (notifier->c) {
            printf("Connection error: %s\n", notifier->c->errstr);
            redisFree(notifier->c);
            notifier->c = NULL;
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
    }
}

void _connectAsync(keyspaceNotifier *notifier) {
    const char *hostname = "127.0.0.1";
    int port = 6379;
    
    // Open connection to local redis.
    notifier->async = redisAsyncConnect(hostname, port);
    if (notifier->async->err) {
        printf("Connection error: %s\n", notifier->async->errstr);
    }
}

int _enableNotifications(const keyspaceNotifier *notifier) {
    /* Enables all types of events */
    /* KEA - enable every possible event */ 
    redisReply *reply = redisCommand(notifier->c, "CONFIG SET notify-keyspace-events KEA");
    
    // TODO: validate reply
    return 0;
}

void _handleRedisArrayReply(redisReply *r, const keyspaceNotifier *n) {
    if(r->elements == 3) {
        if(strcmp(r->element[0]->str, "message") == 0) {
            char* channel = r->element[1]->str;
            char* action = r->element[2]->str;

            // See if someone registered on channel.
            for(int i = 0; i < n->callbacksIdx; i++) {
                channelCallback callback = n->callbacks[i];
                if(strcmp(callback.channel, channel) == 0) {
                    strtok(channel, ":");
                    channel = strtok(NULL, ":");
                    callback.cb(channel, action);
                    break;
                }
            }
        }
    }
}

void onMessage(redisAsyncContext *c, void *reply, void *privdata) {
    if (reply == NULL) return;

    redisReply *r = reply;
    keyspaceNotifier *n = (keyspaceNotifier*)privdata;
    
    if(r->type == REDIS_REPLY_ARRAY) {
        _handleRedisArrayReply(r, n);
    }
}

int _register(keyspaceNotifier *notifier, const char* channel, keyNotifyCallback fn) {
    // Make sure we've got a free slot for registration.
    if(notifier->callbacksIdx >= MAX_CALLBACKS) {
        printf("Sorry registration limit %d has been reached.\n", MAX_CALLBACKS);
        return -1;
    }
    
    int res = redisAsyncCommand(notifier->async, onMessage, notifier, "SUBSCRIBE %s", channel);
    if(res == REDIS_ERR) {
        printf("Failed to subscribe on %s\n", channel);
        return res;
    }
    
    notifier->callbacks[notifier->callbacksIdx].channel = strdup(channel);
    notifier->callbacks[notifier->callbacksIdx].cb = fn;
    notifier->callbacksIdx++;
    
    return 0;
}

int notifierRegisterKey(keyspaceNotifier *notifier, const char *key, keyNotifyCallback fn) {
    char buf[256];
    snprintf(buf, 256, "__keyspace@0__:%s", key);
    
    return _register(notifier, buf, fn);
}

int notifierRegisterEvent(keyspaceNotifier *notifier, const char *event, keyNotifyCallback fn) {
    char buf[256];
    snprintf(buf, 256, "__keyevent@0__:%s", event);
    
    return _register(notifier, buf, fn);
}

int _deregister(keyspaceNotifier *n, const char *channel) {
    int res = redisAsyncCommand(n->async, onMessage, n, "UNSUBSCRIBE %s", channel);
    if(res == REDIS_ERR) {
        printf("Failed to unsubscribe from: %s\n", channel);
    }

    return res;
}

int notifierDeregisterKey(keyspaceNotifier *notifier, const char *key) {
    char buf[256];
    snprintf(buf, 256, "__keyspace@0__:%s", key);
    return _deregister(notifier, buf);
}

int notifierDeregisterEvent(keyspaceNotifier *notifier, const char *event) {
    char buf[256];
    snprintf(buf, 256, "__keyevent@0__:%s", event);
    return _deregister(notifier, buf);
}

static void* thread_start(void *t) {
    keyspaceNotifier* notifier = (keyspaceNotifier*) t;
    event_base_dispatch(notifier->base);
    printf("out of loop\n");
    return t;
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("DISCONNECTED\n");
}

void notifierIssueRedisCommand(const keyspaceNotifier *notifier,const char *command) {
    redisCommand(notifier->c, command);
}

keyspaceNotifier* NewKeyspaceNotifier() {
    signal(SIGPIPE, SIG_IGN);

    // http://www.wangafu.net/~nickm/libevent-book/Ref1_libsetup.html
    // http://stackoverflow.com/questions/9153528/libevent-multithread-support
    evthread_use_pthreads();

    keyspaceNotifier* notifier = malloc(sizeof(keyspaceNotifier));
    notifier->base = event_base_new();
    notifier->callbacksIdx = 0;

    _connectSync(notifier);
    _connectAsync(notifier);
    _enableNotifications(notifier);
    redisAsyncSetDisconnectCallback(notifier->async, disconnectCallback);
    redisLibeventAttach(notifier->async, notifier->base);

    int err = pthread_create(&notifier->thread_id, NULL, thread_start, (void *)notifier);
    if(err != 0) {
        // TODO: clean up.
        printf("Failed to create thread\n");
    }

    return notifier;
}

void FreeKeyspaceNotifier(keyspaceNotifier *notifier) {
    if(notifier != NULL) {
        redisFree(notifier->c);
        
        for(int i = 0; i < notifier->callbacksIdx; i++) {
            if(notifier->callbacks[i].channel != NULL) {
                _deregister(notifier, notifier->callbacks[i].channel);
                free(notifier->callbacks[i].channel);
            }
        }

        free(notifier);
    }
}