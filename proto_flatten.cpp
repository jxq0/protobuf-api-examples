#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include "proto_utils_comm.h"

using namespace std;
namespace po = boost::program_options;

int fd_to_proto(const po::variables_map &vm,
                const boost::filesystem::path &dir,
                const google::protobuf::FileDescriptor *fd,
                google::protobuf::DescriptorPool &pool) {
  string src_msg(vm["src_msg"].as<string>());
  string dst_msg(vm["dst_msg"].as<string>());
  string src_pkg(vm["src_pkg"].as<string>());
  string src_msg_type(src_pkg + "." + src_msg);
  string dst_msg_type(src_pkg + "." + dst_msg);

  google::protobuf::FileDescriptorProto fdp;
  fd->CopyTo(&fdp);

  for (int i = 0; i < fd->dependency_count(); i++) {
    const google::protobuf::FileDescriptor *deps_fd = fd->dependency(i);
    fd_to_proto(vm, dir, deps_fd, pool);
  }

  fdp.set_name(boost::replace_all_copy(fd->name(), "/", "_"));

  if (pool.FindFileByName(fdp.name()) != NULL) {
    return 0;
  }

  for (int i = 0; i < fdp.dependency_size(); i++) {
    string dep(fdp.dependency(i));
    string new_dep(boost::replace_all_copy(dep, "/", "_"));

    fdp.set_dependency(i, new_dep);
  }

  for (int i = 0; i < fdp.message_type_size(); i++) {
    if (fdp.package() == src_pkg && fdp.message_type(i).name() == src_msg) {
      fdp.mutable_message_type(i)->set_name(dst_msg);
    }

    for (int j = 0; j < fdp.message_type(i).field_size(); ++j) {
      google::protobuf::FieldDescriptorProto *field_dp =
          fdp.mutable_message_type(i)->mutable_field(j);
      if (boost::find_first(field_dp->type_name(), src_msg_type)) {
        string new_type_name(boost::replace_all_copy(
            field_dp->type_name(), src_msg_type, dst_msg_type));
        field_dp->set_type_name(new_type_name);
      }

      if (field_dp->type() ==
          google::protobuf::FieldDescriptorProto::TYPE_UINT32) {
        field_dp->set_type(google::protobuf::FieldDescriptorProto::TYPE_UINT64);
      }
    }
  }

  fdp.clear_options();

  const google::protobuf::FileDescriptor *new_fd = pool.BuildFile(fdp);

  boost::filesystem::path new_path(dir / fdp.name());
  boost::filesystem::ofstream ofs(new_path,
                                  std::fstream::out | std::fstream::trunc);
  ofs << new_fd->DebugString();
  ofs.close();

  return 0;
}

int main(int argc, char *argv[]) {
  po::options_description desc("Options");
  // clang-format off
  desc.add_options()
      ("help,h", "show help")
      ("root,r", po::value<string>()->required(), "proto compile root")
      ("proto,p", po::value<string>()->required(), "proto path")
      ("dst,d", po::value<string>()->required(), "dst dir path")
      ("src_msg,m", po::value<string>()->required(), "src message name")
      ("dst_msg,t", po::value<string>()->required(), "dst message name")
      ("src_pkg,k", po::value<string>()->required(), "src message pkg name");
  // clang-format on

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) {
    cout << desc << endl;
    return 0;
  }

  po::notify(vm);

  string root(vm["root"].as<string>());
  string proto(vm["proto"].as<string>());

  google::protobuf::compiler::DiskSourceTree src_tree;
  src_tree.MapPath("", root);

  MyErrorPrinter err_printer;
  google::protobuf::compiler::Importer importer(&src_tree, &err_printer);
  google::protobuf::FileDescriptor *fd = importer.Import(proto);
  if (fd == NULL) {
    cout << "import failed" << endl;
    return -1;
  }

  boost::filesystem::path p(vm["dst"].as<string>());

  if (boost::filesystem::exists(p)) {
    if (boost::filesystem::is_directory(p) == false) {
      cout << "dst " << p << " is not a directory" << endl;
      exit(-1);
    }
  } else {
    boost::filesystem::create_directory(p);
  }

  google::protobuf::DescriptorPool pool;
  fd_to_proto(vm, p, fd, pool);

  return 0;
}
