#include <stdlib.h> 
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "scanner.h"
#include "recognizeExp.h"

int main(int argc, char *argv[]) {
  recognizeEquation();
	/* recognizeExpression has the function acceptExpresssion.
	input for that function is a token list which is constructed by the scanner.c
	acceptNumber, acceptIdentifier and acceptCharacter have as argument a pointer to a token list
	*/
	
	
  return 0;
}
