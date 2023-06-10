#pragma once
#include <string>
#include <cctype>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <functional>
#include <string_view>
#include <unordered_map>

namespace INIParser {
    namespace DOM {
        class KVPair {
            public:
                KVPair() = default;
                KVPair(const std::string_view& key, const std::string_view& value) : m_key(key), m_value(value) {}

                inline const std::string& Key() const noexcept { return m_key; }
                inline const std::string& Value() const noexcept { return m_value; }

                template<typename T> // working but stringstream break at space " "
                T Get(const T& defaultValue = T()) {
                    T temp = defaultValue;
                    std::stringstream ss;
                    ss << m_value;
                    ss >> temp;
                    return temp;
                }
                template<>
                std::string Get(const std::string& defaultValue) { return m_value; } // for get string with space " "

            private:
                std::string m_key, m_value;
        }; // end class KVPair

        class Section {
            public:
                using Key = std::string;

                Section() = default;
                Section(const std::string_view& name) : m_name(name) {}

                inline const std::string& Name() const noexcept { return m_name; }

                inline KVPair& operator[](const Key& key) { return m_kvPairs.find(key)->second; }
                inline const KVPair& operator[](const Key& key) const { return m_kvPairs.find(key)->second; }

                std::vector<Key> Keys() const {
                    std::vector<Key> keys;
                    for (const auto& pair : m_kvPairs) {
                        keys.push_back(pair.first);
                    }
                    return keys;
                }

                void Append(KVPair&& value) { m_kvPairs.emplace(value.Key(), std::move(value)); }

            private:
                std::string m_name;
                std::unordered_map<Key, KVPair> m_kvPairs;
        }; // end class Section

        class Document {
            public:
                using Key = std::string;

                inline Section& operator[](const Key& name) {
                    // Existing section
                    auto it = m_sections.find(name);
                    if (it != m_sections.end())
                        return it->second;

                    // New section
                    auto& section = m_sections[name];
                    section = std::move(Section(name));
                    return section;
                }
                inline const Section& operator[](const Key& name) const { return m_sections.find(name)->second; }

                std::vector<Key> Sections() const {
                    std::vector<Key> sections;
                    for (const auto& section : m_sections)
                        sections.push_back(section.first);
                    return sections;
                }

            private:
                std::unordered_map<Key, Section> m_sections;
        }; // end class Document
    } // end namespace SAX

    class Exception : public std::runtime_error {
        public:
            using std::runtime_error::runtime_error;
    };

    class Parser {
        private:
            enum class State {
                ReadForData,    // Internal state flushed and ready for any ini data
                Comment,        // Comment started
                Section,        // Section is beeing defined
                KVKey,          // Key value
                KVKeyDone,
                KVEqual,        // Equal sign of KV value reached
                KVValue         // KV value coming
            };

        // Functions
        public:
            void AddFile(const std::filesystem::path& path);
            void AddString(const std::string_view& str);
            void Reset();

            virtual void ParseKVPair(const std::string& section, const std::string& key, const std::string& value) = 0;

        private:
            void ProcessChar(char c);
        // End functions

        private:
            State m_state = State::ReadForData;
            std::string m_currentSection, m_currentKey, m_currentValue;
    }; // end class Parser

    class SAXParser : public Parser { // for output all datas
        public:
            using Callback = std::function<void(const std::string&, const std::string&, const std::string&)>;

            SAXParser() = default;
            SAXParser(Callback callback) :m_callback(callback) {}

            void ParseKVPair(const std::string& section, const std::string& key, const std::string& value) override;
        private:
            Callback m_callback;
    };

    class DOMparser : public Parser { // for get elements
        public:
            void ParseKVPair(const std::string& section, const std::string& key, const std::string& value) override;
            inline DOM::Document& Get() { return m_document; }

        private:
            DOM::Document m_document;
    };
} // end INIParser