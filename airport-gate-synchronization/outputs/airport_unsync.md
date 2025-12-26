~/Desktop/swe2409162 > ./airport_unsync
===============================================
AIRPORT GATE ASSIGNMENT - UNSYNCHRONIZED
===============================================
Total Gates: 5 (3 Domestic, 2 International)
Flights: 10 (Random types, some emergencies)

Expected:
- No two flights at same gate
- International flights only at international gates
- Emergency flights get priority
- Proper gate cleaning between flights


=== AIRPORT STATUS (Time: 00:00) ===
Gate	Type		Status		Flight	Until	Emergency
----	----		------		------	-----	---------
A1	Domestic    	Available   	--	--	
A2	Domestic    	Available   	--	--	
B1	International	Available   	--	--	
B2	International	Available   	--	--	
C1	Domestic    	Available   	--	--	

Statistics:
- Flights served: 0
- Flights diverted: 0
- Emergency flights handled: 0
- Available gates: A1 A2 B1 B2 C1 

Flight FL1 [EMERGENCY] arriving at 00:00 (Type: International, Turnaround: 2 hours)
  ✓ Assigned Gate A1 (Available until 03:00)

Flight FL1 leaving Gate A1 at 00:00
Flight FL1 completed operations

Flight FL2 arriving at 05:00 (Type: International, Turnaround: 4 hours)

Flight FL3 arriving at 01:00 (Type: International, Turnaround: 5 hours)
  ✓ Assigned Gate B1 (Available until 10:00)
  Gate B1 needs cleaning until 10:00
  ✓ Assigned Gate B2 (Available until 07:00)

Flight FL4 arriving at 04:00 (Type: International, Turnaround: 2 hours)
  ✗ No suitable gate available! Flight FL4 diverted
Flight FL4 completed operations

Flight FL2 leaving Gate B1 at 00:00
Flight FL2 completed operations

Flight FL5 [EMERGENCY] arriving at 02:00 (Type: Domestic, Turnaround: 2 hours)
  ✓ Assigned Gate A1 (Available until 05:00)

Flight FL3 leaving Gate B2 at 01:00
Flight FL3 completed operations

Flight FL6 arriving at 01:00 (Type: Domestic, Turnaround: 5 hours)
  ✓ Assigned Gate A2 (Available until 07:00)

Flight FL5 leaving Gate A1 at 01:00
Flight FL5 completed operations

Flight FL7 arriving at 03:00 (Type: International, Turnaround: 3 hours)
  ✓ Assigned Gate B1 (Available until 07:00)

Flight FL6 leaving Gate A2 at 01:00
Flight FL6 completed operations

Flight FL8 arriving at 06:00 (Type: International, Turnaround: 5 hours)

Flight FL7 leaving Gate B1 at 01:00
Flight FL7 completed operations
  ✓ Assigned Gate B2 (Available until 12:00)

Flight FL9 [EMERGENCY] arriving at 05:00 (Type: Domestic, Turnaround: 2 hours)

Flight FL10 arriving at 02:00 (Type: Domestic, Turnaround: 4 hours)
  ✓ Assigned Gate A1 (Available until 08:00)
  ✓ Assigned Gate A2 (Available until 07:00)

Flight FL9 leaving Gate A1 at 02:00
Flight FL9 completed operations

Flight FL8 leaving Gate B2 at 02:00
Flight FL8 completed operations

Flight FL10 leaving Gate A2 at 02:00
Flight FL10 completed operations

===============================================
FINAL RESULTS (UNSYNCHRONIZED)
===============================================

=== AIRPORT STATUS (Time: 02:00) ===
Gate	Type		Status		Flight	Until	Emergency
----	----		------		------	-----	---------
A1	Domestic    	Available   	--	--	
A2	Domestic    	Available   	--	--	
B1	International	Available   	--	--	
B2	International	Available   	--	--	
C1	Domestic    	Available   	--	--	

Statistics:
- Flights served: 9
- Flights diverted: 1
- Emergency flights handled: 3
- Available gates: A1 A2 B1 B2 C1 

--- RACE CONDITION DETECTIONS ---

Statistics analysis:
Total flights served: 9
Flights diverted: 1
Sum: 10 (Should equal 10)
~/Desktop/swe2409162 > 
