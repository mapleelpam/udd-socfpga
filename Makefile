CC=/opt/altera-linux/linaro/gcc-linaro-arm-linux-gnueabihf-4.7-2012.11-20121123_linux/bin/arm-linux-gnueabihf-gcc


CFLAGS=-O3

ALL_TARGETS= led_test ocm_test ocm_test_w64 TestFunction pc ocm_test_asm_o3.s ocm_test_asm.s ocm_test_O3 pc_ocm TestFunction2

ALL: ${ALL_TARGETS}

led_test: led_test.c

ocm_test: ocm_test.c
	${CC} -o $@ $< 

ocm_test_O3: ocm_test.c
	${CC} -o $@ $< -O3

ocm_test_w64: ocm_test_w64.c

ocm_test_asm_o3.s: ocm_test.c
	${CC} -o $@ $< -S -O3 -fverbose-asm

ocm_test_asm.s: ocm_test.c
	${CC} -o $@ $< -S  -fverbose-asm

TestFunction: TestFunction.c

TestFunction2: TestFunction2.c

pc_ocm: pc_ocm.c
	${CC} -o $@ $< -lpthread

pc: pc.c
	${CC} -o $@ $< -lpthread

copy: led_test ocm_test
	cp led_test ocm_test /tmp/

clean:
	rm -f ${ALL_TARGETS}

.PHONY: ALL
