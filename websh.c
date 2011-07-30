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
	
	headers = create_map();
	
	url = argv[1];
	char prompt[strlen(url)+3];
	sprintf(prompt, "%s> ", url);
	
	linenoiseSetCompletionCallback(completionCallback);

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

int handle_upload(void * ptr, size_t size, size_t nmemb, void * userdata){
	int len = size * nmemb;
	upload_buffer * upbuf = (upload_buffer*)userdata;
	
	/* if we can copy over the entire length, do so */
	if(upbuf->index + len < upbuf->length){
		memcpy(ptr, upbuf->data, len);
		upbuf->index += len;
	/* otherwise, if end of string hasn't been reached, copy the remainder */
	} else if(upbuf->index < upbuf->length){
		len = upbuf->length - upbuf->index;
		upbuf->index = upbuf->length;
		memcpy(ptr, upbuf->data, len);
	/* otherwise, return 0 to signal EOF */
	} else {
		len = 0;
	}
	
	return len;
}

void perform_request(CURL * curl, char * url, char * path){
	char * full_url;
	int i, line_len;
	char * line;
	char * header, * value;
	struct curl_slist *slist=NULL;
	
	full_url = (char*)malloc(strlen(url)+strlen(path)+1);
	strcpy(full_url, url);
	strcat(full_url, path);
	
	curl_easy_setopt(curl, CURLOPT_URL, full_url);
	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	
	/* attach all the headers */
	for(i=0; i<headers->keys->length; i++){
		header = vector_get(headers->keys, i);
		value = map_get(headers, header);
		line_len = strlen(header)+strlen(value)+3;
		line = (char*)calloc(line_len, sizeof(char));
		sprintf(line, "%s: %s", header, value);
		slist = curl_slist_append(slist, line);
		free(line);
	}
	
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	
	curl_easy_perform(curl);
	/* print a newline incase the response has no ending newline */
	printf("\n");
	
	/* cleanup */
	curl_slist_free_all(slist);
	free(full_url);
}

void prepare_get(CURL * curl){
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
}

char * prepare_post(CURL * curl){
	char * data;
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	data = linenoise("...");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	return data;
}

char * prepare_put(CURL * curl, upload_buffer * upbuf){
	char * data;
	
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
	data = linenoise("...");
	upbuf->data = data;
	upbuf->index = 0;
	upbuf->length = strlen(data);
	curl_easy_setopt(curl, CURLOPT_READDATA, upbuf);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, handle_upload);
	
	return data;
}

void prepare_head(CURL * curl){
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
}

void prepare_delete(CURL * curl){
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
}

void setheader(vector * vec){
	char * header, * value;
	vector * subvec;
	if(vec->length > 2){
		header = vector_get(vec, 1);
		subvec = subvector(vec, 2, vec->length);
		value = str_join((char**)subvec->data, "", subvec->length);
		map_put(headers, header, value, strlen(value)+1);
		destroy_vector(subvec);
		free(value);
	}
}

void getheader(vector * vec){
	char * header, * value;
	header = vector_get(vec, 1);
	value = map_get(headers, header);
	if(value!=NULL)
		printf("%s\n", value);
}

void delheader(vector * vec){
	char * header;
	header = vector_get(vec, 1);
	map_remove(headers, header);
}

void handle_input(char * url, char * input){
	vector * vec = str_split(input, " ");
	CURL * curl;
	char *com, *path;
	char *data = NULL;
	upload_buffer upbuf;
	
	int perform = 1;
	
	if(vec->length < 2){
		destroy_vector(vec);
		return;
	}
	
	/* init some stuff */
	curl = curl_easy_init();
	com = vector_get(vec, 0);
	str_lower(com);
	
	/* handle specific request types */
	if(strcmp(com, "get")==0){
		prepare_get(curl);
	} else if(strcmp(com, "post")==0){
		data = prepare_post(curl);
	} else if (strcmp(com, "head")==0){
		prepare_head(curl);
	} else if (strcmp(com, "put")==0){
		data = prepare_put(curl, &upbuf);
	} else if(strcmp(com, "delete")==0) {
		prepare_delete(curl);
	} else if (strcmp(com, "setheader")==0){
		perform = 0;
		setheader(vec);
	} else if(strcmp(com, "getheader")==0){
		perform = 0;
		getheader(vec);
	} else if(strcmp(com, "delheader")==0){
		perform = 0;
		delheader(vec);
	} else perform = 0;
	
	if(perform){
		path = vector_get(vec, 1);
		perform_request(curl, url, path);
	}
	
	/* free data if needed */
	if(data != NULL)
		free(data);
	
	destroy_vector(vec);
	curl_easy_cleanup(curl);
}

void completionCallback(const char * input, linenoiseCompletions * lc){
	int len = strlen(input);
	char temp[len+1];
	strcpy(temp, input);
	str_lower(temp);
	if(len < 3){
		if(temp[0]=='g')
			linenoiseAddCompletion(lc, "get ");
		else if(temp[0]=='p' && len>1){
			if(temp[1]=='u')
				linenoiseAddCompletion(lc, "put ");
			else if(temp[1]=='o')
				linenoiseAddCompletion(lc, "post ");
		}
		else if(temp[0]=='d')
			linenoiseAddCompletion(lc, "delete ");
		else if(temp[0]=='h')
			linenoiseAddCompletion(lc, "head ");
		else if(temp[0]=='e')
			linenoiseAddCompletion(lc, "exit");
	}
	else if(len < 10){
		if(strncmp(temp, "get", 3)==0)
			linenoiseAddCompletion(lc, "getheader ");
		else if(strncmp(temp, "set", 3)==0)
			linenoiseAddCompletion(lc, "setheader ");
		else if(strncmp(temp, "del", 3)==0)
			linenoiseAddCompletion(lc, "delheader ");
	}
}
