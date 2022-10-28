#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#pragma warning(disable : 4996)

typedef struct JSONObject JSONObject;
typedef struct JSONValue JSONValue;

#define JSON_STRING 1
#define JSON_OBJECT 0
#define JSON_NUMBER 2
#define JSON_BOOL 3


struct JSONObject {
	JSONValue** values;
	int length;
};

struct  JSONValue {
	char* key;
	int value_type;
	char* stringValue;
	double numberValue;
	bool boolValue;
	JSONObject* objectValue;
};





int countKeys(char* string) {
	int count = 0;
	bool inString = false;
	bool inObject = false;
	bool started = false;
	//count keys
	if (string[0] == '[') {
		if (string[1] == ']') {
			return count;
		}
		else {
			count = 1;
			for (int i = 0; string[i] != 0; i++) {
				if (string[i] == '[') started = true;
				if (string[i] == '{') {
					if (started == true && inObject == false && inString == false) inObject = true;
				}
				if (string[i] == '}') {
					if (started == true && inObject == true && inString == false) {
						inObject = false;
					}
					else if (inObject == false && inString == false) started = false;
				}

				if (string[i] == '"') {
					if (inObject == false) {
						if (inString == false) inString = true;
						else inString = false;
					}
				}
				if (string[i] == ',' && inString == false && inObject == false) {
					count++;
				}
			}
		}
	}
	else {
		for (int i = 0; string[i] != 0; i++) {
			if (string[i] == '{') {
				if (started == true && inObject == false && inString == false) inObject = true;
				started = true;
			}
			if (string[i] == '}') {
				if (started == true && inObject == true && inString == false) inObject = false;
				else if (inObject == false && inString == false) started = false;
			}

			if (string[i] == '"') {
				if (inObject == false) {
					if (inString == false) inString = true;
					else inString = false;
				}
			}
			if (string[i] == ':' && inString == false && inObject == false) {
				count++;
			}
		}
	}
	return count;
}

JSONObject* json_parse(char* string);
void print_json(JSONObject* obj, int nest);

void print_json(JSONObject* obj, int nest) {
	for (int i = 0; i < obj->length; i++) {
		JSONValue* val = obj->values[i];
		for (int j = 0; j < nest; j++) {
			printf("\t");
		}
		printf("%s: ", val->key);
		switch (val->value_type) {
		case JSON_STRING:
			printf(val->stringValue);
			break;
		case JSON_BOOL:
			printf(val->boolValue ? "TRUE" : "FALSE");
			break;
		case JSON_OBJECT:
			printf("\n");
			print_json(val->objectValue, nest + 1);
			break;
		case JSON_NUMBER:
			printf("%f", val->numberValue);
			break;
		default:
			break;

		}
		if(i < obj->length - 1) printf("\n\n");
	}
}

JSONObject* json_parse(char* string) {

	int count = countKeys(string);
	JSONObject* obj = malloc(sizeof(JSONObject));
	JSONValue** values = malloc(count * sizeof(JSONValue*));
	obj->length = count;

	bool inKey = false;
	bool inValue = false;
	bool enteredMainObject = false;
	bool hasKey = false;
	bool inString = false;
	bool inObject = false;
	bool isList = false;
	int keyStartPos = 0;
	int valueStartPos = 0;
	int bracketCount = 0;
	int pairIndex = 0;
	int keySize = 0;
	int valueSize = 0;

	char* keyBuf = malloc(1);
	char* valueBuf = malloc(1);


	for (int i = 0; string[i] != 0; i++) {
		/*
		read first '{', write to a variable that remembers whether we have entered the main object
		read until a " is found. toggle inKey mode.
		read until the ending " is found. disable inKey mode and store the key to a buffer of some sort.
		read until a character that is not a : or a space is found. start inValue mode.
			if a " is read before a comma, enable string mode.
			if a { is read before a comma, enable object mode
			if a comma or a } is read while not in string or object mode, mark the end of the value. exit inValue mode.
			create a JSONValue using the key and value (memcpy here)
			reset the key and value

		added stuff to work with ordered lists

		*/
		const char currentChar = string[i];
		if (currentChar == '[' && !enteredMainObject) {
			isList = true;
			enteredMainObject = true;
			continue;
		}
		if (currentChar == '{') enteredMainObject = true;
		if (!inValue) {
			if (isList)
			{
				keyBuf = malloc(sizeof(int) * 8 + 1);
				keySize = sprintf(keyBuf, "%d", pairIndex);
				keySize += 1;
				hasKey = true;
			}
			else {
				if (currentChar == '"') {
					inKey = !inKey;
					if (inKey) {
						keyStartPos = i;
					}
					else {

						keyBuf = malloc(i - keyStartPos);
						keySize = i - keyStartPos;
						memcpy(keyBuf, &string[keyStartPos + 1], i - keyStartPos - 1);
						keyBuf[i - keyStartPos - 1] = 0;
						hasKey = true;
						continue;
					}
				}


			}
		}
		if (hasKey && !inValue) {
			if (currentChar != ':' && currentChar != ' ') {
				inValue = true;
				valueStartPos = i;
			}
		}

		if (inValue) {
			if (currentChar == '"' && bracketCount == 0) inString = !inString;
			if (currentChar == '{' && !inString) bracketCount++;
			if (currentChar == '[' && !inString) bracketCount++;
			else if (currentChar == '}' && !inString) bracketCount--;
			else if (currentChar == ']' && !inString) bracketCount--;
			if ((currentChar == ',' && bracketCount == 0 && !inString) || bracketCount == -1 || (string[i - 1] == '"' && !inString && bracketCount == 0)) {


				valueBuf = malloc(i - valueStartPos + 1);
				valueSize = i - valueStartPos + 1;
				memcpy(valueBuf, &string[valueStartPos], i - valueStartPos);
				valueBuf[i - valueStartPos] = 0;
				hasKey = false;
				inValue = false;
				inKey = false;
				keyStartPos = 0;
				valueStartPos = 0;

				JSONValue* val = malloc(sizeof(JSONValue));
				val->key = malloc(keySize);
				memcpy(val->key, keyBuf, keySize);
				if (valueBuf[0] == '{' || valueBuf[0] == '[') {
					JSONObject* nested = json_parse(valueBuf);
					val->objectValue = nested;
					val->value_type = JSON_OBJECT;
				}
				else if (valueBuf[0] == '"') {
					char* result = malloc(strlen(valueBuf) - 2);
					memcpy(result, valueBuf + 1, strlen(valueBuf + 1) - 1);
					result[strlen(valueBuf) - 2] = 0;
					val->stringValue = result;
					val->value_type = JSON_STRING;
				}
				else if (isdigit(valueBuf[0])) {
					val->numberValue = atof(valueBuf);
					val->value_type = JSON_NUMBER;
				}
				else {
					val->boolValue = strcmp(valueBuf, "true") == 0;
					val->value_type = JSON_BOOL;
				}

				values[pairIndex] = val;
				pairIndex++;



				free(valueBuf);
				free(keyBuf);
			}


		}





	}
	obj->values = values;

	return obj;


}