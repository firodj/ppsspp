package binarysearchtree

import (
	"fmt"
	"sync"
)

// Node a single node that composes the tree
type Node[T any] struct {
	key   int
	value T
	left  *Node[T] //left
	right *Node[T] //right
}

// ItemBinarySearchTree the binary search tree of Items
type ItemBinarySearchTree[T any] struct {
	root *Node[T]
	lock sync.RWMutex
}

// Insert inserts the Item t in the tree
func (bst *ItemBinarySearchTree[T]) Insert(key int, value T) {
	bst.lock.Lock()
	defer bst.lock.Unlock()
	n := &Node[T]{key, value, nil, nil}
	if bst.root == nil {
			bst.root = n
	} else {
			insertNode(bst.root, n)
	}
}

// internal function to find the correct place for a node in a tree
func insertNode[T any](node, newNode *Node[T]) {
	if newNode.key < node.key {
			if node.left == nil {
					node.left = newNode
			} else {
					insertNode(node.left, newNode)
			}
	} else {
			if node.right == nil {
					node.right = newNode
			} else {
					insertNode(node.right, newNode)
			}
	}
}

// InOrderTraverse visits all nodes with in-order traversing
func (bst *ItemBinarySearchTree[T]) InOrderTraverse(f func(T)) {
	bst.lock.RLock()
	defer bst.lock.RUnlock()
	inOrderTraverse(bst.root, f)
}

// internal recursive function to traverse in order
func inOrderTraverse[T any](n *Node[T], f func(T)) {
	if n != nil {
			inOrderTraverse(n.left, f)
			f(n.value)
			inOrderTraverse(n.right, f)
	}
}

// PreOrderTraverse visits all nodes with pre-order traversing
func (bst *ItemBinarySearchTree[T]) PreOrderTraverse(f func(T)) {
	bst.lock.Lock()
	defer bst.lock.Unlock()
	preOrderTraverse(bst.root, f)
}

// internal recursive function to traverse pre order
func preOrderTraverse[T any](n *Node[T], f func(T)) {
	if n != nil {
			f(n.value)
			preOrderTraverse(n.left, f)
			preOrderTraverse(n.right, f)
	}
}

// PostOrderTraverse visits all nodes with post-order traversing
func (bst *ItemBinarySearchTree[T]) PostOrderTraverse(f func(T)) {
	bst.lock.Lock()
	defer bst.lock.Unlock()
	postOrderTraverse(bst.root, f)
}

// internal recursive function to traverse post order
func postOrderTraverse[T any](n *Node[T], f func(T)) {
	if n != nil {
			postOrderTraverse(n.left, f)
			postOrderTraverse(n.right, f)
			f(n.value)
	}
}

// Min returns the Item with min value stored in the tree
func (bst *ItemBinarySearchTree[T]) Min() *T {
	bst.lock.RLock()
	defer bst.lock.RUnlock()
	n := bst.root
	if n == nil {
			return nil
	}
	for {
			if n.left == nil {
					return &n.value
			}
			n = n.left
	}
}

// Max returns the Item with max value stored in the tree
func (bst *ItemBinarySearchTree[T]) Max() *T {
	bst.lock.RLock()
	defer bst.lock.RUnlock()
	n := bst.root
	if n == nil {
			return nil
	}
	for {
			if n.right == nil {
					return &n.value
			}
			n = n.right
	}
}

// Search returns true if the Item t exists in the tree
func (bst *ItemBinarySearchTree[T]) Search(key int) bool {
	bst.lock.RLock()
	defer bst.lock.RUnlock()
	return search(bst.root, key)
}

// internal recursive function to search an item in the tree
func search[T any](n *Node[T], key int) bool {
	if n == nil {
			return false
	}
	if key < n.key {
			return search(n.left, key)
	}
	if key > n.key {
			return search(n.right, key)
	}
	return true
}

// Remove removes the Item with key `key` from the tree
func (bst *ItemBinarySearchTree[T]) Remove(key int) {
	bst.lock.Lock()
	defer bst.lock.Unlock()
	remove(bst.root, key)
}

// internal recursive function to remove an item
func remove[T any](node *Node[T], key int) *Node[T] {
	if node == nil {
			return nil
	}
	if key < node.key {
			node.left = remove(node.left, key)
			return node
	}
	if key > node.key {
			node.right = remove(node.right, key)
			return node
	}
	// key == node.key
	if node.left == nil && node.right == nil {
			node = nil
			return nil
	}
	if node.left == nil {
			node = node.right
			return node
	}
	if node.right == nil {
			node = node.left
			return node
	}
	leftmostrightside := node.right
	for {
			//find smallest value on the right side
			if leftmostrightside != nil && leftmostrightside.left != nil {
					leftmostrightside = leftmostrightside.left
			} else {
					break
			}
	}
	node.key, node.value = leftmostrightside.key, leftmostrightside.value
	node.right = remove(node.right, node.key)
	return node
}

// String prints a visual representation of the tree
func (bst *ItemBinarySearchTree[T]) String() {
	bst.lock.Lock()
	defer bst.lock.Unlock()
	fmt.Println("------------------------------------------------")
	stringify(bst.root, 0)
	fmt.Println("------------------------------------------------")
}

// internal recursive function to print a tree
func stringify[T any](n *Node[T], level int) {
	if n != nil {
		format := ""
		for i := 0; i < level; i++ {
				format += "       "
		}
		format += "---[ "
		level++
		stringify(n.left, level)
		fmt.Printf(format+"%d\n", n.key)
		stringify(n.right, level)
	}
}
