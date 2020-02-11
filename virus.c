#include <stdio.h>
#include <math.h>
#include <omp.h>

#define MAX_PEOPLE 5000
#define PEOPLE_ID_LUT_SIZE 10001
#define MAX_DAY 1000
#define INFECTION_LENGTH 15

#define DEBUG_LEVEL 1
// 0 = OFF, 1 = INFO, 2 = VERBOSE

#define TIME_REPORT 1
// Benchmark time taken to calculate (not including stdin reading)
// 0 = OFF, 1 = ON

#if TIME_REPORT

#include <time.h>

struct timespec time_start;
unsigned long time_run;
#endif

// Functions Prototype
void parseInput();

unsigned short simulate();

// Type declaration
typedef struct {
    unsigned short id;
    int x;
    int y;
    int toX;
    int toY;
    short status; // 0 = normal, 1 - 16 = infected, -1 = died lol
} Person;

typedef struct {
    unsigned short id;
    int x;
    int y;
} PersonInit;

typedef struct {
    unsigned short id;
    int dx;
    int dy;
} Movement;

// Config and Variables
short numPeople = 0; //number of people in the simulation
short daySimulated = 0; //day of infection

Movement movementTable[MAX_DAY][MAX_PEOPLE];
PersonInit peopleInitValue[MAX_PEOPLE];
Person people[MAX_PEOPLE];
unsigned short idTable[PEOPLE_ID_LUT_SIZE];
unsigned short peopleInfectiousValue[MAX_PEOPLE];

unsigned short checkNear(int sx1, int sy1, int sx2, int sy2, int px1, int py1, int px2, int py2) {
    double xsp1 = sx2 - sx1 - px2 + px1;
    double ysp1 = sy2 - sy1 - py2 + py1;
    double xsp2 = sx1 - px1;
    double ysp2 = sy1 - py1;

    double a = (xsp1 * xsp1) + (ysp1 * ysp1);
    double b = (2 * xsp1 * xsp2) + (2 * ysp1 * ysp2);
    double c = (xsp2 * xsp2) + (ysp2 * ysp2) - 100;

// TODO Check if this is needed ( prevent ( /2a ) from causing problem)
//    if (a == 0) {
//        return 0;
//    }

    double a2 = 2 * a;
    double bb = b * b;
    double ac4 = a * c * 4;
    double bb4ac = bb - ac4;

    if (bb4ac <= 0) {
        return 0;
    }

    double sqrtbb4ac = sqrt(bb4ac);

    double min = (0 - b - sqrtbb4ac) / a2;
    double max = (0 - b + sqrtbb4ac) / a2;

    return max >= 0 && min <= 1;
}

void parseInput() {
    scanf("%hu", &numPeople);

    int p = 0;
    int q = 0;
    for (short i = 0; i < numPeople; i++) {
        scanf("%hu", &(peopleInitValue[i].id)); //scan id
        scanf("%d", &(peopleInitValue[i].x)); //scan initial x
        scanf("%d", &(peopleInitValue[i].y)); //scan initial y
    }

    for (int i = 0; i < PEOPLE_ID_LUT_SIZE; i++) {
        idTable[i] = 0;
    }
    for (int i = 0; i < numPeople; i++) {
        idTable[peopleInitValue[i].id] = i;
    }

    scanf("%hu", &daySimulated);
    for (short date = 0; date < daySimulated; date++) {
        for (short pep = 0; pep < numPeople; pep++) {
            unsigned short id;
            int dx, dy;
            scanf("%hu %d %d", &id, &dx, &dy);
            Movement *movement = &movementTable[date][idTable[id]];
            movement->id = id;
            movement->dx = dx;
            movement->dy = dy;
            //scanf("%hu %d %d", &(movementTable[date][pep].id), &(movementTable[date][pep].dx),&(movementTable[date][pep].dy));
        }
    }

    //debug input
#if DEBUG_LEVEL >= 1
    printf("No. of people: %hu\n", numPeople);
    printf("No. of day: %hu\n", daySimulated);
#endif
#if DEBUG_LEVEL >= 2
    printf("IDs: \n");
    for (short i = 0; i < numPeople; i++) {
        printf("%hu\n", peopleInitValue[i].id);
    }
#endif
}

void init(Person localPeople[]) {
    for (int i = 0; i < numPeople; i++) {
        Person *person = &localPeople[i];
        person->x = peopleInitValue[i].x;
        person->y = peopleInitValue[i].y;
        person->id = peopleInitValue[i].id;
        person->status = 0;
    }
}

unsigned short simulate(Person localPeople[]) {
    for (int day = 0; day < daySimulated; day++) {
        for (int i = 0; i < numPeople; i++) {
            Person *person = &localPeople[i];
            if (person->status == -1) continue;

            if (day > 0) {
                person->x = person->toX;
                person->y = person->toY;

                if (person->status > 0) person->status++;
            }

            person->toX = person->x + movementTable[day][i].dx;
            person->toY = person->y + movementTable[day][i].dy;

#if DEBUG_LEVEL >= 2
            printf("ID=%d moving from (%d,%d) to (%d,%d)\n", person->id, person->x, person->y, person->toX,
                   person->toY);
#endif
        }


        for (int i = 0; i < numPeople; i++) {
            Person *person = &localPeople[i];
            if (person->status == -1) continue;

            if (person->status > INFECTION_LENGTH) {
                person->status = -1;
            } else if (person->status >= 2) {
                // Ready to spread

                for (int j = 0; j < numPeople; j++) {
                    if (j == i) continue;

                    Person *personB = &localPeople[j];
                    if (personB->status != 0) continue;

                    if (checkNear(person->x, person->y, person->toX, person->toY,
                                  personB->x, personB->y, personB->toX, personB->toY)) {
                        personB->status = 1;
#if DEBUG_LEVEL >= 2
                        printf("ID=%d infected ID=%d\n", person->id, personB->id);
#endif
                    }
                }
            }
        }


#if DEBUG_LEVEL >= 2
        register unsigned short todayCounter = 0;
        for (int i = 0; i < numPeople; i++) {
            if (localPeople[i].status != 0) {
                todayCounter++;
            }
        }

        printf("Day %d ended\n", day);
        printf("Infected today: %d \n", todayCounter);
#endif
    }

    unsigned short counter = 0;
    for (int i = 0; i < numPeople; i++) {
        if (localPeople[i].status != 0) {
            counter++;
        }
    }

    return counter;
}

int main() {
    parseInput();

#if TIME_REPORT
    clock_gettime(CLOCK_REALTIME, &time_start);
#endif

#pragma omp parallel for private(people) schedule(dynamic)
    for (int i = 0; i < numPeople; i++) {
        init(&people[0]);
        people[i].status = 1;
        unsigned short infectedCount = simulate(&people[0]);

#if DEBUG_LEVEL >= 1
        printf("Starting with id=%d will infect %d people\n", people[i].id, infectedCount);
#endif
        peopleInfectiousValue[i] = infectedCount;

    }


    unsigned short maxInfection = 0;
    unsigned short maxInfectionId = 0;

    for (int i = 0; i < numPeople; i++) {
        if (peopleInfectiousValue[i] > maxInfection) {
            maxInfection = peopleInfectiousValue[i];
            maxInfectionId = peopleInitValue[i].id;
        }
    }


    printf("%d\n", maxInfectionId);

#if TIME_REPORT
    struct timespec time_end;
    clock_gettime(CLOCK_REALTIME, &time_end);
    time_run =
            ((time_end.tv_sec - time_start.tv_sec) * (long) 1e9 + (time_end.tv_nsec - time_start.tv_nsec)) / 1000;
    printf("Program finished in %0.2f ms\n", time_run / 1000.0);
#endif
}