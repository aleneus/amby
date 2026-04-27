build:
	gcc -o amby src/amby.c src/beep.c -lasound

todo:
	@grep "TODO now" -rn src || true
