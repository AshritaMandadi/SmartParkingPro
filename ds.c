/* smart_parking.c
   Self-contained Smart Parking System (single-file)
   - min-heap for free slot allocation
   - circular waiting queue
   - parking history (linked list)
   - safer input (fgets + sscanf)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SLOTS 10
#define MAX_CARS 100
#define WAIT_CAP 10
#define FEE_PER_HOUR 50

/* ----- Heap (min-heap of free slots) ----- */
int heapArr[MAX_SLOTS + 1]; /* 1-based heap */
int heapSize = 0;

void heapSwap(int i, int j) {
    int t = heapArr[i];
    heapArr[i] = heapArr[j];
    heapArr[j] = t;
}

void heapInsert(int val) {
    if (heapSize >= MAX_SLOTS) return;
    heapSize++;
    heapArr[heapSize] = val;
    int i = heapSize;
    while (i > 1) {
        int parent = i / 2;
        if (heapArr[parent] <= heapArr[i]) break;
        heapSwap(parent, i);
        i = parent;
    }
}

int heapRemoveMin() {
    if (heapSize == 0) return -1;
    int ret = heapArr[1];
    heapArr[1] = heapArr[heapSize];
    heapSize--;
    int i = 1;
    while (1) {
        int l = 2 * i, r = l + 1, smallest = i;
        if (l <= heapSize && heapArr[l] < heapArr[smallest]) smallest = l;
        if (r <= heapSize && heapArr[r] < heapArr[smallest]) smallest = r;
        if (smallest == i) break;
        heapSwap(i, smallest);
        i = smallest;
    }
    return ret;
}

/* ----- Waiting queue (circular) ----- */
int waitQ[WAIT_CAP];
int waitFront = 0, waitRear = -1, waitCount = 0;

int enqueueWait(int car) {
    if (waitCount == WAIT_CAP) return 0;
    waitRear = (waitRear + 1) % WAIT_CAP;
    waitQ[waitRear] = car;
    if (waitCount == 0) waitFront = waitRear;
    waitCount++;
    return 1;
}

int dequeueWait() {
    if (waitCount == 0) return -1;
    int c = waitQ[waitFront];
    waitFront = (waitFront + 1) % WAIT_CAP;
    waitCount--;
    if (waitCount == 0) { waitFront = 0; waitRear = -1; }
    return c;
}

/* ----- Parking data structures ----- */
int slotOfCar[MAX_CARS];        /* car -> slot (1..MAX_SLOTS), -1 not present, -2 waiting */
time_t entryTimeOfCar[MAX_CARS];
int passUser[MAX_CARS];         /* 1 if monthly pass */

int slotToCar[MAX_SLOTS + 1];   /* slot -> car, -1 if empty */

typedef struct Node {
    int car;
    int slot;
    time_t entryTime;
    time_t exitTime; /* 0 if still parked */
    struct Node *next;
} Node;

Node *history = NULL;
int totalRevenue = 0;

/* ----- Utilities ----- */
void addHistoryNode(int car, int slot, time_t entry, time_t exitT) {
    Node *n = malloc(sizeof(Node));
    if (!n) return;
    n->car = car; n->slot = slot; n->entryTime = entry; n->exitTime = exitT;
    n->next = history;
    history = n;
}

void format_time(time_t t, char *buf, size_t bufsz) {
    struct tm tmst;
    localtime_r(&t, &tmst);
    strftime(buf, bufsz, "%Y-%m-%d %H:%M:%S", &tmst);
}

/* safer input helpers */
int read_int(const char *prompt, int *out) {
    char line[128];
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }
    if (!fgets(line, sizeof(line), stdin)) return 0;
    char *nl = strchr(line, '\n'); if (nl) *nl = '\0';
    int tmp;
    if (sscanf(line, "%d", &tmp) != 1) return 0;
    *out = tmp;
    return 1;
}

int read_char(const char *prompt, char *out) {
    char line[32];
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }
    if (!fgets(line, sizeof(line), stdin)) return 0;
    *out = line[0];
    return 1;
}

/* ----- System initialization & functions ----- */
void initSystem() {
    heapSize = 0;
    for (int i = 1; i <= MAX_SLOTS; i++) heapInsert(i);
    for (int i = 0; i < MAX_CARS; i++) {
        slotOfCar[i] = -1;
        entryTimeOfCar[i] = 0;
        passUser[i] = 0;
    }
    for (int i = 0; i <= MAX_SLOTS; i++) slotToCar[i] = -1;
    waitFront = 0; waitRear = -1; waitCount = 0;
    totalRevenue = 0;
    Node *t;
    while (history) { t = history; history = history->next; free(t); }
}

void showSlotMap() {
    printf("\n Slot Map \n");
    for (int s = 1; s <= MAX_SLOTS; s++) {
        if (slotToCar[s] == -1) printf("Slot %d: [Empty]\n", s);
        else printf("Slot %d: [Car %d]\n", s, slotToCar[s]);
    }
}

void searchCar(int car) {
    if (car < 0 || car >= MAX_CARS) { printf("Invalid car id.\n"); return; }
    if (slotOfCar[car] >= 1) {
        char buf[32];
        format_time(entryTimeOfCar[car], buf, sizeof(buf));
        printf("Car %d parked at Slot %d (entry %s)\n", car, slotOfCar[car], buf);
    } else if (slotOfCar[car] == -2) {
        printf("Car %d is in the waiting queue.\n", car);
    } else {
        printf("Car %d not found.\n", car);
    }
}

void showParkedVehicles() {
    printf("\nParked Cars \n");
    int any = 0;
    for (int s = 1; s <= MAX_SLOTS; s++) {
        int c = slotToCar[s];
        if (c != -1) {
            char buf[32];
            format_time(entryTimeOfCar[c], buf, sizeof(buf));
            printf("Slot %d: Car %d (entry %s)\n", s, c, buf);
            any = 1;
        }
    }
    if (!any) printf("None\n");
}

void showWaitingQueue() {
    printf("\nWaiting Queue (%d/%d) \n", waitCount, WAIT_CAP);
    if (waitCount == 0) { printf("Empty\n"); return; }
    int idx = waitFront;
    for (int i = 0; i < waitCount; i++) {
        printf("%d. Car %d\n", i+1, waitQ[idx]);
        idx = (idx + 1) % WAIT_CAP;
    }
}

void showRevenue() {
    printf("\nTotal Revenue: Rs %d\n", totalRevenue);
}

void emergencyMode() {
    printf("\n!!! EMERGENCY MODE ACTIVE !!!\nSystem cleared. History retained.\n");
    for (int c = 0; c < MAX_CARS; c++) {
        slotOfCar[c] = -1;
        entryTimeOfCar[c] = 0;
    }
    for (int s = 1; s <= MAX_SLOTS; s++) slotToCar[s] = -1;
    heapSize = 0;
    for (int i = 1; i <= MAX_SLOTS; i++) heapInsert(i);
    waitFront = 0; waitRear = -1; waitCount = 0;
    /* keep totalRevenue and history as-is */
}

void addMonthlyPass(int car) {
    if (car < 0 || car >= MAX_CARS) { printf("Invalid.\n"); return; }
    passUser[car] = 1;
    printf("Car %d registered as Monthly Pass.\n", car);
}

int canEnter(int car) {
    if (car < 0 || car >= MAX_CARS) { printf("Invalid.\n"); return 0; }
    if (slotOfCar[car] >= 1) { printf("Duplicate: Car %d already parked.\n", car); return 0; }
    if (slotOfCar[car] == -2) { printf("Duplicate: Car %d already in waiting.\n", car); return 0; }
    return 1;
}

void vehicleEntry() {
    int car;
    if (!read_int("Enter car id (0..99): ", &car)) { printf("Invalid input.\n"); return; }
    if (!canEnter(car)) return;
    int slot = heapRemoveMin();
    if (slot == -1) {
        if (waitCount == WAIT_CAP) { printf("Parking & Waiting FULL!\n"); return; }
        if (!enqueueWait(car)) { printf("Unable to join waiting.\n"); return; }
        slotOfCar[car] = -2;
        printf("Parking full: Car %d added to waiting at position %d.\n", car, waitCount);
        return;
    }
    time_t now = time(NULL);
    slotOfCar[car] = slot;
    entryTimeOfCar[car] = now;
    slotToCar[slot] = car;
    char buf[32];
    format_time(now, buf, sizeof(buf));
    printf("Car %d parked at Slot %d (Entry: %s)\n", car, slot, buf);
    addHistoryNode(car, slot, now, 0);
}

void vehicleExit() {
    int car;
    if (!read_int("Enter car id to exit: ", &car)) { printf("Invalid input.\n"); return; }
    if (car < 0 || car >= MAX_CARS) { printf("Invalid car id.\n"); return; }
    if (slotOfCar[car] == -1) {
        printf("Car %d not parked.\n", car);
        return;
    }
    if (slotOfCar[car] == -2) {
        /* remove from waiting queue by rebuilding queue */
        int tmp[WAIT_CAP];
        int idx = 0;
        int removed = 0;
        int origCount = waitCount;
        for (int i = 0; i < origCount; i++) {
            int w = dequeueWait();
            if (w == car) { removed = 1; slotOfCar[w] = -1; }
            else tmp[idx++] = w;
        }
        for (int i = 0; i < idx; i++) enqueueWait(tmp[i]);
        if (removed) printf("Car %d removed from waiting queue.\n", car);
        else printf("Car %d not found in waiting queue.\n", car);
        return;
    }
    int slot = slotOfCar[car];
    time_t now = time(NULL);
    time_t entry = entryTimeOfCar[car];
    double diff = difftime(now, entry);
    if (diff < 0) diff = 0;
    long secs = (long) diff;
    int hours = secs / 3600;
    int mins = (secs % 3600) / 60;
    int secs_rem = secs % 60;
    int charged_hours = (secs + 3599) / 3600; /* ceil to next hour */
    if (charged_hours < 0) charged_hours = 0;
    int fee = passUser[car] ? 0 : charged_hours * FEE_PER_HOUR;
    char bufEntry[32], bufExit[32];
    format_time(entry, bufEntry, sizeof(bufEntry));
    format_time(now, bufExit, sizeof(bufExit));
    printf("Car %d exited from Slot %d\n", car, slot);
    printf("Entry : %s\n", bufEntry);
    printf("Exit  : %s\n", bufExit);
    printf("Duration: %d hr %d min %d sec\n", hours, mins, secs_rem);
    printf("Fee: Rs %d\n", fee);
    totalRevenue += fee;
    /* update history node */
    Node *t = history;
    while (t) {
        if (t->car == car && t->slot == slot && t->exitTime == 0) { t->exitTime = now; break; }
        t = t->next;
    }
    /* free slot */
    slotOfCar[car] = -1;
    entryTimeOfCar[car] = 0;
    slotToCar[slot] = -1;
    heapInsert(slot);
    /* allocate to next waiting car immediately (if any) */
    if (waitCount > 0) {
        int next = dequeueWait();
        if (next >= 0 && next < MAX_CARS) {
            int newSlot = heapRemoveMin();
            if (newSlot == -1) { enqueueWait(next); }
            else {
                time_t now2 = time(NULL);
                slotOfCar[next] = newSlot;
                entryTimeOfCar[next] = now2;
                slotToCar[newSlot] = next;
                addHistoryNode(next, newSlot, now2, 0);
                char buf2[32];
                format_time(now2, buf2, sizeof(buf2));
                printf("Allocated Slot %d to waiting Car %d (Entry: %s)\n", newSlot, next, buf2);
            }
        }
    }
}

void showHistory() {
    printf("\nParking History (most recent first)\n");
    if (!history) { printf("None\n"); return; }
    Node *t = history;
    while (t) {
        char be[32], bx[32];
        format_time(t->entryTime, be, sizeof(be));
        if (t->exitTime == 0) {
            printf("Car %d -> Slot %d | %s -> STILL PARKED\n", t->car, t->slot, be);
        } else {
            format_time(t->exitTime, bx, sizeof(bx));
            printf("Car %d -> Slot %d | %s -> %s\n", t->car, t->slot, be, bx);
        }
        t = t->next;
    }
}

void showFreeSlots() {
    printf("Free Slots: ");
    int any = 0;
    for (int s = 1; s <= MAX_SLOTS; s++) if (slotToCar[s] == -1) { printf("%d ", s); any = 1; }
    if (!any) printf("None");
    printf("\n");
}

//Main menu 
int main() {
    initSystem();
    printf("Smart Parking System - Slots: %d, Waiting: %d\n", MAX_SLOTS, WAIT_CAP);
    char ch;
    if (!read_char("Add monthly pass users? (y/n): ", &ch)) ch = 'n';
    if (ch == 'y' || ch == 'Y') {
        int k = 0;
        if (!read_int("How many? ", &k)) k = 0;
        for (int i = 0; i < k; i++) {
            int c = -1;
            char prompt[48];
            snprintf(prompt, sizeof(prompt), "Car #: ");
            if (read_int(prompt, &c)) addMonthlyPass(c);
        }
    }
    while (1) {
        printf("\n--- MENU ---\n");
        printf("1 Entry\n2 Exit\n3 History\n4 Slot Map\n5 Search Car\n6 Revenue\n7 Parked Cars\n8 Waiting Queue\n9 Add Monthly Pass\n10 Emergency\n11 Free Slots\n12 Quit\n");
        int choice;
        if (!read_int("Choice: ", &choice)) continue;
        switch (choice) {
            case 1: vehicleEntry(); break;
            case 2: vehicleExit(); break;
            case 3: showHistory(); break;
            case 4: showSlotMap(); break;
            case 5: { int c2; if (read_int("Car id: ", &c2)) searchCar(c2); break; }
            case 6: showRevenue(); break;
            case 7: showParkedVehicles(); break;
            case 8: showWaitingQueue(); break;
            case 9: { int c3; if (read_int("Car id: ", &c3)) addMonthlyPass(c3); break; }
            case 10: {
                char e; if (read_char("Activate emergency? (y/n): ", &e)) { if (e=='y' || e=='Y') emergencyMode(); }
                break;
            }
            case 11: showFreeSlots(); break;
            case 12: printf("Exiting...\n"); return 0;
            default: printf("Invalid choice.\n");
        }
    }
    return 0;
}
