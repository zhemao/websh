#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "linenoise.h"
#include "vector.h"
#include "strutils.h"
#include "websh.h"

int main(int argc, char *argv[]) {
	char *line, *url;
	if(argc < 2){
		fprintf(stderr, "Usage: %s url\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	url = argv[1];
	char prompt[strlen(url)+3];
	sprintf(prompt, "%s> ", url);

	while((line = linenoise(prompt)) != NULL) {
		if(strcmp(line, "exit")==0)
			break;
		if (line[0] != '\0') {
			linenoiseHistoryAdd(line);
			handle_input(url, line);
		}
		
		free(line);
	}
	
	return 0;
}

void handle_input(char * url, char * input){
	vector * vec = str_split(input, " ");
	CURL * curl;
	char *com, *path;
	char *data = NULL;
	char * full_url;
	
	if(vec->length < 2){
		destroy_vector(vec);
		return;
	}
		
	curl = curl_easy_init();
	com = vector_get(vec, 0);
	path = vector_get(vec, 1);
	str_upper(com);
	
	full_url = (char*)malloc(strlen(url)+strlen(path)+1);
	strcpy(full_url, url);
	strcat(full_url, path);
	
	curl_easy_setopt(curl, CURLOPT_URL, full_url);
	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	
	if(strcmp(com, "GET")==0){
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
	} else if(strcmp(com, "POST")==0){
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		data = linenoise("...");
		curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, data);
		free(data);
	} 
	
	curl_easy_perform(curl);
	
	free(full_url);
	curl_easy_cleanup(curl);
	destroy_vector(vec);
}