#include "test_common.hpp"
#include "test_helpers.hpp"

#include <http/server.hpp>

#include <filesystem>
#include <format>
#include <fstream>
#include <source_location>
#include <vector>
#include <chrono>

namespace fs = std::filesystem;

static fs::path test_public_dir = fs::path(std::source_location::current().file_name()).remove_filename() / "public";

std::string read_file(const fs::path& file_path)
{
	std::ifstream file(file_path, std::ios::binary | std::ios::ate);
	
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file " + file_path.string());
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string file_content;
	file_content.resize(size);

	if (file.read(file_content.data(), size)) {
		return file_content;
	} else {
		throw std::runtime_error("Failed to read contents of " + file_path.string());
	}
}

static bool measure_server_performance(http::Server& srv, const std::vector<std::string>& requests, const std::vector<http::Response> expected_responses, std::size_t iter)
{
	std::jthread srv_thread([&srv]() {
		srv.run();
	});

	bool result = true;

	auto start = std::chrono::steady_clock::now();

		for (std::size_t i = 0; i < iter; ++i) {
			for (std::size_t j = 0; j < requests.size(); ++j) {
				result &= test_response_eq(srv, requests[j], expected_responses[j]);
			}
		}

	auto end = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << std::format("Processed {} requests in ... {} ms", iter * requests.size(), duration.count()) << std::endl;

	srv.stop();
	return result;
}

/*
TEST_CASE(performance_comparison)
{
	http::Server multi_threaded_srv = http::ServerBuilder()
		.set_port(http::kSelectRandomPort)
		.set_public_dir(test_public_dir)
		.build();

	http::Server single_threaded_srv = http::ServerBuilder()
		.set_port(http::kSelectRandomPort)
		.set_public_dir(test_public_dir)
		.set_thread_count(1)
		.build();

	std::vector<std::string> file_paths = { "index.html", "html_tutorial.html", "example.html" };
	std::vector<std::string> requests;
	std::vector<http::Response> expected_responses;
	constexpr std::size_t iters = 1000;

	for (const auto& file_path : file_paths) {
		requests.emplace_back(std::format(
			"GET /{} HTTP/1.1\r\n"
			"Host: www.example.com\r\n\r\n",
			file_path
		));
		expected_responses.emplace_back(
			http::Response(http::StatusCode::Ok)
			.add_header(http::header::kContentType, http::header::ContentType::Value::TextHtml)
			.add_body(read_file(test_public_dir / file_path))
		);
	}

	bool result = true;

	std::cout << "----- Single threaded server performance -----" << std::endl;
	result &= measure_server_performance(single_threaded_srv, requests, expected_responses, iters);
	std::cout << "----- Multi threaded server performance  -----" << std::endl;
	result &= measure_server_performance(multi_threaded_srv, requests, expected_responses, iters);

	return result;
}
*/
