#include "project.h"
#include "timestamp.h"

#include <Lib/hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>

using namespace std::string_literals;
using namespace Lib;

namespace po = boost::program_options;

static const char kFormatHelp[] = R"(
JSON IO Format Definitions:
  Input ::= {
    "dataset": Matx | string,   # string for binary output path
    "cata": string?,            # output binary if exists
    "kmin": number,             # serach range [kmin, kmax]
    "kmax": number,
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
 * @param[out] kmin 最小聚类数。
 * @param[out] kmax 最大聚类数。
 */
void
parse_input(const char* path,
            DataSet* ds,
            std::string* cataOut,
            int* kmin,
            int* kmax)
{
  bj::value val;
  {
    bj::parse_options parseOption;
    parseOption.max_depth = 4;
    parseOption.allow_comments = true;
    parseOption.allow_trailing_commas = true;
    parseOption.allow_invalid_utf8 = false;

    std::ifstream fin(path);
    val = bj::parse(fin, {}, parseOption);
  }

  const auto& obj = val.as_object();

  const auto& dataset = obj.at("dataset");
  if (dataset.is_object())
    *ds = json_to_matx<DataSet::value_type>(dataset);
  else
    matx_load_bin(ds, dataset.as_string().c_str());

  std::cout << "DataSet: " << ds->rows() << " rows, " << ds->cols()
            << " cols\nFirst: ";
  for (int i = 0; i < ds->rows(); ++i)
    std::cout << (*ds)(i) << " ";
  std::cout << std::endl;

  auto iter = obj.find("cata");
  if (iter != obj.end())
    *cataOut = iter->value().as_string();

  *kmin = obj.at("kmin").as_int64();
  *kmax = obj.at("kmax").as_int64();
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

static const auto kReportFilter = []() -> std::unique_ptr<std::regex> {
  const char* re = std::getenv("REPORT_FILTER");
  if (!re)
    return nullptr;
  return std::make_unique<std::regex>(re, std::regex_constants::basic);
}();

template<typename T>
class Algo : public T
{
  void report(Profiler::Entry& entry) noexcept override
  {
    if (kReportFilter && !std::regex_match(entry.mTag, *kReportFilter))
      return;
    // 不知道为什么，std::regex_match 会在KITSUNE数据集上报一个：
    //   malloc(): unaligned tcache chunk detected
    // 然后就崩溃了。

    std::cout << (entry.mTime - Profiler::initial()) << " " << entry.mTag;
    if (entry.mInfo)
      std::cout << " " << entry.mInfo->info();
    std::cout << '\n';
  }
};

int
kmeans(int argc, char* argv[])
{
  po::options_description od("'kemans' Options");
  od.add_options()                                             //
    ("help,h", "print help info")                              //
    ("input,i", po::value<std::string>(), "input json path")   //
    ("output,o", po::value<std::string>(), "output json path") //
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

  DataSet ds;
  std::string cataOut;
  int minK, maxK;
  parse_input(input.c_str(), &ds, &cataOut, &minK, &maxK);

  Catalog cata;
  double mse;

  Algo<KMeans> algo;
  algo(ds, minK, &cata, &mse);

  generate_output(output.c_str(), cata, cataOut, minK, mse, MseHistory(), algo);

  return 0;
}

int
algo_run(int argc, char* argv[], int which)
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
  int minK, maxK;
  parse_input(input.c_str(), &ds, &cataOut, &minK, &maxK);

  Catalog cata;
  MseHistory mseHist;
  std::size_t ansIndex;
  Profiler prof;

  switch (which) {
    case 0: {
      Algo<Elbow> elbow;
      elbow(ds, &cata, &mseHist, &ansIndex, minK, maxK);
      prof = elbow;
    } break;

    case 1: {
      Algo<LogMeans> logmeans;
      logmeans(ds, &cata, &mseHist, &ansIndex, minK, maxK);
      prof = logmeans;
    } break;

    case 2: {
      Algo<LogMeans> logmeans;
      logmeans.binary_search(ds, &cata, &mseHist, &ansIndex, minK, maxK);
      prof = logmeans;
    } break;
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
  return algo_run(argc, argv, 0);
}

int
logmeans(int argc, char* argv[])
{
  return algo_run(argc, argv, 1);
}

int
logmeans_m(int argc, char* argv[])
{
  return algo_run(argc, argv, 2);
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
  "kmin": 2,
  "kmax": 2,
})" << std::endl;
  return 0;
}

int
example_2(int argc, char* argv[])
{
  std::cout << R"({
  "dataset": "data.matx",
  "kmin": 2,
  "kmax": 3,
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
  { "logmeans-m", "Log Means algorithm (modified)", &logmeans_m },
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
