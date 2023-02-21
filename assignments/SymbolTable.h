#include<iostream>
#include<map>
using namespace std;

class node {
public:
	string id;
	node* next;

	node(string id) {
		this->id = id;
		next = nullptr;
	}
};

class SymbolTable {
public:
	node* head;
	int len;

	SymbolTable() {
		head = nullptr;
		len = 0;
	}
	void insert(string id) {
		node* newnode = new node(id);
		if (head == nullptr) {
			head = newnode;
			return;
		}
		node* curr = head;
		while (curr->next != nullptr) {
			if (curr->id == id) {
				return;//已存在 返回
			}
			curr = curr->next;
		}
		curr->next = newnode;
		return;
	}
	node* find(string id) {
		if (head == nullptr)return nullptr;
		node* curr = head;
		while (curr != nullptr) {
			//cout<<"while";
			if (curr->id == id) {
				return curr;
			}
			curr=curr->next;
		}
		return nullptr;
	}
};

class SymbolTableNode {
public:
	SymbolTable* idlist;//一个表
	SymbolTableNode* prev;

	SymbolTableNode(SymbolTableNode* prev) {
		this->idlist = new SymbolTable; 
		this->prev = prev; 
	}
};