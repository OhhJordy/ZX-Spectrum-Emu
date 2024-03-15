#include "utils.h"

std::string readFileToString(std::string filename)
{
    std::ifstream t(filename);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());

    return str;
}

std::vector<std::string> splitByWhitespace(std::string str)
{
    std::istringstream iss(str);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                      std::istream_iterator<std::string>{}};
    return tokens;
}