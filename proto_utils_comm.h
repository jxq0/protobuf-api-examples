#ifndef PROTO_UTILS_COMM_H
#define PROTO_UTILS_COMM_H
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <iostream>
#include <string>

class MyErrorPrinter
    : public google::protobuf::compiler::MultiFileErrorCollector {
 public:
  MyErrorPrinter() {}
  ~MyErrorPrinter() {}

  void AddError(const std::string &filename,
                int line,
                int column,
                const std::string &message) {
    std::string errmsg;
    errmsg += "filename:" + filename + ",";
    if (line != -1) {
      char tmp[20] = {0};
      snprintf(tmp, sizeof(tmp), "line: %d column: %d", line + 1, column + 1);
      errmsg += std::string(tmp);
    }
    errmsg += " err: " + message;
    std::cout << errmsg << std::endl;
  }
};
#endif /* PROTO_UTILS_COMM_H */
