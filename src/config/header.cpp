/**
 * Copyright (c) 2011-2022 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/system/config/header.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <bitcoin/system/chain/header.hpp>
#include <bitcoin/system/config/base16.hpp>
#include <bitcoin/system/radix/radix.hpp>

namespace libbitcoin {
namespace system {
namespace config {

using namespace boost::program_options;

header::header() NOEXCEPT
  : value_()
{
}

header::header(chain::header&& value) NOEXCEPT
  : value_(std::move(value))
{
}

header::header(const chain::header& value) NOEXCEPT
  : value_(value)
{
}

header::header(const std::string& hexcode) THROWS
  : value_()
{
    std::istringstream(hexcode) >> *this;
}

header::operator const chain::header&() const NOEXCEPT
{
    return value_;
}

std::istream& operator>>(std::istream& stream, header& argument) THROWS
{
    std::string base16;
    stream >> base16;

    data_chunk bytes;
    if (!decode_base16(bytes, base16))
        throw istream_exception(base16);

    argument.value_ = chain::header{ bytes };

    if (!argument.value_.is_valid())
        throw istream_exception(base16);

    return stream;
}

std::ostream& operator<<(std::ostream& stream, const header& argument) NOEXCEPT
{
    stream << encode_base16(argument.value_.to_data());
    return stream;
}

} // namespace config
} // namespace system
} // namespace libbitcoin
