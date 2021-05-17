fuzzer : fuzzer.c
ifdef extractor
	gcc -o fuzzer -m64 fuzzer.c
	./fuzzer $(extractor)
else
	gcc -o fuzzer -m64 fuzzer.c
endif
