/*
    System call example

    In this tutorial I'll show you how to implement a system call in Linux and how 
    to call it from the user space from a C++ program. First I'll show you how the
    parameters are passed to system call via the registers and how it's ultimately
    called in extended inline assembly and after that an alternative way where an 
    assembly function is declared in the cpp file and the assembly code is moved 
    to a separate file. Finally I'll show the conventional way to call the system
    call using the sycall() library function. 

    The system call that we are about to implement is a simple one. It takes 3
    parameters; 2 integers and a pointer to a third integer. First it checks
    whether the sum of the 2 first integers is going to overflow or not. If it 
    overflows -1 is returned and stored in the rax register. If it doesn't 
    overflow, it adds those 2 integers together and copies the sum to the third 
    integer using the copy_to_user() function which performs a number of checks 
    before proceeding to the copying. One of the things it checks is whether the 
    C++ program is allowed to write to the integer pointed by the third parameter. 
    If not, a non positive value is returned back to the system call which in turn 
    returns -2. That's it!

    I implemented this system call in a virtual machine (Oracle virtualbox 6.0.8)
    with Ubuntu LTS 18.04 64 bit installed 

    1.  Get a hold of the Linux source code

        a)  Go to www.kernel.org and copy the address of the source code tarball.
            I'm going to go with the linux-5.2.5 and use it henceforth.

            open the terminal and enter:                                     
                                                                             */
            wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.2.5.tar.xz
                                                                             /*
        b)  Extract the kernel source by entering:
                                                                             */
            sudo tar -xvf linux-5.2.5.tar.gz -C/usr/src/
                                                                             /*
    2.  Add the implementation of the system call

        /usr/src/linux-5.2.5/kernel/add_syscall.c:                           
                                                                             */
            #include <linux/syscalls.h> // SYCALL_DEFINEx
            #include <asm/uaccess.h>    // copy_to_user()

            SYSCALL_DEFINE3(add_syscall, int, a, int, b, int*, c){
                int sum;
                if(a > 2147483647 - b){
                    return -1;
                }
                sum = a + b;
                if(copy_to_user(c, &sum, sizeof(sum))){
                    return -2;
                }
                return 0;
            }                                                                /*

    3.  Configure the Makefile to compile and link the system call

        /usr/src/linux-5.2.5/kernel/Makefile:                                
                                                                             */
            obj-y = fork.o exec_domain.o panic.o \
            cpu.o exit.o softirq.o resource.o \
            sysctl.o sysctl_binary.o capability.o ptrace.o user.o \
            signal.o sys.o umh.o workqueue.o pid.o task_work.o \
            extable.o params.o \
            kthread.o sys_ni.o nsproxy.o \
            notifier.o ksysfs.o cred.o reboot.o \
            async.o range.o smpboot.o ucount.o \
            add_syscall.o                                                    
                                                                             /*
    4.  Add the protoype for the system call

        Add the decleration at the end of the following file

        /usr/src/linux-5.2.5/include/linux/syscalls.h:                       
                                                                             */
            ...

            asmlinkage long add_syscall(int, int, int*);

            #endif
                                                                             /*
    5.  Add the system call to the system call table

        Pick the next free number that's going to be associated with the system call

        /usr/src/linux-5.2.5/arch/x86/entry/syscalls/syscall_64.tbl:         
                                                                             */
            335 common ass_syscall __x64_sys_add_syscall
                                                                             /*
            REMEMBER THE NUMBER!!!

    6.  Install packages needed for building the kernel
                                                                             */
            sudo apt-get update
            sudo apt-get install libncurses5-dev
            sudo apt-get install libelf-dev
            sudo apt-get install libssl-dev
            sudo apt-get install bison
            sudo apt-get install flex
            sudo apt-get install make
            sudo apt-get install gcc
                                                                             /*
    7.  Change your current working directory
                                                                             */
            cd /usr/src/linux-5.2.5
                                                                             /*
    8.  Configure the kernel
                                                                             */
            sudo make menuconfig
                                                                             /*
            Exit and save
            
    9.  Build the kernel
                                                                             */
            sudo make -jn
                                                                             /*
            n indicates the number of cores used for building the kernel. 
            If you are using a virtual machine you might want to leave 1 
            core for the host. I allocated 3 cores and 6000 gb of ram for 
            the virtual machine and it took about 1 1/2 hour to build.

    10. Installs the modules
                                                                             */
            sudo make modules_install -jn
                                                                             /*
    11. Make the kernel bootable                                               
                                                                             */
            sudo make install -jn
                                                                             /*
    12. Reboot
                                                                             */
            sudo shutdown -r now
                                                                             /*
    12: Write the following assembly file
                                                                             
        add_syscall.s:
                                                                             */
            .globl addition_syscall
            .text
            add_syscall:
            mov $335, %rax
            syscall
            ret
                                                                             /*
    13. Write the C++ file and assembly file (below)

    14. Install the GNU C++ compiler
                                                                             */
            sudo apt-get install g++
                                                                             /*
    15. Compile and run: 
                                                                             */
            g++ example.cpp add_syscall.s && ./a.out
                                                                             
// add_syscall.s:

    .global add_syscall
    .text
add_syscall:
    mov $335, %rax
    syscall
    ret

// example.cpp:

#include <iostream>
#include <unistd.h>

using namespace std;

// system call decleration
extern "C" long int add_syscall(int, int, int*);

// prints the output
void output(int &oper1, int &oper2, int &sum, long int &amma){
    switch(amma){
        case 0:
            cout << oper1 << " + " << oper2 << " = " << sum << endl;
            break;
        case -1:
            cout << oper1 << " + " << oper2 << " = " << " overflow" << endl;
            break;
        case -2: 
            cout << "bad destination address" << endl;
    }
}

int main(){

    int oper1 = 2147483647, oper2 = 3;
    int sum; 
    long int amma; // return value, 0 = ok, -1 == overflow, -2 = bad address 

    /* extended inline assembly
       The order in which the parameters are passed to the system call in x86_64:
       %rdi %rsi, %rdx, %rcx, %r8 and %r9. If the system call takes more than 6
       arguments, then 1 register is used for storing a pointer to the user space
       where the rest of parameters are located. */
    __asm__("mov $335, %%rax;" 
            "syscall;"                               
            : "=a" (amma)                            // output operands                    
            : "D" (oper1), "S" (oper2), "d" (&sum)); // input operands

    output(oper1, oper2, sum, amma);

    // assembly wrapper function
    amma = add_syscall(oper1, oper2, &sum);
    output(oper1, oper2, sum, amma);

    // conventional way (wrapper function)
    amma = syscall(335, oper1, oper2, &sum);
    output(oper1, oper2, sum, amma);

    return 0;
}
