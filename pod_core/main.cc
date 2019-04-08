#include "capi/scheme_plain_otrange_test_capi.h"
#include "capi/scheme_plain_range_test_capi.h"
#include "capi/scheme_table_batch2_test_capi.h"
#include "capi/scheme_table_batch_test_capi.h"
#include "capi/scheme_table_otbatch_test_capi.h"
#include "ecc_pub.h"
#include "public.h"
#include "scheme_misc.h"
#include "scheme_plain_otrange_test.h"
#include "scheme_plain_range_test.h"
#include "scheme_table_batch2_test.h"
#include "scheme_table_batch_test.h"
#include "scheme_table_otbatch_test.h"
#include "scheme_table_otvrfq_test.h"
#include "scheme_table_vrfq_test.h"

int main(int argc, char** argv) {
  setlocale(LC_ALL, "");

  using scheme::Action;
  using scheme::Mode;
  Mode mode;
  Action action;
  std::string publish_path;
  std::string output_path;
  std::string ecc_pub_file;
  Range demand_range;
  Range phantom_range;
  std::string query_key;
  std::vector<std::string> query_values;
  std::vector<std::string> phantom_values;
  uint32_t omp_thread_num = 0;
  std::vector<Range> demand_ranges;
  std::vector<Range> phantom_ranges;
  bool use_capi = false;
  bool test_evil = false;

  try {
    po::options_description options("command line options");
    options.add_options()("help,h", "Use -h or --help to list all arguments")(
        "ecc_pub_file,e", po::value<std::string>(&ecc_pub_file),
        "Provide the ecc pub file")("mode,m", po::value<Mode>(&mode),
                                    "Provide pod mode (plain, table)")(
        "action,a", po::value<Action>(&action),
        "Provide action (range_pod, ot_range_pod, vrf_query, ot_vrf_query...)")(
        "publish_path,p", po::value<std::string>(&publish_path),
        "Provide the publish path")("output_path,o",
                                    po::value<std::string>(&output_path),
                                    "Provide the output path")(
        "demand_range,d", po::value<Range>(&demand_range),
        "Provide the demand range")("phantom_range,g",
                                    po::value<Range>(&phantom_range),
                                    "Provide the phantom range")(
        "demand_ranges",
        po::value<std::vector<Range>>(&demand_ranges)->multitoken(),
        "Provide the demand ranges")(
        "phantom_ranges",
        po::value<std::vector<Range>>(&phantom_ranges)->multitoken(),
        "Provide the phantom range(plain mode)")(
        "query_key,k", po::value<std::string>(&query_key),
        "Provide the query key name(table mode)")(
        "key_value,v",
        po::value<std::vector<std::string>>(&query_values)->multitoken(),
        "Provide the query key values(table mode, for example "
        "-v value_a "
        "value_b value_c)")(
        "phantom_key,n",
        po::value<std::vector<std::string>>(&phantom_values)->multitoken(),
        "Provide the query key phantoms(table mode, for example -n "
        "phantoms_a "
        "phantoms_b phantoms_c)")(
        "omp_thread_num", po::value<uint32_t>(&omp_thread_num),
        "Provide the number of the openmp thread, 1: disable "
        "openmp, 0: default.")("use_c_api,c", "")("test_evil", "");

    boost::program_options::variables_map vmap;

    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, options), vmap);
    boost::program_options::notify(vmap);

    if (vmap.count("help")) {
      std::cerr << options << std::endl;
      return -1;
    }

    if (vmap.count("use_c_api")) {
      use_capi = true;
    }

    if (vmap.count("test_evil")) {
      test_evil = true;
    }
  } catch (std::exception& e) {
    std::cerr << "Unknown parameters.\n"
              << e.what() << "\n"
              << "-h or --help to list all arguments.\n";
    return -1;
  }

  if (omp_thread_num) {
    std::cout << "omp_set_num_threads: " << omp_thread_num << "\n";
    omp_set_num_threads(omp_thread_num);
  }

  std::cout << "omp_get_max_threads: " << omp_get_max_threads() << "\n";

  if (ecc_pub_file.empty() || !fs::is_regular(ecc_pub_file)) {
    std::cerr << "Open ecc_pub_file " << ecc_pub_file << " failed" << std::endl;
    return -1;
  }

  if (output_path.empty()) {
    std::cerr << "Want output_path(-o)" << std::endl;
    return -1;
  }

  if (publish_path.empty() || !fs::is_directory(publish_path)) {
    std::cerr << "Open publish_path " << publish_path << " failed" << std::endl;
    return -1;
  }

  if (!fs::is_directory(output_path) && !fs::create_directories(output_path)) {
    std::cerr << "Create " << output_path << " failed" << std::endl;
    return -1;
  }
  // clean the output_path
  for (auto& entry :
       boost::make_iterator_range(fs::directory_iterator(output_path), {})) {
    fs::remove_all(entry);
  }

  if (mode == Mode::kPlain) {
    if (action == Action::kVrfQuery || action == Action::kOtVrfQuery ||
        action == Action::kVrfPod || action == Action::kOtVrfPod) {
      std::cerr << "Plain mode does not support vrf query&pod action\n";
      return -1;
    }
  }

  InitEcc();

  if (!LoadEccPub(ecc_pub_file)) {
    std::cerr << "Open ecc pub file " << ecc_pub_file << " failed" << std::endl;
    return -1;
  }

  if (mode == Mode::kPlain) {
    if (action == Action::kRangePod) {
      if (use_capi) {
        using scheme::plain::range::capi::Test;
        return Test(publish_path, output_path, demand_range, test_evil) ? 0
                                                                        : -1;
      } else {
        using scheme::plain::range::Test;
        return Test(publish_path, output_path, demand_range, test_evil) ? 0
                                                                        : -1;
      }
    } else if (action == Action::kOtRangePod) {
      if (use_capi) {
        using scheme::plain::otrange::capi::Test;
        return Test(publish_path, output_path, demand_range, phantom_range,
                    test_evil)
                   ? 0
                   : -1;
      } else {
        using scheme::plain::otrange::Test;
        return Test(publish_path, output_path, demand_range, phantom_range,
                    test_evil)
                   ? 0
                   : -1;
      }
    } else {
      std::cerr << "Not implement yet.\n";
      return -1;
    }
  } else {
    if (action == Action::kVrfQuery) {
      using scheme::table::vrfq::Test;
      return Test(publish_path, query_key, query_values) ? 0 : -1;
    } else if (action == Action::kOtVrfQuery) {
      using scheme::table::otvrfq::Test;
      return Test(publish_path, query_key, query_values, phantom_values) ? 0
                                                                         : -1;
    } else if (action == Action::kBatchPod) {
      if (use_capi) {
        using scheme::table::batch::capi::Test;
        return Test(publish_path, output_path, demand_ranges, test_evil) ? 0
                                                                         : -1;
      } else {
        using scheme::table::batch::Test;
        return Test(publish_path, output_path, demand_ranges, test_evil) ? 0
                                                                         : -1;
      }
    } else if (action == Action::kOtBatchPod) {
      if (use_capi) {
        using scheme::table::otbatch::capi::Test;
        return Test(publish_path, output_path, demand_ranges, phantom_ranges,
                    test_evil)
                   ? 0
                   : -1;
      } else {
        using scheme::table::otbatch::Test;
        return Test(publish_path, output_path, demand_ranges, phantom_ranges,
                    test_evil)
                   ? 0
                   : -1;
      }
    } else if (action == Action::kBatch2Pod) {
      if (use_capi) {
        using scheme::table::batch2::capi::Test;
        return Test(publish_path, output_path, demand_ranges, test_evil) ? 0
                                                                         : -1;
      } else {
        using scheme::table::batch2::Test;
        return Test(publish_path, output_path, demand_ranges, test_evil) ? 0
                                                                         : -1;
      }
    } else {
      std::cerr << "Not implement yet.\n";
      return -1;
    }
  }
}
