#ifndef _BIDERECTIONAL_ITERATORS_H_
#define _BIDERECTIONAL_ITERATORS_H_

#include <utility>

namespace BD_ITER_NAMESPACE
{
	template<typename Container>
	class BDIterator;

	template<typename Container>
	class SetIterator;

	template<typename Container>
	class MapIterator;

	template<typename Container>
	class ConstMapIterator;
}

template<typename Container>
class BD_ITER_NAMESPACE::BDIterator
{
protected:

	using NodeType = typename Container::NodeType;

	const Container* cont;
	NodeType* node;

	BDIterator(const Container* _cont, NodeType* _node) :
		cont(_cont), node(_node) {};

public:

	bool isEnd() const noexcept
	{
		return node == cont->endPtr;
	}

	bool isBeforeBegin() const noexcept
	{
		return node == cont->beforeBeginPtr;
	}

	bool isValid() const noexcept
	{
		return !isEnd() && !isBeforeBegin();
	}

	bool hasSameContainer(const BDIterator& _other) const noexcept
	{
		return cont == _other.cont;
	}

	void operator=(const BDIterator& _other) noexcept
	{
		cont = _other.cont;
		node = _other.node;
	}

	friend bool operator==(const BDIterator& _it1, const BDIterator& _it2)
	{
		return _it1.node == _it2.node;
	}

	friend bool operator!=(const BDIterator& _it1, const BDIterator& _it2)
	{
		return _it1.node != _it2.node;
	}

};

template<typename Container>
class BD_ITER_NAMESPACE::SetIterator : public BD_ITER_NAMESPACE::BDIterator<Container>
{
protected:

	using KeyType = typename Container::KeyType;
	
	using BasicIter = BDIterator<Container>;	
	using typename BasicIter::NodeType;
	using BasicIter::node;

	SetIterator(const Container* _cont, NodeType* _node) :
		BasicIter(_cont, _node) {};
		
public:

	const KeyType& operator*() const noexcept
	{
		return node->key;
	}
	const KeyType* operator->() const noexcept
	{
		return &node->key;
	}
};

template<typename Container>
class BD_ITER_NAMESPACE::MapIterator : public BD_ITER_NAMESPACE::BDIterator<Container>
{
protected:

	using KeyType = typename Container::KeyType;
	using ValueType = typename Container::ValueType;

	using BasicIter = BDIterator<Container>;
	using typename BasicIter::NodeType;
	using BasicIter::node;

	MapIterator(const Container* _cont, NodeType* _node) :
		BasicIter(_cont, _node) {};		

public:

	std::pair<const KeyType, ValueType>& operator*() const noexcept
	{
		return reinterpret_cast<std::pair<const KeyType, ValueType>&>(node->pair);
	}

	std::pair<const KeyType, ValueType>* operator->() const noexcept
	{
		return reinterpret_cast<std::pair<const KeyType, ValueType>*>(&node->pair);
	}	
};

template<typename Container>
class BD_ITER_NAMESPACE::ConstMapIterator : public BD_ITER_NAMESPACE::BDIterator<Container>
{
protected:

	using KeyType = typename Container::KeyType;
	using ValueType = typename Container::KeyType;

	using BasicIter = BDIterator<Container>;
	using typename BasicIter::NodeType;
	using BasicIter::node;

	ConstMapIterator(const Container* _cont, NodeType* _node) :
		BasicIter(_cont, _node) {};

public:

	std::pair<const KeyType, const ValueType>& operator*() const noexcept
	{
		return reinterpret_cast<std::pair<const KeyType, const ValueType>&>(node->pair);
	}

	std::pair<const KeyType, const ValueType>* operator->() const noexcept
	{
		return reinterpret_cast<std::pair<const KeyType, const ValueType>*>(&node->pair);
	}
};


#endif // _BIDERECTIONAL_ITERATORS_H_


