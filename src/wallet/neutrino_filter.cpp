/**
 * Copyright (c) 2011-2019 libbitcoin developers (see AUTHORS)
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

// Sponsored in part by Digital Contract Design, LLC

#include <bitcoin/system/wallet/neutrino_filter.hpp>

#include <algorithm>
#include <bitcoin/system/constants.hpp>
#include <bitcoin/system/data/data.hpp>
#include <bitcoin/system/math/golomb_coding.hpp>
#include <bitcoin/system/stream/stream.hpp>

namespace libbitcoin {
namespace system {
namespace neutrino {

constexpr auto rate = golomb_target_false_positive_rate;

bool compute_filter(const chain::block& validated_block, data_chunk& out_filter)
{
    const auto hash = validated_block.hash();
    const auto key = to_siphash_key(slice<zero, to_half(hash_size)>(hash));
    data_stack scripts;

    for (const auto& tx: validated_block.transactions())
    {
        if (!tx.is_coinbase())
        {
            for (const auto& input: tx.inputs())
            {
                const auto& prevout = input.previous_output();
                if (!prevout.metadata.cache.is_valid())
                    return false;

                const auto& script = prevout.metadata.cache.script();
                if (!script.empty())
                    scripts.push_back(script.to_data(false));
            }
        }

        for (const auto& output: tx.outputs())
        {
            const auto& script = output.script();

            if (!script.empty() &&
                (script.front().code() != machine::opcode::return_))
                scripts.push_back(script.to_data(false));
        }
    }

    // Order and remove duplicates.
    distinct(std::move(scripts));

    write::bits::data writer(out_filter);
    writer.write_variable(scripts.size());
    golomb::construct(writer, scripts, golomb_bits, key, rate);
    writer.flush();

    return true;
}

hash_digest compute_filter_header(const hash_digest& previous_block_hash,
    const data_chunk& filter)
{
    data_chunk data;
    data.reserve(hash_size + hash_size);
    write::bytes::data out(data);
    out.write_bytes(bitcoin_hash(filter));
    out.write_bytes(previous_block_hash);
    out.flush();

    return bitcoin_hash(data);
}

bool match_filter(const message::compact_filter& filter,
    const chain::script& script)
{
    if (script.empty() || filter.filter_type() != neutrino_filter_type)
        return false;

    read::bits::copy reader(filter.filter());
    const auto set_size = reader.read_variable();

    if (!reader)
        return false;

    const auto target = script.to_data(false);
    const auto hash = slice<zero, to_half(hash_size)>(filter.block_hash());
    const auto key = to_siphash_key(hash);

    return golomb::match(target, reader, set_size, key, golomb_bits, rate);
}

bool match_filter(const message::compact_filter& filter,
    const chain::script::list& scripts)
{
    if (scripts.empty() || filter.filter_type() != neutrino_filter_type)
        return false;

    data_stack stack;
    stack.reserve(scripts.size());

    std::for_each(scripts.begin(), scripts.end(),
        [&](const chain::script& script)
        {
            if (!script.empty())
                stack.push_back(script.to_data(false));
        });

    if (stack.empty())
        return false;

    read::bits::copy reader(filter.filter());
    const auto set_size = reader.read_variable();

    if (!reader)
        return false;

    const auto hash = slice<zero, to_half(hash_size)>(filter.block_hash());
    const auto key = to_siphash_key(hash);

    stack.shrink_to_fit();
    return golomb::match(stack, reader, set_size, key, golomb_bits, rate);
}

bool match_filter(const message::compact_filter& filter,
    const wallet::payment_address& address)
{
    return match_filter(filter, address.output_script());
}

bool match_filter(const message::compact_filter& filter,
    const wallet::payment_address::list& addresses)
{
    if (addresses.empty() || filter.filter_type() != neutrino_filter_type)
        return false;

    chain::script::list stack;
    stack.reserve(addresses.size());

    std::transform(addresses.begin(), addresses.end(), stack.begin(),
        [](const wallet::payment_address& address)
        {
            return address.output_script();
        });

    return match_filter(filter, stack);
}

} // namespace chain
} // namespace system
} // namespace libbitcoin
