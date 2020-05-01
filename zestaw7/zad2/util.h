#define PACKAGES_COUNT 10
#define MAX_CREATED_COUNT 10

#define RECEIVERS_COUNT 8
#define PACKERS_COUNT 4
#define SENDERS_COUNT 2

typedef enum { CREATED, PACKED, SENT } package_status;

typedef struct {
    package_status status;
    int value;
} package_t;

typedef struct {
    int index;
    int size;
    package_t packages[PACKAGES_COUNT];
} memory_t;