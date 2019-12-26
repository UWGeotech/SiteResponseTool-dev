#include <string>
#include <vector>
#include <sstream>

extern void
split_string(const std::string& str, std::vector<std::string>& container, char delimiter='|')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter))
        container.push_back(token);
}