#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keyspacenote.h"

void keyNotify(const char* key, const char* event) {
    printf("key: %s\n", key);
    printf("event: %s\n", event);
}

void eventNotify(const char* event, const char* key) {
    printf("key: %s\n", key);
    printf("event: %s\n", event);
}

int main(int argc, char **argv) {
    char input[48];
    
    keyspaceNotifier *notifier = NewKeyspaceNotifier();
    
    notifierRegisterKey(notifier, "foo", keyNotify);
    notifierRegisterKey(notifier, "bar", keyNotify);
    notifierRegisterEvent(notifier, "del", eventNotify);

    printf("Listening for events on foo and bar keys\n");
    printf("Listening on del event\n");
    printf("Use Redis-Cli to issue some commands\n");
    printf("Press any key to quite\n");
    scanf ("%s", input);
    printf("quiting\n");

    FreeKeyspaceNotifier(notifier);
    return 0;
}