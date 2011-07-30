#include "map.h"

typedef struct{
	char * data;
	int index;
	int length;
} upload_buffer;

map * headers;

/* take input for a POST or PUT command from linenoise */
void handle_input(char * url, char * input);
/* callback for PUT command */
int handle_upload(void * ptr, size_t size, size_t nmemb, void * userdata);
void perform_request(CURL * curl, char * url, char * path);
void prepare_get(CURL * curl);
char * prepare_post(CURL * curl);
char * prepare_put(CURL * curl, upload_buffer * upbuf);
void prepare_head(CURL * curl);
void prepare_delete(CURL * curl);
void setheader(vector * vec);
void getheader(vector * vec);
void delheader(vector * vec);
void completionCallback(const char * input, linenoiseCompletions * lc);
