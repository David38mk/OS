KDIR ?= /lib/modules/$(shell uname -r)/build
obj-m += 01_linear.o 02_drain_flag.o 03_ring.o 04_offp_catsafe.o

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
