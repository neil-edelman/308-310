2014 Neil Edelman; released into public-domain.
neil dot edelman each mail dot mcgill dot ca

Usage: PrinterSimulation [-c <clients>] [-p <printers>] [-b <buffer>] [-h(elp)]
Version 1.0.

Hint: direct stderr to null to see the big picture.

PrinterSimulation 2014 Neil Edelman
This program comes with ABSOLUTELY NO WARRANTY.

This is a sample:

wpa147019:a2 neil$ bin/PrinterSimulation 2>/dev/null
Iongnbubhosh has 3 pages to print, puts request in Buffer[0] [0,1]
Ultoolog has 5 pages to print, puts request in Buffer[1] [0,2]
Allkhamhai has 1 pages to print, puts request in Buffer[2] [0,0]
Argohk-glob has 7 pages to print, but buffer full.
Printer 1 starts printing 3 pages from buffer[0] (from Iongnbubhosh)
Printer 2 starts printing 5 pages from buffer[1] (from Ultoolog)
Argohk-glob has 7 pages to print, puts request in Buffer[0] [2,1]
Printer 1 starts printing 1 pages from buffer[2] (from Allkhamhai)
Printer 2 starts printing 7 pages from buffer[0] (from Argohk-glob)

wpa147019:a2 neil$ bin/PrinterSimulation 
Spool::semaphores: opening.
Printer: new, 1 with 1000.000000s/page #0x100100100.
Printer: new, 2 with 1000.000000s/page #0x1001001e0.
Client: new, Eevakglob (1) #0x1001001f0.
Client: new, Oukhamghash (2) #0x100100220.
Client: new, Orcuukash (3) #0x100100250.
Client: new, Ithurukbag (4) #0x100100280.
Spool: new, jobs 3, printers 2, clients 4, #0x100100080.
~Spool: begin erasing!
Printer 2 waiting.
Job: new, 1 pages for Eevakglob #0x1001002b0.
Eevakglob has 1 pages to print, puts request in Buffer[0] [0,1]
Eevakglob signing off.
Job: new, 1 pages for Oukhamghash #0x100500000.
Oukhamghash has 1 pages to print, puts request in Buffer[1] [0,2]
Job: new, 1 pages for Orcuukash #0x100500020.
Orcuukash has 1 pages to print, puts request in Buffer[2] [0,0]
Job: new, 10 pages for Ithurukbag #0x100500040.
Ithurukbag has 10 pages to print, but buffer full.
Printer 1 waiting.
Oukhamghash signing off.
Orcuukash signing off.
Printer 2 go!
Printer 2 starts printing 1 pages from buffer[0] (from Eevakglob)
Printer 1 go!
Printer 1 starts printing 1 pages from buffer[1] (from Oukhamghash)
~Job: erase, print job for Eevakglob, done 1 / 1, #0x1001002b0.
Printer 2 waiting.
Ithurukbag has 10 pages to print, puts request in Buffer[0] [2,1]
Ithurukbag signing off.
~Job: erase, print job for Oukhamghash, done 1 / 1, #0x100500000.
Printer 1 waiting.
Printer 2 go!
Printer 2 starts printing 1 pages from buffer[2] (from Orcuukash)
Printer 1 go!
Printer 1 starts printing 10 pages from buffer[0] (from Ithurukbag)
~Job: erase, print job for Orcuukash, done 1 / 1, #0x100500020.
Printer 2 waiting.
Printer 2: exiting; no jobs.
~Job: erase, print job for Ithurukbag, done 10 / 10, #0x100500040.
Printer 1 waiting.
Printer 1: exiting; no jobs.
~Printer: 1 thread return #0x0, erase #0x100100100.
~Printer: 2 thread return #0x0, erase #0x1001001e0.
~Client: Eevakglob thread return #0x0, erase #0x1001001f0.
~Client: Oukhamghash thread return #0x0, erase #0x100100220.
~Client: Orcuukash thread return #0x0, erase #0x100100250.
~Client: Ithurukbag thread return #0x0, erase #0x100100280.
~Spool: erase, #0x100100080.
Spool::~semaphores: closing.

There is a lot of information.
