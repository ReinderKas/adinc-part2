/* recognizeExp.c, Gerard Renardel, 29 January 2014
 *
 * In this file a recognizer acceptExpression is definined that can recognize 
 * arithmetical expressions generated by the following BNF grammar:
 *
 * <expression>  ::= <term> { '+'  <term> | '-' <term> }
 * 
 * <term>       ::= <factor> { '*' <factor> | '/' <factor> }
 *
 * <factor>     ::= <number> | <identifier> | '(' <expression> ')'
 *
 * Input for the recognizer is the token list constructed by the scanner (in scanner.c). 
 * For the recognition of a token list the method of *recursive descent* is used. 
 * It relies on the use of three functions for the recognition and processing of
 * terms, factors and expressions, respectively. 
 * These three functions are defined with mutual recursion, corresponding with the 
 * structure of the BNF grammar.
 */

#include <stdio.h>  /* getchar, printf */
#include <stdlib.h> /* NULL */
#include "scanner.h"
#include "recognizeExp.h"
#include "evalExp.h"
#include <string.h>
#include <stdbool.h>

/* The functions acceptNumber, acceptIdentifier and acceptCharacter have as 
 * (first) argument a pointer to an token list; moreover acceptCharacter has as
 * second argument a character. They check whether the first token
 * in the list is a number, an identifier or the given character, respectively.
 * When that is the case, they yield the value 1 and the pointer points to the rest of
 * the token list. Otherwise they yield 0 and the pointer remains unchanged.
 */

int acceptNumber(List *lp) {
	if (*lp != NULL && (*lp)->tt == Number) {
		*lp = (*lp)->next;
		return 1;
	}
	return 0;
}

int acceptIdentifier(List *lp) {
  if (*lp != NULL && (*lp)->tt == Identifier) {
	*lp = (*lp)->next;
	return 1;
  }
  return 0;
}

int acceptCharacter(List *lp, char c) {
  if (*lp != NULL && (*lp)->tt == Symbol && ((*lp)->t).symbol == c) {
	//next line changes the current position in the list
	*lp = (*lp)->next;
	return 1;
  }
  return 0;
}

/* The functions acceptFactor, acceptTerm and acceptExpression have as 
 * argument a pointer to a token list. They check whether the token list 
 * has an initial segment that can be recognized as factor, term or expression, respectively.
 * When that is the case, they yield the value 1 and the pointer points to the rest of
 * the token list. Otherwise they yield 0 and the pointer remains unchanged.
 */
 
int acceptTerm(List *lp) {
//  2017: if natnum | natnum identifier | natnum identifier ^ natnum
//	2018: natnum | identifier | natnum identiefier | natnum identifier ^ natnum
//	So we also need to accept terms starting with an identifier.
	int caseType = 0;

	if(acceptNumber(lp)){
		caseType++;
		if(acceptIdentifier(lp)){
			caseType++;
			if((acceptCharacter(lp, '^') && acceptNumber(lp))){
				caseType++;
			}
		}
		//	Try to change it to accept identifier
	} else if (acceptIdentifier(lp)) {
		caseType++;
		if (acceptCharacter(lp, '^') && acceptNumber(lp)){
			caseType++;
		}
	}

	if (caseType>0){
		return 1;
	}
	return 0;
}
// An equation needs to have the following: only one equal sign and two valid expressions.
int acceptEquation(List *lp, List *lp1){
//	2018: Made this ourselves
//    Count equals should not have to be checked
	if(!countEquals(lp1))
		return 0;
	if(!acceptExpression(lp))
		return 0;
	if(!acceptCharacter(lp, '='))
		return 0;
	if(!acceptExpression(lp)){
		return 0;
	}
	if(*lp != NULL){
		return 0;
	}

	return 1;
}

// An expression is of the following grammar: term | - term | + term
// A term can be followed by a multiple of terms.
int acceptExpression(List *lp) {
//	- term | term | + term
// 2018: acceptCharacter is not in original skeleton code presented to us.

	acceptCharacter(lp, '-');
	if(!acceptTerm(lp)){
		return 0;
	}

  while ( acceptCharacter(lp,'+') || acceptCharacter(lp,'-') ) {
	if ( !acceptTerm(lp) ) {
	  return 0;
	}
  } /* no + or -, so we reached the end of the expression */
  return 1;
}

// This function counts all equal signs in the tokenlist.
int countEquals(List *lp){

	int equalCount = 0;
	while ((*lp) != NULL){
		if(((*lp)->t).symbol == '='){
			equalCount++;
		}
		//when using *lp = (*lp)->next you change the position of the pointer in the list.
		*lp = (*lp)->next;
	}
	if(equalCount==1){
		return 1;
	} else {
		return 0;
	}

}

int acceptVariables(List *lp){
    int same = 1, numVar=0;
    List tempList = NULL;
    while ((*lp) != NULL){
        if((*lp)->tt == Identifier){
			++numVar;
            // if we detect a new variable (t.identifier) then copy it by using the built in scanner (new TokenList)
            // and compare it
            if(tempList == NULL){
                // if tempList is empty we have found the first variable in the List
//                printf("Found a variable\n");
                tempList = tokenList((*lp)->t.identifier);
            } else {
                // found a new variable
//                printf("Found a new variable\n");
                if(strcmp(tempList->t.identifier, (*lp)->t.identifier) == 0){
                    //they are the same
                    same = 1;
                } else {
                    //they are not
                    same = 0;
                    break;
                }
            }

        }
        (*lp) = (*lp)->next;
    }
    freeTokenList(tempList);
	if(numVar > 0 ){return same;}
	return 0;
}

int isDegree(List *lp){
	while((*lp) != NULL){
		if(((*lp)->t).symbol == '^'){
			*lp = (*lp)->next;
			if ( *lp == NULL || (*lp)->tt != Number || acceptCharacter(lp,'-')){
				return 0;
			}
		}
		*lp = (*lp)->next;
	}
	return 1;
}

int checkDegree(List *lp){
//	We know that we have a correct equation with 1 variable
//	What happens when we encounter x = 1 or x + x^0 = 1?
	int highest = 1;
	int counter = 0;
	while((*lp) != NULL){
		if (acceptIdentifier(lp)) {
			if ((*lp) != NULL) {
				if (acceptCharacter(lp, '^')){
					// We know that ^ is always followed by a number (because of recognizeEq)
					if (counter == 0) {
						highest = ((*lp)->t).number;
						++counter;
					} else {
						// found a new number/power/degree
						if (((*lp)->t).number > highest) {
							highest = ((*lp)->t).number;
						}
					}
				}
//				No ^ so degree = 1
				counter = 1;

			}
		}
		if((*lp) != NULL) (*lp) = (*lp)->next;
	}	
	printf("%d",highest);
	return highest;	
}

void recognizeEquation(){
	char *ar;
	int degree;
	List tl, tl1,tl2,tl3;
	//might need to change the order of readinput and printf
	ar = readInput();
	printf("give an equation: ");
	while (ar[0] != '!'){
		tl = tokenList(ar);
		printList(tl);
		tl1 = tl;
		tl2 = tl;
		tl3 = tl;
		//printf("This input has %d equal signs", countEquals(&tl1));
		if(!acceptEquation(&tl1, &tl2)){
			printf("this is not an equation\n");
		} else if (!isDegree(&tl3)){
			printf("this is not an equation\n");
		} else {
			printf("this is an equation");
			tl1 = tl;
			if(acceptVariables(&tl1)){
				printf(" in 1 variable of degree ");
				tl1 = tl;
				degree = checkDegree(&tl1);
				if(degree == 1){
					recognizeEquation();
				} else if (degree == 2){
					//abc formula
				} 
			} else {
				printf(", but not in 1 variable");
			}
			printf("\n");
			
		}
		//check list for number of unique variable chars
		// and check variable degree. The number after character ^
		
//		freeTokenList(tl1);
//		freeTokenList(tl2);
//		freeTokenList(tl3);
		free(ar);
		freeTokenList(tl);
		printf("\ngive an equation: ");
		ar = readInput();
	}
	free(ar);
	printf("good bye\n");
}
