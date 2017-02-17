#ifndef __KEYSPACENOTE_H
#define __KEYSPACENOTE_H

#include "../deps/hiredis/hiredis.h"
#include "../deps/hiredis/async.h"

#define MAX_CALLBACKS 255

/* Callback function used to register for key notifications */
typedef void (*keyNotifyCallback)(const char* key, const char* event);

typedef struct channelCallback {
    char *channel;          /* redis subscriber channel name */
    keyNotifyCallback cb;   /* function to call when there's data on channel */
} channelCallback;

typedef struct keyspaceNotifier {
    redisContext *c;                            /* Connection to redis */
    redisAsyncContext *async;                   /* Async connection to redis */
    struct event_base *base;                    /* Libevent base */
    pthread_t thread_id;                        /* Thread executing event loop */
    int callbacksIdx;							/* Index into a free position within callbacks*/
    // TODO: replace this array with KHash.
    channelCallback callbacks[MAX_CALLBACKS];   /* List of registered key/event callbacks */
} keyspaceNotifier;


keyspaceNotifier* NewKeyspaceNotifier();

/* Register to receive notifications for key */
int notifierRegisterKey(keyspaceNotifier *notifier, const char* key, keyNotifyCallback fn);

/* Register to receive notifications for event */
int notifierRegisterEvent(keyspaceNotifier *notifier, const char* event, keyNotifyCallback fn);

/* Stop receiving notifications for key */
int notifierDeregisterKey(keyspaceNotifier *notifier, const char *key);

/* Stop receiving notifications for event */
int notifierDeregisterEvent(keyspaceNotifier *notifier, const char *event);

/* Executes given command */
void notifierIssueRedisCommand(const keyspaceNotifier *notifier, const char *command);

/* Releases all memory allocated by notifier, de-register from all channels*/
void FreeKeyspaceNotifier(keyspaceNotifier *notifier);

#endif