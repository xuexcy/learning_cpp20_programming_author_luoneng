/**
########################################################################
#
# Copyright (c) 2026 xx.com, Inc. All Rights Reserved
#
########################################################################
# Author : xuechengyun
# E-mail : xuechengyunxue@gmail.com
# Date   : 2026/01/05 15:47:32
# Desc   :
########################################################################
*/
#pragma once

#include <type_traits>

#include "algorithm.h"
#include "higher_order_functions.h"
#include "type_list.h"

// 非运行时计算最短路径: 1. 提前算好 2. 存到文件 3. 加载文件
// 最终不存在图，只存在最短路径数据

// eDSL: Embedded Domain-Specific Language，嵌入式领域特定语言
// 利用宿主语言的特性来创建领域特定的抽象

/*
1. Graph 描述图
2. link 描述链
3. node 描述节点
4. -> 表达链接关系
*/
template <char ID>
struct Node {
  constexpr static char id = ID;
};  // struct Node

/*
1. 每条链由至少一条边组成
2. 每条边由两个端点组成
*/
template <typename Node>
concept Vertex = requires {
  Node::id;
};
template <Vertex F, Vertex T>
struct Edge {
  using From = F;
  using To = T;
};  // struct Edge

template <typename... Edge>
using Edges = TypeList<Edge...>;

template <typename Node = void>
requires (Vertex<Node> || std::is_void_v<Node>)
struct EdgeTrait {
  template <typename Edge>
  using IsFrom = std::is_same<typename Edge::From, Node>;
  template <typename Edge>
  using IsTo = std::is_same<typename Edge::To, Node>;
  template <typename Edge>
  using GetFrom = Return<typename Edge::From>;
  template <typename Edge>
  using GetTo = Return<typename Edge::To>;
};  // struct EdgeTrait

// 输入一条链，输出链上的边集信息
template <typename Link, TL Out = TypeList<>>
struct Chain;

// 偏特化边界情况 -> void 表明链尾
template <Vertex F, TL Out>
struct Chain<auto(*)(F) -> void, Out> {
  using From = F;
  using type = Out;
};  // struct Chain<auto(*)(F) -> void, Out>

template <Vertex F, typename T, TL Out>
struct Chain<auto(*)(F) -> T, Out> {
  using To = typename Chain<T, Out>::From;
public:
  using From = F;
  using type = typename Chain<T, typename Out::template append<Edge<From, To>>>::type;
};  // struct Chain<auto(*)(F) -> T, Out>

template <typename Link, TL Out = TypeList<>>
using Chain_t = typename Chain<Link, Out>::type;

template <typename... Chains>
class Graph {
  using Edges = Unique_t<Concat_t<Chain_t<Chains>...>>;

  template <Vertex From, Vertex Target, TL Path = TypeList<>>
  struct PathFinder;

  // From == Target, 结束
  template <Vertex Target, TL Path>
  struct PathFinder<Target, Target, Path> : Path::template append<Target> {
  };  // struct PathFinder<Target, Target, Path>

  // From 在 Path 中，说明成环
  template <Vertex CurrNode, Vertex Target, TL Path>
  requires (Elem2_v<Path, CurrNode>)
  struct PathFinder<CurrNode, Target, Path> : TypeList<> {
  };  // struct PathFinder<CurrNode, Target, Path>

  template <Vertex CurrentNode, Vertex Target, TL Path>
  class PathFinder {
    using EdgesFrom = Filter_t<Edges, EdgeTrait<CurrentNode>::template IsFrom>;
    using NextNodes = Map_t<EdgesFrom, EdgeTrait<>::GetTo>;

    template <Vertex AdjacentNode>
    using GetPath = PathFinder<AdjacentNode, Target, typename Path::template append<CurrentNode>>;
    using AllPathFromCurNode = Map_t<NextNodes, GetPath>;

    template <TL AccMinPath, TL Path_>
    using GetMinPath = std::conditional_t<
        AccMinPath::size == 0 || (AccMinPath::size > Path_::size && Path_::size> 0),
        Path_, AccMinPath>;
  public:
    using type = Fold_t<AllPathFromCurNode, TypeList<>, GetMinPath>;
  };  // class PathFinder
  template <Vertex CurrentNode, Vertex Target, TL Path = TypeList<>>
  using PathFinder_t = PathFinder<CurrentNode, Target, Path>::type;

  template <TL A, TL B, template <typename, typename> class Pair>
  struct CrossProduct {
    template <TL ResultOuter, typename ElemA>
    struct OuterAppend {
      template <TL ResultInner, typename ElemB>
      using InnerAppend = typename ResultInner::template append<Pair<ElemA, ElemB>>;
      using type = Fold_t<B, ResultOuter, InnerAppend>;
    };  // struct OuterAppend
  public:
    using type = Fold_t<A, TypeList<>, OuterAppend>;
  };  // struct CrossProduct
  template <TL A, TL B, template <typename, typename> class Pair>
  using CrossProduct_t = CrossProduct<A, B, Pair>::type;


  using AllNodePairs = CrossProduct_t<
      Unique_t<Map_t<Edges, EdgeTrait<>::GetFrom>>,
      Unique_t<Map_t<Edges, EdgeTrait<>::GetTo>>,
      std::pair>;  // TypeList<std::pair<A, B>...>

  template <typename NodePair>
  using IsNonEmptyPath = std::bool_constant<(
      PathFinder_t<typename NodePair::first_type, typename NodePair::second_type>::size > 0
  )>;

  using ReachableNodePairs = Filter_t<AllNodePairs, IsNonEmptyPath>;


  // 动静结合: 路径信息都存放于类型中，即模板参数，这里需要将类型转换成实际数据
  template <typename NodeType>
  struct PathRef {
    const NodeType* path;  // 数组指针
    std::size_t sz;  // 数组长度
  };  // struct PathRef

  template <Vertex Node, Vertex... Nodes>
  class PathStorage {
    using NodeType = std::decay_t<decltype(Node::id)>;
    constexpr static NodeType path_storage[]{Node::id, Nodes::id...};
  public:
    constexpr static PathRef<NodeType> path {
      .path = path_storage,
      .sz = sizeof...(Nodes) + 1
    };
  };  // class PathStorage
  // 将数据存在 PathStorage::path 中，是一个数组

  // pair<<from, to>, path>
  template <typename NodePair>
  using SavePath = Return<typename std::pair<
      NodePair,
      typename PathFinder_t<
          typename NodePair::first_type, typename NodePair::second_type
      >::template to<PathStorage>
  >>;

  using SaveAllPath = Map_t<ReachableNodePairs, SavePath>;

  template <typename NodeType, typename... PathPairs>
  constexpr static void match_path(
      NodeType from, NodeType to, PathRef<NodeType>& result,
      TypeList<PathPairs...>) {
    (match_path(from, to, result, PathPairs{}) || ...);
  }

  template <typename NodeType, Vertex From, Vertex Target, typename PathStorage_>
  constexpr static bool match_path(
      NodeType from, NodeType to, PathRef<NodeType>& result,
      std::pair<std::pair<From, Target>, PathStorage_>) {
    if (From::id == from && Target::id == to) {
      result = PathStorage_::path;
      return true;
    }
    return false;
  }

public:
  template <typename NodeType>
  constexpr static PathRef<NodeType> get_path(NodeType from, NodeType to) {
    PathRef<NodeType> result{};
    match_path(from, to, result, SaveAllPath{});
    return result;
  }
};  // class Graph
// 编译时触发元函数调用，生成最短路径
// 运行时类型不复存在，只剩一堆路径数据

// Graph 是个空类，所以 size 是 1
static_assert(1 == sizeof(Graph<>));


#define node(ID) auto(*)(ID)
#define link(func) func ->void
