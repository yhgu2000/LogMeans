#include "project.h"
#include "timestamp.h"

#include <Lib/hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std::string_literals;
using namespace Lib;

namespace po = boost::program_options;

static const char kFormatHelp[] = R"(
JSON IO Format Definitions:
  Input ::= {
    "dataset": Matx | string,   # string for binary output path
    "cata": string?,            # output binary if exists
    }
  Output ::= {
    "cata": Matx | string,      # string for binary output path
    "k": int,
    "mse": number,
    "msehist": MseHistory,
    "prof": Profile,
    }
  Matx ::= {
    "rows": int,
    "cols": int,
    "data": number[],
    }
  MseHistory ::=
    [number, number][]          # cata, mse
  Profile ::=
    [string, number, string?]   # tag, time, info
)";

/**
 * @brief 解析输入文件。
 *
 * @param[in] path JSON 输入文件路径。
 * @param[out] ds 数据集。
 * @param[out] cataOut 类别输出路径，如果为空则表示以 JSON 格式输出。
 */
void
parse_input(const char* path, DataSet* ds, std::string* cataOut)
{
  bj::value val;
  {
    static const bj::parse_options kParseOption{
      .max_depth = 4,
      .allow_comments = true,
      .allow_trailing_commas = true,
      .allow_invalid_utf8 = false,
    };

    std::ifstream fin(path);
    val = bj::parse(fin, {}, kParseOption);
  }

  const auto& obj = val.as_object();

  const auto& dataset = obj.at("dataset");
  if (dataset.is_object())
    *ds = json_to_matx<DataSet::value_type>(dataset);
  else
    matx_load_bin(ds, dataset.as_string().c_str());

  auto iter = obj.find("cata");
  if (iter != obj.end())
    *cataOut = iter->value().as_string();
}

/**
 * @brief 生成输出文件。
 *
 * @param[in] path JSON 文件输出路径。
 * @param[in] cata 聚类结果。
 * @param[in] cataOut 二进制聚类结果输出路径，为空则将结果内联到 JSON 中。
 * @param[in] k 聚类数。
 * @param[in] mse 误差。
 * @param[in] mseHist 误差历史。
 * @param[in] prof 用时统计。
 */
void
generate_output(const char* path,
                const Catalog& cata,
                const std::string& cataOut,
                int k,
                DataSet::value_type mse,
                const MseHistory& mseHist,
                const Profiler& prof)
{
  bj::object obj;

  if (cataOut.empty())
    obj["cata"] = matx_to_json(cata);
  else {
    obj["cata"] = cataOut;
    matx_dump_bin(cata, cataOut.c_str());
  }

  obj["k"] = k;
  obj["mse"] = mse;
  obj["msehist"] = mseHist.to_json();
  obj["prof"] = prof.to_json();

  std::ofstream fout(path, std::ios::binary);
  fout << obj << std::endl;
}

int
kmeans(int argc, char* argv[])
{
  po::options_description od("'kemans' Options");
  od.add_options()                                             //
    ("help,h", "print help info")                              //
    ("input,i", po::value<std::string>(), "input json path")   //
    ("output,o", po::value<std::string>(), "output json path") //
    ("k", po::value<int>(), "number of clusters")              //
    ;

  po::positional_options_description pod;
  pod.add("input", 1);
  pod.add("output", 1);
  pod.add("k", 1);

  po::variables_map vmap;
  po::store(
    po::command_line_parser(argc, argv).options(od).positional(pod).run(),
    vmap);
  po::notify(vmap);

  if (vmap.count("help") || argc == 1) {
    std::cout << od << kFormatHelp << std::endl;
    return 0;
  }

  auto input = vmap["input"].as<std::string>();
  auto output = vmap["output"].as<std::string>();
  auto k = vmap["k"].as<int>();

  DataSet ds;
  std::string cataOut;
  parse_input(input.c_str(), &ds, &cataOut);

  Catalog cata;
  DataSet::value_type mse;
  KMeans algo;
  algo(ds, k, &cata, &mse);

  generate_output(output.c_str(), cata, cataOut, k, mse, MseHistory(), algo);

  return 0;
}

int
elbow_or_logmeans(int argc, char* argv[], bool which)
{
  po::options_description od("Algorithm Options");
  od.add_options()                                             //
    ("help,h", "print help info")                              //
    ("input,i", po::value<std::string>(), "input json path")   //
    ("output,o", po::value<std::string>(), "output json path") //
    ;

  po::positional_options_description pod;
  pod.add("input", 1);
  pod.add("output", 1);

  po::variables_map vmap;
  po::store(
    po::command_line_parser(argc, argv).options(od).positional(pod).run(),
    vmap);
  po::notify(vmap);

  if (vmap.count("help") || argc == 1) {
    std::cout << od << kFormatHelp << std::endl;
    return 0;
  }

  auto input = vmap["input"].as<std::string>();
  auto output = vmap["output"].as<std::string>();

  DataSet ds;
  std::string cataOut;
  parse_input(input.c_str(), &ds, &cataOut);

  Catalog cata;
  MseHistory mseHist;
  std::size_t ansIndex;
  Profiler prof;

  if (which) {
    Elbow elbow;
    elbow(ds, &cata, &mseHist, &ansIndex);
    prof = elbow;
  } else {
    LogMeans logmeans;
    logmeans(ds, &cata, &mseHist, &ansIndex);
    prof = logmeans;
  }

  generate_output(output.c_str(),
                  cata,
                  cataOut,
                  mseHist[ansIndex].first,
                  mseHist[ansIndex].second,
                  mseHist,
                  prof);

  return 0;
}

int
elbow(int argc, char* argv[])
{
  return elbow_or_logmeans(argc, argv, true);
}

int
logmeans(int argc, char* argv[])
{
  return elbow_or_logmeans(argc, argv, false);
}

int
example_1(int argc, char* argv[])
{
  std::cout << R"({
  "dataset": {
    "rows": 3,
    "cols": 5,
    "data": [
      1, 2, 3, 4, 5,
      6, 7, 8, 9, 10,
      11, 12, 13, 14, 15,
    ]
  },
  "cata": "cata.matx",
})" << std::endl;
  return 0;
}

int
example_2(int argc, char* argv[])
{
  std::cout << R"({
  "dataset": "data.matx",
})" << std::endl;

  DataSet ds;
  ds.setRandom(10, 100);

  matx_dump_bin(ds, "data.matx");

  return 0;
}

struct SubCmdFunc
{
  const char *mName, *mInfo;
  int (*mFunc)(int argc, char* argv[]);
};

const SubCmdFunc kSubCmdFuncs[] = {
  { "kmeans", "manual K-Means cluster", &kmeans },
  { "elbow", "Elbow algorithm", &elbow },
  { "logmeans", "Log Means algorithm", &logmeans },
  { "example-1", "print input example 1", &example_1 },
  { "example-2", "print input example 2", &example_2 },
};

int
main(int argc, char* argv[])
try {
  po::options_description od("Options");
  od.add_options()                          //
    ("version,v", "print version info")     //
    ("help,h", "print help info")           //
    ("...",                                 //
     po::value<std::vector<std::string>>(), //
     "sub arguments")                       //
    ;

  po::positional_options_description pod;
  pod.add("...", -1);

  std::vector<std::string> opts{ argv[0] };
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-')
      opts.emplace_back(argv[i]);
    else
      break;
  }

  po::variables_map vmap;
  auto parsed = po::command_line_parser(opts)
                  .options(od)
                  .positional(pod)
                  .allow_unregistered()
                  .run();
  po::store(parsed, vmap);
  po::notify(vmap);

  if (vmap.count("version")) {
    std::cout
      << "Command-Line App"
         "\n"
         "\nBuilt: " LogMeans_TIMESTAMP "\nProject: " LogMeans_VERSION "\n"
         "\nCopyright (C) 2023 Yuhao Gu and Tong Duan. All Rights Reserved."
      << std::endl;
    return 0;
  }

  if (vmap.count("help") || argc == 1) {
    std::cout << od
              << "\n"
                 "Sub Commands:\n";
    for (auto&& i : kSubCmdFuncs)
      std::cout << "  " << std::left << std::setw(12) << i.mName << i.mInfo
                << '\n';
    std::cout << "\n"
                 "[HINT: use '<subcmd> --help' to get help for sub commands.]\n"
              << std::endl;
    return 0;
  }

  if (opts.size() < argc) {
    std::string cmd = argv[opts.size()];
    for (auto&& i : kSubCmdFuncs) {
      if (cmd == i.mName)
        return i.mFunc(argc - opts.size(), argv + opts.size());
    }
    std::cout << "invalid sub command '" << cmd << "'." << std::endl;
    return 1;
  }
}

catch (Lib::Err& e) {
  std::cout << "\nERROR! " << e.what() << "\n" << e.info() << std::endl;
  return -3;
}

catch (std::exception& e) {
  std::cout << "\nERROR! " << e.what() << std::endl;
  return -2;
}

catch (...) {
  return -1;
}
