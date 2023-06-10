#include "../lib/ini_parser.hpp"
#include <iostream>

int main(void) {
    INIParser::DOMparser parser;
    parser.AddFile("./config.ini");

    INIParser::DOM::Document doc = parser.Get();
    std::cout << doc["Section1"]["var1"].Value() << std::endl;

    auto str = doc["Section4"]["var3"].Get<std::string>();
    std::cout << str << std::endl;

    int i = doc["Section2"]["var1"].Get<int>();
    std::cout << i << std::endl;
    i += 5;
    std::cout << i << std::endl;

    return 0;
}