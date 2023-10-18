#ifndef _CIRCULAR_LIST_H_
#define _CIRCULAR_LIST_H_

template <typename T>
class CircularList {
private:
	struct Node {
		T value;
		Node *next;
	};
	int size;
	int max_size;
	Node *first;
public:
	CircularList(int max_size): max_size(max_size), size(0), first(nullptr) {}
	~CircularList() {
		Node* p = first, *q = first->next;
		for(int i = 0; i < size; ++i) {
			delete p;
			p = q;
			q = q->next;
		}
	}
	void insert(const T& value) {
		if (first == nullptr) {
			first = new Node;
			first->next = first;
			size++;
		}
		else if (size < max_size) {
			Node * node = new Node;
			node->value = value;
			Node * last = first;
			while(last->next != first) {
				last = last->next;
			}
			last->next = node;
			node->next = first;
			size++;
		}
		else {
			first->value = value;
			first = first->next;
		}
	}

	class Iterator {
	private:
		Node * cur;
		int remaining;
	public:
		Iterator(Node * node, int count) : cur(node), remaining(count) {}

		Iterator& operator++() {
			if (0 == remaining) {
				throw "Iterator out of range.";
			}
			cur = cur->next;
			remaining--;
			return *this;
		}

		bool operator==(const Iterator& other) const {
			return cur == other.cur && remaining == other.remaining;
		}

		bool operator!=(const Iterator& other) const {
			return !(*this == other);
		}

		T& operator*() {
			if(0 == remaining) {
				throw "Iterator out of range.";
			}
			return cur->value;
		}

	};

	Iterator begin() {
		return Iterator(first, size);
	}

	Iterator end() {
		return Iterator(first, 0);
	}

};
 
#endif  // _CIRCULAR_LIST_H_