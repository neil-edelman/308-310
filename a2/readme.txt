2014 Neil Edelman; released into public-domain.
neil dot edelman each mail dot mcgill dot ca

Usage: PrinterSimulation [-c <clients>] [-p <printers>] [-b <buffer>] [-h(elp)]
Version 1.0.

Hint: direct stderr to null to not get spammed.

To make, /make (tested GNU Make 3.81,) the executable will be placed in bin/.

Bugs:

* No unnamed semaphores on my machine! I used named semaphores; therefore
running two or more instances is inadvisable.

* Sometimes it just skips the sleep and deletes the printers before anything is
printed; the printer queue then fills up with no printers, causing deadlock.
This is weird and non-deterministic. I detect it and say to Try again.

* No sem_timedwait because struct timespec has no way to get filled portably;
CLOCK_MONOTONIC is not on my machine. I have hacked together a solution: sleep
and then check; the printers therefore have a 1s sluggishness.

Tested on MacOS10.8 and mimi.cs.mcgill.ca; linux.cs.mcgill.ca always gets
'try again;' also, it doesn't seed the random value, always Uthith-glob.
I'd find a cause, but I have to go to bed. Workaround: don't use
Debian GNU/Linux.

PrinterSimulation 2014 Neil Edelman
This program comes with ABSOLUTELY NO WARRANTY.

The defaults are four clients, two printers, buffer size of three:

wpa147019:a2 neil$ bin/PrinterSimulation 2>/dev/null
Uuarg-glob has 5 pages to print, puts request in Buffer[0] [0,1]
Uuarg-glob signing off.
Askookurta has 3 pages to print, puts request in Buffer[1] [0,2]
Mukkhamkrimp has 9 pages to print, puts request in Buffer[2] [0,0]
Orccomebubhosh has 1 pages to print, but buffer full.
Printer 1 starts printing 5 pages from buffer[0] (from Uuarg-glob)
Printer 2 starts printing 3 pages from buffer[1] (from Askookurta)
Askookurta has 6 pages to print, but buffer full.
Mukkhamkrimp has 6 pages to print, but buffer full.
Orccomebubhosh has 1 pages to print, puts request in Buffer[0] [2,1]
Orccomebubhosh signing off.
Printer 2 starts printing 9 pages from buffer[2] (from Mukkhamkrimp)
Askookurta has 6 pages to print, puts request in Buffer[1] [0,2]
Askookurta signing off.
Printer 1 starts printing 1 pages from buffer[0] (from Orccomebubhosh)
Mukkhamkrimp has 6 pages to print, puts request in Buffer[2] [1,0]
Mukkhamkrimp has 8 pages to print, but buffer full.
Printer 1 starts printing 6 pages from buffer[1] (from Askookurta)
Mukkhamkrimp has 8 pages to print, puts request in Buffer[0] [2,1]
Mukkhamkrimp signing off.
Printer 2 starts printing 6 pages from buffer[2] (from Mukkhamkrimp)
Printer 1 starts printing 8 pages from buffer[0] (from Mukkhamkrimp)
No request in buffer, Printer 2 sleeps.
No request in buffer, Printer 1 sleeps.
Shutting down.


wpa147019:a2 neil$ bin/PrinterSimulation -h
Usage: PrinterSimulation [-c <clients>] [-p <printers>] [-b <buffer>] [-h(elp)]
Version 1.0.

PrinterSimulation 2014 Neil Edelman
This program comes with ABSOLUTELY NO WARRANTY.


One printer:

wpa147019:a2 neil$ bin/PrinterSimulation -c 2 -p1 -b5
Spool::semaphores: opening.
Printer: new, 1 with 1000.000000s/page #0x100100100.
Client: new, Uuenkrimp (1) #0x1001001e0.
Client: new, Ethickbubhosh (2) #0x100100210.
Spool: new, jobs 5, printers 1, clients 2, #0x100100080.
~Spool: begin erasing!
Printer 1 waiting.
Job: new, 2 pages for Uuenkrimp #0x100100240.
Uuenkrimp has 2 pages to print, puts request in Buffer[0] [0,1]
Uuenkrimp signing off.
Job: new, 3 pages for Ethickbubhosh #0x1001002a0.
Ethickbubhosh has 3 pages to print, puts request in Buffer[1] [0,2]
Ethickbubhosh signing off.
Printer 1 go!
Printer 1 starts printing 2 pages from buffer[0] (from Uuenkrimp)
~Job: erase, print job for Uuenkrimp, done 2 / 2, #0x100100240.
Printer 1 waiting.
Printer 1 go!
Printer 1 starts printing 3 pages from buffer[1] (from Ethickbubhosh)
~Job: erase, print job for Ethickbubhosh, done 3 / 3, #0x1001002a0.
Printer 1 waiting.
No request in buffer, Printer 1 sleeps.
~Printer: 1 thread return #0x0, erase #0x100100100.
~Client: Uuenkrimp thread return #0x0, erase #0x1001001e0.
~Client: Ethickbubhosh thread return #0x0, erase #0x100100210.
~Spool: erase, #0x100100080.
Shutting down.
Spool::~semaphores: closing.


No buffer:

wpa147019:a2 neil$ bin/PrinterSimulation -c 2 -p4 -b1
Spool::semaphores: opening.
Printer: new, 1 with 1000.000000s/page #0x1001000f0.
Printer: new, 2 with 1000.000000s/page #0x1001001d0.
Printer: new, 3 with 1000.000000s/page #0x1001001e0.
Printer: new, 4 with 1000.000000s/page #0x1001001f0.
Client: new, Ofooburz (1) #0x100100200.
Client: new, Ulethglob (2) #0x100100230.
Spool: new, jobs 1, printers 4, clients 2, #0x100100080.
Printer 1 waiting.
Printer 2 waiting.
~Spool: begin erasing!
Printer 3 waiting.
Printer 4 waiting.
Job: new, 7 pages for Ofooburz #0x100500040.
Ofooburz has 7 pages to print, puts request in Buffer[0] [0,0]
Job: new, 2 pages for Ulethglob #0x100100260.
Ulethglob has 2 pages to print, but buffer full.
Ofooburz signing off.
No request in buffer, Printer 2 sleeps.
Printer 1 go!
Printer 1 starts printing 7 pages from buffer[0] (from Ofooburz)
No request in buffer, Printer 3 sleeps.
No request in buffer, Printer 4 sleeps.
~Job: erase, print job for Ofooburz, done 7 / 7, #0x100500040.
Printer 1 waiting.
Ulethglob has 2 pages to print, puts request in Buffer[0] [0,0]
Job: new, 7 pages for Ulethglob #0x100500040.
Ulethglob has 7 pages to print, but buffer full.
Printer 1 go!
Printer 1 starts printing 2 pages from buffer[0] (from Ulethglob)
~Job: erase, print job for Ulethglob, done 2 / 2, #0x100100260.
Printer 1 waiting.
Ulethglob has 7 pages to print, puts request in Buffer[0] [0,0]
Job: new, 1 pages for Ulethglob #0x100500060.
Ulethglob has 1 pages to print, but buffer full.
Printer 1 go!
Printer 1 starts printing 7 pages from buffer[0] (from Ulethglob)
~Job: erase, print job for Ulethglob, done 7 / 7, #0x100500040.
Printer 1 waiting.
Ulethglob has 1 pages to print, puts request in Buffer[0] [0,0]
Ulethglob signing off.
Printer 1 go!
Printer 1 starts printing 1 pages from buffer[0] (from Ulethglob)
~Job: erase, print job for Ulethglob, done 1 / 1, #0x100500060.
Printer 1 waiting.
No request in buffer, Printer 1 sleeps.
~Printer: 1 thread return #0x0, erase #0x1001000f0.
~Printer: 2 thread return #0x0, erase #0x1001001d0.
~Printer: 3 thread return #0x0, erase #0x1001001e0.
~Printer: 4 thread return #0x0, erase #0x1001001f0.
~Client: Ofooburz thread return #0x0, erase #0x100100200.
~Client: Ulethglob thread return #0x0, erase #0x100100230.
~Spool: erase, #0x100100080.
Shutting down.
Spool::~semaphores: closing.
