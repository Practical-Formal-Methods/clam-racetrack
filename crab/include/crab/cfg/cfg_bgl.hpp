#pragma once

/*
 * Convert a CFG into a BGL (Boost Graph Library) graph
 */
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <crab/cfg/cfg.hpp>

namespace crab {
namespace cfg {
namespace graph {
template <typename G>
struct mk_in_edge : public std::unary_function<
                        typename boost::graph_traits<G>::vertex_descriptor,
                        typename boost::graph_traits<G>::edge_descriptor> {
  typedef typename boost::graph_traits<G>::vertex_descriptor Node;
  typedef typename boost::graph_traits<G>::edge_descriptor Edge;

  Node _dst;
  mk_in_edge() {}
  mk_in_edge(const Node &dst) : _dst(dst) {}
  Edge operator()(const Node &src) const { return Edge(src, _dst); }
};

template <typename G>
struct mk_out_edge : public std::unary_function<
                         typename boost::graph_traits<G>::vertex_descriptor,
                         typename boost::graph_traits<G>::edge_descriptor> {
  typedef typename boost::graph_traits<G>::vertex_descriptor Node;
  typedef typename boost::graph_traits<G>::edge_descriptor Edge;

  Node _src;
  mk_out_edge() {}
  mk_out_edge(const Node &src) : _src(src) {}
  Edge operator()(const Node &dst) const { return Edge(_src, dst); }
};
} // end namespace graph
} // end namespace cfg
} // end namespace crab

namespace boost {

// cfg
template <typename BasicBlockLabel, typename VariableName, typename Number>
struct graph_traits<crab::cfg::cfg<BasicBlockLabel, VariableName, Number>> {
  typedef crab::cfg::cfg<BasicBlockLabel, VariableName, Number> graph_t;
  typedef BasicBlockLabel vertex_descriptor;
  typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
  typedef std::pair<const vertex_descriptor, const vertex_descriptor>
      const_edge_descriptor;

  typedef disallow_parallel_edge_tag edge_parallel_category;
  typedef bidirectional_tag directed_category;
  struct this_graph_tag : virtual bidirectional_graph_tag,
                          virtual vertex_list_graph_tag {};
  typedef this_graph_tag traversal_category;

  typedef size_t vertices_size_type;
  typedef size_t edges_size_type;
  typedef size_t degree_size_type;

  static vertex_descriptor null_vertex() {
    if (std::is_pointer<vertex_descriptor>::value)
      return nullptr;
    else {
      // XXX: if vertex_descriptor is a basic type then
      // null_vertex will return an undefined value, otherwise it
      // will return the result of calling the default
      // constructor.
      vertex_descriptor n;
      return n;
    }
  }

  // iterator of basic_block_label_t's
  typedef typename graph_t::label_iterator vertex_iterator;
  // iterator of pairs of basic_block_label_t's
  typedef transform_iterator<crab::cfg::graph::mk_in_edge<graph_t>,
                             typename graph_t::pred_iterator>
      in_edge_iterator;
  // iterator of pairs of basic_block_label_t's
  typedef transform_iterator<crab::cfg::graph::mk_out_edge<graph_t>,
                             typename graph_t::succ_iterator>
      out_edge_iterator;
}; // end class graph_traits

// cfg_ref
template <class CFG> struct graph_traits<crab::cfg::cfg_ref<CFG>> {
  typedef crab::cfg::cfg_ref<CFG> graph_t;
  typedef typename graph_t::basic_block_label_t vertex_descriptor;
  typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
  typedef std::pair<const vertex_descriptor, const vertex_descriptor>
      const_edge_descriptor;

  typedef disallow_parallel_edge_tag edge_parallel_category;
  typedef bidirectional_tag directed_category;
  struct this_graph_tag : virtual bidirectional_graph_tag,
                          virtual vertex_list_graph_tag {};
  typedef this_graph_tag traversal_category;

  typedef size_t vertices_size_type;
  typedef size_t edges_size_type;
  typedef size_t degree_size_type;

  static vertex_descriptor null_vertex() {
    if (std::is_pointer<vertex_descriptor>::value)
      return nullptr;
    else {
      // XXX: if vertex_descriptor is a basic type then
      // null_vertex will return an undefined value, otherwise it
      // will return the result of calling the default
      // constructor.
      vertex_descriptor n;
      return n;
    }
  }

  typedef typename graph_t::label_iterator vertex_iterator;
  typedef boost::transform_iterator<crab::cfg::graph::mk_in_edge<graph_t>,
                                    typename graph_t::pred_iterator>
      in_edge_iterator;
  typedef boost::transform_iterator<crab::cfg::graph::mk_out_edge<graph_t>,
                                    typename graph_t::succ_iterator>
      out_edge_iterator;
}; // end class graph_traits

// cfg_rev
template <class CFG> struct graph_traits<crab::cfg::cfg_rev<CFG>> {
  typedef crab::cfg::cfg_rev<CFG> graph_t;
  typedef typename graph_t::basic_block_label_t vertex_descriptor;
  typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
  typedef std::pair<const vertex_descriptor, const vertex_descriptor>
      const_edge_descriptor;

  typedef disallow_parallel_edge_tag edge_parallel_category;
  typedef bidirectional_tag directed_category;
  struct this_graph_tag : virtual bidirectional_graph_tag,
                          virtual vertex_list_graph_tag {};
  typedef this_graph_tag traversal_category;

  typedef size_t vertices_size_type;
  typedef size_t edges_size_type;
  typedef size_t degree_size_type;

  static vertex_descriptor null_vertex() {
    if (std::is_pointer<vertex_descriptor>::value)
      return nullptr;
    else {
      // XXX: if vertex_descriptor is a basic type then
      // null_vertex will return an undefined value, otherwise it
      // will return the result of calling the default
      // constructor.
      vertex_descriptor n;
      return n;
    }
  }

  typedef typename graph_t::label_iterator vertex_iterator;
  typedef boost::transform_iterator<crab::cfg::graph::mk_in_edge<graph_t>,
                                    typename graph_t::pred_iterator>
      in_edge_iterator;
  typedef boost::transform_iterator<crab::cfg::graph::mk_out_edge<graph_t>,
                                    typename graph_t::succ_iterator>
      out_edge_iterator;
}; // end class graph_traits
} // namespace boost

// XXX: do not put it in the boost namespace because it won't compile
namespace crab {
namespace cfg {

// cfg

//// boost/graph/graph_traits.hpp has already default source/target
//// functions in case the edge_descriptor is std::pair.

// this is not part of BGL but needed by Crab's algorithms
template <typename BasicBlockLabel, typename VariableName, typename Number>
typename boost::graph_traits<
    cfg<BasicBlockLabel, VariableName, Number>>::vertex_descriptor
entry(const cfg<BasicBlockLabel, VariableName, Number> &g) {
  return g.entry();
}

template <typename BasicBlockLabel, typename VariableName, typename Number>
inline std::pair<typename boost::graph_traits<cfg<BasicBlockLabel, VariableName,
                                                  Number>>::vertex_iterator,
                 typename boost::graph_traits<cfg<BasicBlockLabel, VariableName,
                                                  Number>>::vertex_iterator>
vertices(cfg<BasicBlockLabel, VariableName, Number> g) {
  return std::make_pair(g.label_begin(), g.label_end());
}

template <typename BasicBlockLabel, typename VariableName, typename Number>
inline std::pair<typename boost::graph_traits<cfg<BasicBlockLabel, VariableName,
                                                  Number>>::out_edge_iterator,
                 typename boost::graph_traits<cfg<BasicBlockLabel, VariableName,
                                                  Number>>::out_edge_iterator>
out_edges(typename boost::graph_traits<
              cfg<BasicBlockLabel, VariableName, Number>>::vertex_descriptor v,
          cfg<BasicBlockLabel, VariableName, Number> g) {
  typedef cfg<BasicBlockLabel, VariableName, Number> G;

  auto &node = g.get_node(v);
  auto p = node.next_blocks();
  return std::make_pair(
      boost::make_transform_iterator(p.first, graph::mk_out_edge<G>(v)),
      boost::make_transform_iterator(p.second, graph::mk_out_edge<G>(v)));
}

template <typename BasicBlockLabel, typename VariableName, typename Number>
inline std::pair<typename boost::graph_traits<cfg<BasicBlockLabel, VariableName,
                                                  Number>>::in_edge_iterator,
                 typename boost::graph_traits<cfg<BasicBlockLabel, VariableName,
                                                  Number>>::in_edge_iterator>
in_edges(typename boost::graph_traits<
             cfg<BasicBlockLabel, VariableName, Number>>::vertex_descriptor v,
         cfg<BasicBlockLabel, VariableName, Number> g) {
  typedef cfg<BasicBlockLabel, VariableName, Number> G;

  auto &node = g.get_node(v);
  auto p = node.prev_blocks();
  return std::make_pair(
      boost::make_transform_iterator(p.first, graph::mk_in_edge<G>(v)),
      boost::make_transform_iterator(p.second, graph::mk_in_edge<G>(v)));
}

template <typename BasicBlockLabel, typename VariableName, typename Number>
typename boost::graph_traits<
    cfg<BasicBlockLabel, VariableName, Number>>::vertices_size_type
num_vertices(cfg<BasicBlockLabel, VariableName, Number> g) {
  return std::distance(g.label_begin(), g.label_end());
}

template <typename BasicBlockLabel, typename VariableName, typename Number>
typename boost::graph_traits<
    cfg<BasicBlockLabel, VariableName, Number>>::degree_size_type
in_degree(typename boost::graph_traits<
              cfg<BasicBlockLabel, VariableName, Number>>::vertex_descriptor v,
          cfg<BasicBlockLabel, VariableName, Number> g) {
  auto preds = g.prev_nodes(v);
  return std::distance(preds.begin(), preds.end());
}

template <typename BasicBlockLabel, typename VariableName, typename Number>
typename boost::graph_traits<
    cfg<BasicBlockLabel, VariableName, Number>>::degree_size_type
out_degree(typename boost::graph_traits<
               cfg<BasicBlockLabel, VariableName, Number>>::vertex_descriptor v,
           cfg<BasicBlockLabel, VariableName, Number> g) {
  auto succs = g.next_nodes(v);
  return std::distance(succs.begin(), succs.end());
}

template <typename BasicBlockLabel, typename VariableName, typename Number>
typename boost::graph_traits<
    cfg<BasicBlockLabel, VariableName, Number>>::degree_size_type
degree(typename boost::graph_traits<
           cfg<BasicBlockLabel, VariableName, Number>>::vertex_descriptor v,
       cfg<BasicBlockLabel, VariableName, Number> g) {
  return out_degree(v, g) + in_degree(v, g);
}

// cfg_ref

//// boost/graph/graph_traits.hpp has already default source/target
//// functions in case the edge_descriptor is std::pair.

// this is not part of BGL but needed by Crab's algorithms
template <class CFG>
typename boost::graph_traits<cfg_ref<CFG>>::vertex_descriptor
entry(const cfg_ref<CFG> &g) {
  return g.entry();
}

template <class CFG>
inline std::pair<typename boost::graph_traits<cfg_ref<CFG>>::vertex_iterator,
                 typename boost::graph_traits<cfg_ref<CFG>>::vertex_iterator>
vertices(cfg_ref<CFG> g) {
  return std::make_pair(g.label_begin(), g.label_end());
}

template <class CFG>
inline std::pair<typename boost::graph_traits<cfg_ref<CFG>>::out_edge_iterator,
                 typename boost::graph_traits<cfg_ref<CFG>>::out_edge_iterator>
out_edges(typename boost::graph_traits<cfg_ref<CFG>>::vertex_descriptor v,
          cfg_ref<CFG> g) {
  typedef cfg_ref<CFG> G;
  auto &node = g.get_node(v);
  auto p = node.next_blocks();
  return std::make_pair(
      boost::make_transform_iterator(p.first, graph::mk_out_edge<G>(v)),
      boost::make_transform_iterator(p.second, graph::mk_out_edge<G>(v)));
}

template <class CFG>
inline std::pair<typename boost::graph_traits<cfg_ref<CFG>>::in_edge_iterator,
                 typename boost::graph_traits<cfg_ref<CFG>>::in_edge_iterator>
in_edges(typename boost::graph_traits<cfg_ref<CFG>>::vertex_descriptor v,
         cfg_ref<CFG> g) {
  typedef cfg_ref<CFG> G;
  auto &node = g.get_node(v);
  auto p = node.prev_blocks();
  return std::make_pair(
      boost::make_transform_iterator(p.first, graph::mk_in_edge<G>(v)),
      boost::make_transform_iterator(p.second, graph::mk_in_edge<G>(v)));
}

template <class CFG>
typename boost::graph_traits<cfg_ref<CFG>>::vertices_size_type
num_vertices(cfg_ref<CFG> g) {
  return std::distance(g.label_begin(), g.label_end());
}

template <class CFG>
typename boost::graph_traits<cfg_ref<CFG>>::degree_size_type
in_degree(typename boost::graph_traits<cfg_ref<CFG>>::vertex_descriptor v,
          cfg_ref<CFG> g) {
  auto preds = g.prev_nodes(v);
  return std::distance(preds.begin(), preds.end());
}

template <class CFG>
typename boost::graph_traits<cfg_ref<CFG>>::degree_size_type
out_degree(typename boost::graph_traits<cfg_ref<CFG>>::vertex_descriptor v,
           cfg_ref<CFG> g) {
  auto succs = g.next_nodes(v);
  return std::distance(succs.begin(), succs.end());
}

template <class CFG>
typename boost::graph_traits<cfg_ref<CFG>>::degree_size_type
degree(typename boost::graph_traits<cfg_ref<CFG>>::vertex_descriptor v,
       cfg_ref<CFG> g) {
  return out_degree(v, g) + in_degree(v, g);
}

// cfg_rev

//// boost/graph/graph_traits.hpp has already default source/target
//// functions in case the edge_descriptor is std::pair.

// this is not part of BGL but needed by Crab's algorithms
template <class CFG>
typename boost::graph_traits<cfg_rev<CFG>>::vertex_descriptor
entry(const cfg_rev<CFG> &g) {
  return g.entry();
}

template <class CFG>
inline std::pair<typename boost::graph_traits<cfg_rev<CFG>>::vertex_iterator,
                 typename boost::graph_traits<cfg_rev<CFG>>::vertex_iterator>
vertices(cfg_rev<CFG> g) {
  return std::make_pair(g.label_begin(), g.label_end());
}

template <class CFG>
inline std::pair<typename boost::graph_traits<cfg_rev<CFG>>::out_edge_iterator,
                 typename boost::graph_traits<cfg_rev<CFG>>::out_edge_iterator>
out_edges(typename boost::graph_traits<cfg_rev<CFG>>::vertex_descriptor v,
          cfg_rev<CFG> g) {
  typedef cfg_rev<CFG> G;
  auto &node = g.get_node(v);
  auto p = node.next_blocks();
  return std::make_pair(
      boost::make_transform_iterator(p.first, graph::mk_out_edge<G>(v)),
      boost::make_transform_iterator(p.second, graph::mk_out_edge<G>(v)));
}

template <class CFG>
inline std::pair<typename boost::graph_traits<cfg_rev<CFG>>::in_edge_iterator,
                 typename boost::graph_traits<cfg_rev<CFG>>::in_edge_iterator>
in_edges(typename boost::graph_traits<cfg_rev<CFG>>::vertex_descriptor v,
         cfg_rev<CFG> g) {
  typedef cfg_rev<CFG> G;
  auto &node = g.get_node(v);
  auto p = node.prev_blocks();
  return std::make_pair(
      boost::make_transform_iterator(p.first, graph::mk_in_edge<G>(v)),
      boost::make_transform_iterator(p.second, graph::mk_in_edge<G>(v)));
}

template <class CFG>
typename boost::graph_traits<cfg_rev<CFG>>::vertices_size_type
num_vertices(cfg_rev<CFG> g) {
  return std::distance(g.label_begin(), g.label_end());
}

template <class CFG>
typename boost::graph_traits<cfg_rev<CFG>>::degree_size_type
in_degree(typename boost::graph_traits<cfg_rev<CFG>>::vertex_descriptor v,
          cfg_rev<CFG> g) {
  auto preds = g.prev_nodes(v);
  return std::distance(preds.begin(), preds.end());
}

template <class CFG>
typename boost::graph_traits<cfg_rev<CFG>>::degree_size_type
out_degree(typename boost::graph_traits<cfg_rev<CFG>>::vertex_descriptor v,
           cfg_rev<CFG> g) {
  auto succs = g.next_nodes(v);
  return std::distance(succs.begin(), succs.end());
}

template <class CFG>
typename boost::graph_traits<cfg_rev<CFG>>::degree_size_type
degree(typename boost::graph_traits<cfg_rev<CFG>>::vertex_descriptor v,
       cfg_rev<CFG> g) {
  return out_degree(v, g) + in_degree(v, g);
}

} // namespace cfg
} // namespace crab
