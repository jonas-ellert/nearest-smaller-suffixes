#pragma once

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <stdint.h>
#include <vector>

namespace linear_time_runs {

template <typename index_type>
struct run_type {
  index_type start;
  index_type period;
  index_type len;

  inline bool operator==(run_type const& other) {
    return start == other.start && period == other.period && len == other.len;
  }

  inline bool operator!=(run_type const& other) {
    return !(*this == other);
  }

  inline bool operator<(run_type const& other) {
    return start < other.start ||
           (start == other.start && period < other.period);
  }
};

namespace internal {

template <bool increasing,
    typename value_type,
    typename index_type> // true for increasing runs
auto compute_unidirectional_runs(value_type const* const text,
                                 index_type const n) {

  auto compare = [&text](index_type const& i, index_type const& j,
                         index_type const& lce) {
    if constexpr (increasing) {
      return j > 0 && text[i + lce] > text[j + lce];
    } else {
      return text[i + lce] < text[j + lce];
    }
  };

  auto get_rlce = [&](index_type const& i, index_type const& j,
                      index_type lce = 0) {
    while (text[i + lce] == text[j + lce])
      ++lce;
    return lce;
  };

  auto get_llce = [&](index_type const& i, index_type const& j,
                      index_type llce = 0) {
    while (text[i - llce] == text[j - llce])
      ++llce;
    return llce;
  };

  struct distance_edge_type {
    index_type distance, lce;

    distance_edge_type(index_type const d, index_type const l)
        : distance(d), lce(l) {}
  };

  struct target_edge_type {
    index_type target, lce;

    target_edge_type(index_type const t, index_type const l)
        : target(t), lce(l) {}
  };

  std::deque<target_edge_type> q;
  auto top_j = [&q]() { return q.back().target; };
  auto top_lce = [&q]() { return q.back().lce; };
  auto pop = [&q]() { q.pop_back(); };
  auto push = [&q](index_type const j, index_type const lce) {
    q.emplace_back(j, lce);
  };

  push(0, 0);
  push(1, 0);

  std::vector<index_type> first_edge_of_node(n + 1);
  std::vector<distance_edge_type> edges;
  edges.reserve(2 * n);

  first_edge_of_node[1] = 0;
  edges.emplace_back(1, 0);

  index_type distance = 1;
  index_type rhs = 1;
  for (index_type i = 2; i < n - 1; ++i) {
    first_edge_of_node[i] = edges.size();

    index_type const copy_from = i - distance;
    index_type const stop_edge = first_edge_of_node[copy_from + 1];
    index_type e = first_edge_of_node[copy_from];

    for (; e < stop_edge; ++e) {
      if (i + edges[e].lce < rhs) {
        edges.emplace_back(edges[e]);
      } else
        break;
    }

    if (e == stop_edge) {
      index_type const target = i - edges.back().distance;

      while (top_j() > target)
        pop();
      push(i, edges.back().lce);
      continue;
    }

    index_type j = i - edges[e].distance;
    index_type lce = get_rlce(i, j, (rhs > i) ? (rhs - i) : (index_type) 0);
    rhs = i + lce;
    distance = i - j;
    edges.emplace_back(distance, lce);

    while (top_j() > j)
      pop();

    while (compare(i, j, lce)) {
      if (top_lce() < lce) {
        lce = top_lce();
        pop();
        edges.emplace_back(i - top_j(), lce);
        break;
      }

      pop();
      j = top_j();
      lce = get_rlce(i, j, lce);
      rhs = i + lce;
      distance = i - j;
      edges.emplace_back(distance, lce);
    }

    push(i, lce);
  }

  first_edge_of_node[n - 1] = edges.size();
  while (top_j() > 0) {
    edges.emplace_back(n - 1 - top_j(), 0);
    pop();
  }
  first_edge_of_node[n] = edges.size() + 1;

  struct nss_edge_type {
    index_type distance, llce, rlce;
  };

  std::vector<nss_edge_type> nss_edges(n);
  for (index_type i = 1; i < n; ++i) {
    index_type e = first_edge_of_node[i];
    index_type end_edge = first_edge_of_node[i + 1] - 1; // last edge = pss
    for (; e < end_edge; ++e) {
      nss_edges[i - edges[e].distance] =
          nss_edge_type{edges[e].distance, 0, edges[e].lce};
    }
  }

  nss_edges[n - 2].llce = 0;
  index_type lhs = n - 2;
  distance = 1;

  for (index_type i = n - 3; i > 0; --i) {
    if (i > lhs + nss_edges[i + distance].llce) {
      nss_edges[i].llce = nss_edges[i + distance].llce;
      continue;
    }

    nss_edges[i].llce = get_llce(i, i + nss_edges[i].distance,
                                 (lhs < i) ? (i - lhs) : (index_type) 0);
    lhs = i - nss_edges[i].llce;
    distance = nss_edges[i].distance;
  }

  std::vector<index_type> count_runs_at_idx(n);
  for (index_type i = 1; i < n - 1; ++i) {
    if (nss_edges[i].distance < nss_edges[i].llce + nss_edges[i].rlce) {
      ++count_runs_at_idx[i - nss_edges[i].llce + 1];
    }
  }

  index_type left_border = 0;
  for (index_type i = 1; i < n - 1; ++i) {
    index_type const gsize = count_runs_at_idx[i];
    count_runs_at_idx[i] = left_border;
    left_border += gsize;
  }

  std::vector<run_type<index_type>> runs(left_border);

  for (index_type i = 1; i < n - 1; ++i) {
    if (nss_edges[i].distance < nss_edges[i].llce + nss_edges[i].rlce) {
      auto& border = count_runs_at_idx[i - nss_edges[i].llce + 1];
      runs[border++] = run_type<index_type>{
          i - nss_edges[i].llce + 1, nss_edges[i].distance,
          nss_edges[i].distance + nss_edges[i].llce + nss_edges[i].rlce - 1};
    }
  }

  left_border = 1;
  for (index_type i = 1; i < runs.size(); ++i) {
    if (runs[i] != runs[i - 1]) {
      runs[left_border++] = runs[i];
    }
  }

  runs.resize(left_border);
  runs.shrink_to_fit();
  return runs;
}

} // namespace internal

template <typename value_type, typename index_type>
auto compute_all_runs(value_type const* const text, index_type const n) {
  auto increasing = internal::compute_unidirectional_runs<true>(text, n);
  auto decreasing = internal::compute_unidirectional_runs<false>(text, n);

  decltype(increasing) result(increasing.size() + decreasing.size());
  std::merge(increasing.begin(), increasing.end(), decreasing.begin(),
             decreasing.end(), result.begin());
  return result;
}




} // namespace linear_time_runs