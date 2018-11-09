#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <BSlogger.hpp>
#include <term.h>
#include <utility.h>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>
#include <test.h>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <handle_enrichment.h>
#include <handle_translation.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

class Listener {
public:
  void start();
  Listener(enrichment e, annotation& a, std::string u,
           const web::json::value& o) : ann(a), enr(e), opt(o), uri(u) {}
  annotation& ann;
  enrichment enr;
  std::string uri;
  const web::json::value& opt;
  web::http::experimental::listener::http_listener_config conf;
};

void options_request(const web::http::http_request & request) {
  web::http::http_response resp(web::http::status_codes::OK);
  resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
  resp.headers().add(U("Access-Control-Allow-Headers"),
                     U("Origin, Content-Type, Content-Range, "
                       "Content-Disposition, Content-Description, Accept"));
  resp.headers().add(U("Access-Control-Allow-Methods"),
                     U("OPTIONS, POST, GET"));
  request.reply(resp);
}

void get_request(const web::http::http_request & request) {
  web::http::http_response resp(web::http::status_codes::OK);
  resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
  resp.headers().add(U("Access-Control-Allow-Headers"),
                     U("Origin, Content-Type, Content-Range, "
                       "Content-Disposition, Content-Description, Accept"));
  resp.headers().add(U("Access-Control-Allow-Methods"),
                     U("OPTIONS, POST, GET"));
  web::json::value arr;
  arr["status"] = web::json::value::string("OK. Gofer2 is running.");
  resp.set_body(arr);
  request.reply(resp);
}

void Listener::start() {
  logger log(std::cerr, uri);

  if (opt.has_field("cert") && opt.has_field("chain") && opt.has_field("key"))
  {
    conf.set_ssl_context_callback([&](boost::asio::ssl::context & ctx)
    {
      ctx.set_options(boost::asio::ssl::context::default_workarounds);

      // Password callback needs to be set before setting cert and key.
      // ctx.set_password_callback([](std::size_t max_length, 
      //                              boost::asio::ssl::context::password_purpose purpose)
      // {
      //   return "password";
      // });

      ctx.use_certificate_file(opt.at("cert").as_string(), boost::asio::ssl::context::pem);
      ctx.use_private_key_file(opt.at("key").as_string(), boost::asio::ssl::context::pem);
      ctx.use_certificate_chain_file(opt.at("chain").as_string());
    });

  }

  std::string uri_enrichment = concat_uri(uri, "enrichment");
  std::string uri_t2g = concat_uri(uri, "term-to-gene");
  std::string uri_g2t = concat_uri(uri, "gene-to-term");

  web::http::experimental::listener::http_listener
  listener_enr(U(uri_enrichment), conf);
  listener_enr.open().wait();
  log(LOG_INFO) << "Set up JSON listener at " << uri_enrichment << '\n';

  web::http::experimental::listener::http_listener
  listener_t2g(U(uri_t2g), conf);
  listener_t2g.open().wait();
  log(LOG_INFO) << "Set up JSON listener at " << uri_t2g << '\n';

  web::http::experimental::listener::http_listener
  listener_g2t(U(uri_g2t), conf);
  listener_g2t.open().wait();
  log(LOG_INFO) << "Set up JSON listener at " << uri_g2t << '\n';

  /*////////////////////////////////////
  //
  //  HANDLE ENRICHMENT REQUESTS
  //
  *////////////////////////////////////

  listener_enr.support(web::http::methods::POST,
  [&](const web::http::http_request & request) {

    web::json::value arr;
    web::http::status_code status = web::http::status_codes::OK;

    //std::cerr << request.to_string() << '\n';

    request
    .extract_json()
    .then([&](pplx::task<web::json::value> task)
    {
      try
      {
        const web::json::value& json = task.get();
        if (!json.is_null())
        {
          if (! json.has_field("genes"))
          {
            append_error(arr, "Error: Enrichment selected but no test genes supplied.");
            return;
          }
          if (! json.at("genes").is_array())
          {
            append_error(arr, "Error: 'genes' needs to be a JSON array of multiple genes");
            return;
          }
          web::json::array genes = json.at("genes").as_array();
          std::unordered_set<std::string> test_set;
          for (web::json::value& gene : genes)
            test_set.insert(gene.as_string());
          if (json.at("target").is_array())
          {
            web::json::array tasks = json.at("target").as_array();
            for (web::json::value& task : tasks)
            {
              std::string t = task.as_string();
              handle_enrichment(enr, ann, t, arr, uri, json, test_set);
            }
          }
          else if (json.at("target").is_string())
          {
            web::json::value task = json.at("target");
            std::string t = task.as_string();
            handle_enrichment(enr, ann, t, arr, uri, json, test_set);
          }
          else
          {
            status = web::http::status_codes::BadRequest;
            append_error(arr, "'target' needs to be a JSON array of enrichment "
                         "targets or a string of a single enrichment target.");
          }
        }
        else
        {
          status = web::http::status_codes::BadRequest;
          append_error(arr, "No data");
        }
      }
      catch (web::http::http_exception const & e)
      {
        log(LOG_ERR) << e.what() << '\n';
        append_error(arr, e.what());
      }
      catch (std::exception& e)
      {
        log(LOG_ERR) << e.what() << '\n';
        append_error(arr, e.what());
      }
    }).wait();

    web::http::http_response resp(status);
    resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    resp.headers().add(U("Content-Type"), U("application/json"));
    resp.set_body(arr);
    request.reply(resp);
  });

  /*////////////////////////////////////
  //
  //  HANDLE TERM-TO-GENE REQUESTS
  //
  *////////////////////////////////////

  listener_t2g.support(web::http::methods::POST,
  [&](const web::http::http_request & request) {

    web::json::value arr;
    web::http::status_code status = web::http::status_codes::OK;

    request
    .extract_json()
    .then([&](pplx::task<web::json::value> task)
    {
      try
      {
        const web::json::value& json = task.get();
        if (!json.is_null())
        {
          if (json.at("target").is_array())
          {
            web::json::array tasks = json.at("target").as_array();
            for (web::json::value& task : tasks)
            {
              handle_term_to_gene(enr, ann, task, arr, uri, json);
            }
          }
          else
          {
            web::json::value task = json.at("target");
            handle_term_to_gene(enr, ann, task, arr, uri, json);
          }
        }
        else
        {
          status = web::http::status_codes::BadRequest;
          append_error(arr, "No data");
        }
      }
      catch (web::http::http_exception const & e)
      {
        log(LOG_ERR) << e.what() << '\n';
        append_error(arr, e.what());
      }
      catch (std::exception& e)
      {
        log(LOG_ERR) << e.what() << '\n';
        append_error(arr, e.what());
      }
    }).wait();

    web::http::http_response resp(status);
    resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    resp.headers().add(U("Content-Type"), U("application/json"));
    resp.set_body(arr);
    request.reply(resp);
  });

  /*////////////////////////////////////
  //
  //  HANDLE GENE-TO-TERM REQUESTS
  //
  *////////////////////////////////////

  listener_g2t.support(web::http::methods::POST,
  [&](const web::http::http_request & request) {

    web::json::value arr;
    web::http::status_code status = web::http::status_codes::OK;

    request
    .extract_json()
    .then([&](pplx::task<web::json::value> task)
    {
      try
      {
        const web::json::value& json = task.get();

        if (!json.is_null())
        {
          if (! json.has_field("genes"))
          {
            status = web::http::status_codes::BadRequest;
            append_error(arr, "Error: translation selected but no test genes supplied.");
            return;
          }

          web::json::array genes = json.at("genes").as_array();
          std::unordered_set<std::string> test_set;
          for (web::json::value& gene : genes)
            test_set.insert(gene.as_string());
          if (json.at("target").is_array())
          {
            web::json::array tasks = json.at("target").as_array();
            for (web::json::value& task : tasks)
            {
              std::string t = task.as_string();
              handle_gene_to_term(enr, ann, t, arr, uri, json, test_set);
            }
          }
          else
          {
            web::json::value task = json.at("target");
            std::string t = task.as_string();
            handle_gene_to_term(enr, ann, t, arr, uri, json, test_set);
          }
        }
        else
        {
          status = web::http::status_codes::BadRequest;
          append_error(arr, "No data");
        }
      }
      catch (web::http::http_exception const & e)
      {
        log(LOG_ERR) << e.what() << '\n';
        append_error(arr, e.what());
      }
      catch (std::exception& e)
      {
        log(LOG_ERR) << e.what() << '\n';
        append_error(arr, e.what());
      }
    }).wait();

    web::http::http_response resp(status);
    resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    resp.headers().add(U("Content-Type"), U("application/json"));
    resp.set_body(arr);
    request.reply(resp);
  });

  /*////////////////////////////////////
  //
  //  HANDLE ALL OPTIONS REQUESTS
  //
  *////////////////////////////////////

  listener_enr.support(web::http::methods::OPTIONS, options_request);
  listener_g2t.support(web::http::methods::OPTIONS, options_request);
  listener_t2g.support(web::http::methods::OPTIONS, options_request);

  listener_enr.support(web::http::methods::GET, get_request);
  listener_g2t.support(web::http::methods::GET, get_request);
  listener_t2g.support(web::http::methods::GET, get_request);

  while (true)
  {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
  }
}

void handle_post(const web::http::http_request& request, std::string x) {
  logger log(std::cerr, "handle_post");
  log(LOG_INFO) << "Received POST\n";
}

unsigned logger::_loglevel = LOG_DEFAULT;
int main(int argc, char ** argv) {

  logger log(std::cerr, "Gofer2");

  std::string file_config;

  try
  {
    po::options_description desc("Gofer2: Serving GSEA as a REST API...\n"
                                 "Required options");
    desc.add_options()
    ("in-file", po::value<std::string>(&file_config)->required(), "Config file");

    po::positional_options_description p;
    p.add("in-file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);

    if (! vm.count("in-file"))
    {
      std::cerr << desc << '\n';
      return 22;
    }

    po::notify(vm);
  }
  catch (std::exception& e)
  {
    log(LOG_ERR) << e.what() << '\n';
  }

  try
  {
    log(LOG_INFO) << "Starting Gofer2\n";

    file_config = fs::canonical(fs::path(file_config)).string();

    web::json::value config = parse_config(file_config);

    annotation ann;
    ann.from_json(config);

    std::string port;
    std::string ip;
    // Port is already checked in parse_config()
    if (config["port"].is_string())
      port = config["port"].as_string();
    else
      port = std::to_string(config["port"].as_integer());

    ip = config["ip"].as_string();

    std::string protocol;

    if (config.has_field("cert") && 
        config.has_field("chain") && 
        config.has_field("key"))
    {
      protocol = "https://";
    }
    else
    {
      protocol = "http://";
    }

    std::string base_uri = protocol + ip + ":" + port;

    std::vector<Listener> listeners;
    std::vector<boost::thread> threads;

    if (config["org"].is_array())
    {
      web::json::array arr = config["org"].as_array();
      for (web::json::value& org : arr)
      {
        std::string uri = org["uri"].as_string();
        uri = concat_uri(base_uri, uri);
        enrichment enr;
        enr.from_json(org, ann);
        Listener l(enr, ann, uri, config);
        listeners.push_back(l);
      }
    }

    for (auto& listener : listeners)
    {
      threads.push_back(boost::thread{ &Listener::start, &listener });
    }

    for (auto& t : threads)
    {
      t.join();
    }
  }
  catch (std::system_error& e)
  {
    log(LOG_ERR) << e.what() << '\n';
    return e.code().value();
  }
  catch (std::exception& e)
  {
    log(LOG_ERR) << e.what() << '\n';
    return 1;
  }
  return 0;
}