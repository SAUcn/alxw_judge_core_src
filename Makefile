all: 
	gcc src/main.c src/cjson/cJSON.c -lm -o runner
