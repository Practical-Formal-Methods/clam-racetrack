#pragma once

#include <boost/optional.hpp>
#include <crab/domains/graphs/util/Heap.h>
//============================
// A set of utility algorithms for manipulating graphs.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

namespace crab {
// Graph views - for when we want to traverse some mutation
// of the graph without actually constructing it.
// ============
template <class V> class num_range {
public:
  class value_ref {
  public:
    value_ref(const V &_v) : v(_v) {}
    const V &operator*(void)const { return v; }
    value_ref &operator++(void) {
      v++;
      return *this;
    }
    value_ref &operator--(void) {
      v--;
      return *this;
    }
    bool operator!=(const value_ref &o) const { return v < o.v; }

  protected:
    V v;
  };
  num_range(const V &_after) : after(_after) {}

  value_ref begin(void) const { return value_ref((V)0); }
  value_ref end(void) const { return value_ref(after); }

protected:
  V after;
};

// Processing a graph under a (possibly incomplete)
// permutation of vertices.
// We assume perm[x] is unique; otherwise, we'd have
// to introduce a edges for induced equivalence classes.
template <class G> class GraphPerm {
public:
  typedef typename G::vert_id vert_id;
  typedef typename G::Wt Wt;
  typedef typename G::pred_range g_pred_range;
  typedef typename G::succ_range g_succ_range;
  typedef typename G::mut_val_ref_t mut_val_ref_t;

  GraphPerm(std::vector<vert_id> &_perm, G &_g)
      : g(_g), perm(_perm), inv(_g.size(), -1) {
    for (unsigned int vi = 0; vi < perm.size(); vi++) {
      if (perm[vi] == -1)
        continue;
      assert(inv[perm[vi]] == -1);
      inv[perm[vi]] = vi;
    }
  }

  // Check whether an edge is live
  bool elem(vert_id x, vert_id y) const {
    if (perm[x] > g.size() || perm[y] > g.size())
      return false;
    return g.elem(perm[x], perm[y]);
  }

  bool lookup(vert_id x, vert_id y, mut_val_ref_t *w) const {
    if (perm[x] > g.size() || perm[y] > g.size())
      return false;
    return g.lookup(perm[x], perm[y], w);
  }

  // Precondition: elem(x, y) is true.
  Wt edge_val(vert_id x, vert_id y) const {
    //      assert(perm[x] < g.size() && perm[y] < g.size());
    return g.edge_val(perm[x], perm[y]);
  }

  // Precondition: elem(x, y) is true.
  Wt operator()(vert_id x, vert_id y) const {
    //      assert(perm[x] < g.size() && perm[y] < g.size());
    return g(perm[x], perm[y]);
  }

  // Number of allocated vertices
  int size(void) const { return perm.size(); }

  typedef num_range<vert_id> vert_range;
  typedef typename num_range<vert_id>::value_ref vert_iterator;
  vert_range verts(void) const { return vert_range(perm.size()); }

  // GKG: Should probably modify this to handle cases where
  // the vertex iterator isn't just a vert_id*.
  template <class ItG> class adj_iterator {
  public:
    adj_iterator(std::vector<vert_id> &_inv, const ItG &_v)
        : inv(_inv), v(_v) {}

    vert_id operator*(void)const { return inv[*v]; }

    adj_iterator &operator++(void) {
      ++v;
      return *this;
    }

    bool operator!=(const adj_iterator &other) {
      while (v != other.v && inv[*v] == (-1))
        ++v;
      return v != other.v;
    }

  protected:
    std::vector<vert_id> &inv;
    ItG v;
  };

  template <class ItG> class e_adj_iterator {
  public:
    typedef typename ItG::edge_ref edge_ref;

    e_adj_iterator(std::vector<vert_id> &_inv, const ItG &_v)
        : inv(_inv), v(_v) {}

    edge_ref operator*(void)const { return edge_ref(inv[(*v).vert], (*v).val); }

    e_adj_iterator &operator++(void) {
      ++v;
      return *this;
    }

    bool operator!=(const e_adj_iterator &other) {
      while (v != other.v && inv[(*v).vert] == (-1))
        ++v;
      return v != other.v;
    }

  protected:
    std::vector<vert_id> &inv;
    ItG v;
  };

  template <class RG, class It> class adj_list {
  public:
    typedef typename RG::iterator ItG;
    typedef adj_list<RG, It> adj_list_t;
    //      typedef adj_iterator<ItG> adj_iter_t;

    typedef It iterator;

    adj_list(std::vector<vert_id> &_perm, std::vector<vert_id> &_inv,
             const RG &_adj)
        : perm(_perm), inv(_inv), adj(_adj) {}

    adj_list(std::vector<vert_id> &_perm, std::vector<vert_id> &_inv)
        : perm(_perm), inv(_inv), adj() {}

    iterator begin(void) const {
      if (adj)
        return iterator(inv, (*adj).begin());
      else
        return iterator(inv, ItG::empty_iterator());
    }
    iterator end(void) const {
      if (adj)
        return iterator(inv, (*adj).end());
      else
        return iterator(inv, ItG::empty_iterator());
    }

    bool mem(unsigned int v) const {
      if (!adj || perm[v] == (-1))
        return false;
      return (*adj).mem(perm[v]);
    }

  protected:
    std::vector<vert_id> &perm;
    std::vector<vert_id> &inv;
    boost::optional<RG> adj;
  };

  typedef adj_list<typename G::pred_range,
                   adj_iterator<typename G::pred_range::iterator>>
      pred_range;
  typedef adj_list<typename G::succ_range,
                   adj_iterator<typename G::succ_range::iterator>>
      succ_range;

  typedef adj_list<typename G::e_pred_range,
                   e_adj_iterator<typename G::e_pred_range::iterator>>
      e_pred_range;
  typedef adj_list<typename G::e_succ_range,
                   e_adj_iterator<typename G::e_succ_range::iterator>>
      e_succ_range;

  succ_range succs(vert_id v) {
    if (perm[v] == (-1))
      return succ_range(perm, inv);
    else
      return succ_range(perm, inv, g.succs(perm[v]));
  }
  pred_range preds(vert_id v) {
    if (perm[v] == (-1))
      return pred_range(perm, inv);
    else
      return pred_range(perm, inv, g.preds(perm[v]));
  }

  e_succ_range e_succs(vert_id v) {
    if (perm[v] == (-1))
      return e_succ_range(perm, inv);
    else
      return e_succ_range(perm, inv, g.e_succs(perm[v]));
  }
  e_pred_range e_preds(vert_id v) {
    if (perm[v] == (-1))
      return e_pred_range(perm, inv);
    else
      return e_pred_range(perm, inv, g.e_preds(perm[v]));
  }

  void write(crab_os &o) {
    o << "[|";
    bool first = true;
    for (vert_id v : verts()) {
      auto it = e_succs(v).begin();
      auto end = e_succs(v).end();
      if (it != end) {
        if (first) {
          first = false;
        } else {
          o << ", ";
        }
        o << "[v" << v << " -> ";
        o << "(" << (*it).val << ":" << (*it).vert << ")";
        for (++it; it != end; ++it) {
          o << ", (" << (*it).val << ":" << (*it).vert << ")";
        }
        o << "]";
      }
    }
    o << "|]";
  }

  G &g;
  std::vector<vert_id> perm;
  std::vector<vert_id> inv;
};

// View of a graph, omitting a given vertex
template <class G> class SubGraph {
public:
  typedef typename G::vert_id vert_id;
  typedef typename G::Wt Wt;

  typedef typename G::pred_range g_pred_range;
  typedef typename G::succ_range g_succ_range;

  typedef typename G::e_pred_range g_e_pred_range;
  typedef typename G::e_succ_range g_e_succ_range;

  typedef typename G::mut_val_ref_t mut_val_ref_t;

  SubGraph(G &_g, vert_id _v_ex) : g(_g), v_ex(_v_ex) {}

  bool elem(vert_id x, vert_id y) const {
    return (x != v_ex && y != v_ex && g.elem(x, y));
  }

  bool lookup(vert_id x, vert_id y, mut_val_ref_t *w) const {
    return (x != v_ex && y != v_ex && g.lookup(x, y, w));
  }

  Wt edge_val(vert_id x, vert_id y) const { return g.edge_val(x, y); }

  // Precondition: elem(x, y) is true.
  Wt operator()(vert_id x, vert_id y) const { return g(x, y); }

  void clear_edges(void) { g.clear_edges(); }

  void clear(void) { assert(0 && "SubGraph::clear not implemented."); }

  // Number of allocated vertices
  int size(void) const { return g.size(); }

  // Assumption: (x, y) not in mtx
  void add_edge(vert_id x, Wt wt, vert_id y) {
    // assert(x != v_ex && y != v_ex);
    assert(!elem(x, y));
    g.add_edge(x, wt, y);
  }

  void set_edge(vert_id s, Wt w, vert_id d) {
    // assert(s != v_ex && d != v_ex);
    g.set_edge(s, w, d);
  }

  template <class Op> void update_edge(vert_id s, Wt w, vert_id d, Op &op) {
    // assert(s != v_ex && d != v_ex);
    g.update_edge(s, w, d, op);
  }

  class vert_iterator {
  public:
    vert_iterator(const typename G::vert_iterator &_iG, vert_id _v_ex)
        : v_ex(_v_ex), iG(_iG) {}

    // Skipping of v_ex is done entirely by !=.
    // So we _MUST_ test it != verts.end() before dereferencing.
    vert_id operator*(void) { return *iG; }
    vert_iterator operator++(void) {
      ++iG;
      return *this;
    }
    bool operator!=(const vert_iterator &o) {
      if (iG != o.iG && (*iG) == v_ex)
        ++iG;
      return iG != o.iG;
    }

    vert_id v_ex;
    typename G::vert_iterator iG;
  };
  class vert_range {
  public:
    vert_range(const typename G::vert_range &_rG, vert_id _v_ex)
        : rG(_rG), v_ex(_v_ex) {}

    vert_iterator begin(void) const { return vert_iterator(rG.begin(), v_ex); }
    vert_iterator end(void) const { return vert_iterator(rG.end(), v_ex); }

    typename G::vert_range rG;
    vert_id v_ex;
  };
  vert_range verts(void) const { return vert_range(g.verts(), v_ex); }

  template <class It> class adj_iterator {
  public:
    adj_iterator(const It &_iG, vert_id _v_ex) : iG(_iG), v_ex(_v_ex) {}
    vert_id operator*(void)const { return *iG; }
    adj_iterator &operator++(void) {
      ++iG;
      return *this;
    }
    bool operator!=(const adj_iterator &o) {
      if (iG != o.iG && (*iG) == v_ex)
        ++iG;
      return iG != o.iG;
    }

    It iG;
    vert_id v_ex;
  };

  template <class It> class e_adj_iterator {
  public:
    typedef typename It::edge_ref edge_ref;

    e_adj_iterator(const It &_iG, vert_id _v_ex) : iG(_iG), v_ex(_v_ex) {}
    edge_ref operator*(void)const { return *iG; }
    e_adj_iterator &operator++(void) {
      ++iG;
      return *this;
    }
    bool operator!=(const e_adj_iterator &o) {
      if (iG != o.iG && (*iG).vert == v_ex)
        ++iG;
      return iG != o.iG;
    }

    It iG;
    vert_id v_ex;
  };

  template <class R, class It> class adj_list {
  public:
    typedef typename R::iterator g_iter;
    typedef It iterator;

    adj_list(const R &_rG, vert_id _v_ex) : rG(_rG), v_ex(_v_ex) {}
    iterator begin() const { return iterator(rG.begin(), v_ex); }
    iterator end() const { return iterator(rG.end(), v_ex); }

  protected:
    R rG;
    vert_id v_ex;
  };
  typedef adj_list<g_pred_range, adj_iterator<typename g_pred_range::iterator>>
      pred_range;
  typedef adj_list<g_succ_range, adj_iterator<typename g_succ_range::iterator>>
      succ_range;

  typedef adj_list<g_e_pred_range,
                   e_adj_iterator<typename g_e_pred_range::iterator>>
      e_pred_range;
  typedef adj_list<g_e_succ_range,
                   e_adj_iterator<typename g_e_succ_range::iterator>>
      e_succ_range;

  succ_range succs(vert_id v) {
    //      assert(v != v_ex);
    return succ_range(g.succs(v), v_ex);
  }
  pred_range preds(vert_id v) {
    //      assert(v != v_ex);
    return pred_range(g.preds(v), v_ex);
  }
  e_succ_range e_succs(vert_id v) { return e_succ_range(g.e_succs(v), v_ex); }
  e_pred_range e_preds(vert_id v) { return e_pred_range(g.e_preds(v), v_ex); }

  G &g;
  vert_id v_ex;
};

// Viewing a graph with all edges reversed.
// Useful if we want to run single-dest shortest paths,
// for updating bounds and incremental closure.
template <class G> class GraphRev {
public:
  typedef typename G::vert_id vert_id;
  typedef typename G::Wt Wt;
  // typedef typename G::adj_list g_adj_list;
  typedef typename G::mut_val_ref_t mut_val_ref_t;

  GraphRev(G &_g) : g(_g) {}

  // Check whether an edge is live
  bool elem(vert_id x, vert_id y) const { return g.elem(y, x); }

  bool lookup(vert_id x, vert_id y, mut_val_ref_t *w) const {
    return g.lookup(y, x, w);
  }

  // Precondition: elem(x, y) is true.
  Wt edge_val(vert_id x, vert_id y) const { return g.edge_val(y, x); }

  // Precondition: elem(x, y) is true.
  Wt operator()(vert_id x, vert_id y) const { return g(y, x); }

  // Number of allocated vertices
  int size(void) const { return g.size(); }

  //    typedef typename G::adj_list adj_list;
  typedef typename G::succ_range pred_range;
  typedef typename G::pred_range succ_range;

  typedef typename G::e_succ_range e_pred_range;
  typedef typename G::e_pred_range e_succ_range;

  typename G::vert_range verts(void) const { return g.verts(); }

  succ_range succs(vert_id v) { return g.preds(v); }
  succ_range preds(vert_id v) { return g.succs(v); }

  e_succ_range e_succs(vert_id v) { return g.e_preds(v); }
  e_pred_range e_preds(vert_id v) { return g.e_succs(v); }
  G &g;
};

// Comparator for use with min-heaps.
template <class V> class DistComp {
public:
  DistComp(V &_A) : A(_A) {}
  bool operator()(int x, int y) const { return A[x] < A[y]; }
  V &A;
};

// GKG - What's the best way to split this out?
template <class Gr> class GraphOps {
public:
  typedef typename Gr::Wt Wt;
  // The following code assumes vert_id is an integer.
  //    typedef SparseWtGraph<Wt> graph_t;
  typedef Gr graph_t;
  typedef typename graph_t::vert_id vert_id;
  typedef typename graph_t::mut_val_ref_t mut_val_ref_t;

  typedef std::vector<std::pair<std::pair<vert_id, vert_id>, Wt>> edge_vector;

  typedef DistComp<std::vector<Wt>> WtComp;
  typedef Heap<WtComp> WtHeap;

  typedef std::pair<std::pair<vert_id, vert_id>, Wt> edge_ref;

  //===========================================
  // Enums used to mark vertices/edges during algorithms
  //===========================================
  // Edge colour during chromatic Dijkstra
  enum CMarkT { E_NONE = 0, E_LEFT = 1, E_RIGHT = 2, E_BOTH = 3 };
  // Whether a vertex is 'stable' during widening
  enum SMarkT { V_UNSTABLE = 0, V_STABLE = 1 };
  // Whether a vertex is in the current SCC/queue for Bellman-Ford.
  enum QMarkT { BF_NONE = 0, BF_SCC = 1, BF_QUEUED = 2 };
  // ===========================================
  // Scratch space needed by the graph algorithms.
  // Should really switch to some kind of arena allocator, rather
  // than having all these static structures.
  // ===========================================
  static char *edge_marks;

  // Used for Bellman-Ford queueing
  static vert_id *dual_queue;
  static int *vert_marks;
  static unsigned int scratch_sz;

  // For locality, should combine dists & dist_ts.
  // Wt must have an empty constructor, but does _not_
  // need a top or infty element.
  // dist_ts tells us which distances are current,
  // and ts_idx prevents wraparound problems, in the unlikely
  // circumstance that we have more than 2^sizeof(uint) iterations.
  static std::vector<Wt> dists;
  static std::vector<Wt> dists_alt;
  static std::vector<unsigned int> dist_ts;
  static unsigned int ts;
  static unsigned int ts_idx;

  static void grow_scratch(unsigned int sz) {
    if (sz <= scratch_sz)
      return;

    if (scratch_sz == 0)
      scratch_sz = 10; // Introduce enums for init_sz and growth_factor
    while (scratch_sz < sz)
      scratch_sz *= 1.5;

    edge_marks =
        (char *)realloc(edge_marks, sizeof(char) * scratch_sz * scratch_sz);
    dual_queue =
        (vert_id *)realloc(dual_queue, sizeof(vert_id) * 2 * scratch_sz);
    vert_marks = (int *)realloc(vert_marks, sizeof(int) * scratch_sz);

    // Initialize new elements as necessary.
    while (dists.size() < scratch_sz) {
      dists.push_back(Wt());
      dists_alt.push_back(Wt());
      dist_ts.push_back(ts - 1);
    }
  }

  // Syntactic join.
  template <class G1, class G2> static graph_t join(G1 &l, G2 &r) {
    // For the join, potentials are preserved
    assert(l.size() == r.size());
    int sz = l.size();

    graph_t g;
    g.growTo(sz);

    mut_val_ref_t wr;
    for (vert_id s : l.verts()) {
      for (auto e : l.e_succs(s)) {
        vert_id d = e.vert;
        if (r.lookup(s, d, &wr))
          g.add_edge(s, std::max(e.val, (Wt)wr), d);
      }
    }
    return g;
  }

  // Syntactic meet
  template <class G1, class G2>
  static graph_t meet(G1 &l, G2 &r, bool &is_closed) {
    assert(l.size() == r.size());

    /*
          for(vert_id s : l.verts())
            for(vert_id d : l.succs(s))
              if(!r.elem(s, d) || l.edge_val(s, d) < r.edge_val(s, d))
                goto r_not_dom;
          // r dominates
          is_closed = true;
          return graph_t::copy(r);

    r_not_dom:
    */
    graph_t g(graph_t::copy(l));
    //      bool l_dom = true;

    mut_val_ref_t wg;
    for (vert_id s : r.verts()) {
      for (auto e : r.e_succs(s)) {
        if (!g.lookup(s, e.vert, &wg)) {
          g.add_edge(s, e.val, e.vert);
        } else {
          if (e.val < wg)
            wg = e.val;
        }
      }
    }
    is_closed = false;
    return g;
  }

  template <class G1, class G2>
  static graph_t widen(G1 &l, G2 &r, std::vector<vert_id> &unstable) {
    assert(l.size() == r.size());
    size_t sz = l.size();
    graph_t g;
    g.growTo(sz);
    mut_val_ref_t wl;
    for (vert_id s : r.verts()) {
      for (auto e : r.e_succs(s)) {
        vert_id d = e.vert;
        if (l.lookup(s, d, &wl) && e.val <= wl)
          g.add_edge(s, wl, d);
      }

      // Check if this vertex is stable
      for (vert_id d : l.succs(s)) {
        if (!g.elem(s, d)) {
          unstable.push_back(s);
          break;
        }
      }
    }

    return g;
  }

  // Compute the strongly connected components
  // Duped pretty much verbatim from Wikipedia
  // Abuses 'dual_queue' to store indices.
  template <class G>
  static void strong_connect(G &x, std::vector<vert_id> &stack, int &index,
                             vert_id v,
                             std::vector<std::vector<vert_id>> &sccs) {
    vert_marks[v] = (index << 1) | 1;
    // assert(vert_marks[v]&1);
    dual_queue[v] = index;
    index++;

    stack.push_back(v);

    // Consider successors of v
    for (vert_id w : x.succs(v)) {
      if (!vert_marks[w]) {
        strong_connect(x, stack, index, w, sccs);
        dual_queue[v] = std::min(dual_queue[v], dual_queue[w]);
      } else if (vert_marks[w] & 1) {
        // W is on the stack
        dual_queue[v] = std::min(dual_queue[v], (vert_id)(vert_marks[w] >> 1));
      }
    }

    // If v is a root node, pop the stack and generate an SCC
    if (dual_queue[v] == (vert_marks[v] >> 1)) {
      sccs.push_back(std::vector<vert_id>());
      std::vector<vert_id> &scc(sccs.back());
      int w;
      do {
        w = stack.back();
        stack.pop_back();
        vert_marks[w] &= (~1);
        scc.push_back(w);
      } while (v != w);
    }
  }

  template <class G>
  static void compute_sccs(G &x, std::vector<std::vector<vert_id>> &out_scc) {
    int sz = x.size();
    grow_scratch(sz);

    for (vert_id v : x.verts())
      vert_marks[v] = 0;
    int index = 1;
    std::vector<vert_id> stack;
    for (vert_id v : x.verts()) {
      if (!vert_marks[v])
        strong_connect(x, stack, index, v, out_scc);
    }
    /*
    printf("[");
    for(int ii = 0; ii < out_scc.size(); ii++)
    {
      printf("[");
      for(int jj = 0; jj < out_scc[ii].size(); jj++)
        printf(" %d", out_scc[ii][jj]);
      printf("]");
    }
    printf("]\n");
    */

    for (vert_id v : x.verts())
      vert_marks[v] = 0;
  }

  // Run Bellman-Ford to compute a valid model of a set of difference
  // constraints. Returns false if there is some negative cycle.
  template <class G, class P>
  static bool select_potentials(G &g, P &potentials) {
    int sz = g.size();
    assert(potentials.size() >= sz);
    grow_scratch(sz);

    std::vector<std::vector<vert_id>> sccs;
    compute_sccs(g, sccs);

    // Currently trusting the call-site to select reasonable
    // initial values.
#if 0
      // Zero existing potentials.
      // Not strictly necessary, but means we're less
      // likely to run into over/underflow.
      // (gmp_z won't overflow, but there's a performance penalty
      // if magnitudes become large)
      //
      // Though this hurts our chances of early cutoff.
      for(vert_id v : g.verts())
        potentials[v] = 0;
#endif

    // Run Bellman-ford on each SCC.
    // for(std::vector<vert_id>& scc : sccs)
    // Current implementation returns sccs in reverse topological order.
    for (auto it = sccs.rbegin(); it != sccs.rend(); ++it) {
      std::vector<vert_id> &scc(*it);

      vert_id *qhead = dual_queue;
      vert_id *qtail = qhead;

      vert_id *next_head = dual_queue + sz;
      vert_id *next_tail = next_head;

      for (vert_id v : scc) {
        *qtail = v;
        vert_marks[v] = BF_SCC | BF_QUEUED;
        qtail++;
      }

      for (int iter = 0; iter < scc.size(); iter++) {
        for (; qtail != qhead;) {
          vert_id s = *(--qtail);
          // If it _was_ on the queue, it must be in the SCC
          vert_marks[s] = BF_SCC;

          Wt s_pot = potentials[s];

          for (auto e : g.e_succs(s)) {
            vert_id d = e.vert;
            Wt sd_pot = s_pot + e.val;
            if (sd_pot < potentials[d]) {
              potentials[d] = sd_pot;
              if (vert_marks[d] == BF_SCC) {
                *next_tail = d;
                vert_marks[d] = (BF_SCC | BF_QUEUED);
                next_tail++;
              }
            }
          }
        }
        // Prepare for the next iteration
        std::swap(qhead, next_head);
        qtail = next_tail;
        next_tail = next_head;
        if (qhead == qtail)
          break;
      }
      // Check if the SCC is feasible.
      for (; qtail != qhead;) {
        vert_id s = *(--qtail);
        Wt s_pot = potentials[s];
        for (auto e : g.e_succs(s)) {
          vert_id d = e.vert;
          if (s_pot + e.val < potentials[d]) {
            // Cleanup vertex marks
            for (vert_id v : g.verts())
              vert_marks[v] = BF_NONE;
            return false;
          }
        }
      }
    }
    return true;
  }

  template <class G, class G1, class G2, class P>
  static void close_after_meet(G &g, const P &pots, G1 &l, G2 &r,
                               edge_vector &delta) {
    // We assume the syntactic meet has already been computed,
    // and potentials have been initialized.
    // We just want to restore closure.
    assert(l.size() == r.size());
    unsigned int sz = l.size();
    grow_scratch(sz);
    delta.clear();

    std::vector<std::vector<vert_id>> colour_succs(2 * sz);
    mut_val_ref_t w;

    // Partition edges into r-only/rb/b-only.
    for (vert_id s : g.verts()) {
      //        unsigned int g_count = 0;
      //        unsigned int r_count = 0;
      for (auto e : g.e_succs(s)) {
        char mark = 0;
        vert_id d = e.vert;
        if (l.lookup(s, d, &w) && w.get() == e.val)
          mark |= E_LEFT;
        if (r.lookup(s, d, &w) && w.get() == e.val)
          mark |= E_RIGHT;
        // Add them to the appropriate coloured successor list
        // Could do it inline, but this'll do.
        assert(mark != 0);
        switch (mark) {
        case E_LEFT:
          colour_succs[2 * s].push_back(d);
          break;
        case E_RIGHT:
          colour_succs[2 * s + 1].push_back(d);
          break;
        default:
          break;
        }
        edge_marks[sz * s + d] = mark;
      }
    }

    // We can run the chromatic Dijkstra variant
    // on each source.
    std::vector<std::pair<vert_id, Wt>> adjs;
    //      for(vert_id v = 0; v < sz; v++)
    for (vert_id v : g.verts()) {
      adjs.clear();
      chrome_dijkstra(g, pots, colour_succs, v, adjs);

      for (std::pair<vert_id, Wt> &p : adjs)
        delta.push_back(std::make_pair(std::make_pair(v, p.first), p.second));
    }
  }

  static void apply_delta(graph_t &g, edge_vector &delta) {
    for (std::pair<std::pair<vert_id, vert_id>, Wt> &e : delta) {
      //        assert(e.first.first != e.first.second);
      //        assert(e.first.first < g.size());
      //        assert(e.first.second < g.size());
      g.set_edge(e.first.first, e.second, e.first.second);
    }
  }

  // Straight implementation of Dijkstra's algorithm
  template <class G, class P>
  static void dijkstra(G &g, const P &p, vert_id src,
                       std::vector<std::pair<vert_id, Wt>> &out) {
    unsigned int sz = g.size();
    if (sz == 0)
      return;
    grow_scratch(sz);

    // Reset all vertices to infty.
    dist_ts[ts_idx] = ts++;
    ts_idx = (ts_idx + 1) % dists.size();

    dists[src] = Wt(0);
    dist_ts[src] = ts;

    WtComp comp(dists);
    WtHeap heap(comp);

    for (auto e : g.e_succs(src)) {
      vert_id dest = e.vert;
      dists[dest] = p[src] + e.val - p[dest];
      dist_ts[dest] = ts;

      vert_marks[dest] = edge_marks[sz * src + dest];
      heap.insert(dest);
    }

    mut_val_ref_t w;
    while (!heap.empty()) {
      int es = heap.removeMin();
      Wt es_cost =
          dists[es] + p[es]; // If it's on the queue, distance is not infinite.
      Wt es_val = es_cost - p[src];
      if (!g.lookup(src, es, &w) || w.get() > es_val)
        out.push_back(std::make_pair(es, es_val));

      for (auto e_ed : g.e_succs(es)) {
        vert_id ed(e_ed.vert);
        Wt v = es_cost + e_ed.val - p[ed];
        if (dist_ts[ed] != ts || v < dists[ed]) {
          dists[ed] = v;
          dist_ts[ed] = ts;

          if (heap.inHeap(ed)) {
            heap.decrease(ed);
          } else {
            heap.insert(ed);
          }
        }
      }
    }
  }

  template <class G, class P>
  static void close_johnson(G &g, const P &p, edge_vector &out) {
    std::vector<std::pair<vert_id, Wt>> adjs;
    for (vert_id v : g.verts()) {
      adjs.clear();
      dijkstra(g, p, v, adjs);
      for (auto p : adjs)
        out.push_back(std::make_pair(std::make_pair(v, p.first), p.second));
    }
  }

  // P is some vector-alike holding a valid system of potentials.
  // Don't need to clear/initialize
  template <class G, class P>
  static void chrome_dijkstra(G &g, const P &p,
                              std::vector<std::vector<vert_id>> &colour_succs,
                              vert_id src,
                              std::vector<std::pair<vert_id, Wt>> &out) {
    unsigned int sz = g.size();
    if (sz == 0)
      return;
    grow_scratch(sz);

    // Reset all vertices to infty.
    dist_ts[ts_idx] = ts++;
    ts_idx = (ts_idx + 1) % dists.size();

    dists[src] = Wt(0);
    dist_ts[src] = ts;

    WtComp comp(dists);
    WtHeap heap(comp);

    for (auto e : g.e_succs(src)) {
      vert_id dest = e.vert;
      dists[dest] = p[src] + e.val - p[dest];
      dist_ts[dest] = ts;

      vert_marks[dest] = edge_marks[sz * src + dest];
      heap.insert(dest);
    }

    mut_val_ref_t w;
    while (!heap.empty()) {
      int es = heap.removeMin();
      Wt es_cost =
          dists[es] + p[es]; // If it's on the queue, distance is not infinite.
      Wt es_val = es_cost - p[src];
      if (!g.lookup(src, es, &w) || w.get() > es_val)
        out.push_back(std::make_pair(es, es_val));

      if (vert_marks[es] == (E_LEFT | E_RIGHT))
        continue;

      // Pick the appropriate set of successors
      std::vector<vert_id> &es_succs = (vert_marks[es] == E_LEFT)
                                           ? colour_succs[2 * es + 1]
                                           : colour_succs[2 * es];
      for (vert_id ed : es_succs) {
        Wt v = es_cost + g.edge_val(es, ed) - p[ed];
        if (dist_ts[ed] != ts || v < dists[ed]) {
          dists[ed] = v;
          dist_ts[ed] = ts;
          vert_marks[ed] = edge_marks[sz * es + ed];

          if (heap.inHeap(ed)) {
            heap.decrease(ed);
          } else {
            heap.insert(ed);
          }
        } else if (v == dists[ed]) {
          vert_marks[ed] |= edge_marks[sz * es + ed];
        }
      }
    }
  }

  // Run Dijkstra's algorithm, but similar to the chromatic algorithm, avoid
  // expanding anything that _was_ stable. GKG: Factor out common elements of
  // this & the previous algorithm.
  template <class G, class P, class S>
  static void dijkstra_recover(G &g, const P &p, const S &is_stable,
                               vert_id src,
                               std::vector<std::pair<vert_id, Wt>> &out) {
    unsigned int sz = g.size();
    if (sz == 0)
      return;
    if (is_stable[src])
      return;

    grow_scratch(sz);

    // Reset all vertices to infty.
    dist_ts[ts_idx] = ts++;
    ts_idx = (ts_idx + 1) % dists.size();

    dists[src] = Wt(0);
    dist_ts[src] = ts;

    WtComp comp(dists);
    WtHeap heap(comp);

    for (auto e : g.e_succs(src)) {
      vert_id dest = e.vert;
      dists[dest] = p[src] + e.val - p[dest];
      dist_ts[dest] = ts;

      vert_marks[dest] = V_UNSTABLE;
      heap.insert(dest);
    }

    mut_val_ref_t w;
    while (!heap.empty()) {
      int es = heap.removeMin();
      Wt es_cost =
          dists[es] + p[es]; // If it's on the queue, distance is not infinite.
      Wt es_val = es_cost - p[src];
      if (!g.lookup(src, es, &w) || w.get() > es_val)
        out.push_back(std::make_pair(es, es_val));

      if (vert_marks[es] == V_STABLE)
        continue;

      char es_mark = is_stable[es] ? V_STABLE : V_UNSTABLE;

      // Pick the appropriate set of successors
      for (auto e : g.e_succs(es)) {
        vert_id ed = e.vert;
        Wt v = es_cost + e.val - p[ed];
        if (dist_ts[ed] != ts || v < dists[ed]) {
          dists[ed] = v;
          dist_ts[ed] = ts;
          vert_marks[ed] = es_mark;

          if (heap.inHeap(ed)) {
            heap.decrease(ed);
          } else {
            heap.insert(ed);
          }
        } else if (v == dists[ed]) {
          vert_marks[ed] |= es_mark;
        }
      }
    }
  }

  class forall_except {
  public:
    forall_except(vert_id _v) : v(_v) {}

    bool operator[](vert_id w) const { return w != v; }

  protected:
    vert_id v;
  };

  /*
  template<class V>
  class vec_neg {
  public:
    vec_neg(const V& _vec)
      : vec(_vec)
    { }

    auto operator[](size_t idx) const { return -(vec[idx]); }
  protected:
    const V& vec;
  };
  template<class V>
  vec_neg<V> negate_vec(const V& vec) { return vec_neg<V>(vec); }
  */
  template <class G, class P>
  static bool repair_potential(G &g, P &p, vert_id ii, vert_id jj) {
    // Ensure there's enough scratch space.
    unsigned int sz = g.size();
    // assert(src < (int) sz && dest < (int) sz);
    grow_scratch(sz);

    for (vert_id vi : g.verts()) {
      dists[vi] = Wt(0);
      dists_alt[vi] = p[vi];
    }
    dists[jj] = p[ii] + g.edge_val(ii, jj) - p[jj];

    if (dists[jj] >= Wt(0))
      return true;

    WtComp comp(dists);
    WtHeap heap(comp);

    heap.insert(jj);

    while (!heap.empty()) {
      int es = heap.removeMin();

      dists_alt[es] = p[es] + dists[es];

      for (auto e : g.e_succs(es)) {
        vert_id ed = e.vert;
        if (dists_alt[ed] == p[ed]) {
          Wt gnext_ed = dists_alt[es] + e.val - dists_alt[ed];
          if (gnext_ed < dists[ed]) {
            dists[ed] = gnext_ed;
            if (heap.inHeap(ed)) {
              heap.decrease(ed);
            } else {
              heap.insert(ed);
            }
          }
        }
      }
    }
    if (dists[ii] < Wt(0))
      return false;

    for (vert_id v : g.verts())
      p[v] = dists_alt[v];

    return true;
  }

  template <class G, class P, class V>
  static void close_after_widen(G &g, P &p, const V &is_stable,
                                edge_vector &delta) {
    unsigned int sz = g.size();
    grow_scratch(sz);
    //      assert(orig.size() == sz);

    for (vert_id v : g.verts()) {
      // We're abusing edge_marks to store _vertex_ flags.
      // Should really just switch this to allocating regions of a fixed-size
      // buffer.
      edge_marks[v] = is_stable[v] ? V_STABLE : V_UNSTABLE;
    }

    std::vector<std::pair<vert_id, Wt>> aux;
    for (vert_id v : g.verts()) {
      if (!edge_marks[v]) {
        aux.clear();
        dijkstra_recover(g, p, edge_marks, v, aux);
        for (auto p : aux)
          delta.push_back(std::make_pair(std::make_pair(v, p.first), p.second));
      }
    }
  }

  // Used for sorting successors of some vertex by increasing slack.
  // operator() may only be called on vertices for which
  // dists is initialized.
  template <class P> class AdjCmp {
  public:
    AdjCmp(const P &_p) : p(_p) {}

    bool operator()(vert_id d1, vert_id d2) const {
      return (dists[d1] - p[d1]) < (dists[d2] - p[d2]);
    }

  protected:
    const P &p;
  };

  template <class P> static AdjCmp<P> make_adjcmp(const P &p) {
    return AdjCmp<P>(p);
  }

  template <class P> class NegP {
  public:
    NegP(const P &_p) : p(_p) {}
    Wt operator[](vert_id v) const { return -(p[v]); }

    const P &p;
  };
  template <class P> static NegP<P> make_negp(const P &p) { return NegP<P>(p); }

  // Compute the transitive closure of edges reachable from v, assuming
  // (1) the subgraph G \ {v} is closed, and (2) P is a valid model of G.
  template <class G, class P>
  static void close_after_assign_fwd(G &g, const P &p, vert_id v,
                                     std::vector<std::pair<vert_id, Wt>> &aux) {
    // Initialize the queue and distances.
    for (vert_id u : g.verts())
      vert_marks[u] = 0;

    vert_marks[v] = BF_QUEUED;
    dists[v] = Wt(0);
    vert_id *adj_head = dual_queue;
    vert_id *adj_tail = adj_head;
    for (auto e : g.e_succs(v)) {
      vert_id d = e.vert;
      vert_marks[d] = BF_QUEUED;
      dists[d] = e.val;
      //        assert(p[v] + dists[d] - p[d] >= Wt(0));
      *adj_tail = d;
      adj_tail++;
    }

    // Sort the immediate edges by increasing slack.
    std::sort(adj_head, adj_tail, make_adjcmp(p));

    vert_id *reach_tail = adj_tail;
    for (; adj_head < adj_tail; adj_head++) {
      vert_id d = *adj_head;

      Wt d_wt = dists[d];
      for (auto edge : g.e_succs(d)) {
        vert_id e = edge.vert;
        Wt e_wt = d_wt + edge.val;
        if (!vert_marks[e]) {
          dists[e] = e_wt;
          vert_marks[e] = BF_QUEUED;
          *reach_tail = e;
          reach_tail++;
        } else {
          dists[e] = std::min(e_wt, dists[e]);
        }
      }
    }

    // Now collect the adjacencies, and clear vertex flags
    // FIXME: This collects _all_ edges from x, not just new ones.
    for (adj_head = dual_queue; adj_head < reach_tail; adj_head++) {
      aux.push_back(std::make_pair(*adj_head, dists[*adj_head]));
      vert_marks[*adj_head] = 0;
    }
  }

  template <class G, class P>
  static void close_after_assign(G &g, P &p, vert_id v, edge_vector &delta) {
    unsigned int sz = g.size();
    grow_scratch(sz);

    std::vector<std::pair<vert_id, Wt>> aux;

    close_after_assign_fwd(g, p, v, aux);
    for (auto p : aux)
      delta.push_back(std::make_pair(std::make_pair(v, p.first), p.second));

    aux.clear();
    GraphRev<G> g_rev(g);
    close_after_assign_fwd(g_rev, make_negp(p), v, aux);
    for (auto p : aux)
      delta.push_back(std::make_pair(std::make_pair(p.first, v), p.second));
  }
};

// Static data allocation
template <class Wt> char *GraphOps<Wt>::edge_marks = NULL;

// Used for Bellman-Ford queueing
template <class Wt>
typename GraphOps<Wt>::vert_id *GraphOps<Wt>::dual_queue = NULL;

template <class Wt> int *GraphOps<Wt>::vert_marks = NULL;

template <class Wt> unsigned int GraphOps<Wt>::scratch_sz = 0;

template <class G> std::vector<typename G::Wt> GraphOps<G>::dists;
template <class G> std::vector<typename G::Wt> GraphOps<G>::dists_alt;
template <class G> std::vector<unsigned int> GraphOps<G>::dist_ts;
template <class G> unsigned int GraphOps<G>::ts = 0;
template <class G> unsigned int GraphOps<G>::ts_idx = 0;

} // namespace crab
#pragma GCC diagnostic pop
