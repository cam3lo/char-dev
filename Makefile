obj-m += chardev.o

KDIR = /usr/src/linux-headers-4.4.0-97-generic


all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order
