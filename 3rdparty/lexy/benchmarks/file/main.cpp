// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <cstdio>
#include <fstream>
#include <lexy/input/file.hpp>

std::size_t use_buffer(const lexy::buffer<>& buffer)
{
    std::size_t sum = 0;
    for (auto ptr = buffer.data(); ptr != buffer.data() + buffer.size(); ++ptr)
        sum += std::size_t(*ptr);

    if (sum % 2 == 0)
        return buffer.size();
    else
        return buffer.size() + 1;
}

std::size_t file_lexy(const char* path)
{
    auto result = lexy::read_file(path);
    return use_buffer(result.buffer());
}

std::size_t file_cfile(const char* path)
{
    auto file = std::fopen(path, "rb");

    std::fseek(file, 0, SEEK_END);
    auto size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    lexy::buffer<>::builder builder{std::size_t(size)};
    std::fread(builder.data(), 1, builder.size(), file);
    std::fclose(file);

    // To get a fair comparison, we also need to use the copy.
    auto make = lexy::make_buffer_from_raw<lexy::default_encoding, lexy::encoding_endianness::bom>;
    auto buffer = make(builder.data(), builder.size());
    return use_buffer(buffer);
}

std::size_t file_stream(const char* path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    auto          size = file.tellg();
    file.seekg(0, std::ios::beg);

    lexy::buffer<>::builder builder{std::size_t(size)};
    file.read(builder.data(), size);

    // To get a fair comparison, we also need to use the copy.
    auto make = lexy::make_buffer_from_raw<lexy::default_encoding, lexy::encoding_endianness::bom>;
    auto buffer = make(builder.data(), builder.size());
    return use_buffer(buffer);
}

constexpr auto bm_file_path = "bm-file.delete-me";

void write_file(std::size_t size)
{
    std::ofstream out(bm_file_path, std::ios::binary);
    for (auto i = 0u; i != size / sizeof(i); ++i)
        out.write(reinterpret_cast<const char*>(&i), sizeof(i));
}

int main()
{
    ankerl::nanobench::Bench b;

    auto bench_data = [&](const char* title, std::size_t size, std::size_t iterations) {
        b.minEpochIterations(iterations);
        b.title(title).relative(true);
        b.unit("byte").batch(size);

        write_file(size);
        auto benchmark = [&](auto f) { return [f] { return f(bm_file_path); }; };

        b.run("lexy", benchmark(file_lexy));

        b.run("cfile", benchmark(file_cfile));
        b.run("stream", benchmark(file_stream));
    };

    bench_data("128 B", 128, 10 * 1000);
    bench_data("1 KiB", 1024, 10 * 1000);
    bench_data("2 KiB", 2 * 1024, 10 * 1000);
    bench_data("4 KiB", 4 * 1024, 10 * 1000);

    bench_data("8 KiB", 8 * 1024, 1000);
    bench_data("16 KiB", 16 * 1024, 1000);
    bench_data("32 KiB", 32 * 1024, 1000);
    bench_data("64 KiB", 64 * 1024, 1000);
    bench_data("128 KiB", 128 * 1024, 1000);

    bench_data("1 MiB", 1024 * 1024, 100);

    std::remove(bm_file_path);
}

