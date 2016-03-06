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

It may also be interesting to point out here that I've realised all this in a real microkernel fashion, as I had learnt from the
MINIX book mentioned before. This means that the TTY driver is not allowed to directly access the hardware but must communicate
with the kernel and ask for help (as in "send me a message when an interrupt occurs" or "read so many bytes from that port",
etc).

I more or less left off at that point. Years later I wanted to get back to it and implement some file system functionality but
didn't get very far, as I later had other things in mind. I created a disk image file and hooked it up to my
[bochs](http://bochs.sourceforge.net/) as an IDE ATA volume. I also created a new process to identify the drive and read the
Ext filesystem entries in the root inode. But that's about it.

As I am currently on the lookout for a new job and would like to get into hardware level programming, I thought it would be a
good idea to add some documentation on this sadly orphaned project of mine. I know the code will be hard to read for any outsider, as
many things made sense in my head back then but were never documented. I also know that the code is badly-groomed and contains legacy
stuff that was later replaced by other functionality and is now probably dead. Maybe at some point in time I will try to again
make sense of what I did and clean everything up a bit. But don't count on that!

For now, at the very least I'll add some documentation on how to set your environment in order to compile and run, which shall
also be a help for myself, as I myself struggle every time I try to get it to work after years.

## How to run mycrokernel

### I'll first summarise what we'll be doing:

* Create a virtual machine with an i386-compatible CPU and a virgin 32-Bit [Lubuntu 14.04](http://cdimages.ubuntu.com/lubuntu/releases/trusty/release/lubuntu-14.04.4-desktop-i386.iso). If you prefer a different
setup, of course, your mileage may vary
* the [High Level Assembler](http://www.plantation-productions.com/Webster/) by [Randall Hyde](https://en.wikipedia.org/wiki/Randall_Hyde): You won't be able to install from the repositories, but I'll explain how to
set it up
* the [Bochs emulator](http://bochs.sourceforge.net/): Unfortunately, mycrokernel seems to be broken on a current version of bochs installed from the repositories. I found version 2.3 still to be working, so you have to compile and install manually, but I'll also explain how
* Simple DirectMedia Layer Development files: Needed to build Bochs
* the NASM assembler: When writing an OS to boot from the bare metal, you also need to write a tiny little part of assembly in 16-bit Real Mode. This can't be done with HLA, so you'll also need NASM, which can be installed from  the repositories
* Subversion: Needed to checkout Bochs
* GIT: Needed to checkout mycrokernel
* build-essential: Everything and the kitchen sink ...

### And here's what you'll need to do:

#### Install required packages from the repository:

> sudo apt-get install build-essential nasm git subversion libsdl1.2-dev

#### Install the [High Level Assembler](http://www.plantation-productions.com/Webster/):

> mkdir hla  
> cd hla  
> wget http://www.plantation-productions.com/Webster/HighLevelAsm/HLAv2.16/linux.hla.tar.gz  
> gzip -d linux.hla.tar.gz  
> tar xvf linux.hla.tar  
> sudo cp -r usr/hla /usr/  
> cd ..  
> rm -rf hla  
> vi ~/.bashrc  

At the end of your .bashrc, add the following lines:

> PATH=/usr/hla:$PATH  
> hlalib=/usr/hla/hlalib  
> export hlalib  
> hlainc=/usr/hla/include  
> export hlainc  
> hlatemp=/tmp  
> export hlatemp  

Now do:

> source ~/.bashrc

#### Checkout, compile and install [Bochs](http://bochs.sourceforge.net/):

> svn co http://svn.code.sf.net/p/bochs/code/tags/REL_2_3_FINAL/bochs bochs  
> cd bochs  
> ./configure --enable-cpu-level=6 --enable-x86-debugger --enable-cdrom --enable-all-optimizations --enable-readline --enable-disasm --with-sdl  
> make  
> sudo make install  

#### Clone, compile and run:

> git clone https://github.com/kohaerenzstifter/mycrokernel.git mycrokernel  
> cd mycrokernel/  
> make bochs

#### You' first see this screen:
![1](https://github.com/kohaerenzstifter/mycrokernel/blob/master/1.png)
The messages "Relocated myself." and "Successfully loaded GDT!" already come from the the Real Mode bootloader. I added some busy loop into the kernel so you actually get a chance to see these messages and get a better feel of what is going on.

#### Next thing you'll see is this:
![1](https://github.com/kohaerenzstifter/mycrokernel/blob/master/2.png)

These are already messages from the kernel. It's showing information on the memory regions available in the system as obtained from the BIOS. The second part explains how the kernel calculates the space that's available to kernel (everything that's left below 1 MByte after subtracting the sizes of all of the different parts of the loaded image).  
The third part then documents how three processes were created and set up to run as soon as the scheduler kicks in.

#### Now you'll see this:
![1](https://github.com/kohaerenzstifter/mycrokernel/blob/master/3.png)

As mentioned earlier, I later implemented a small dummy hard disk driver process that gets a chance to speak up here. It tells you some of the details of the (virtual) drive it found and spits out the content of the root inode of the Ext filesystem contained therein. Then the "driver" exits, also to show off how a process can gracefully end when it's work has been done.

#### You're now left with this:
![1](https://github.com/kohaerenzstifter/mycrokernel/blob/master/4.png)

This is still basically the same screen, but do try to type in something with your keyboard and hit enter. What you type here is sent to the shell process, which just sleeps waiting for anything you've got to say. It'll count the number of characters you enter and spit out a short message.

I'll stop writing for now, I think you got the basic idea. Comments are welcome, of course ...
