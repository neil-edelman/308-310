2014 Neil Edelman.
neil dot edelman each mail dot mcgill dot ca

Version 1.1.

Usage: <none>

This is a threads 'library.' It is contained in Threads.c and Threads.h. A Sample.c is provided to link against the threads. It uses ucontext, which was removed from the posix std in 2008; it's therefore in the air whether this will run properly in future systems. Make with make; creates bin/Sample. Below are three different OSs.

It now has tested semaphores and an int argument for each of the threads. Then I implemented a doubly-linked-list and it broke a few things (it's asymptotically faster!) These are weak semaphores; it was faster. Collatz.c is using the programme to do useful computation; but why? I am going to bed; you can ignore it.

------------------------------

Darwin wpa145160.wireless.mcgill.ca 10.8.0 Darwin Kernel Version 10.8.0: Tue Jun  7 16:33:36 PDT 2011; root:xnu-1504.15.3~1/RELEASE_I386 i386

thor:a3 neil$ bin/Sample 
Threads is not active.

Threads: new, #0x100100080.
Initial state:
unique address	thread state	cpu time	name

Thread: new, "foo" #0x100801000 (#0x100801008 next #0x100801308.)
unique address	thread state	cpu time	name
#0x100801000  	RUNNING     	0       	foo

Thread: new, "bar" #0x100809400 (#0x100809408 next #0x100809708.)
Thread: new, "baz" #0x100811800 (#0x100811808 next #0x100811b08.)
Thread: new, "qux" #0x100819c00 (#0x100819c08 next #0x100819f08.)
unique address	thread state	cpu time	name
#0x100819c00  	RUNNING     	0       	qux
#0x100811800  	RUNNING     	0       	baz
#0x100809400  	RUNNING     	0       	bar
#0x100801000  	RUNNING     	0       	foo

Now we run:
Loop 1.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
Segmentation fault <-- ucontext has been removed on Darwin

--------------------------------------

Linux teaching 3.2.0-29-generic-pae #46-Ubuntu SMP Fri Jul 27 17:25:43 UTC 2012 i686 i686 i386 GNU/Linux

nedelm@teaching:~/comp-310/a3> bin/Sample
Threads is not active.

Threads: new, #0x9092008.
Initial state:
unique address	thread state	cpu time	name

Thread: new, "foo" #0x90922b8 (#0x90922bc next #0x9092418.)
unique address	thread state	cpu time	name
#0x90922b8  	RUNNING     	0       	foo

Thread: new, "bar" #0x909a2c8 (#0x909a2cc next #0x909a428.)
Thread: new, "baz" #0x90a22d8 (#0x90a22dc next #0x90a2438.)
Thread: new, "qux" #0x90aa2e8 (#0x90aa2ec next #0x90aa448.)
unique address	thread state	cpu time	name
#0x90aa2e8  	RUNNING     	0       	qux
#0x90a22d8  	RUNNING     	0       	baz
#0x909a2c8  	RUNNING     	0       	bar
#0x90922b8  	RUNNING     	0       	foo

Now we run:
Loop 1.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 2.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 3.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 4.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
foo! #0x90922b8<foo>
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
timer_callback: swap(foo, parent).
Loop 5.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 6.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 7.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 8.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 9.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 10.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux! #0x90aa2e8<qux>
timer_callback: swap(qux, baz).
baz! #0x90a22d8<baz>
timer_callback: swap(baz, bar).
bar! #0x909a2c8<bar>
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 11.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux#0x90aa2e8).
qux exit!
destory_callback: <qux> chilling on run queue.
timer_callback: swap(qux, baz).
baz exit!
destory_callback: <baz> chilling on run queue.
timer_callback: swap(baz, bar).
bar exit!
destory_callback: <bar> chilling on run queue.
timer_callback: swap(bar, foo).
foo! #0x90922b8<foo>
timer_callback: swap(foo, parent).
Loop 12.
Threads::run: list <qux:cleanup><baz:cleanup><bar:cleanup><foo:running>.
Threads::run: swap(parent, foo#0x90922b8).
foo exit!
destory_callback: <foo> chilling on run queue.
timer_callback: swap(foo, parent).
Loop 13.
Threads::run: list <foo:cleanup>.
It works.

Don't forget exit cleanup.
~Threads: freeing thread #0x90922b8 from graveyard.
~Threads: freeing thread #0x909a2c8 from graveyard.
~Threads: freeing thread #0x90a22d8 from graveyard.
~Threads: freeing thread #0x90aa2e8 from graveyard.
~Threads: erase, #0x9092008.

-----------------------------------

Linux linux 2.6.32-5-686 #1 SMP Mon Sep 23 23:00:18 UTC 2013 i686 GNU/Linux

nedelm@linux:~/comp-310/a3> bin/Sample
Threads is not active.

Threads: new, #0x8ffd008.
Initial state:
unique address	thread state	cpu time	name

Thread: new, "foo" #0x8ffd218 (#0x8ffd21c next #0x8ffd378.)
unique address	thread state	cpu time	name
#0x8ffd218  	RUNNING     	0       	foo

Thread: new, "bar" #0x9005228 (#0x900522c next #0x9005388.)
Thread: new, "baz" #0x900d238 (#0x900d23c next #0x900d398.)
Thread: new, "qux" #0x9015248 (#0x901524c next #0x90153a8.)
unique address	thread state	cpu time	name
#0x9015248  	RUNNING     	0       	qux
#0x900d238  	RUNNING     	0       	baz
#0x9005228  	RUNNING     	0       	bar
#0x8ffd218  	RUNNING     	0       	foo

Now we run:
Loop 1.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 2.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 3.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 4.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 5.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 6.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
timer_callback: swap(qux, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, bar).
baz! #0x900d238<baz>
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 7.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x8ffd218<foo>
timer_callback: swap(qux, baz).
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
timer_callback: swap(foo, parent).
Loop 8.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 9.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 10.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
timer_callback: swap(baz, bar).
bar! #0x9005228<bar>
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 11.
Threads::run: list <qux:running><baz:running><bar:running><foo:running>.
Threads::run: swap(parent, qux).
qux! #0x9015248<qux>
timer_callback: swap(qux, baz).
timer_callback: swap(baz, bar).
bar exit!
destory_callback: set exit flag.
destory_callback: chilling on run queue.
timer_callback: swap(bar, foo).
foo! #0x8ffd218<foo>
timer_callback: swap(foo, parent).
Loop 12.
Threads::run: list <qux:running><baz:running><bar:cleanup><foo:running>.
Threads::run: swap(parent, qux).
qux exit!
destory_callback: set exit flag.
destory_callback: chilling on run queue.
timer_callback: swap(qux, baz).
timer_callback: swap(baz, foo).
foo exit!
destory_callback: set exit flag.
destory_callback: chilling on run queue.
timer_callback: swap(foo, parent).
Loop 13.
Threads::run: list <qux:cleanup><baz:running><foo:cleanup>.
Threads::run: swap(parent, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, parent).
timer_callback: missing active context. <-- recovery from bug :[
Loop 14.
Threads::run: list <baz:running>.
Threads::run: swap(parent, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, parent).
Loop 15.
Threads::run: list <baz:running>.
Threads::run: swap(parent, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, parent).
Loop 16.
Threads::run: list <baz:running>.
Threads::run: swap(parent, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, parent).
Loop 17.
Threads::run: list <baz:running>.
Threads::run: swap(parent, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, parent).
Loop 18.
Threads::run: list <baz:running>.
Threads::run: swap(parent, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, parent).
Loop 19.
Threads::run: list <baz:running>.
Threads::run: swap(parent, baz).
baz! #0x900d238<baz>
timer_callback: swap(baz, parent).
Loop 20.
Threads::run: list <baz:running>.
Threads::run: swap(parent, baz).
baz exit!
destory_callback: set exit flag.
destory_callback: chilling on run queue.
timer_callback: swap(baz, parent).
Loop 21.
Threads::run: list <baz:cleanup>.
It works.

Don't forget exit cleanup.
~Threads: freeing thread #0x900d238 from graveyard.
~Threads: freeing thread #0x8ffd218 from graveyard.
~Threads: freeing thread #0x9015248 from graveyard.
~Threads: freeing thread #0x9005228 from graveyard.
~Threads: erase, #0x8ffd008.
