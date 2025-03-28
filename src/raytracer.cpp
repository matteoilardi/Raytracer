// NOTE Fyi, I have added the tags NOTE and QUESTION to the TodoTree extension
// QUESTION Can you see them? Or should you modify the settings of your
// extension too? When this works for you too, just delete these lines

#include "colors.hpp"
#include <iostream>

using namespace std;

int main(int argv, char **argc) {
  if (argv != 2) {
    cerr << "Uso del programma: inserisci anche il tuo nome!" << endl;
    return 1;
  }

  cout << "Hello, " << argc[1] << "!" << endl;
  cout << "Uso una funzione della libreria \"colors.h\" per calcolare la somma "
          "tra 2 e 3:"
       << endl;
  // cout << "2 + 3 = " << somma(2, 3) << endl;

  return 0;
}
