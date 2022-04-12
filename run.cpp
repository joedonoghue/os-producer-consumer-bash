#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;


int getCmdOption(char ** begin, char ** end, const std::string & option)
{
  char ** itr = std::find(begin, end, option);
  if (itr != end && ++itr != end)
  {
    try {
      int ret = stoi(*itr);
      return ret;
    }
    catch (const std::exception&) {
      cout << "Error with option -" << option << endl;

      return -1;
    }
  }
  cout << "Missing -" << option << endl;
  return -1;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return std::find(begin, end, option) != end;
}

int main(int argc, char * argv[])
{
  int item_cnt;
  int buf_len;
  int prod_cnt;
  int fprod_cnt;
  int cons_cnt;
  bool debug = true;

  item_cnt = getCmdOption(argv, argv + argc, "-n");
  buf_len = getCmdOption(argv, argv + argc, "-l");
  prod_cnt = getCmdOption(argv, argv + argc, "-p");
  fprod_cnt = getCmdOption(argv, argv + argc, "-f");
  cons_cnt = getCmdOption(argv, argv + argc, "-c");

  if(debug) {
    printf("item_cnt: %d\n", item_cnt);
    printf("buf_len: %d\n", buf_len);
    printf("prof_cnt: %d\n", prod_cnt);
    printf("fprof_cnt: %d\n", fprod_cnt);
    printf("cons_cnt: %d\n", cons_cnt);
  }
  

  return 0;
}