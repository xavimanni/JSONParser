#include <stdio.h>
#include <stdlib.h>
#include "json.h"

int main() {

	//read testing json
	FILE* f = fopen("test.json", "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	rewind(f);
	char* string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	string[fsize] = 0;
	fclose(f);
	
	JSONObject* json = json_parse(string);
	printf(json_get_string(json_get_object(json, "object"), "anotherString"));

	return 0;
}