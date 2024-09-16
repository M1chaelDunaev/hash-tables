#ifndef _HASH_SET_H_
#define _HASH_SET_H_

#include "HashTableBasic.h"
#include "BiderectionalIterators.h"

template<typename HashTable>
class HashSetIterator;

namespace BHS_NAMESPACE
{
	template<typename _KeyType>
	struct HashSetNode;

	template<bool _Multi, typename _NodeType, typename _Hasher, typename _EqualComp>
	class BasicHashSet;
}

template<typename _KeyType>
struct BHS_NAMESPACE::HashSetNode
{
	using KeyType = _KeyType;

	const size_t hash;
	const KeyType key;
	HashSetNode* prev = nullptr;
	HashSetNode* next = nullptr;

	template<typename KT>
	HashSetNode(size_t _hash, KT&& _key) :
		hash(_hash), key(std::forward<KT>(_key)) {};

	HashSetNode(HashSetNode* _other) :
		hash(_other->hash), key(_other->key) {};

	const KeyType& getKey() const noexcept { return key; }
};


template<bool _Multi, typename _NodeType, typename _Hasher, typename _EqualComp>
class BHS_NAMESPACE::BasicHashSet : public BHT_NAMESPACE::HashTable<_Multi, _NodeType, _Hasher, _EqualComp>
{
//Public Types----------------------------------------------------------------------------------------------------------------------------------
public:

	using KeyType = typename _NodeType::KeyType;
	using HasherType = _Hasher;
	using EqualCompType = _EqualComp;
	using IteratorType = HashSetIterator<BasicHashSet>;

//Private Types & Usings------------------------------------------------------------------------------------------------------------------------
private:
	using NodeType = _NodeType;
	using BasicHashTable = BHT_NAMESPACE::HashTable<_Multi, NodeType, HasherType, EqualCompType>;
	using BasicHashTable::hasher;
	using BasicHashTable::bucketCount;
	using BasicHashTable::innerSetInsert;
	using BasicHashTable::eraseSingleNode;
	using BasicHashTable::innerFind;
	using BasicHashTable::markEqualRange;
	using BasicHashTable::copyConstruct;
	using BasicHashTable::moveConstruct;
	using BasicHashTable::getBegin;
	using BasicHashTable::getBack;
	using BasicHashTable::beforeBeginPtr;
	using BasicHashTable::endPtr;

	friend class BD_ITER_NAMESPACE::BDIterator<BasicHashSet>;
	friend class IteratorType;

//Protected Methods-----------------------------------------------------------------------------------------------------------------------------
protected:

	//Constructors------------------------------------------------------------------------------------------------------------------------------

	BasicHashSet() : BasicHashTable() {};

	BasicHashSet(size_t _bucketCount) : BasicHashTable(_bucketCount) {};

	BasicHashSet(size_t _bucketCount, std::initializer_list<KeyType> _initList) : BasicHashTable(_bucketCount)
	{
		insert(_initList);
	}

	template<typename InputIterator>
	BasicHashSet(size_t _bucketCount, InputIterator _first, InputIterator _last) : BasicHashTable(_bucketCount)
	{
		insert(_first, _last);
	}

	BasicHashSet(const BasicHashSet& _other) : BasicHashTable()
	{
		copyConstruct(_other);
	}

	BasicHashSet(BasicHashSet&& _other) noexcept : BasicHashTable()
	{
		moveConstruct(_other);
	}

//Public Methods--------------------------------------------------------------------------------------------------------------------------------
public:

	//Insertion---------------------------------------------------------------------------------------------------------------------------------

	IteratorType insert(const KeyType& _key)
	{	
		return IteratorType(this, innerSetInsert(_key));
	}

	IteratorType insert(KeyType&& _key) 
	{
		return IteratorType(this, innerSetInsert(std::move(_key)));
	}

	void insert(std::initializer_list<KeyType> _initList)
	{
		for (auto&& elem : _initList)
			innerSetInsert(std::move(elem));
	}

	template<typename InputIterator>
	void insert(InputIterator _first, InputIterator _last)
	{
		while (_first != _last)
		{
			innerSetInsert(*_first);
			++_first;
		}

		innerSetInsert(*_last);
	}

	//Erasing-----------------------------------------------------------------------------------------------------------------------------------

	size_t erase(const KeyType& _key) 
	{
		NodeType* res = innerFind(_key, hasher(_key));

		if (res == endPtr)
			return 0;

		return eraseSingleNode(res);
	}

	size_t erase(IteratorType _iter) 
	{
		if (!_iter.isValid())
			return 0;

		return eraseSingleNode(_iter.node);
	}

	size_t erase(IteratorType _first, IteratorType _last)
	{
		if (!_first.isValid() || !_last.isValid() || !_first.hasSameContainer(_last))
			return 0;

		IteratorType temp = _first;
		size_t count = 0;

		while (_first != _last)
		{
			temp = _first;
			++_first;

			count += eraseSingleNode(temp.node);
		}

		return count + eraseSingleNode(_last.node);
	}

	//Search------------------------------------------------------------------------------------------------------------------------------------

	IteratorType find(const KeyType& _key) const noexcept
	{
		return IteratorType(this, innerFind(_key, hasher(_key)));
	}

	std::pair<IteratorType, IteratorType> equalRange(const KeyType& _key) const noexcept
	{
		NodeType* first = nullptr;
		NodeType* last = nullptr;

		markEqualRange(_key, first, last);
		
		return { IteratorType(this, first), IteratorType(this, last) };
	}

	//Iterator----------------------------------------------------------------------------------------------------------------------------------

	IteratorType beforeBegin() const noexcept
	{
		return IteratorType(this, beforeBeginPtr);
	}

	IteratorType begin() const noexcept
	{
		return IteratorType(this, getBegin());
	}

	IteratorType back() const noexcept
	{
		return IteratorType(this, getBack());
	}

	IteratorType end() const noexcept
	{
		return IteratorType(this, endPtr);
	}
};


template<typename KeyType, typename Hasher = std::hash<KeyType>, typename EqualComp = std::equal_to<KeyType>>
class HashSet : public BHS_NAMESPACE::BasicHashSet<false, BHS_NAMESPACE::HashSetNode<KeyType>, Hasher, EqualComp>
{
	using BasicSet = BHS_NAMESPACE::BasicHashSet<false, BHS_NAMESPACE::HashSetNode<KeyType>, Hasher, EqualComp>;

//Public Methods--------------------------------------------------------------------------------------------------------------------------------
public:

	//Constructors------------------------------------------------------------------------------------------------------------------------------

	HashSet() : BasicSet() {};

	HashSet(size_t _bucketCount) : BasicSet(_bucketCount) {};

	HashSet(size_t _bucketCount, std::initializer_list<KeyType>& _initList) : BasicSet(_bucketCount, _initList) {};

	template<typename InputIterator>
	HashSet(size_t _bucketCount, InputIterator _first, InputIterator _last) : BasicSet(_bucketCount, _first, _last) {};

	HashSet(const HashSet& _other) : BasicSet(_other) {};

	HashSet(HashSet&& _other) noexcept : BasicSet(std::move(_other)) {};
};


template<typename KeyType, typename Hasher = std::hash<KeyType>, typename EqualComp = std::equal_to<KeyType>>
class HashMultiSet : public BHS_NAMESPACE::BasicHashSet<true, BHS_NAMESPACE::HashSetNode<KeyType>, Hasher, EqualComp>
{
	using BasicSet = BHS_NAMESPACE::BasicHashSet<true, BHS_NAMESPACE::HashSetNode<KeyType>, Hasher, EqualComp>;

//Public Methods-----------------------------------------------------------------------------------------------------------------------------
public:

	//Constructors---------------------------------------------------------------------------------------------------------------------------

	HashMultiSet() : BasicSet() {};

	HashMultiSet(size_t _bucketCount) : BasicSet(_bucketCount) {};

	HashMultiSet(size_t _bucketCount, std::initializer_list<KeyType>& _initList) : BasicSet(_bucketCount, _initList) {};

	template<typename InputIterator>
	HashMultiSet(size_t _bucketCount, InputIterator _first, InputIterator _last) : BasicSet(_bucketCount, _first, _last) {};

	HashMultiSet(const HashMultiSet& _other) : BasicSet(_other) {};

	HashMultiSet(HashMultiSet&& _other) noexcept : BasicSet(std::move(_other)) {};
};


template<typename HashTable>
class HashSetIterator : public BD_ITER_NAMESPACE::SetIterator<HashTable>
{
	using NodeType = typename HashTable::NodeType;

	using BasicIter = BD_ITER_NAMESPACE::SetIterator<HashTable>;
	using BasicIter::cont;
	using BasicIter::node;

	friend class BHS_NAMESPACE::BasicHashSet<false, NodeType, typename HashTable::HasherType, typename HashTable::EqualCompType>; // Äëÿ HashSet
	friend class BHS_NAMESPACE::BasicHashSet<true, NodeType, typename HashTable::HasherType, typename HashTable::EqualCompType>; // Äëÿ HashMultiSet

public:

	HashSetIterator(const HashTable* _htable, NodeType* _node) : 
		BasicIter(_htable, _node) {};


	HashSetIterator& operator++() noexcept
	{
		cont->toNext(node);
		
		return *this;
	}

	HashSetIterator operator++(int) noexcept
	{
		HashSetIterator temp = *this;

		cont->toNext(node);

		return temp;
	}

	HashSetIterator& operator--() noexcept
	{
		cont->toPrev(node);

		return *this;
	}

	HashSetIterator operator--(int) noexcept
	{
		HashSetIterator temp = *this;

		cont->toPrev(node);

		return temp;
	}
};

#endif // !_HASH_SET_H_

