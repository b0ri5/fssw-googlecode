/*
    Copyright 2010 Greg Tener and Elena Erbiceanu
    Released under the Lesser General Public License v3.
*/

#include <fssw/MapPermutation.h>

#include <cstdlib>
#include <vector>
#include <set>

using std::vector;
using std::set;

namespace fssw {

int MapPermutation::get_image(int a) const {
  map<int, int>::const_iterator a_it = images_.find(a);

  if (a_it == images_.end()) {
    return a;
  }

  return a_it->second;
}

int MapPermutation::get_inverse_image(int a) const {
  map<int, int>::const_iterator a_it = inverse_images_.find(a);

  if (a_it == inverse_images_.end()) {
    return a;
  }

  return a_it->second;
}

void MapPermutation::set_image(int a, int b) {
  if (a == b) {
    images_.erase(a);
    inverse_images_.erase(a);
  } else {
    images_[a] = b;
    inverse_images_[b] = a;
  }
}

void MapPermutation::compose_with_mapping(const map<int, int> &mapping) {
  set<int> elements_seen;
  MapPermutation result;

  for (map<int, int>::iterator images_it = images_.begin();
      images_it != images_.end(); ++images_it) {
    int a = images_it->first;
    int b = images_it->second;
    int c = 0;
    // keep track of images
    elements_seen.insert(b);

    map<int, int>::const_iterator g_it = mapping.find(b);
    if (g_it == mapping.end()) {
      c = b;
    } else {
      c = g_it->second;
    }

    result.set_image(a, c);
  }

  for (map<int, int>::const_iterator g_it = mapping.begin(); g_it
      != mapping.end(); ++g_it) {
    // if mapping moves an element that did not exist as an image in this
    // permutation, it was an implied (a -> a)
    if (elements_seen.find(g_it->first) == elements_seen.end()) {
      result.set_image(g_it->first, g_it->second);
    }
  }
  // replace this with the result
  this->images_ = result.images_;
  this->inverse_images_ = result.inverse_images_;
}

void MapPermutation::compose(const MapPermutation &g) {
  compose_with_mapping(g.images_);
}

void MapPermutation::compose_inverse(const MapPermutation &g) {
  compose_with_mapping(g.inverse_images_);
}

void MapPermutation::clear() {
  images_.clear();
  inverse_images_.clear();
}

bool MapPermutation::is_identity() const {
  return images_.size() == 0;
}

static bool is_whitespace(char c) {
  if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
    return true;
  }

  return false;
}

static bool is_digit(char c) {
  return (c >= '0' && c <= '9');
}

// read an integer into a, advancing the pos
bool parse_nonnegative_int(const string &s, int *pos_ptr, int *a_ptr) {
  int len = 0;

  while (*pos_ptr + len < s.length() && is_digit(s.at(*pos_ptr + len))) {
    len++;
  }

  // if we see no digits, return false
  if (len == 0) {
    fprintf(stdout, "Expected integer at pos %d in %s.\n", *pos_ptr, s.c_str());
    return false;
  }

  // parse the integer and increment the position
  *a_ptr = atoi(s.substr(*pos_ptr, len).c_str());
  (*pos_ptr) += len;

  return true;
}

// parses a string "(0 2 3)" into a vector (doesn't check for validity)
// returns true if cycle is well-formed
bool parse_cycle(const string &s, int *pos_ptr, vector<int> *cycle_ptr,
    set<int> *elements_seen_ptr) {
  // starts with '('
  if (s.at(*pos_ptr) != '(') {
    fprintf(stdout, "Expected opening parenthesis at %d.\n", *pos_ptr);
    return false;
  }

  // pass up the '('
  (*pos_ptr)++;

  int a = 0;

  while (*pos_ptr < s.length()) {
    // try to read an int
    if (!parse_nonnegative_int(s, pos_ptr, &a)) {
      return false;
    }

    // if this element was in the permutation before, input is malformed.
    if (elements_seen_ptr->find(a) != elements_seen_ptr->end()) {
      fprintf(stdout, "Element repeated: %d\n", a);
      return false;
    }

    elements_seen_ptr->insert(a);
    cycle_ptr->push_back(a);

    // if we're not looking at a ' ' then break
    if (s.at(*pos_ptr) != ' ') {
      break;
    }

    (*pos_ptr)++;
  }

  if (*pos_ptr >= s.length() || s.at(*pos_ptr) != ')') {
    fprintf(stdout, "Expected ending parenthesis: %s at %d\n",
            s.c_str(), *pos_ptr);
    return false;
  }
  (*pos_ptr)++;

  // cycle was well formed
  return true;
}

// advances to the first non-whitespace character
static void eat_whitespace(const string &s, int *pos_ptr) {
  while (*pos_ptr < s.length() && is_whitespace(s.at(*pos_ptr))) {
    (*pos_ptr)++;
  }
}

bool MapPermutation::from_string(string s) {
  clear();

  int pos = 0;

  // eat initial whitespace
  eat_whitespace(s, &pos);

  // special case, identity
  if (s.substr(pos, 2).compare("()") == 0) {
    return true;
  }

  // parse the cycles
  vector<vector<int> > cycles;
  set<int> elements_seen;

  while (pos < s.length() && s.at(pos) == '(') {
    vector<int> cycle;

    if (!parse_cycle(s, &pos, &cycle, &elements_seen)) {
      return false;
    }
    if (cycle.size() > 1) {
      cycles.push_back(cycle);
    }
  }

  // eat trailing whitespace
  eat_whitespace(s, &pos);

  // if there is still non-whitespace left over, invalid input
  if (pos < s.length()) {
    fprintf(stdout, "Expected only whitespace in: %s at %d\n", s.c_str(), pos);
    return false;
  }

  // form permutation from cycles
  for (vector<vector<int> >::const_iterator cycle_it = cycles.begin();
      cycle_it != cycles.end(); cycle_it++) {
    int element_pos = 0;
    int a = 0;
    int a_image = 0;

    while (element_pos + 1 < cycle_it->size()) {
      a = cycle_it->at(element_pos);
      a_image = cycle_it->at(element_pos + 1);
      set_image(a, a_image);
      element_pos++;
    }
    // connect circular ends
    a = cycle_it->at(cycle_it->size() - 1);
    a_image = cycle_it->at(0);
    set_image(a, a_image);
  }

  return true;
}

bool MapPermutation::is_equal(const MapPermutation &g) const {
  if (images_.size() != g.images_.size()) {
    return false;
  }

  for (map<int, int>::const_iterator a_it = images_.begin();
      a_it != images_.end(); ++a_it) {
    if (a_it->second != g.get_image(a_it->first)) {
      return false;
    }
  }

  return true;
}

string MapPermutation::to_string() const {
  if (is_identity()) {
    return "()";
  }

  string s = "";
  set<int> elements_seen;

  for (map<int, int>::const_iterator a_it = images_.begin();
      a_it != images_.end(); ++a_it) {
    int a = a_it->first;

    // try to get cycle
    int cycle_length = 0;

    while (true) {
      // if this element has already been used, skip
      if (elements_seen.find(a) != elements_seen.end()) {
        // close cycle
        if (cycle_length > 0) {
          s += ")";
          cycle_length = 0;
        }
        break;
      }
      // follow through the cycle
      elements_seen.insert(a);
      char a_s[32];
      itoa(a, a_s, sizeof(a_s) / sizeof(*a_s));
      if (cycle_length == 0) {
        s += "(" + string(a_s);
      } else {
        s += " " + string(a_s);
      }
      a = get_image(a);
      ++cycle_length;
    }
  }

  return s;
}

}  // namespace fssw
