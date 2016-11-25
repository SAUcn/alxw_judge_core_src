all: 
	gcc src/main.c src/cjson/cJSON.c -lm -static -o runner
