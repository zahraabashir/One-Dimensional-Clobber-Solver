
#include <iostream>
#include<sstream>
#include <string.h>
using namespace std;

class Clobber { 
  public:            
      string pos;
      string turn;
      string time_limit; 
};


int main() {

  Clobber clb;
  char input[100];
  cin.getline(input,sizeof(input));
  // cout << input;
  istringstream ss(input);
  string del;
  int count = 0;

  while(getline(ss, del, ' ')) {
      cout << del << '\n';
  
      if (count == 0){
        clb.pos = del;}
      else if (count == 1){
        clb.turn = del;}
      else if (count == 2){
        clb.time_limit = del;}

      count +=1 ;
}
cout << clb.turn;

  return 0;

}