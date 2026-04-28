.PHONY: build

build:
	mkdir -p build

	gcc -o build/amby -pthread -lasound -lm \
		src/amby.c \
		src/synth.c \
		src/channel.c \
		src/mixer.c \
		src/threads.c

todo:
	@grep "TODO now" -rn src || true
