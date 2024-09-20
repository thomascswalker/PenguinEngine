#pragma once
#pragma warning(disable : 4244)

#include <string>
#include <vector>

#include "Math/MathFwd.h"

namespace Strings
{
	/**
	 * @brief Splits the input string with the specified delimiter.
	 *
	 * If we have the string "I am a string" and split it with a space (" "), the result would be:
	 * ["I", "am", "a", "string"]
	 *
	 * @param inString The input string to split.
	 * @param outStrings The array of output strings, the result of the split.
	 * @param delimiter The character(s) to split the string with.
	 */
	inline void split(const std::string& inString, std::vector<std::string>& outStrings, const std::string& delimiter)
	{
		// Clear the output string array first.
		outStrings.clear();

		// Temporary variable to store the current string slice.
		std::string temp;

		// Loop through each character in the input string
		size_t stringSize = inString.size();
		for (size_t index = 0; index < stringSize; index++)
		{
			// If the current character (or string equal to the size of the delimiter) is equal to the delimiter, and
			// the temporary string is not empty, add the temporary string to the output string array, clear the
			// temporary string, and iterate the index.
			if (inString.substr(index, delimiter.size()) == delimiter)
			{
				if (!temp.empty())
				{
					outStrings.push_back(temp);
					temp.clear();
					index += delimiter.size() - 1;
				}
			}
			// If index + delimiter size is equal to or exceeds the input string size, we're at the end of the string.
			// Add the remaining slice of the string to the output strings array.
			else if (index + delimiter.size() >= inString.size())
			{
				temp += inString.substr(index, delimiter.size());
				outStrings.push_back(temp);
				break;
			}
			// Add the current character to the temporary string.
			else
			{
				temp += inString[index];
			}
		}
	}

	/**
	 * @brief Checks if the input string contains the specified character.
	 * @param inString The string to search.
	 * @param c The character to check.
	 * @return bool True if the character is found, false otherwise.
	 */
	inline bool contains(const std::string& inString, const char* c)
	{
		return inString.find(c) != std::string::npos;
	}

	/**
	 * @brief Checks if the input string contains the specified string.
	 * @param inString The string to search.
	 * @param otherString The string to check.
	 * @return bool True if the string is found, false otherwise.
	 */
	inline bool contains(const std::string& inString, const std::string& otherString)
	{
		return inString.find(otherString) != std::string::npos;
	}

	/**
	 * @brief Converts the input std::string into a std::wstring.
	 * @param str The string to convert.
	 * @return std::wstring The converted string.
	 */
	inline std::wstring toWString(const std::string& str)
	{
		return {str.begin(), str.end()};
	}
} // namespace Strings
