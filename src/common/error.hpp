/* CoktelADL2VGM - Tool to convert Coktel Vision's AdLib music to VGM
 *
 * CoktelADL2VGM is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * CoktelADL2VGM is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with CoktelADL2VGM. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file common/error.hpp
 *  Basic exceptions to throw.
 */

#ifndef COMMON_ERROR_HPP
#define COMMON_ERROR_HPP

#include <string>
#include <stack>
#include <exception>

namespace Common {

/** Exeption that provides a stack of explanations. */
class StackException : public std::exception {
public:
	typedef std::stack<std::string> Stack;

	StackException();
	StackException(const char *s, ...);
	StackException(const StackException &e);
	StackException(const std::exception &e);
	~StackException() throw();

	void add(const char *s, ...);
	void add(const std::exception &e);

	const char *what() const throw();

	Stack &getStack();

private:
	Stack _stack;
};

typedef StackException Exception;

extern const Exception kOpenError;
extern const Exception kReadError;
extern const Exception kSeekError;
extern const Exception kWriteError;

void printException(Exception &e, const std::string &prefix = "ERROR: ");

} // End of namespace Common

#endif // COMMON_ERROR_HPP
