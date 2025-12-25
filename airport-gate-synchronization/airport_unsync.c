/*
 * File: airport_unsync.c
 * Description: Airport Gate Assignment with Race Conditions
 * Problem: Multiple flights compete for limited gates without synchronization
 * Unique challenges:
 * 1. Different gate types (domestic/international)
 * 2. Flight turnaround time (gate occupied for specific duration)
 * 3. Emergency flights get priority
 * 4. Connecting flights need specific gates
 * 5. Cleaning/maintenance time between flights
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define NUM_GATES 5
#define NUM_FLIGHTS 10
#define MAX_TURNAROUND 5  // Hours

typedef enum { DOMESTIC, INTERNATIONAL } FlightType;
typedef enum { AVAILABLE, OCCUPIED, MAINTENANCE } GateStatus;

typedef struct {
    int gate_id;
    char gate_name[10];      // A1, B2, etc.
    GateStatus status;
    FlightType type;         // Domestic or International
    int current_flight;
    int occupied_until;      // Simulated time (0-24)
    bool is_emergency;
    int cleaning_time;       // Time needed between flights
} Gate;

Gate airport[NUM_GATES];
int total_flights_served = 0;
int flights_diverted = 0;
int emergency_flights_handled = 0;
int simulation_time = 0;     // Current time in simulation (0-24)

// Initialize airport gates
void init_airport() {
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
        airport[i].cleaning_time = 1; // 1 hour cleaning between flights
    }
}

// Display airport status
void display_airport_status() {
    printf("\n=== AIRPORT STATUS (Time: %02d:00) ===\n", simulation_time);
    printf("Gate\tType\t\tStatus\t\tFlight\tUntil\tEmergency\n");
    printf("----\t----\t\t------\t\t------\t-----\t---------\n");
    
    for (int i = 0; i < NUM_GATES; i++) {
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
    }
    
    printf("\nStatistics:\n");
    printf("- Flights served: %d\n", total_flights_served);
    printf("- Flights diverted: %d\n", flights_diverted);
    printf("- Emergency flights handled: %d\n", emergency_flights_handled);
    printf("- Available gates: ");
    for (int i = 0; i < NUM_GATES; i++) {
        if (airport[i].status == AVAILABLE) {
            printf("%s ", airport[i].gate_name);
        }
    }
    printf("\n");
}

/*
 * CRITICAL PROBLEMS without synchronization:
 * 1. Two flights assigned to same gate (double booking)
 * 2. International flight assigned to domestic gate (wrong type)
 * 3. Emergency flights not getting priority
 * 4. Gate cleaning not accounted for properly
 * 5. Time calculations race conditions
 */

// Find available gate for a flight (UNSAFE - RACE CONDITION)
int find_gate_unsafe(FlightType flight_type, int flight_id, bool is_emergency, 
                    int arrival_time, int turnaround_hours) {
    
    printf("\nFlight FL%d %sarriving at %02d:00 (Type: %s, Turnaround: %d hours)\n",
           flight_id, is_emergency ? "[EMERGENCY] " : "", 
           arrival_time, flight_type == DOMESTIC ? "Domestic" : "International",
           turnaround_hours);
    
    // RACE CONDITION: Multiple flights check gates simultaneously
    for (int i = 0; i < NUM_GATES; i++) {
        // Check if gate is available and matches type
        bool gate_available = false;
        
        if (airport[i].status == AVAILABLE) {
            // Check gate type compatibility
            if (flight_type == DOMESTIC) {
                // Domestic flights can use any gate
                gate_available = true;
            } else {
                // International flights need international gates
                gate_available = (airport[i].type == INTERNATIONAL);
            }
            
            // Emergency flights can take any available gate
            if (is_emergency && !gate_available) {
                gate_available = true;
            }
        }
        
        if (gate_available) {
            // DANGER: Between this check and assignment, another thread might take the gate!
            usleep(50000); // Simulates decision time (makes race condition more likely)
            
            // Check if gate needs cleaning
            if (airport[i].occupied_until > arrival_time) {
                printf("  Gate %s needs cleaning until %02d:00\n", 
                       airport[i].gate_name, airport[i].occupied_until);
                continue;
            }
            
            // Assign gate to flight
            airport[i].status = OCCUPIED;
            airport[i].current_flight = flight_id;
            airport[i].occupied_until = arrival_time + turnaround_hours + airport[i].cleaning_time;
            airport[i].is_emergency = is_emergency;
            
            printf("  ✓ Assigned Gate %s (Available until %02d:00)\n",
                   airport[i].gate_name, airport[i].occupied_until);
            
            // RACE CONDITION: Increment without synchronization
            total_flights_served++;
            if (is_emergency) {
                emergency_flights_handled++;
            }
            
            return i; // Return gate index
        }
    }
    
    printf("  ✗ No suitable gate available! Flight FL%d diverted\n", flight_id);
    
    // RACE CONDITION: Multiple threads might increment simultaneously
    flights_diverted++;
    
    return -1; // No gate available
}

// Release gate after turnaround (UNSAFE)
void release_gate_unsafe(int gate_index, int flight_id) {
    if (gate_index >= 0 && gate_index < NUM_GATES) {
        // RACE CONDITION: Might release wrong gate or double-release
        printf("\nFlight FL%d leaving Gate %s at %02d:00\n",
               flight_id, airport[gate_index].gate_name, simulation_time);
        
        airport[gate_index].status = AVAILABLE;
        airport[gate_index].current_flight = -1;
        airport[gate_index].is_emergency = false;
        
        // Gate now available after cleaning time
        airport[gate_index].occupied_until = simulation_time + airport[gate_index].cleaning_time;
    }
}

// Flight thread function
void* flight_thread(void* arg) {
    int flight_data = *((int*)arg);
    int flight_id = (flight_data & 0xFF);           // Lower 8 bits: flight ID
    FlightType flight_type = (flight_data >> 8) & 1; // Next bit: flight type
    bool is_emergency = (flight_data >> 9) & 1;      // Next bit: emergency flag
    
    // Randomize arrival time (0-23 hours)
    int arrival_time = simulation_time + (rand() % 6);
    
    // Random turnaround time (1-5 hours)
    int turnaround_hours = 1 + (rand() % MAX_TURNAROUND);
    
    // Wait until arrival time
    usleep(arrival_time * 50000); // Simulate time passing
    
    // Try to get a gate
    int gate_assigned = find_gate_unsafe(flight_type, flight_id, is_emergency, 
                                        arrival_time, turnaround_hours);
    
    if (gate_assigned != -1) {
        // Simulate turnaround time
        usleep(turnaround_hours * 100000);
        
        // Release gate
        release_gate_unsafe(gate_assigned, flight_id);
    }
    
    printf("Flight FL%d completed operations\n", flight_id);
    
    return NULL;
}

// Time simulation thread
void* time_simulator(void* arg) {
    while (1) {
        sleep(1); // Simulate time passing faster
        simulation_time = (simulation_time + 1) % 24;
        
        // Auto-release gates whose time has expired
        for (int i = 0; i < NUM_GATES; i++) {
            if (airport[i].status == OCCUPIED && 
                airport[i].occupied_until <= simulation_time) {
                printf("\n[Auto-release] Gate %s now available (Flight FL%d time expired)\n",
                       airport[i].gate_name, airport[i].current_flight);
                airport[i].status = AVAILABLE;
                airport[i].current_flight = -1;
                airport[i].is_emergency = false;
            }
        }
    }
    return NULL;
}

int main() {
    pthread_t flights[NUM_FLIGHTS];
    pthread_t time_thread;
    int flight_params[NUM_FLIGHTS];
    
    srand(time(NULL));
    init_airport();
    
    printf("===============================================\n");
    printf("AIRPORT GATE ASSIGNMENT - UNSYNCHRONIZED\n");
    printf("===============================================\n");
    printf("Total Gates: %d (3 Domestic, 2 International)\n", NUM_GATES);
    printf("Flights: %d (Random types, some emergencies)\n", NUM_FLIGHTS);
    printf("\nExpected:\n");
    printf("- No two flights at same gate\n");
    printf("- International flights only at international gates\n");
    printf("- Emergency flights get priority\n");
    printf("- Proper gate cleaning between flights\n\n");
    
    // Start time simulator
    pthread_create(&time_thread, NULL, time_simulator, NULL);
    
    display_airport_status();
    
    // Create flight threads
    for (int i = 0; i < NUM_FLIGHTS; i++) {
        // Pack flight parameters into int
        flight_params[i] = (i + 1); // Flight ID
        flight_params[i] |= ((rand() % 2) << 8); // Random flight type
        flight_params[i] |= (((i % 4) == 0) << 9); // Every 4th flight is emergency
        
        pthread_create(&flights[i], NULL, flight_thread, &flight_params[i]);
        
        // Stagger flight arrivals
        usleep(200000);
    }
    
    // Wait for all flight threads
    for (int i = 0; i < NUM_FLIGHTS; i++) {
        pthread_join(flights[i], NULL);
    }
    
    // Cancel time thread
    pthread_cancel(time_thread);
    
    printf("\n===============================================\n");
    printf("FINAL RESULTS (UNSYNCHRONIZED)\n");
    printf("===============================================\n");
    display_airport_status();
    
    // Check for problems
    printf("\n--- RACE CONDITION DETECTIONS ---\n");
    
    // Check for double bookings
    int occupied_count = 0;
    for (int i = 0; i < NUM_GATES; i++) {
        if (airport[i].status == OCCUPIED) {
            occupied_count++;
            printf("Gate %s still occupied by Flight FL%d (should be released)\n",
                   airport[i].gate_name, airport[i].current_flight);
        }
    }
    
    // Check statistics consistency
    printf("\nStatistics analysis:\n");
    printf("Total flights served: %d\n", total_flights_served);
    printf("Flights diverted: %d\n", flights_diverted);
    printf("Sum: %d (Should equal %d)\n", 
           total_flights_served + flights_diverted, NUM_FLIGHTS);
    
    if (total_flights_served + flights_diverted != NUM_FLIGHTS) {
        printf("⚠️  INCONSISTENT COUNT! Race condition in statistics!\n");
    }
    
    if (occupied_count > 0) {
        printf("⚠️  GATES NOT RELEASED! Race condition in gate management!\n");
    }
    
    return 0;
}
