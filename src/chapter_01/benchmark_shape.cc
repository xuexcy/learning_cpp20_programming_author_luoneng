/**
########################################################################
#
# Copyright (c) 2025 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2025/05/17 16:35:21
# Desc   :
########################################################################
*/

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <vector>

#include "shape.h"

#include "benchmark/benchmark.h"

constexpr size_t kSize{100000};

static void BM_subtype(benchmark::State& state) {
  using namespace subtype;
  for (auto _ : state) {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.reserve(kSize);
    for (size_t i = 0; i < kSize; ++i) {
      if (std::rand() % 100 > 50) {
        // 根据文中描述，这里的动态内存分配比较耗时
        shapes.emplace_back(std::make_unique<Rectangle>(std::rand() % 10, std::rand() % 10));
      } else {
        shapes.emplace_back(std::make_unique<Circle>(std::rand() % 10));
      }
    }
    for (const auto& shape : shapes) {
      benchmark::DoNotOptimize(shape->get_area());
    }
  }
}
BENCHMARK(BM_subtype);

static void BM_ad_hoc(benchmark::State& state) {
  using namespace ad_hoc;
  for (auto _ : state) {
    std::vector<Shape> shapes;
    shapes.reserve(kSize);
    for (size_t i = 0; i < kSize; ++i) {
      if (std::rand() % 100 > 50) {
        shapes.emplace_back(Rectangle{std::rand() % 10 * 1.0, std::rand() % 10 * 1.0});
      } else {
        shapes.emplace_back(Circle{std::rand() % 10 * 1.0});
      }
    }
    for (const auto& shape : shapes) {
      benchmark::DoNotOptimize(get_area(shape));

    }
  }
}
BENCHMARK(BM_ad_hoc);

BENCHMARK_MAIN();
