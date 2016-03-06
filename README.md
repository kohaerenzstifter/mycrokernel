# mycrokernel

## Background

About 10 years back I was still at the college studying for my language degree and minoring in computer science.
I was already done with the CS part of my course but wanted to keep engaged in CS, so I decided I'd now do what
most interested me personally. I had always been curious to learn how the computer works at the lowest level and find out
if I'd be able to create code that runs on the bare metal, without all the abstraction layers and OS support that programmers
oftentimes just take for granted without worrying how these actually accomplish their task.
I had learned at the college that to code on the machine level, I would need to learn assembly. I had already done some assembly
in the CS lab, but those were just little tasks, and I felt it wasn't enough to get a grasp of what intrigued me so much.
After some googling, I came across a project called [High Level Assembly](http://www.plantation-productions.com/Webster/) written
by CS lecturer [Randall Hyde](https://en.wikipedia.org/wiki/Randall_Hyde), who had also written a book about all this called
[The Art of Assembly Language](http://www.amazon.de/The-Assembly-Language-Randall-Hyde/dp/1593272073), which I devoured in the
hope it would shed light on the topic. And well, it did, but rather than answering my most pressing questions, it helped me to
get the practice in assembly necessary to write great parts of my microkernel, therefore the name _mycrokernel_.
So with these necessary skills in assembly language, I came to the conclusion that what I needed to learn about in order to run
code on the bare metal, I would need to read up on OS design. Yes, I'd seen OS design at the college, but only from a very
abstract high level point of view, which left too many questions unanswered. What I thought I needed was a book that'd really
teach me to _write_ an OS from the ground up, and it turns out, the one I found is still the only one decent book on this topic
to this day: [The MINIX book](http://www.amazon.de/Operating-Systems-Implementation-Prentice-Software/dp/0131429388/)
(funny to see it's currently priced at more than 200 €, given the fact that I purchased it back in 2006 at only around 70 €).
At first I thought it was a bad buy, because I felt I didnt understand anything. In fact I read the first chapter several times
and simultanously searched the internet for tutorials and forums on the topic until the penny finally dropped.
Apart from that, I also needed to read up on bootstrapping and ask many silly questions on
[The OS development forum](http://forum.osdev.org/) before I could finally start coding _mycrokernel_. As I had recently learnt HLA,
I was curious whether it could actually be used to write the assembly part of my would-be OS. That's also why I first called it
HL-OS.
The end of the story is that after several months of scratching my head and coding, I managed to turn mycrokernel into a
rudimentary operating system that does the following:

* boot from a virtual floppy disk
* relocate itself and enter protected mode
* set up the environment necessary to be able to schedule several processes simultaneously
* create a TTY process to catch keyboard scancodes and write to the screen
* create a mock shell to read input from the user, count the number of characters read and
  spit out a message "got message containing n characters"

It may also be interesting to point out here that I've realised all this in a real microkernel way, as I had learnt from the
MINIX book mentioned before. This means that the TTY driver is not allowed to directly access the hardware but must communicate
with the kernel and ask for help (as in "send me a message when an interrupt occurs" or "read so many bytes from that port",
etc).

I more or less left off at that point. Years later I wanted to get back to it and implement some file system functionality but
didn't get very far, as I later had other things in mind. I created a disk image file and hooked it up to my
[bochs](http://bochs.sourceforge.net/) as an IDE ATA volume. I also created a new process to identify the drive and read the
Ext filesystem entries in the root inode. But that's about it.

As I am currently on the lookout for a new job and would like to get into hardware level programming, I thought it would be a
good idea add some documentation this sadly orphaned project of mine. I know the code will be hard to read for any outsider, as
many things made sense in my head but were never documented. I also know that the code is badly-groomed and contains legacy
stuff that was later replaced by other functionality and is now probably dead. Maybe at some point in time I will try to again
make sense of what I did and clean everything up a bit. But don't count on that!

At the very least I'll add some documentation on how to set your environment in order to compile and run ...
environment in order to get mycrokernel running and see
