# Compile module under Linux 2.6.x using something like:
# make -C /lib/modules/`uname -r`/build SUBDIRS=$PWD modules

obj-m += ioctl_module.o

all:
	make -C /lib/modules/`uname -r`/build SUBDIRS=${PWD} modules

sync:
	rsync -a longluu2@csa1.bu.edu:/home/ugrad/longluu2/cs552/primer /root/primer
	touch Makefile

clean:
	make -C /lib/modules/`uname -r`/build SUBDIRS=${PWD} clean
