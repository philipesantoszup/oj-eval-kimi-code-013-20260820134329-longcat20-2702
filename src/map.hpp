/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less<Key>
   > class map {
  public:
   typedef pair<const Key, T> value_type;

  private:
   struct node_t {
       value_type *data;
       node_t *left, *right, *parent;
       bool is_red;

       node_t() : data(nullptr), left(nullptr), right(nullptr), parent(nullptr), is_red(false) {}
       node_t(const value_type &d, node_t *p = nullptr)
           : data(new value_type(d)), left(nullptr), right(nullptr), parent(p), is_red(true) {}
       ~node_t() {
           if (data) delete data;
       }
   };

   node_t *nil_;
   node_t *root_;
   node_t *begin_;
   size_t size_;
   Compare comp_;

   node_t *minimum(node_t *x) const {
       if (!x || x == nil_) return nullptr;
       while (x->left != nil_) x = x->left;
       return x;
   }
   node_t *maximum(node_t *x) const {
       if (!x || x == nil_) return nullptr;
       while (x->right != nil_) x = x->right;
       return x;
   }

   node_t *successor(node_t *x) const {
       if (!x || x == nil_) return nullptr;
       if (x->right != nil_) return minimum(x->right);
       node_t *y = x->parent;
       while (y && x == y->right) { x = y; y = y->parent; }
       return y ? y : nil_;
   }
   node_t *predecessor(node_t *x) const {
       if (x == nil_) return maximum(root_);
       if (!x) return nullptr;
       if (x->left != nil_) return maximum(x->left);
       node_t *y = x->parent;
       while (y && x == y->left) { x = y; y = y->parent; }
       return y;
   }

   void left_rotate(node_t *x) {
       node_t *y = x->right;
       x->right = y->left;
       if (y->left != nil_) y->left->parent = x;
       y->parent = x->parent;
       if (x->parent == nullptr) root_ = y;
       else if (x == x->parent->left) x->parent->left = y;
       else x->parent->right = y;
       y->left = x;
       x->parent = y;
   }

   void right_rotate(node_t *y) {
       node_t *x = y->left;
       y->left = x->right;
       if (x->right != nil_) x->right->parent = y;
       x->parent = y->parent;
       if (y->parent == nullptr) root_ = x;
       else if (y == y->parent->left) y->parent->left = x;
       else y->parent->right = x;
       x->right = y;
       y->parent = x;
   }

   void insert_fixup(node_t *z) {
       while (z->parent && z->parent->is_red) {
           if (z->parent == z->parent->parent->left) {
               node_t *y = z->parent->parent->right;
               if (y && y->is_red) {
                   z->parent->is_red = false;
                   y->is_red = false;
                   z->parent->parent->is_red = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->right) {
                       z = z->parent;
                       left_rotate(z);
                   }
                   z->parent->is_red = false;
                   z->parent->parent->is_red = true;
                   right_rotate(z->parent->parent);
               }
           } else {
               node_t *y = z->parent->parent->left;
               if (y && y->is_red) {
                   z->parent->is_red = false;
                   y->is_red = false;
                   z->parent->parent->is_red = true;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->left) {
                       z = z->parent;
                       right_rotate(z);
                   }
                   z->parent->is_red = false;
                   z->parent->parent->is_red = true;
                   left_rotate(z->parent->parent);
               }
           }
       }
       root_->is_red = false;
   }

   void transplant(node_t *u, node_t *v) {
       if (u->parent == nullptr) root_ = v;
       else if (u == u->parent->left) u->parent->left = v;
       else u->parent->right = v;
       if (v) v->parent = u->parent;
   }

   void erase_fixup(node_t *x, node_t *x_parent) {
       while (x != root_ && (!x || !x->is_red)) {
           if (x == x_parent->left) {
               node_t *w = x_parent->right;
               if (w && w->is_red) {
                   w->is_red = false;
                   x_parent->is_red = true;
                   left_rotate(x_parent);
                   w = x_parent->right;
               }
               if (w && (!w->left || !w->left->is_red) && (!w->right || !w->right->is_red)) {
                   if (w) w->is_red = true;
                   x = x_parent;
                   x_parent = x->parent;
               } else {
                   if (w && (!w->right || !w->right->is_red)) {
                       if (w && w->left) w->left->is_red = false;
                       if (w) w->is_red = true;
                       if (w) right_rotate(w);
                       w = x_parent->right;
                   }
                   if (w) w->is_red = x_parent->is_red;
                   x_parent->is_red = false;
                   if (w && w->right) w->right->is_red = false;
                   left_rotate(x_parent);
                   x = root_;
                   x_parent = nullptr;
               }
           } else {
               node_t *w = x_parent->left;
               if (w && w->is_red) {
                   w->is_red = false;
                   x_parent->is_red = true;
                   right_rotate(x_parent);
                   w = x_parent->left;
               }
               if (w && (!w->right || !w->right->is_red) && (!w->left || !w->left->is_red)) {
                   if (w) w->is_red = true;
                   x = x_parent;
                   x_parent = x->parent;
               } else {
                   if (w && (!w->left || !w->left->is_red)) {
                       if (w && w->right) w->right->is_red = false;
                       if (w) w->is_red = true;
                       if (w) left_rotate(w);
                       w = x_parent->left;
                   }
                   if (w) w->is_red = x_parent->is_red;
                   x_parent->is_red = false;
                   if (w && w->left) w->left->is_red = false;
                   right_rotate(x_parent);
                   x = root_;
                   x_parent = nullptr;
               }
           }
       }
       if (x) x->is_red = false;
   }

   void delete_node(node_t *z) {
       if (!z || z == nil_) return;
       node_t *y = z;
       bool y_orig_is_red = y->is_red;
       node_t *x, *x_parent;

       if (z->left == nil_) {
           x = z->right;
           x_parent = z->parent;
           transplant(z, z->right);
       } else if (z->right == nil_) {
           x = z->left;
           x_parent = z->parent;
           transplant(z, z->left);
       } else {
           y = minimum(z->right);
           y_orig_is_red = y->is_red;
           x = y->right;
           if (y->parent == z) {
               x_parent = y;
           } else {
               x_parent = y->parent;
               transplant(y, y->right);
               y->right = z->right;
               y->right->parent = y;
           }
           transplant(z, y);
           y->left = z->left;
           y->left->parent = y;
           y->is_red = z->is_red;
       }

       delete z;
       --size_;

       if (size_ == 0) {
           root_ = nullptr;
           begin_ = nullptr;
       } else {
           if (z == begin_) begin_ = minimum(root_);
           if (!y_orig_is_red) {
               erase_fixup(x, x_parent);
           }
       }
   }

   void clear_node(node_t *x) {
       if (!x || x == nil_) return;
       clear_node(x->left);
       clear_node(x->right);
       if (x != nil_) delete x;
   }

   node_t *find_node(const Key &key) const {
       node_t *p = root_;
       while (p && p != nil_) {
           if (comp_(key, p->data->first)) p = p->left;
           else if (comp_(p->data->first, key)) p = p->right;
           else return p;
       }
       return nullptr;
   }

   pair<node_t *, bool> insert_node(const value_type &val) {
       if (!root_) {
           node_t *z = new node_t(val);
           root_ = z;
           z->left = z->right = nil_;
           z->is_red = false;
           begin_ = z;
           size_ = 1;
           return {z, true};
       }

       node_t *cur = root_;
       node_t *parent = nullptr;
       while (cur != nil_) {
           parent = cur;
           if (comp_(val.first, cur->data->first)) cur = cur->left;
           else if (comp_(cur->data->first, val.first)) cur = cur->right;
           else return {cur, false};
       }

       node_t *z = new node_t(val, parent);
       z->left = z->right = nil_;
       if (comp_(val.first, parent->data->first)) parent->left = z;
       else parent->right = z;
       ++size_;

       if (comp_(val.first, begin_->data->first)) begin_ = z;

       insert_fixup(z);
       return {z, true};
   }

   node_t *copy_node(node_t *src, node_t *src_nil, node_t *nil_dst) {
       if (!src || src == src_nil) return nil_dst;
       node_t *dst = new node_t();
       dst->data = new value_type(*src->data);
       dst->is_red = src->is_red;
       dst->parent = nullptr;
       dst->left = copy_node(src->left, src_nil, nil_dst);
       dst->right = copy_node(src->right, src_nil, nil_dst);
       if (dst->left != nil_dst) dst->left->parent = dst;
       if (dst->right != nil_dst) dst->right->parent = dst;
       return dst;
   }

  public:
   class const_iterator;
   class iterator {
      private:
       const map *container;
       node_t *node;
      public:
       iterator() : container(nullptr), node(nullptr) {}
       iterator(const map *c, node_t *n) : container(c), node(n) {}
       iterator(const iterator &other) : container(other.container), node(other.node) {}

       iterator operator++(int) {
           if (!container || !node || node == container->nil_)
               throw invalid_iterator();
           iterator tmp = *this;
           node = container->successor(node);
           return tmp;
       }
       iterator &operator++() {
           if (!container || !node || node == container->nil_)
               throw invalid_iterator();
           node = container->successor(node);
           return *this;
       }
       iterator operator--(int) {
           if (!container || !node)
               throw invalid_iterator();
           iterator tmp = *this;
           node_t *pred = container->predecessor(node);
           if (!pred) throw invalid_iterator();
           node = pred;
           return tmp;
       }
       iterator &operator--() {
           if (!container || !node)
               throw invalid_iterator();
           node_t *pred = container->predecessor(node);
           if (!pred) throw invalid_iterator();
           node = pred;
           return *this;
       }

       value_type &operator*() const {
           if (!container || !node || node == container->nil_)
               throw invalid_iterator();
           return *node->data;
       }
       bool operator==(const iterator &rhs) const { return node == rhs.node; }
       bool operator==(const const_iterator &rhs) const { return node == rhs.node; }
       bool operator!=(const iterator &rhs) const { return node != rhs.node; }
       bool operator!=(const const_iterator &rhs) const { return node != rhs.node; }

       value_type *operator->() const {
           if (!container || !node || node == container->nil_)
               throw invalid_iterator();
           return node->data;
       }

       friend class const_iterator;
       friend class map;
   };

   class const_iterator {
      private:
       const map *container;
       node_t *node;
      public:
       const_iterator() : container(nullptr), node(nullptr) {}
       const_iterator(const map *c, node_t *n) : container(c), node(n) {}
       const_iterator(const const_iterator &other) : container(other.container), node(other.node) {}
       const_iterator(const iterator &other) : container(other.container), node(other.node) {}

       const_iterator operator++(int) {
           if (!container || !node || node == container->nil_)
               throw invalid_iterator();
           const_iterator tmp = *this;
           node = container->successor(node);
           return tmp;
       }
       const_iterator &operator++() {
           if (!container || !node || node == container->nil_)
               throw invalid_iterator();
           node = container->successor(node);
           return *this;
       }
       const_iterator operator--(int) {
           if (!container || !node)
               throw invalid_iterator();
           const_iterator tmp = *this;
           node_t *pred = container->predecessor(node);
           if (!pred) throw invalid_iterator();
           node = pred;
           return tmp;
       }
       const_iterator &operator--() {
           if (!container || !node)
               throw invalid_iterator();
           node_t *pred = container->predecessor(node);
           if (!pred) throw invalid_iterator();
           node = pred;
           return *this;
       }

       const value_type &operator*() const {
           if (!container || !node || node == container->nil_)
               throw invalid_iterator();
           return *node->data;
       }
       bool operator==(const const_iterator &rhs) const { return node == rhs.node; }
       bool operator==(const iterator &rhs) const { return node == rhs.node; }
       bool operator!=(const const_iterator &rhs) const { return node != rhs.node; }
       bool operator!=(const iterator &rhs) const { return node != rhs.node; }

       const value_type *operator->() const {
           if (!container || !node || node == container->nil_)
               throw invalid_iterator();
           return node->data;
       }

       friend class map;
   };

   map() : root_(nullptr), begin_(nullptr), size_(0) {
       nil_ = new node_t();
       nil_->is_red = false;
   }

   map(const map &other) : root_(nullptr), begin_(nullptr), size_(0), comp_(other.comp_) {
       nil_ = new node_t();
       nil_->is_red = false;
       if (other.root_) {
           root_ = copy_node(other.root_, other.nil_, nil_);
           root_->parent = nullptr;
           size_ = other.size_;
           begin_ = minimum(root_);
       }
   }

   map &operator=(const map &other) {
       if (this == &other) return *this;
       clear();
       comp_ = other.comp_;
       if (other.root_) {
           root_ = copy_node(other.root_, other.nil_, nil_);
           root_->parent = nullptr;
           size_ = other.size_;
           begin_ = minimum(root_);
       }
       return *this;
   }

   ~map() {
       clear();
       delete nil_;
   }

   T &at(const Key &key) {
       node_t *n = find_node(key);
       if (!n) throw index_out_of_bound();
       return n->data->second;
   }

   const T &at(const Key &key) const {
       node_t *n = find_node(key);
       if (!n) throw index_out_of_bound();
       return n->data->second;
   }

   T &operator[](const Key &key) {
       node_t *n = find_node(key);
       if (n) return n->data->second;
       value_type val(key, T());
       auto res = insert_node(val);
       return res.first->data->second;
   }

   const T &operator[](const Key &key) const {
       node_t *n = find_node(key);
       if (!n) throw index_out_of_bound();
       return n->data->second;
   }

   iterator begin() {
       if (!root_) return iterator(this, nil_);
       return iterator(this, begin_);
   }

   const_iterator cbegin() const {
       if (!root_) return const_iterator(this, nil_);
       return const_iterator(this, begin_);
   }

   iterator end() { return iterator(this, nil_); }
   const_iterator cend() const { return const_iterator(this, nil_); }

   bool empty() const { return size_ == 0; }
   size_t size() const { return size_; }

   void clear() {
       if (root_) clear_node(root_);
       root_ = nullptr;
       begin_ = nullptr;
       size_ = 0;
   }

   pair<iterator, bool> insert(const value_type &value) {
       auto res = insert_node(value);
       return {iterator(this, res.first), res.second};
   }

   void erase(iterator pos) {
       if (!pos.container || pos.container != this || !pos.node || pos.node == nil_)
           throw invalid_iterator();
       node_t *z = pos.node;
       node_t *actual = find_node(z->data->first);
       if (actual != z) throw invalid_iterator();

       if (z == begin_) {
           node_t *next = successor(z);
           if (size_ == 1) begin_ = nullptr;
           else if (next) begin_ = next;
       }

       delete_node(z);
   }

   size_t count(const Key &key) const {
       return find_node(key) ? 1 : 0;
   }

   iterator find(const Key &key) {
       node_t *n = find_node(key);
       if (!n) return end();
       return iterator(this, n);
   }

   const_iterator find(const Key &key) const {
       node_t *n = find_node(key);
       if (!n) return cend();
       return const_iterator(this, n);
   }
};

}

#endif
