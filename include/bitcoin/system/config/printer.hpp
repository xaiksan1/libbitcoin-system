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
#ifndef LIBBITCOIN_SYSTEM_CONFIG_PRINTER_HPP
#define LIBBITCOIN_SYSTEM_CONFIG_PRINTER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <bitcoin/system/config/parameter.hpp>
#include <bitcoin/system/define.hpp>

namespace libbitcoin {
namespace system {
namespace config {

/// Shorthand for property declarations in printer class.
#define BC_PROPERTY(type, name) \
    public: virtual const type& name() const noexcept { return name##_; } \
    private: type name##_

/// Shorthand for reference getter declarations in printer class.
#define BC_PROPERTY_GET_REF(type, name) \
    public: virtual type& get_##name() noexcept { return name##_; } \
    private: type name##_

/// Class for managing the serialization of command line options and arguments.
class BC_API printer
{
public:

    /// Number of arguments above which the argument is considered unlimited.
    static const int max_arguments;

    /// Construct an instance of the printer class.
    /// settings     Populated config file settings metadata.
    /// application  This application (e.g. 'bitcoin_server').
    /// description  This application description (e.g. 'Server').
    printer(const boost::program_options::options_description& settings,
        const std::string& application,
        const std::string& description="") noexcept;

    /// Construct an instance of the printer class.
    /// options      Populated command line options metadata.
    /// arguments    Populated command line arguments metadata.
    /// application  This application (e.g. 'bx').
    /// description  This command description (e.g. 'Convert BTC').
    /// command      This command (e.g. 'btc').
    printer(const boost::program_options::options_description& options,
        const boost::program_options::positional_options_description& arguments,
        const std::string& application, const std::string& description="",
        const std::string& command="") noexcept;


    /// Defaults.
    printer(printer&&) = delete;
    printer(const printer&) = delete;
    printer& operator=(printer&&) = delete;
    printer& operator=(const printer&) = delete;
    virtual ~printer() = default;

    /// Convert a paragraph of text into a column.
    /// This formats to 80 char width as: [ 23 | ' ' | 55 | '\n' ].
    /// If one word exceeds width it will cause a column overflow.
    /// This always sets at least one line and always collapses whitespace.
    /// paragraph  The paragraph to columnize.
    /// return     The column, as a list of fragments.
    virtual std::vector<std::string> columnize(const std::string& paragraph,
        size_t width) noexcept;

    /// Format the command description.
    virtual std::string format_description() noexcept;

    /// Format the parameters table.
    /// positional  True for positional otherwize named.
    /// return      The formatted help arguments table.
    virtual std::string format_parameters_table(bool positional) noexcept;

    /// Format the settings table.
    /// return  The formatted settings table.
    virtual std::string format_settings_table() noexcept;

    /// Format a paragraph.
    /// paragraph  The text to format.
    /// return     The formatted paragraph.
    virtual std::string format_paragraph(
        const std::string& paragraph) noexcept;

    /// Format the command line usage.
    /// return  The formatted usage.
    virtual std::string format_usage() noexcept;

    /// Format the command line parameters.
    /// return  The formatted command line parameters.
    virtual std::string format_usage_parameters() noexcept;

    /// Build the list of argument name/count tuples.
    virtual void generate_argument_names() noexcept;

    /// Build the list of parameters.
    virtual void generate_parameters() noexcept;

    /// Parse the arguments and options into the normalized parameter list.
    virtual void initialize() noexcept;

    /// Serialize command line help (full details).
    /// output  Stream that is sink for output.
    virtual void commandline(std::ostream& output) noexcept;

    /// Serialize as config settings (full details).
    /// output  Stream that is sink for output.
    virtual void settings(std::ostream& output) noexcept;

    /// Virtual property declarations, passed on construct.
    BC_PROPERTY(boost::program_options::options_description, options);
    BC_PROPERTY(boost::program_options::positional_options_description, arguments);
    BC_PROPERTY(std::string, application);
    BC_PROPERTY(std::string, description);
    BC_PROPERTY(std::string, command);

    /// Virtual property declarations, generated from metadata.
    BC_PROPERTY_GET_REF(argument_list, argument_names);
    BC_PROPERTY_GET_REF(parameter_list, parameters);
};

#undef BC_PROPERTY
#undef BC_PROPERTY_GET_REF

} // namespace config
} // namespace system
} // namespace libbitcoin

#endif
