#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <tclap/CmdLine.h>
#include <BSlogger.hpp>
#include <term.h>
#include <utility.h>
#include <datastructures_go.h>
#include <datastructures_flat.h>
#include <datastructures_hierarchical.h>
#include <test.h>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <handle_enrichment.h>
#include <handle_translation.h>


class Listener {
public:
  void start();
  Listener(enrichment e, annotation& a, std::string u) : ann(a), enr(e), uri(u) {}
  annotation& ann;
  enrichment enr;
  std::string uri;
};

void Listener::start() {
  logger log(std::cerr, uri);
  log(LOG_INFO) << "Setting up JSON listener at " << uri << '\n';
  web::http::experimental::listener::http_listener listener(U(uri));
  listener.open().wait();

  // annotation& this_ann = ann;
  // enrichment& this_enr = enr;
  // std::string& this_uri = uri;

  listener.support(web::http::methods::POST,
  [&](const web::http::http_request & request) {

    web::json::value arr;
    web::http::status_code status = web::http::status_codes::OK;
    log(LOG_INFO) << "Received POST \n";
    //log(LOG_INFO) << request.to_string();

    request
    .extract_json()
    .then([&](pplx::task<web::json::value> task)
    {
      try
      {
        const web::json::value& json = task.get();
        if (!json.is_null())
        {
          if (json.has_field("enrichment"))
          {
            if (! json.has_field("genes"))
            {
              arr["err"] = web::json::value::string("Error: Enrichment selected "
                                                    "but no test genes supplied.");
              return;
            }
            if (! json.at("genes").is_array())
            {
              arr["err"] = web::json::value::string("Error: 'genes' needs to "
                                                    "be an array of multiple genes");
              return;
            }
            web::json::array genes = json.at("genes").as_array();
            std::unordered_set<std::string> test_set;
            for (web::json::value& gene : genes)
              test_set.insert(gene.as_string());
            if (json.at("enrichment").is_array())
            {
              web::json::array tasks = json.at("enrichment").as_array();
              for (web::json::value& task : tasks)
              {
                std::string t = task.as_string();
                handle_enrichment(enr, ann, t, arr, uri, json, test_set);
              }
            }
          }
          else if (json.has_field("gene_to_term"))
          {
            if (! json.has_field("genes"))
            {
              arr["err"] = web::json::value::string("Error: Translation selected "
                                                    "but no test genes supplied.");
              return;
            }
            if (! json.at("genes").is_array())
            {
              arr["err"] = web::json::value::string("Error: 'genes' needs to "
                                                    "be an array of multiple genes");
              return;
            }
            web::json::array genes = json.at("genes").as_array();
            std::unordered_set<std::string> test_set;
            for (web::json::value& gene : genes)
              test_set.insert(gene.as_string());
            if (json.at("gene_to_term").is_array())
            {
              web::json::array tasks = json.at("gene_to_term").as_array();
              for (web::json::value& task : tasks)
              {
                std::string t = task.as_string();
                handle_gene_to_term(enr, ann, t, arr, uri, json, test_set);
              }
            }
          }
          else if (json.has_field("term_to_gene"))
          {
            if (json.at("term_to_gene").is_array())
            {
              web::json::array tasks = json.at("term_to_gene").as_array();
              for (web::json::value& task : tasks)
              {
                handle_term_to_gene(enr, ann, task, arr, uri, json);
              }
            }
          }
        }
        else
        {
          status = web::http::status_codes::BadRequest;
          arr["err"] = web::json::value::string("no data");
        }
      }
      catch (web::http::http_exception const & e)
      {
        log(LOG_ERR) << e.what() << '\n';
        arr["err"] = web::json::value::string(e.what());
      }
      catch (std::exception& e)
      {
        log(LOG_ERR) << e.what() << '\n';
        arr["err"] = web::json::value::string(e.what());
      }
    }).wait();

    web::http::http_response resp(status);
    resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    resp.headers().add(U("Content-Type"), U("application/json"));
    resp.set_body(arr);
    request.reply(resp);
  });

  listener.support(web::http::methods::OPTIONS,
  [&](const web::http::http_request & request) {
    log(LOG_INFO) << "Received OPTIONS\n";
    web::http::http_response resp(web::http::status_codes::OK);
    resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    resp.headers().add(U("Access-Control-Allow-Headers"),
                       U("Origin, Content-Type, Content-Range, "
                         "Content-Disposition, Content-Description, Accept"));
    resp.headers().add(U("Access-Control-Allow-Methods"),
                       U("OPTIONS, POST"));
    request.reply(resp);
  });

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

  logger log(std::cerr, "gopher2");

  log(LOG_INFO) << "Starting gopher2\n";

  std::string file_config;

  try
  {
    TCLAP::CmdLine cmd ("gopher2", ' ', "0.1");
    TCLAP::UnlabeledValueArg<std::string>
    arg_config("config.json", "Description", true, "Default", "string");

    cmd.add(arg_config);

    cmd.parse(argc, argv);

    file_config = arg_config.getValue();
  }
  catch (TCLAP::ArgException& e)
  {
    log(LOG_ERR) << e.error() << " in argument " << e.argId() << '\n';
  }



  try
  {
    web::json::value config = parse_config(file_config);

    annotation ann;
    ann.from_json(config);

    std::string port;
    // Port is already checked in parse_config()
    if (config["port"].is_string())
      port = config["port"].as_string();
    else
      port = std::to_string(config["port"].as_integer());

    std::string base_uri = "http://0.0.0.0:" + port;

    std::vector<Listener> listeners;
    std::vector<boost::thread> threads;

    if (config["org"].is_array())
    {
      web::json::array arr = config["org"].as_array();
      for (web::json::value& org : arr)
      {
        std::string uri = org["uri"].as_string();
        uri = base_uri + "/" + uri;
        enrichment enr;
        enr.from_json(org, ann);
        Listener l(enr, ann, uri);
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