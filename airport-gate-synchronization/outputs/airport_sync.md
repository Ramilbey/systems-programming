~/Desktop/swe2409162 > ./airport_sync
===============================================
AIRPORT GATE ASSIGNMENT - SYNCHRONIZED
===============================================
Gates: 5 (A1,A2: Domestic, B1,B2: International, C1: Domestic)
Synchronization: Mutex + Semaphores + Condition Variables
Features: Priority, Type Safety, Cleaning Time, Thread-Safe Stats


=== AIRPORT STATUS [SYNC] (Time: 00:00) ===
Gate	Type		Status		Flight	Until	Emergency
----	----		------		------	-----	---------
A1	Domestic    	Available   	--	--	
A2	Domestic    	Available   	--	--	
B1	International	Available   	--	--	
B2	International	Available   	--	--	
C1	Domestic    	Available   	--	--	

Statistics [THREAD-SAFE]:
- Flights served: 0
- Flights diverted: 0
- Emergency flights handled: 0

Flight FL1 [EMERGENCY] arriving at 04:00 (Type: Domestic, Turnaround: 5 hours)
  ✓ Assigned Gate A1 [SYNC SAFE] (Available until 10:00)

Flight FL2 arriving at 03:00 (Type: International, Turnaround: 5 hours)
  ✓ Assigned Gate B1 [SYNC SAFE] (Available until 09:00)

Flight FL3 arriving at 05:00 (Type: Domestic, Turnaround: 2 hours)
  ✓ Assigned Gate A2 [SYNC SAFE] (Available until 08:00)

Flight FL1 leaving Gate A1 at 00:00 [SYNC RELEASE]
Flight FL1 completed operations [THREAD-SAFE]

Flight FL4 arriving at 04:00 (Type: Domestic, Turnaround: 2 hours)
  ✓ Assigned Gate A1 [SYNC SAFE] (Available until 07:00)

Flight FL3 leaving Gate A2 at 00:00 [SYNC RELEASE]
Flight FL3 completed operations [THREAD-SAFE]

Flight FL2 leaving Gate B1 at 00:00 [SYNC RELEASE]
Flight FL2 completed operations [THREAD-SAFE]

Flight FL4 leaving Gate A1 at 01:00 [SYNC RELEASE]
Flight FL4 completed operations [THREAD-SAFE]

Flight FL5 [EMERGENCY] arriving at 05:00 (Type: Domestic, Turnaround: 3 hours)
  ✓ Assigned Gate A1 [SYNC SAFE] (Available until 09:00)

Flight FL6 arriving at 01:00 (Type: Domestic, Turnaround: 3 hours)
  ✓ Assigned Gate A2 [SYNC SAFE] (Available until 05:00)

Flight FL7 arriving at 01:00 (Type: International, Turnaround: 3 hours)
  ✓ Assigned Gate B1 [SYNC SAFE] (Available until 05:00)

Flight FL5 leaving Gate A1 at 01:00 [SYNC RELEASE]
Flight FL5 completed operations [THREAD-SAFE]

Flight FL6 leaving Gate A2 at 01:00 [SYNC RELEASE]
Flight FL6 completed operations [THREAD-SAFE]

Flight FL8 arriving at 02:00 (Type: International, Turnaround: 4 hours)
  ✓ Assigned Gate B2 [SYNC SAFE] (Available until 07:00)

Flight FL7 leaving Gate B1 at 01:00 [SYNC RELEASE]
Flight FL7 completed operations [THREAD-SAFE]

Flight FL8 leaving Gate B2 at 01:00 [SYNC RELEASE]
Flight FL8 completed operations [THREAD-SAFE]

Flight FL9 [EMERGENCY] arriving at 06:00 (Type: Domestic, Turnaround: 5 hours)
  ✓ Assigned Gate A1 [SYNC SAFE] (Available until 12:00)

Flight FL10 arriving at 02:00 (Type: Domestic, Turnaround: 5 hours)
  ✓ Assigned Gate A2 [SYNC SAFE] (Available until 08:00)

Flight FL9 leaving Gate A1 at 02:00 [SYNC RELEASE]
Flight FL9 completed operations [THREAD-SAFE]

Flight FL10 leaving Gate A2 at 02:00 [SYNC RELEASE]
Flight FL10 completed operations [THREAD-SAFE]

===============================================
FINAL RESULTS (SYNCHRONIZED)
===============================================

=== AIRPORT STATUS [SYNC] (Time: 02:00) ===
Gate	Type		Status		Flight	Until	Emergency
----	----		------		------	-----	---------
A1	Domestic    	Available   	--	--	
A2	Domestic    	Available   	--	--	
B1	International	Available   	--	--	
B2	International	Available   	--	--	
C1	Domestic    	Available   	--	--	

Statistics [THREAD-SAFE]:
- Flights served: 10
- Flights diverted: 0
- Emergency flights handled: 3

--- VERIFICATION ---

Statistics check:
Flights served + diverted = 10 + 0 = 10
Expected total flights: 10
✓ STATISTICS CONSISTENT!
✓ ALL GATES PROPERLY RELEASED!

✓ ALL SYNCHRONIZATION PRIMITIVES CLEANED UP
