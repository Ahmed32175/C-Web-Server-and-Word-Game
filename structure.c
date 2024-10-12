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
wordList *head;

//linked list to store all acceptable guesses
struct gameListNode{
	char validWord[30];
	bool found;
	struct gameListNode *next;
};
typedef struct gameListNode gameList;
gameList *gameListHead;
//used to load dictionary into memory
int initilization(){
	srand(time(NULL));//seed random generator

	FILE *fp;
	fp = fopen("2of12.txt", "r");

	char myWord[30];
	//create the first node
	wordList *curr = (wordList*)malloc(sizeof(wordList));
	fgets(myWord, 30, fp);
	strcpy(curr -> word, myWord); 
	curr -> next = NULL;

	//store head node
	head = curr;

	//read dict and add words to list
	while(fgets(myWord, 30, fp)){
		//create a new node each iteratin
		wordList *n = (wordList*)malloc(sizeof(wordList));
		strcpy( n -> word, myWord); 
		n -> next = NULL;
		//make curr node point to out new node
		curr -> next = n;
		//make our new node the curr node
		curr = n;
		wordCount++;
	}
	fclose(fp);
	 printf("%d\n", wordCount);
	// printf("%s\n", head -> word);
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
			printf("%s", temp -> word);
			return temp;
		}else{
			temp = temp -> next;
		}
	}
	printf("no suitable word found");
}


//used to print puzzle
void displayWorld(){
	printf("--------\n");
}

//used to get users word guess
void acceptInput(){
	char guess[100];// variable where guess will be stored

	printf("Enter a guess:\n");
	fgets(guess, sizeof(guess), stdin);

	guess[strcspn(guess, "\r\n")] = 0;//remove carriage retitn and newline characters

	for(int i =0; i<strlen(guess); i++){//convert each character to uppercase
		guess[i] = toupper(guess[i]);
	}
	printf("%s\n", guess);
}

bool isDone(){
	return true;
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
}

//used to count the number of times each letter appears in word
//takes a word and an array for 26 letter in alphabet
void getLetterDistribution(char* word, int* usedLetters){
	for(int i =0; i<strlen(word); i++){
		usedLetters[word[i] - 'A'] +=1;//using ascii characters, will increment the correspoonding positoin of letter
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
void findWords(char masterWord[30]){
	wordList *temp = head;
	//create the first node
	gameList *curr;
	for(int i =0; i < wordCount; i++){
		if(compareCounts(masterWord, temp -> word)){
			gameList *n = (gameList*)malloc(sizeof(gameList));
			strcpy(n -> validWord, temp -> word);
			n -> next = NULL;
			//make curr node point to out new node
			curr = n;
		}
		temp = temp -> next;
	}
	gameList *tmp = gameListHead;
	int c =0;
	while(tmp != NULL){
		c++;
		tmp = tmp -> next;
	}
	printf("%d\n", c);
}

int main(int argc, char **argv){
     initilization();
     wordList *w = getRandomWord();
     findWords(w -> word);
	 gameLoop();
	 teardown();

	return 0;
}