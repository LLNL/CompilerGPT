
ifneq ($(CLANG),)
  CC=$(CLANG)
endif

.PHONY: setup
setup:
	mkdir -p ./bin
	make CC=$(CC) -C ./sys -f compilergpt-setup.mk
	make CC=$(CC) -C ./common -f compilergpt-setup.mk
	make CC=$(CC) -C ./BT -f compilergpt-setup.mk

.PHONY: clean
clean:
	make clean -C ./sys -f compilergpt-setup.mk
	make clean -C ./common -f compilergpt-setup.mk
	make clean -C ./BT -f compilergpt-setup.mk
