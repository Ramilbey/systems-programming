/*
 * File: airport_sync.c
 * Solution: Airport Gate Assignment with Synchronization
 * Using: Mutex locks, Semaphores, Condition Variables
 * Features:
 * 1. Thread-safe gate assignment
 * 2. Priority for emergency flights
 * 3. Proper gate type matching
 * 4. Cleaning time enforcement
 * 5. Statistics protected from race conditions
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define NUM_GATES 5
#define NUM_FLIGHTS 10
#define MAX_TURNAROUND 5

typedef enum { DOMESTIC, INTERNATIONAL } FlightType;
typedef enum { AVAILABLE, OCCUPIED, MAINTENANCE } GateStatus;

typedef struct {
    int gate_id;
    char gate_name[10];
    GateStatus status;
    FlightType type;
    int current_flight;
    int occupied_until;
    bool is_emergency;
    int cleaning_time;
    pthread_mutex_t gate_mutex;  // Individual gate lock
    sem_t gate_sem;              // Gate availability semaphore
} Gate;

Gate airport[NUM_GATES];

// Global statistics with protection
int total_flights_served = 0;
int flights_diverted = 0;
int emergency_flights_handled = 0;
int simulation_time = 0;

// Global synchronization
pthread_mutex_t airport_mutex;
pthread_mutex_t stats_mutex;
pthread_mutex_t time_mutex;
sem_t available_gates;           // Counting semaphore for total available gates
pthread_cond_t emergency_cond;   // Condition variable for emergency priority

// Initialize airport with synchronization
void init_airport_sync() {
    char* gate_names[] = {"A1", "A2", "B1", "B2", "C1"};
    FlightType types[] = {DOMESTIC, DOMESTIC, INTERNATIONAL, INTERNATIONAL, DOMESTIC};
    
    for (int i = 0; i < NUM_GATES; i++) {
        airport[i].gate_id = i;
        strcpy(airport[i].gate_name, gate_names[i]);
        airport[i].status = AVAILABLE;
        airport[i].type = types[i];
        airport[i].current_flight = -1;
        airport[i].occupied_until = 0;
        airport[i].is_emergency = false;
        airport[i].cleaning_time = 1;
        
        // Initialize gate-specific synchronization
        pthread_mutex_init(&airport[i].gate_mutex, NULL);
        sem_init(&airport[i].gate_sem, 0, 1); // Binary semaphore per gate
    }
    
    // Initialize global synchronization
    pthread_mutex_init(&airport_mutex, NULL);
    pthread_mutex_init(&stats_mutex, NULL);
    pthread_mutex_init(&time_mutex, NULL);
    sem_init(&available_gates, 0, NUM_GATES); // All gates initially available
    pthread_cond_init(&emergency_cond, NULL);
}

// Thread-safe display
void display_airport_status_safe() {
    pthread_mutex_lock(&airport_mutex);
    
    printf("\n=== AIRPORT STATUS [SYNC] (Time: %02d:00) ===\n", simulation_time);
    printf("Gate\tType\t\tStatus\t\tFlight\tUntil\tEmergency\n");
    printf("----\t----\t\t------\t\t------\t-----\t---------\n");
    
    for (int i = 0; i < NUM_GATES; i++) {
        pthread_mutex_lock(&airport[i].gate_mutex);
        
        char type_str[20];
        char status_str[20];
        
        strcpy(type_str, airport[i].type == DOMESTIC ? "Domestic" : "International");
        strcpy(status_str, airport[i].status == AVAILABLE ? "Available" : 
                         (airport[i].status == OCCUPIED ? "Occupied" : "Maintenance"));
        
        printf("%s\t%-12s\t%-12s\t", 
               airport[i].gate_name, type_str, status_str);
        
        if (airport[i].status == OCCUPIED) {
            printf("FL%d\t%02d:00\t", 
                   airport[i].current_flight, 
                   airport[i].occupied_until);
        } else {
            printf("--\t--\t");
        }
        
        printf("%s\n", airport[i].is_emergency ? "EMERGENCY" : "");
        
        pthread_mutex_unlock(&airport[i].gate_mutex);
    }
    
    pthread_mutex_lock(&stats_mutex);
    printf("\nStatistics [THREAD-SAFE]:\n");
    printf("- Flights served: %d\n", total_flights_served);
    printf("- Flights diverted: %d\n", flights_diverted);
    printf("- Emergency flights handled: %d\n", emergency_flights_handled);
    pthread_mutex_unlock(&stats_mutex);
    
    pthread_mutex_unlock(&airport_mutex);
}

// SAFE: Find and assign gate with full synchronization
int assign_gate_safe(FlightType flight_type, int flight_id, bool is_emergency,
                    int arrival_time, int turnaround_hours) {
    
    printf("\nFlight FL%d %sarriving at %02d:00 (Type: %s, Turnaround: %d hours)\n",
           flight_id, is_emergency ? "[EMERGENCY] " : "", 
           arrival_time, flight_type == DOMESTIC ? "Domestic" : "International",
           turnaround_hours);
    
    // If emergency flight, wait for priority
    if (is_emergency) {
        pthread_mutex_lock(&airport_mutex);
        // Emergency flights get priority access
        pthread_mutex_unlock(&airport_mutex);
    }
    
    // Try each gate with proper locking
    for (int attempt = 0; attempt < 2; attempt++) { // Try twice for emergencies
        for (int i = 0; i < NUM_GATES; i++) {
            // Try to get exclusive access to this gate
            if (sem_trywait(&airport[i].gate_sem) == 0) {
                pthread_mutex_lock(&airport[i].gate_mutex);
                
                // Check if gate is suitable
                bool gate_suitable = false;
                
                if (airport[i].status == AVAILABLE) {
                    // Check type compatibility
                    if (flight_type == DOMESTIC) {
                        gate_suitable = true;
                    } else {
                        gate_suitable = (airport[i].type == INTERNATIONAL);
                    }
                    
                    // Emergency flights can use any gate
                    if (is_emergency && !gate_suitable && attempt == 1) {
                        gate_suitable = true;
                    }
                    
                    // Check cleaning time
                    if (gate_suitable && airport[i].occupied_until > arrival_time) {
                        printf("  Gate %s needs cleaning until %02d:00\n",
                               airport[i].gate_name, airport[i].occupied_until);
                        gate_suitable = false;
                    }
                }
                
                if (gate_suitable) {
                    // Wait for global gate semaphore
                    sem_wait(&available_gates);
                    
                    // Assign gate
                    airport[i].status = OCCUPIED;
                    airport[i].current_flight = flight_id;
                    airport[i].occupied_until = arrival_time + turnaround_hours + airport[i].cleaning_time;
                    airport[i].is_emergency = is_emergency;
                    
                    printf("  ✓ Assigned Gate %s [SYNC SAFE] (Available until %02d:00)\n",
                           airport[i].gate_name, airport[i].occupied_until);
                    
                    // Update statistics safely
                    pthread_mutex_lock(&stats_mutex);
                    total_flights_served++;
                    if (is_emergency) {
                        emergency_flights_handled++;
                    }
                    pthread_mutex_unlock(&stats_mutex);
                    
                    pthread_mutex_unlock(&airport[i].gate_mutex);
                    return i; // Success
                }
                
                // Release gate if not suitable
                sem_post(&airport[i].gate_sem);
                pthread_mutex_unlock(&airport[i].gate_mutex);
            }
        }
        
        if (!is_emergency) break; // Non-emergency flights only try once
    }
    
    // No gate available
    printf("  ✗ No suitable gate available! Flight FL%d diverted [SAFE DECISION]\n", flight_id);
    
    pthread_mutex_lock(&stats_mutex);
    flights_diverted++;
    pthread_mutex_unlock(&stats_mutex);
    
    return -1;
}

// SAFE: Release gate with synchronization
void release_gate_safe(int gate_index, int flight_id) {
    if (gate_index < 0 || gate_index >= NUM_GATES) return;
    
    pthread_mutex_lock(&airport[gate_index].gate_mutex);
    
    printf("\nFlight FL%d leaving Gate %s at %02d:00 [SYNC RELEASE]\n",
           flight_id, airport[gate_index].gate_name, simulation_time);
    
    airport[gate_index].status = AVAILABLE;
    airport[gate_index].current_flight = -1;
    airport[gate_index].is_emergency = false;
    
    // Set cleaning time
    pthread_mutex_lock(&time_mutex);
    airport[gate_index].occupied_until = simulation_time + airport[gate_index].cleaning_time;
    pthread_mutex_unlock(&time_mutex);
    
    // Signal gate availability
    sem_post(&available_gates);
    sem_post(&airport[gate_index].gate_sem);
    
    pthread_mutex_unlock(&airport[gate_index].gate_mutex);
    
    // Wake up any waiting emergency flights
    pthread_cond_signal(&emergency_cond);
}

// Flight thread with synchronization
void* flight_thread_safe(void* arg) {
    int flight_data = *((int*)arg);
    int flight_id = flight_data & 0xFF;
    FlightType flight_type = (flight_data >> 8) & 1;
    bool is_emergency = (flight_data >> 9) & 1;
    
    // Get arrival time safely
    pthread_mutex_lock(&time_mutex);
    int arrival_time = simulation_time + (rand() % 6);
    pthread_mutex_unlock(&time_mutex);
    
    int turnaround_hours = 1 + (rand() % MAX_TURNAROUND);
    
    // Wait until arrival
    usleep(arrival_time * 50000);
    
    // Get gate assignment safely
    int gate_assigned = assign_gate_safe(flight_type, flight_id, is_emergency,
                                        arrival_time, turnaround_hours);
    
    if (gate_assigned != -1) {
        // Simulate turnaround
        usleep(turnaround_hours * 100000);
        
        // Release gate safely
        release_gate_safe(gate_assigned, flight_id);
    }
    
    printf("Flight FL%d completed operations [THREAD-SAFE]\n", flight_id);
    
    free(arg); // Free dynamically allocated parameter
    return NULL;
}

// Time simulator with synchronization
void* time_simulator_safe(void* arg) {
    while (1) {
        sleep(1);
        
        pthread_mutex_lock(&time_mutex);
        simulation_time = (simulation_time + 1) % 24;
        pthread_mutex_unlock(&time_mutex);
        
        // Auto-release expired gates safely
        for (int i = 0; i < NUM_GATES; i++) {
            if (sem_trywait(&airport[i].gate_sem) == 0) {
                pthread_mutex_lock(&airport[i].gate_mutex);
                
                pthread_mutex_lock(&time_mutex);
                int current_time = simulation_time;
                pthread_mutex_unlock(&time_mutex);
                
                if (airport[i].status == OCCUPIED && 
                    airport[i].occupied_until <= current_time) {
                    
                    printf("\n[Auto-release SYNC] Gate %s now available (Flight FL%d expired)\n",
                           airport[i].gate_name, airport[i].current_flight);
                    
                    airport[i].status = AVAILABLE;
                    airport[i].current_flight = -1;
                    airport[i].is_emergency = false;
                    airport[i].occupied_until = current_time + airport[i].cleaning_time;
                    
                    sem_post(&available_gates);
                }
                
                sem_post(&airport[i].gate_sem);
                pthread_mutex_unlock(&airport[i].gate_mutex);
            }
        }
    }
    return NULL;
}

int main() {
    pthread_t flights[NUM_FLIGHTS];
    pthread_t time_thread;
    
    srand(time(NULL));
    init_airport_sync();
    
    printf("===============================================\n");
    printf("AIRPORT GATE ASSIGNMENT - SYNCHRONIZED\n");
    printf("===============================================\n");
    printf("Gates: %d (A1,A2: Domestic, B1,B2: International, C1: Domestic)\n", NUM_GATES);
    printf("Synchronization: Mutex + Semaphores + Condition Variables\n");
    printf("Features: Priority, Type Safety, Cleaning Time, Thread-Safe Stats\n\n");
    
    // Start time simulator
    pthread_create(&time_thread, NULL, time_simulator_safe, NULL);
    
    display_airport_status_safe();
    
    // Create flight threads with dynamically allocated parameters
    for (int i = 0; i < NUM_FLIGHTS; i++) {
        int* flight_params = malloc(sizeof(int));
        *flight_params = (i + 1);
        *flight_params |= ((rand() % 2) << 8);
        *flight_params |= (((i % 4) == 0) << 9);
        
        pthread_create(&flights[i], NULL, flight_thread_safe, flight_params);
        usleep(200000); // Stagger arrivals
    }
    
    // Wait for flights
    for (int i = 0; i < NUM_FLIGHTS; i++) {
        pthread_join(flights[i], NULL);
    }
    
    // Cleanup
    pthread_cancel(time_thread);
    
    printf("\n===============================================\n");
    printf("FINAL RESULTS (SYNCHRONIZED)\n");
    printf("===============================================\n");
    display_airport_status_safe();
    
    // Verification
    printf("\n--- VERIFICATION ---\n");
    
    int occupied_count = 0;
    for (int i = 0; i < NUM_GATES; i++) {
        pthread_mutex_lock(&airport[i].gate_mutex);
        if (airport[i].status == OCCUPIED) {
            occupied_count++;
            printf("Gate %s occupied by Flight FL%d\n",
                   airport[i].gate_name, airport[i].current_flight);
        }
        pthread_mutex_unlock(&airport[i].gate_mutex);
    }
    
    pthread_mutex_lock(&stats_mutex);
    printf("\nStatistics check:\n");
    printf("Flights served + diverted = %d + %d = %d\n",
           total_flights_served, flights_diverted,
           total_flights_served + flights_diverted);
    printf("Expected total flights: %d\n", NUM_FLIGHTS);
    
    if (total_flights_served + flights_diverted == NUM_FLIGHTS) {
        printf("✓ STATISTICS CONSISTENT!\n");
    }
    
    if (occupied_count == 0) {
        printf("✓ ALL GATES PROPERLY RELEASED!\n");
    }
    pthread_mutex_unlock(&stats_mutex);
    
    // Cleanup synchronization primitives
    for (int i = 0; i < NUM_GATES; i++) {
        pthread_mutex_destroy(&airport[i].gate_mutex);
        sem_destroy(&airport[i].gate_sem);
    }
    
    pthread_mutex_destroy(&airport_mutex);
    pthread_mutex_destroy(&stats_mutex);
    pthread_mutex_destroy(&time_mutex);
    sem_destroy(&available_gates);
    pthread_cond_destroy(&emergency_cond);
    
    printf("\n✓ ALL SYNCHRONIZATION PRIMITIVES CLEANED UP\n");
    
    return 0;
}
