#pragma once

#include <exception>
#include <string>

namespace Jupiter {

	/// <summary>
	/// Exception class for when errors occur during freeing memory
	/// </summary>
	class jpt_bad_free : public std::exception {

	public:
		jpt_bad_free(const std::string& message) : m_Message(message) {}

		const char* what() const noexcept override {
			return m_Message.c_str();
		}

	private:
		std::string m_Message;
	};

}