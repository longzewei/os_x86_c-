

BUILD:=../build
SRC:= .

MULTIBOOT2:=0x10000
ENTRYPOINT:=$(shell python3 -c "print(f'0x{$(MULTIBOOT2) + 64:x}')")


CFLAGS:= -m32 # 表示编译为32位的程序
CFLAGS+= -fno-builtin	# 不需要 gcc 内置函数
CFLAGS+= -nostdinc		# 不需要标准头文件
CFLAGS+= -fno-pic		# 不需要位置无关的代码  position independent code
CFLAGS+= -fno-pie		# 不需要位置无关的可执行程序 position independent executable
CFLAGS+= -nostdlib		# 不需要标准库
CFLAGS+= -fno-stack-protector	# 不需要栈保护

CFLAGS+= -fno-use-cxa-atexit # 禁用C析构函数__cxa_atexit
# CFLAGS+= -fleading-underscore # 编译时在C语言符号前加上下划线
# CFLAGS+= -fno-exceptions # 禁用异常机制
# CFLAGS+= -fno-rtti # 禁用运行时类型信息

CFLAGS:=$(strip ${CFLAGS}) # 这是干嘛的??

DEBUG:= -g # 表示需要ddebug信息
INCLUDE:= -I$(SRC)/include



$(BUILD)/boot/%.bin:$(SRC)/boot/%.asm
	# 得到目标文件的目录 $(dir $@)
	$(shell	mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	# 得到目标文件的目录 $(dir $@)
	$(shell	mkdir -p $(dir $@))
	nasm -gdwarf -f elf32 $(DEBUG) $< -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	# 得到目标文件的目录 $(dir $@)
	$(shell	mkdir -p $(dir $@))
	g++ $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

LDFLAGS:= -m elf_i386 \
		-static \
		-Ttext $(ENTRYPOINT)\
		--section-start=.multiboot2=$(MULTIBOOT2) # 然后顺便start_就到了text的第0号字节,因为在一个文件里,遍历算法刚好第一个

		
LDFLAGS:=$(strip ${LDFLAGS}) # strip只是让参数好看

$(BUILD)/kernel.bin: \
	$(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/gdt.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/interrupt_standardlize.o \
	$(BUILD)/kernel/interrupt.o \
	$(BUILD)/kernel/clock.o \
	$(BUILD)/kernel/physical_memory.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/kernel/thread.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/stdlib.o \
	$(BUILD)/lib/list.o \
	
	


	
	

	# 得到目标文件的目录 $(dir $@)
	$(shell	mkdir -p $(dir $@))
	ld $(LDFLAGS) $^ -o $@ 


$(BUILD)/system.bin:$(BUILD)/kernel.bin
	objcopy -O binary $^ $@

$(BUILD)/system.map:$(BUILD)/kernel.bin
	nm $< | sort > $@


$(BUILD)/master.img:$(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map

	# 创建一个 16M 的硬盘镜像
	yes | bximage -q -hd=16M -func=create -sectsize=512 -imgmode=flat $@
	# 将 boot.bin 写入主引导扇区
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	# 将 loader.bin 写入硬盘
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
	# 测试 system.bin 小于 100k，否则需要修改下面的 count
	test -n "$$(find $(BUILD)/system.bin -size -100k)"
	# 将 system.bin 写入硬盘
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

$(BUILD)/slave.img:
	# 创建一个 32M 的硬盘镜像
	yes | bximage -q -hd=32 -func=create -sectsize=512 -imgmode=flat $@

.PHONY: clean
clean:
	rm -rf $(BUILD)

IMAGES:= $(BUILD)/master.img $(BUILD)/slave.img

test:$(IMAGES)

.PHONY: bochs
bochs:$(IMAGES)
	bochs -q -f ../bochs/bochsrc -unlock

# .PHONY: bochsb
# bochsb: $(BUILD)/kernel.iso
# 	bochs -q -f ../bochs/bochsrc.grub -unlock




QEMU:= qemu-system-i386 # 虚拟机
QEMU+= -m 32M # 内存
QEMU+= -audiodev pa,id=hda # 音频设备
# QEMU+= -machine pcspk-audiodev=hda # pcspeaker 设备
QEMU+= -rtc base=localtime # 设备本地时间

QEMU+= -drive file=$(BUILD)/master.img,if=ide,index=0,media=disk,format=raw # 主硬盘
QEMU+= -drive file=$(BUILD)/slave.img,if=ide,index=1,media=disk,format=raw # 从硬盘


QEMU_DISK:=-boot c

# QEMU_CDROM:=-boot -d \
# 	-drive file=$(BUILD)/kernel.iso,media=cdrom \

QEMU_DEBUG:= -s -S 


.PHONY: qemu
qemu: $(IMAGES)
	$(QEMU) \
	$(QEMU_DISK) 

.PHONY: qemug
qemug: $(IMAGES)
	$(QEMU) \
	$(QEMU_DISK) \
	$(QEMU_DEBUG) 

# .PHONY: qemubg
# qemubg: $(BUILD)/kernel.iso
# 	$(QEMU) \
# 	$(QEMU_CDROM) \
# 	$(QEMU_DEBUG)
	
# .PHONY: qemub
# qemub: $(BUILD)/kernel.iso
# 	$(QEMU) $(QEMU_CDROM)

