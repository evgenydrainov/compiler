#pragma once

#include "base_types.h"

template <typename Node>
struct list
{
	struct iterator
	{
		Node *node;

		iterator &operator++()
		{
			node = node->next;
			return *this;
		}

		Node * operator*()
		{
			return node;
		}

		bool operator==(iterator other)
		{
			return node == other.node;
		}

		bool operator!=(iterator other)
		{
			return node != other.node;
		}
	};

	Node *head;
	Node *tail;

	iterator begin() { return {head};    }
	iterator end()   { return {nullptr}; }
};

template <typename Node>
inline void
list_append(list<Node> *list, Node *node)
{
	if (list->head)
	{
		list->tail->next = node;
		list->tail = node;
	}
	else
	{
		list->head = node;
		list->tail = node;
	}
}
