#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../keyspacenote.h"


int KeyCounter = 0;
int EventCounter = 0;

void keyNotify(const char* key, const char* event) {
    KeyCounter++;
}

void eventNotify(const char* event, const char* key) {
    EventCounter++;
}

int main(int argc, char **argv) {

    keyspaceNotifier *notifier = NewKeyspaceNotifier("127.0.0.1", 6379);
    
    // Register to for events.
    printf("Registering for events notification\n");
    notifierRegisterKey(notifier, "foo", keyNotify);
    notifierRegisterKey(notifier, "bar", keyNotify);
    notifierRegisterEvent(notifier, "del", eventNotify);
    
    // Issue commands
    printf("Sending commands to Redis\n");
    notifierIssueRedisCommand(notifier, "SET foo 1");
    notifierIssueRedisCommand(notifier, "SET bar 1");
    notifierIssueRedisCommand(notifier, "DEL foo");

    // Wait some time.
    sleep(4);

    if(KeyCounter != 3 && EventCounter != 1) {
        printf("FAIL!\n");
        printf("Expecting KeyCounter to be 3 got %d\n", KeyCounter);
        printf("Expecting EventCounter to be 1 got %d\n", EventCounter);
    }

    // Unregister from events
    printf("Un-Registering for events notification\n");
    notifierDeregisterKey(notifier, "foo");
    notifierDeregisterEvent(notifier, "del");

    // Wait some time.
    sleep(4);

    // Issue commands
    printf("Sending commands to Redis\n");
    notifierIssueRedisCommand(notifier, "SET foo 1");
    notifierIssueRedisCommand(notifier, "SET bar 1");
    notifierIssueRedisCommand(notifier, "DEL foo");

    // Wait some time.
    sleep(4);

    if(KeyCounter == 4 && EventCounter == 1) {
        printf("PASS!\n");
    } else {        
        printf("FAIL!\n");
        printf("Expecting KeyCounter to be 4 got %d\n", KeyCounter);
        printf("Expecting EventCounter to be 1 got %d\n", EventCounter);
        return 1;
    }

cleanup:
    FreeKeyspaceNotifier(notifier);

    return 0;
}