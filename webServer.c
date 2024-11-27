#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>



int wordCount = 0;
//linked list to store words
struct wordListNode {
	char word[30];
	struct wordListNode *next;
};
typedef struct wordListNode wordList;
wordList *head;//head of dictionary

//linked list to store all acceptable guesses
struct gameListNode{
	char validWord[30];
	bool found;
	struct gameListNode *next;
};
typedef struct gameListNode gameList;
gameList *gameListHead;//head of valid words

wordList *master;//masterword for game

//clear memory of game list
void cleanUpGameListNodes(){
	gameList *current = gameListHead;
	gameList *nextNode;
	while (current != NULL) {
        nextNode = current->next; // save the next node
        free(current);           
        current = nextNode;     
    }
    gameListHead = NULL;
}

//clear memory of word List
void cleanUpWordListNodes(){
	wordList *current = head;
	wordList *nextNode;
	while (current != NULL) {
        nextNode = current->next; // save the next node
        free(current);           
        current = nextNode;     
    }
    head = NULL;
}
//used to load dictionary into memory
int initilization(){
	srand(time(NULL));//seed random generator

	FILE *fp;
	fp = fopen("2of13.txt", "r");

	char myWord[30];
	fgets(myWord, 30, fp);
	myWord[strcspn(myWord, "\r\n")] = 0;//remove carriage retitn and newline characters

	//create the first node
	wordList *currentNode = (wordList*)malloc(sizeof(wordList));
	strcpy(currentNode -> word, myWord); 
	currentNode -> next = NULL;

	//store head node
	head = currentNode;

	//read dict and add words to list
	while(fgets(myWord, 30, fp)){
		myWord[strcspn(myWord, "\r\n")] = 0;//remove carriage retitn and newline characters
		//create a new node each iteratin
		wordList *newNode = (wordList*)malloc(sizeof(wordList));
		strcpy( newNode -> word, myWord); 
		newNode -> next = NULL;
		//make curr node point to out new node
		currentNode -> next = newNode;
		//make our new node the curr node
		currentNode = newNode;
		wordCount++;
	}
	fclose(fp);
	return wordCount;
}	

//creates random word to be used
wordList* getRandomWord(){
	int randomWord = rand() % wordCount;
	wordList *temp = head;
	int i;
	for(i =0; i < randomWord; i++){
		temp = temp -> next;
	}
	while(i < wordCount){
		if(strlen(temp -> word) > 6){
			return temp;
		}else{
			temp = temp -> next;
		}
	}
	printf("no suitable word found");
}

//used to print puzzle
void displayWorld(char *buffer){
	strcat(buffer, "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n");
	strcat(buffer, "<html><title>Word Game</title><body><h1>Letter Bank:<h1><h1 class=bank>");
	for(int i =0; i< strlen(master->word); i++){//print the word bank
		char letter[5];
		sprintf(letter, "%c   ", toupper(master->word[i]));
		strcat(buffer, letter);
	}
	strcat(buffer, "</h1><p class = rules>HOW TO PLAY:<br>*Each letter in bank can only be used once.<br>*Each dash represents the number of letters in that word.<br>*Must find all words to complete game.</p><br>");
	strcat(buffer, "<form submit=\"words\">Enter a guess: <input type=\"text\" name=move autofocus > </input></form>");
	//print status of words
	gameList *current = gameListHead;
	int wordNumber = 1;//tracks number of words
	while(current != NULL){
		char temp[100];//store word line
		if(current ->found == false){
			sprintf(temp, "<p class = missing>%d. MISSING: ", wordNumber);
			strcat(buffer, temp);
			for (int i = 0; i < strlen(current->validWord); i++) {
            	strcat(buffer, "- ");
        	}
        	strcat(buffer, "</p>");
   		}
   		else{
 			sprintf(temp, "<p class = found>%d. FOUND: ", wordNumber);
			strcat(buffer, temp);
            strcat(buffer, current->validWord);
            strcat(buffer, "</p>");   	
        }
        current = current->next;
        wordNumber++;
    }
    wordNumber =1;//reset count
    strcat(buffer, "<style> .rules{outline-style: solid; outline-color:red; font-weight:bold;font-size:25px; margin-left: 15px;}form{font-size:25px; margin-left: 20px;}"
    	".found{color:green; font-size:25px; margin-left: 20px;}.missing{color: red; font-size:25px; margin-left: 25px;} h1{text-align: center; font-size: 45px; color: #4682b4;} .bank{text-align: center; font-size: 65px; color: #4682b4; outline-style: dotted; outline-color:#4682b4;}</style><html>");
}

//used to get users word guess
void acceptInput(char *url){
	
	if (strcmp(url, "/favicon.ico") == 0) {
   		printf("Ignoring /favicon.ico request.\n");
    	return;
	}
	char* guess = NULL;// variable where guess will be stored
    char* rest = url;
 	//parse header for file path
    for (int i=0; i<2; i++){
    	guess = strtok_r(rest, "=", &rest);
    }

	gameList *current = gameListHead;//change stauts to found if word is in list
	while(current != NULL){
		if(strcmp(guess, current->validWord) == 0){
			current->found = true;
			break;//dont need to check rest if word found
		}
		current = current->next;
	}
	for(int i =0; i<strlen(guess); i++){//convert each character to uppercase
		guess[i] = toupper(guess[i]);
	}
}

bool isDone(){
	gameList *current = gameListHead;
	while(current != NULL){
		if(current->found == false){
			return false;
		}
		current = current->next;
	}
	return true;//all words found
}

//used to count the number of times each letter appears in word
//takes a word and an array for 26 letter in alphabet
void getLetterDistribution(char* word, int* usedLetters){
	for(int i =0; i<strlen(word); i++){
		usedLetters[word[i] - 'a'] +=1;//using ascii characters, will increment the correspoonding positoin of letter
	}
}

//used to see if guessed word contians the letters of availabe choices
bool compareCounts(char* dist1, char* dist2){
	int wordBank[26] ={0};//to store distr. of choices
	int guess[26]={0};//to store distr. of guessed word

	getLetterDistribution(dist1, wordBank);
	getLetterDistribution(dist2, guess);

	for(int i =0; i<26; i++){
		if(guess[i] != 0){//for used letters of the guessed word
			if(wordBank[i] < guess[i]){//check if letter is inlcuded in available choices
			return false;
			}
		}
	}
	return true; 
}

//adds all valid words to linked list
void findWords(char *masterWord){
	wordList *temp = head;
	//create the first node
	gameList *currentNode = NULL;
	for(int i =0; i < wordCount; i++){
		if(compareCounts(masterWord, temp -> word)){
			gameList *newNode = (gameList*)malloc(sizeof(gameList));
			strcpy(newNode -> validWord, temp -> word);
			newNode -> next = NULL;
			newNode -> found = false;
			if(currentNode != NULL){
				currentNode -> next = newNode;
				currentNode = newNode;	//make curr node point to out new node
			}else{//if first node
				currentNode = newNode;
				gameListHead = currentNode;
			}
		}
		temp = temp -> next;
	}
}

//compare function for qsort
int compare(const void *a, const void *b) {
    return *(char *)a - *(char *)b;
}

//Handles Server Logic:

const char *base_path;//holds base path passed in as argument
void *request(void *param){
	int client_sock = *(int *)param;
	char buffer[1000];//get header
	char content[10000]; //file content
	int b = recv(client_sock, buffer, sizeof(buffer) -1, 0);// GET request header
	buffer[b] = '\0';
	printf("%s\n", buffer);
	char* token;//holds get path
    char* rest = buffer;
 	//parse header for file path
    for (int i=0; i<2; i++){
    	token = strtok_r(rest, " ", &rest);
    }
    if(strcmp(token, "/wordfind") == 0){//if its a new game
    	initilization();  
     	master = getRandomWord();
     	findWords(master -> word);
     	qsort(master->word, strlen(master->word), sizeof(char), compare);

     	char htmlBuffer[10000];
        displayWorld(htmlBuffer);

        send(client_sock, htmlBuffer, strlen(htmlBuffer), 0);
        close(client_sock);
    	pthread_exit(NULL);
    }
    else if(strstr(token, "?move=") != NULL) {//if guess a word in an ongoing game
		//if send empty guess
		if(token[strlen(token)-1] != '='){
			acceptInput(token);//check if word exists
		}

        if(isDone()){//if game us over
        	cleanUpWordListNodes();
        	cleanUpGameListNodes();
        	wordCount =0;
        	char *finished = "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n"
        	"<html><body><h1>Congratulations!</h1><h2>You solved it!</h2> <a href=\"/wordfind\">Want to play Again?</a></body>"
        	"<style> h1, h2{color:white;}body{margin-top: 35px; text-align:center; background-color: rgb(34, 139, 34);} a{font-size:35px; color:white;}</style></html>";
            send(client_sock, finished, strlen(finished), 0);
        }
        else{
        	char htmlBuffer[10000];
        	displayWorld(htmlBuffer);  
        	send(client_sock, htmlBuffer, strlen(htmlBuffer), 0);
        }
        close(client_sock);
    	pthread_exit(NULL);
    }
 
    //check if the base path has trailing slash
    int len = strlen(base_path);
    if(base_path[len - 1] == '/'){
    	memmove(token, token+1, strlen(token));//remove slash
    }
    char full_path[100];//holds full path to open file
    sprintf(full_path, "%s%s", base_path, token);
	

    int fd = open(full_path, O_RDONLY);
    if(fd == -1){//if file not foud
    	char *fail = "HTTP/1.0 404 Not Found : File Does Not Exist\r\n";
    	send(client_sock, fail, strlen(fail), 0);
    	close(client_sock);
    	pthread_exit(NULL);
    }else{//if file found
    	struct stat file;
    	stat(full_path, &file);
    	char success[1000];//response header
    	sprintf(success, "HTTP/1.1 200 OK\r\nContent-Length:%ld\r\n\r\n", file.st_size);
    	
    	send(client_sock, success, strlen(success), 0);//send response header
    	read(fd, content, sizeof(content));
    	send(client_sock, content, strlen(content), 0);//send file content
    	
    	close(client_sock);
    	close(fd);
    	pthread_exit(NULL);
    }
}

int main(int argc, char  **argv)
{
	if(argc == 1){//if path not passed as parameter
		printf("ERROR: Must input a path\n");
		return 0;
	}
	base_path = argv[1];//base path where files will be served from

	struct addrinfo hints, *servinfo, *cur;
	memset (&hints, 0, sizeof(hints));
	hints.ai_socktype=SOCK_STREAM; // TCP only
	hints.ai_flags = AI_PASSIVE; // fill in my IP address for me

	int status = getaddrinfo(NULL, "8000",&hints, &servinfo); // self
	if(status != 0){
		printf("error adding info");
	}

	int sockfd;//main socket
	for (cur=servinfo;cur!=NULL;cur=cur->ai_next) { // loop over addresses
		sockfd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
		if(sockfd == -1){
			continue;
		}
		if(bind(sockfd,cur->ai_addr, cur->ai_addrlen) == -1 ){
			perror("bind failed");
			close(sockfd);
			continue;
		}
		break;//bind successfull
	}
	freeaddrinfo(servinfo);

	if(listen(sockfd, 10) == -1){
    	perror("listen failed");
		exit(1);
	}
	printf("Listening on Port 8000...\n");
	printf("To play word game type the following url into your browser\nhttp://localhost:8000/wordfind\n");
	struct sockaddr_storage client_addr; // connector's address information
	socklen_t sin_size; //socket size
	sin_size = sizeof(client_addr);
	int client_sock;//response socket
	while(1){
		client_sock = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size); //accepts requests
		int *client_sock_ptr = malloc(sizeof(int));
		*client_sock_ptr = client_sock;

		pthread_t id;//create thread for request
		pthread_create(&id, NULL, request, client_sock_ptr);
		// pthread_join (id,&result);
		sleep(1);
	}
    close(sockfd);
	return 0;
}