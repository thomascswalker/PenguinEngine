#pragma once
#include <filesystem>
#include <fstream>

namespace IO
{
    static bool Exists(const std::string& FileName)
    {
        return std::filesystem::exists(FileName);
    }

    static bool IsType(const std::string& FileName, const std::string& Type)
    {
        return FileName.ends_with(Type);
    }

    static bool ReadFile(const std::string& FileName, std::string& Buffer)
    {
        if (!Exists(FileName))
        {
            return false;
        }
        std::ifstream Stream(FileName, std::ios::in | std::ios::binary);
        if (Stream.bad())
        {
            return false;
        }
        const auto Size = std::filesystem::file_size(FileName);
        Buffer.resize(Size, '\0');
        Stream.read(Buffer.data(), Size);
        return true;
    }

    static std::istream& ReadLine(std::istream& Stream, std::string& Line)
    {
        // Clear the content of the Line
        Line.clear();

        // Prepare Stream for input
        const std::istream::sentry Sentry(Stream, true);
        if (!Sentry)
        {
            return Stream;
        }

        // Create a new buffer from our Stream
        std::streambuf* StreamBuffer = Stream.rdbuf();


        // Switch on current char
        while (true)
        {
            // Read the current character
            const int Char = StreamBuffer->sbumpc();
            switch (Char)
            {
            // For \n
            case '\n' :
                {
                    return Stream;
                }
            // For \r\n
            case '\r' :
                {
                    // If the next character is a new Line, we'll bump to that next Line
                    if (StreamBuffer->sgetc() == '\n')
                    {
                        StreamBuffer->sbumpc();
                    }
                    return Stream;
                }
            // For \r
            case EOF : // -1
                {
                    if (Line.empty())
                    {
                        Stream.setstate(std::ios::eofbit);
                    }
                    return Stream;
                }
            // All other cases, the rest of the Line
            default :
                {
                    Line += static_cast<char>(Char);
                }
            }
        }
    }


}
