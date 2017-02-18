# rmnotify

In Redis it's possible to subscribe to Pub/Sub channels in order to receive events affecting the Redis data set in some way, see [docs](https://redis.io/topics/notifications).

For example, you might be interested in knowing about all operations touching a certain key (key space notification), or you might want to know if all keys have been deleted (event space notification).

Unfortunately, the Redis modules [API](https://github.com/antirez/redis/blob/unstable/src/modules/API.md) has yet to support module registration for either key/event space notifications. Until native support is implemented you can use this library to register for both key and event space notifications.

# Usage
To receive notifications for a specific key ("foo") call

```sh
void fnKeyNotify(const char* key, const char* event) {...}
notifierRegisterKey(notifier, "foo", fnKeyNotify);
```

And for event ("del") notifications:

```sh
void fnEventNotify(const char* event, const char* key) {...}
notifierRegisterEvent(notifier, "del", fnEventNotify);
```

See either [demo.c](https://github.com/RedisLabs/rmnotify/src/demo.c) or [test.c](https://github.com/RedisLabs/rmnotify/src/test/test_notifications.c) for additional examples.

# Build
Build dependencies, from rmnotify_root/deps/hiredis run:
```sh
make
```
Build rmnotify, from rmnotify_root/src run:
```sh
make all
```
Run tests (make sure a local Redis server is running), from rmnotify_root/src/test run:
```sh
make test && ./test
```
