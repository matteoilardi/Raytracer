#include <iostream>

using namespace std;

int main(int argv, char** argc){
  	if(argv != 2){
          cerr << "Uso del programma: inserisci anche il tuo nome!" << endl;
          return 1;
    }

	cout << "Hello, "<<argc[1]<<"!" << endl;
	return 0;

}
