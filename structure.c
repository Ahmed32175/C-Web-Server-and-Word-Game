#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

int wordCount =0;
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
        free(current->validWord); 
        free(current);           
        current = nextNode;     
    }
}

//clear memory of word List
void cleanUpWordListNodes(){
	wordList *current = head;
	wordList *nextNode;
	while (current != NULL) {
        nextNode = current->next; // save the next node
        free(current->word); 
        free(current);           
        current = nextNode;     
    }
}
//used to load dictionary into memory
int initilization(){
	srand(time(NULL));//seed random generator

	FILE *fp;
	fp = fopen("2of12.txt", "r");

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
	 //printf("%d\n", wordCount);
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
			// printf("%s", temp -> word);
			return temp;
		}else{
			temp = temp -> next;
		}
	}
	printf("no suitable word found");
}
 

//used to print puzzle
void displayWorld(){
	for(int i =0; i< strlen(master->word); i++){//print the word bank
	printf("%c ", toupper(master->word[i]));
	}
	printf("\n-------------------------------\n\n");
	//print status of words
	gameList *current = gameListHead;
	while(current != NULL){
		if(current ->found == false){
			for (int i = 0; i < strlen(current->validWord); i++) {
            	printf("- ");
        		}
        		printf("\n");
   		}
   		else{
   			printf("FOUND : %s\n", current -> validWord);
   		}
        current = current->next;
    }
}
//used to get users word guess
void acceptInput(){
	char guess[100];// variable where guess will be stored

	printf("Enter a guess: ");
	fgets(guess, sizeof(guess), stdin);
	guess[strcspn(guess, "\r\n")] = 0;//remove carriage retitn and newline characters

	gameList *current = gameListHead;//change stauts to found if word is in list
	while(current != NULL){
		if(strcmp(guess, current->validWord) == 0){
			current->found = true;
			break;//VALID????
		}
		current = current->next;
	}
	for(int i =0; i<strlen(guess); i++){//convert each character to uppercase
		guess[i] = toupper(guess[i]);
	}
	printf("%s\n", guess);
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
	cleanUpGameListNodes();
}

//used to loop game
void gameLoop(){
	do{
		displayWorld();
		acceptInput();
	}
	while(!isDone());
}

void teardown(){
	printf("All Done\n");
	cleanUpWordListNodes();
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
	// printf("%s\n", gameListHead -> validWord);
	// gameList *tmp = gameListHead;
	// int c =0;
	// while(tmp != NULL){
	// 	c++;
	// 	tmp = tmp -> next;
	// }
	// printf("%d\n", c);
}

//compare function for qsort
int compare(const void *a, const void *b) {
    return *(char *)a - *(char *)b;
}

int main(int argc, char **argv){
    initilization();  
     master = getRandomWord();
     findWords(master -> word);
     qsort(master->word, strlen(master->word), sizeof(char), compare);
     //printf("%s\n", master -> word);
	gameLoop();
	teardown();

	return 0;
}