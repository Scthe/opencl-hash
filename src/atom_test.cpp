// #include <string>
#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[]) {
  cout << "hello !" << endl;
  vector<int> v;
  v.push_back(0);
  for (size_t i = 0; i < 5; i++) {
    v.push_back(i);
  }

  for(auto i = begin(v); i != end(v); ++i){
    cout << *i << endl;
  }

}
