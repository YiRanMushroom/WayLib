#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <Macro/DefWayMacro.hpp>
#include <Util/Exceptions.hpp>

namespace WayLib::Utils {
    inline std::string GetExecutablePath() {
        static std::string path{
            []() -> std::string {
                return std::filesystem::current_path().string();
            }()
        };
        return path;
    }

    inline std::string PathOf(auto &&path) {
        return GetExecutablePath() + '/' + _forward_(path);
    }

    // Not Thread-Safe
    class FileLocation {
        std::string m_PathStr;
        std::filesystem::path m_Path;

        void remainOrCreateParentDir() {
            if (!std::filesystem::exists(m_Path.parent_path())) {
                std::filesystem::create_directories(m_Path.parent_path());
            }
        }

        void remainOrCreateFile() {
            remainOrCreateParentDir();
            if (!std::filesystem::exists(m_Path)) {
                std::ofstream ofs{m_Path};
                ofs.close();
            }
        }

        decltype(auto) assertOpens(_declself_, auto &&stream) {
            if (!stream.is_open()) {
                throw FileIOException("Failed to open file")
                      .pushOptionalData("stream", std::make_shared<
                                            std::remove_reference_t<decltype(stream)> >(stream))
                      > (std::move(stream));
            }
        }

    public:
        WayLibOnlyMovable(FileLocation)

        FileLocation() = default;

        explicit FileLocation(auto &&path) : m_PathStr{PathOf(_forward_(path))}, m_Path{m_PathStr} {
            remainOrCreateFile();
        }


        [[nodiscard]] const char *getPath() const {
            return m_PathStr.c_str();
        }

        decltype(auto) setPath(_declself_, auto &&path) {
            m_PathStr = PathOf(_forward_(path));
            m_Path = m_PathStr;
            remainOrCreateFile();
            return _self_;
        }

        [[nodiscard]] std::ofstream ofstream() const {
            return std::ofstream{m_Path};
        }

        [[nodiscard]] std::ifstream ifstream() const {
            return std::ifstream{m_Path};
        }
    };
}

#include <Macro/UndefWayMacro.hpp>
