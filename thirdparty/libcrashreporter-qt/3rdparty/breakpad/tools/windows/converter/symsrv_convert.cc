#include <cstdio>
#include <string>
#include <vector>

#include "tools/windows/converter/ms_symbol_server_converter.h"

using std::string;
using std::vector;
using google_breakpad::MissingSymbolInfo;
using google_breakpad::MSSymbolServerConverter;

int main(int argc, char *argv[])
{
  if (argc < 5) {
    fprintf(stderr, "Usage: %s <symbol server> <symbol path> <debug file> <debug identifier>\n ", argv[0]);
    return 1;
  }

  MissingSymbolInfo missing_info;
  missing_info.debug_file = argv[3];
  missing_info.debug_identifier = argv[4];

  MSSymbolServerConverter converter(argv[2], vector<string>(1, argv[1]));
  string converted_file;

  MSSymbolServerConverter::LocateResult result =
    converter.LocateAndConvertSymbolFile(missing_info,
                                         false,
                                         &converted_file,
                                         NULL);
  printf("%s: ", argv[3]);
  int return_code;
  switch(result) {
  case MSSymbolServerConverter::LOCATE_SUCCESS:
    printf("converted: %s\n", converted_file.c_str());
    return_code =  0;
    break;

  case MSSymbolServerConverter::LOCATE_RETRY:
    printf("try again later\n");
    return_code = 1;
    break;

  case MSSymbolServerConverter::LOCATE_FAILURE:
  case MSSymbolServerConverter::LOCATE_NOT_FOUND:
    printf("failed to locate symbols\n");
    return_code = 2;
    break;

  default:
    // ???
    return_code = 3;
    break;
  }
  fflush(stdout);
  fflush(stderr);

  return return_code;
}
