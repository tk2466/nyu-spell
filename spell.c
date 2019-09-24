#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dictionary.h"
//#include <dictionary.c>
#define IS_CTRL  (1 << 0)
#define IS_EXT	 (1 << 1)
#define IS_ALPHA (1 << 2)


unsigned int char_tbl[256] = {0};

void init_table()
{
	int i;

	for (i = 0; i < 32; i++) char_tbl[i] |= IS_CTRL;
	char_tbl[127] |= IS_CTRL;

	for (i = 'A'; i <= 'Z'; i++) {
		char_tbl[i] |= IS_ALPHA;
		char_tbl[i + 0x20] |= IS_ALPHA; /* lower case */
	}

	for (i = 128; i < 256; i++) char_tbl[i] |= IS_EXT;
}

void strip(char * str, int what)
{
	unsigned char *ptr, *s = (void*)str;
	ptr = s;
	while (*s != '\0') {
		if ((char_tbl[(int)*s] & what) == 0)
			*(ptr++) = *s;
		s++;
	}
	*ptr = '\0';
}

bool invalidChar (char c)
{
   return !(c>=32 && c<128);

}


node* hashtable[HASH_SIZE];
int hashsize = 0;

int hash_function(const char* word);
int number_of_words = 0;

int isLetter(char c){
    if ( (c>='a'&&c<='z') || (c>='A'&&c<='Z'))
        return 1;
    return 0;
}

void removePunc(char article[]){
    int i, j=0;
    for ( i =0; article[i] != 0; i++){
        if (isLetter(article[i])){
            article[j] = article[i];
            j++;
        }
        else if (!isLetter(article[i])){
            article[j] = ' ';
            j++;
        }
    }
}
void toLower( char article[]){
    int i=0;
    for( int i=0; article[i] != 0; i++){
        if ( article[i] >= 'A' && article[i] <='Z')
            article[i] = article[i] + 32;
    }
}

int check_words(FILE* fp, hashmap_t hashtable[], char* misspelled[])
{
    int num_misspelled = 0;
  int wordlength = 0;
  int newlength = 0;
  char c = '\0';
  char word[LENGTH+1];
  int i = 0;
  int source = 0;
  int destination = 0;


  while (c != EOF)
  {
     while (EOF!=(c=getc(fp)) && ( (c == '\n') || (c == ' ')));
     if ( c == EOF ) return num_misspelled;
     i = 0;
     do {
        if(i <= LENGTH && !invalidChar(c))
	{
	   word[i++] = c;
        }
	else
	{
           i++;
	}

     } while(EOF!=(c=fgetc(fp)) && (c != ' ') && (c != '\n'));


     if(i > (LENGTH+1))
     {
       word[LENGTH+1] = '\0';
     }
     else
     {
       word[i] = '\0';

     }

       strip(word, IS_CTRL);
       wordlength = strlen(word);
       newlength = wordlength;
       source = 0;
       destination = 0;

       while (word[source] != '\0')
       {

	  if ((source == 0 || source == (wordlength -1)) && !isalpha(word[source]))
          {

            ++source;
            --newlength;
          }
          else
          {
            word[destination] = word[source];
            ++source;
            ++destination;
          }


       }
       word[newlength] = '\0';


       if (word[0] == '\0') continue;
       if (!check_word(word, hashtable))
       {

	misspelled[num_misspelled] = malloc(sizeof(word)+1);
	strcpy(misspelled[num_misspelled],word);
	num_misspelled++;


        }

        for (int j = 0; j < (LENGTH + 1); j++)
	{
            word[j] = '\0';

	}

  }

  return num_misspelled;
}
bool check_word(const char* word, hashmap_t hashtable[])
{

    int word_length = strlen(word);
    char lower_word[LENGTH+1];


    for (int i = 0; i < word_length; i++)
    {
            lower_word[i] = word[i];

    }

    removePunc(lower_word);
    toLower(lower_word);
    lower_word[word_length] = '\0';


    int bucket = hash_function(lower_word);

    node* cursor = hashtable[bucket];
    while (cursor != NULL)
    {
        if (strcmp(lower_word, cursor->word) == 0)
        {

            return true;
        }
        cursor = cursor->next;
    }

    return false;
}

bool load_dictionary(const char* dictionary_file, hashmap_t hashtable[])
{

    char row[LENGTH+1];
  FILE *wordlist = fopen(dictionary_file, "r");
  char word[LENGTH+1];
  for (int i = 0; i < HASH_SIZE; i++) hashtable[i] = NULL;



  if(!wordlist)
  {
    return false;
  }

  // Read line
  while (fgets(row, sizeof row, wordlist) != NULL)
  {
    // Read word from line and strip spaces
    strcpy(word, (row + strspn(row, "\t")));
    word[strcspn(word, " \t\r\n")] = '\0';
    if (*word == '\0' || *word == '#' || *word == ';')
    {
      continue;
    }


    int hashval = hash_function(word);
    if (hashtable[hashval] == NULL)
    {
      hashtable[hashval] = malloc(sizeof(node));
      strcpy(hashtable[hashval]->word, word);
      hashtable[hashval]->next = NULL;
      hashsize++;
    }
    else
    {

      node *hashentry = malloc(sizeof(node));
      strcpy(hashentry->word, (word));
      hashentry->next = hashtable[hashval];
      hashtable[hashval] = hashentry;
      hashsize++;

    }


  }

  fclose(wordlist);

  for (int k = 0; k < HASH_SIZE; k++)
  {
    if (hashtable[k] == NULL) free(hashtable[k]);

  }


  return 1;
}

