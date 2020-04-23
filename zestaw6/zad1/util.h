#define TEXT_LEN 128
#define SERVER_KEY_ID 1
#define MAX_CLIENTS 10
#define STOP_SERVER 1L
#define STOP 2L
#define DISCONNECT 3L
#define LIST 4L
#define INIT 5L
#define CONNECT 6L
#define SEND 7L
#define TYPES_COUNT 7L

typedef struct {
    long type;
    char text[TEXT_LEN];
} message;

typedef struct {
    int id;
    int queue_id;
    int connected_client_id;
} client;