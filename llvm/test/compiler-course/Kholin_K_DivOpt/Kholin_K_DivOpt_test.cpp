#include <iostream>

using namespace std;


int main() {
  unsigned int lhs1 = 1024;

  unsigned int result_type_OK1 = lhs1 / 2;
  unsigned int UB1 = lhs1 / (-2);

  unsigned int lhs2 = 512;

  unsigned int result_type_OK2 = lhs2 / 8u;
  
  signed int lhs3 = 256;

  signed int result_type_OK3 = lhs3 / 64;
  signed int Not_UB1 = lhs3 / (- 64);
  signed int result_type_OK4 = (-lhs3) / 64;
  signed int Not_UB2 = (-lhs3) / (-64); 

  signed int lhs4 = 128;

  signed int result_type_OK5 = lhs4 / 32u;
  signed int UB2 = (-lhs4) / 32u;

  signed int lhs5 = 128;

  signed int result_type_OK7 = lhs5 / 3u;
  signed int UB3 = (-lhs5) / 3u;

  signed int lhs6 = 126;

  signed int result_type_OK9 = lhs6 / 4;
  signed int UB4 = (-lhs6) / 4u;

  double lhs7 = 894.93;

  double result_type_OK11 = lhs7 / 4;
  double result_type_OK12 = lhs7 / (-4);

  int lhs8 = 6;

  int result_type_OK13 = lhs8 / 37.47f;
  int result_type_OK14 = lhs8 / (-37.47f);

  long long lhs9 = 312345LL;

  long long result_type_OK15 = lhs9 / 32;
  long long result_type_OK16 = lhs9 / (-32);

  int over_bits = 32 / 64;

  return 0;
}
