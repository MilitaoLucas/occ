#include "inputfile.h"
#include "logger.h"
#include <scn/scn.h>
#include <fstream>
#include <regex>
#include "util.h"

namespace tonto::io {

GaussianInputFile::GaussianInputFile(const std::string &filename)
{
    std::ifstream file(filename);
    parse(file);
}

GaussianInputFile::GaussianInputFile(std::istream &stream)
{
    parse(stream);
}

void GaussianInputFile::parse(std::istream &stream)
{
    using tonto::util::trim;
    std::string line;
    while(std::getline(stream, line)) {
        trim(line);
        if(line[0] == '%') parse_link0(line);
        else if(line[0] == '#') {
            parse_route_line(line);
            tonto::log::info("Found route line, breaking");
            break;
        }
    }
}

void GaussianInputFile::parse_link0(const std::string &line)
{
    using tonto::util::trim_copy;
    auto eq = line.find('=');
    std::string cmd = line.substr(1, eq - 1);
    std::string arg = line.substr(eq + 1);
    tonto::log::info("Found link0 command {} = {}", cmd, arg);
    link0_commands.push_back({std::move(cmd), std::move(arg)});
}

void GaussianInputFile::parse_route_line(const std::string &line)
{
    using tonto::util::to_lower_copy;
    using tonto::util::trim;
    auto ltrim = to_lower_copy(line);
    trim(ltrim);
    if(ltrim.size() == 0) return;

    const std::regex method_regex(R"###(([\w-\+\*\(\)]+)\s*\/\s*([\w-\+\*\(\)]+))###", std::regex_constants::ECMAScript);
    std::smatch sm;
    if(std::regex_search(ltrim, sm, method_regex))
    {
        method = sm[1].str();
        basis_name = sm[2].str();
    }
    else
    {
        tonto::log::error("Did not find method/basis in route line in gaussian input!");
    }
    tonto::log::info("Found route command: method = {} basis = {}", method, basis_name);
}

}
