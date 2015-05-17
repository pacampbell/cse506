CC=gcc
# CC=clang
AS=as
DEBUG=
# DEBUG = -DDEBUG
CFLAGS=-O1 -g -std=c99 -D__thread= -Wall -Werror -nostdinc -Iinclude -msoft-float -mno-sse -mno-red-zone -fno-builtin -fPIC -march=amdfam10 -g3 -fno-stack-protector $(DEBUG)
LD=ld
LDLAGS=-nostdlib
AR=ar

ROOTFS=rootfs
ROOTBIN=$(ROOTFS)/bin
ROOTLIB=$(ROOTFS)/lib
ROOTBOOT=$(ROOTFS)/boot

KERN_SRCS:=$(wildcard sys/*.c sys/*.s sys/*/*.c sys/*/*.s)
BIN_SRCS:=$(wildcard bin/*/*.c)
LIBC_SRCS:=$(wildcard libc/*.c libc/*/*.c)
INCLUDES:=$(shell find include/ -type f -name *.h)
BINS:=$(addprefix $(ROOTFS)/,$(wildcard bin/*))

.PHONY: all binary

all: $(USER).iso $(USER).img

$(USER).iso: kernel
	cp kernel $(ROOTBOOT)/kernel/kernel
	mkisofs -r -no-emul-boot -input-charset utf-8 -b boot/cdboot -o $@ $(ROOTFS)/

$(USER).img: newfs.506
	qemu-img create -f raw $@ 16M
	./newfs.506 $@

newfs.506: $(wildcard newfs/*.c)
	$(CC) -o $@ $^

kernel: $(patsubst %.s,obj/%.asm.o,$(KERN_SRCS:%.c=obj/%.o)) obj/tarfs.o
	$(LD) $(LDLAGS) -o $@ -T linker.script $^

obj/tarfs.o: $(BINS)
	tar --format=ustar -cvf tarfs --no-recursion -C $(ROOTFS) $(shell find $(ROOTFS)/ -name boot -prune -o ! -name .empty -printf "%P\n")
	objcopy --input binary --binary-architecture i386 --output elf64-x86-64 tarfs $@
	@rm tarfs

$(ROOTLIB)/libc.a: $(LIBC_SRCS:%.c=obj/%.o)
	$(AR) rcs $@ $^

$(ROOTLIB)/crt1.o: obj/crt/crt1.o
	cp $^ $@

$(BINS): $(ROOTLIB)/crt1.o $(ROOTLIB)/libc.a $(shell find bin/ -type f -name *.c) $(INCLUDES)
	@$(MAKE) --no-print-directory BIN=$@ binary

binary: $(patsubst %.c,obj/%.o,$(wildcard $(BIN:rootfs/%=%)/*.c))
	$(LD) $(LDLAGS) -o $(BIN) $(ROOTLIB)/crt1.o $^ $(ROOTLIB)/libc.a

obj/%.o: %.c $(INCLUDES)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<

obj/%.asm.o: %.s
	@mkdir -p $(dir $@)
	$(AS) -o $@ $<

obj/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) -o $@ $<

.PHONY: submit clean tags run

SUBMITTO:=~mferdman/cse506-submit/

submit: clean
	tar -czvf $(USER).tgz --exclude=.empty --exclude=.*.sw? --exclude=*~ LICENSE README Makefile linker.script sys bin crt libc newfs include $(ROOTFS)
	@gpg --quiet --import cse506-pubkey.txt
	gpg --yes --encrypt --recipient 'CSE506' $(USER).tgz
	rm -fv $(SUBMITTO)$(USER)=*.tgz.gpg
	cp -v $(USER).tgz.gpg $(SUBMITTO)$(USER)=`date +%F=%T`.tgz.gpg

clean:
	find $(ROOTLIB) $(ROOTBIN) -type f ! -name .empty -print -delete
	rm -rfv obj kernel newfs.506 $(ROOTBOOT)/kernel/kernel $(USER).iso $(USER).img

tags:
	find . -name "*.[chw]" > cscope.files
	ctags -R *
	cscope -b -q -k

run: all
	qemu-system-x86_64 -serial file:debug.log -curses -cdrom $(USER).iso -drive id=disk,file=$(USER).img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -net nic -net user,hostfwd=tcp::10080-:80 -net user,hostfwd=tcp::10023-:23 --no-reboot -gdb tcp::9998
