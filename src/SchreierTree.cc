/*
    Copyright 2010 Greg Tener and Elena Erbiceanu
    Released under the Lesser General Public License v3.
*/

#include <fssw/SchreierTree.h>

#include <deque>
#include <sstream>

using std::deque;
using std::stringstream;

namespace fssw {

OrbitIterator::OrbitIterator(map<int, const PermutationWord *> *tree_ptr,
    int root) : use_queue_(true) {
  // use the queue initially to iterate over the root
  smaller_elements_.push_back(root);
  tree_ptr_ = tree_ptr;
  tree_it_ = tree_ptr->begin();
}

bool OrbitIterator::not_at_end() {
  return !smaller_elements_.empty() || tree_it_ != tree_ptr_->end();
}

void OrbitIterator::append(int a) {
  // if the iterator is at its end, use the queue
  if (tree_it_ == tree_ptr_->end()) {
    smaller_elements_.push_back(a);
    use_queue_ = true;
  // otherwise if the iterator is not at its end, only smaller elements which
  // cannot be emitted anymore should be appended
  } else if (a < tree_it_->first) {
    smaller_elements_.push_back(a);
  }
}

OrbitIterator &OrbitIterator::operator++() {
  if (!use_queue_) {
    ++tree_it_;
    // switch to queue if it is nonempty
    if (!smaller_elements_.empty()) {
      use_queue_ = true;
    }
  } else {
    smaller_elements_.pop_front();
    // switch back to iterator if queue is empty
    if (smaller_elements_.empty()) {
      use_queue_ = false;
    }
  }

  return *this;
}

int OrbitIterator::operator*() const {
  if (!use_queue_) {
    return tree_it_->first;
  }

  return smaller_elements_.front();
}

void SchreierTree::set_root(int root) {
  root_ = root;
}

int SchreierTree::get_root() {
  return root_;
}

bool SchreierTree::build_tree() {
  bool tree_changed = false;

  // iterate over the elements in the orbit
  for (SchreierTree::iterator orbit_it = get_orbit_iterator();
      orbit_it.not_at_end(); ++orbit_it) {
    int a = *orbit_it;

    // iterate over the generators
    for (vector<const PermutationWord *>::iterator gens_it =
             generators_.begin(); gens_it != generators_.end();
             ++gens_it) {
      int inverse_image = (*gens_it)->get_inverse_image(a);

      // if we found something not in the orbit
      if (!is_in_orbit(inverse_image)) {
        tree_changed = true;
        tree_[inverse_image] = *gens_it;
        orbit_it.append(inverse_image);  // be sure to tell the orbit iterator
      }
    }
  }

  return tree_changed;
}


void SchreierTree::add_generator(const PermutationWord &g) {
  generators_.push_back(&g);
}

bool SchreierTree::is_in_orbit(int a) {
  return (a == root_) || (tree_.find(a) != tree_.end());
}

bool SchreierTree::path_to_root(int a, PermutationWord *path_ptr) {
  path_ptr->clear();

  if (!is_in_orbit(a)) {
    return false;
  }

  while (a != root_) {
    const PermutationWord *w_ptr = tree_[a];
    path_ptr->compose(*w_ptr);
    a = w_ptr->get_image(a);
  }

  return true;
}

bool SchreierTree::path_from_root(int a, PermutationWord *path_ptr) {
  PermutationWord w;
  bool rv = path_to_root(a, &w);
  path_ptr->compose_inverse(w);

  return rv;
}

OrbitIterator SchreierTree::get_orbit_iterator() {
  return OrbitIterator(&tree_, root_);
}

bool SchreierTree::has_generator(const MapPermutation &g) const {
  // search for the generator it that exact form, by matching strings
  string g_str = g.to_string();

  for (vector<const PermutationWord*>::const_iterator gen_it =
    generators_.begin(); gen_it != generators_.end(); ++gen_it) {
    if ((*gen_it)->to_string() == g_str) {
      return true;
    }
  }
  return false;
}

int SchreierTree::size() const {
  return 1 + tree_.size();  // + 1 for the root
}

string SchreierTree::to_string() const {
  stringstream ss;

  ss << "root: " << root_ << "\n";

  ss << "generators: ";
  for (int i = 0; i < generators_.size(); ++i) {
    ss << generators_[i]->to_string();
    ss << " [" << generators_[i]->to_evaluated_string() << "]";
    if (i + 1 < generators_.size()) {
      ss << ", ";
    }
  }
  ss << "\n";

  ss << "tree: " << "{ ";
  for (map<int, const PermutationWord *>::const_iterator it = tree_.begin();
       it != tree_.end(); ++it) {
    ss << it->first << ": " << it->second->to_evaluated_string();

    if (it != --tree_.end()) {
      ss << ", ";
    }
  }
  ss << "}";

  return ss.str();
}

}  // namespace fssw
