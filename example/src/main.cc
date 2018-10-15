#include <iostream>

#include "repo1/repo1.h"
#include "repo2/repo2.h"

int main() {
  std::cout << "random number: " << repo1::repo1() << " - " << repo2::repo2()
            << "\n";
}