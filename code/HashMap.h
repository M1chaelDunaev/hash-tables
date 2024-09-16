#ifndef _HASH_MAP_H_
#define _HASH_MAP_H_

#include "HashTableBasic.h"
#include "BiderectionalIterators.h"

template<typename HashMap>
class HashMapIterator;

template<typename HashMap>
class ConstHashMapIterator;


namespace BHM_NAMESPACE
{
	template<typename _KeyType, typename _ValueType>
	struct HashMapNode;

	template<bool _Multi, typename _NodeType, typename _Hasher, typename _EqualComp>
	class BasicHashMap;
}


template<typename _KeyType, typename _ValueType>
struct BHM_NAMESPACE::HashMapNode
{
	using KeyType = _KeyType;
	using ValueType = _ValueType;
	using PairType = std::pair<KeyType, ValueType>;

	const size_t hash;
	PairType pair;

	HashMapNode* prev = nullptr;
	HashMapNode* next = nullptr;

	template<typename KT, typename VT>
	HashMapNode(size_t _hash, KT&& _key, VT&& _value) :
		hash(_hash), pair(std::forward<KT>(_key), std::forward<VT>(_value)) {};

	template<typename Pair>
	HashMapNode(size_t _hash, Pair&& _pair) :
		hash(_hash), pair(std::forward<Pair>(_pair)) {};

	HashMapNode(HashMapNode* _other) :
		hash(_other->hash), pair(_other->pair) {};


	const KeyType& getKey() const noexcept { return pair.first; }
};


template<bool _Multi, typename _NodeType, typename _Hasher, typename _EqualComp>
class BHM_NAMESPACE::BasicHashMap : public BHT_NAMESPACE::HashTable<_Multi, _NodeType, _Hasher, _EqualComp>
{
//Public Types-------------------------------------------------------------------------------------------------------------------------------
public:

	using KeyType = typename _NodeType::KeyType;
	using ValueType = typename _NodeType::ValueType;
	using PairType = typename _NodeType::PairType;
	using HasherType = _Hasher;
	using EqualCompType = _EqualComp;
	using IteratorType = HashMapIterator<BasicHashMap>;
	using ConstIteratorType = ConstHashMapIterator<BasicHashMap>;

//Private Types & Usings------------------------------------------------------------------------------------------------------------------------
private:

	using NodeType = HashMapNode<KeyType, ValueType>;
	using BasicHashTable = BHT_NAMESPACE::HashTable<_Multi, NodeType, HasherType, EqualCompType>;
	using BasicHashTable::hasher;
	using BasicHashTable::bucketCount;
	using BasicHashTable::innerMapInsert;
	using BasicHashTable::innerMapPairInsert;
	using BasicHashTable::eraseSingleNode;
	using BasicHashTable::innerFind;
	using BasicHashTable::markEqualRange;
	using BasicHashTable::copyConstruct;
	using BasicHashTable::moveConstruct;
	using BasicHashTable::getBegin;
	using BasicHashTable::getBack;
	using BasicHashTable::beforeBeginPtr;
	using BasicHashTable::endPtr;

	friend class BD_ITER_NAMESPACE::BDIterator<BasicHashMap>;
	friend class IteratorType;
	friend class ConstIteratorType;

//Protected Methods---------------------------------------------------------------------------------------------------------------------------
protected:

	//Constructors----------------------------------------------------------------------------------------------------------------------------

	BasicHashMap() : BasicHashTable() {};

	BasicHashMap(size_t _bucketCount) : BasicHashTable(_bucketCount) {};

	BasicHashMap(size_t _bucketCount, std::initializer_list<PairType> _initList) : BasicHashTable(_bucketCount)
	{
		insert(_initList);
	}

	template<typename InputIterator>
	BasicHashMap(size_t _bucketCount, InputIterator _first, InputIterator _last) : BasicHashTable(_bucketCount)
	{
		insert(_first, _last);
	}

	BasicHashMap(const BasicHashMap& _other) : BasicHashTable()
	{
		copyConstruct(_other);
	}

	BasicHashMap(BasicHashMap&& _other) : BasicHashTable()
	{
		moveConstruct(_other);
	}

//Public Methods------------------------------------------------------------------------------------------------------------------------------
public:

	//Insertion-------------------------------------------------------------------------------------------------------------------------------

	IteratorType insert(const KeyType& _key, const ValueType& _value)
	{
		return IteratorType(this, innerMapInsert(_key, _value));
	}

	IteratorType insert(KeyType&& _key, const ValueType& _value)
	{
		return IteratorType(this, innerMapInsert(std::move(_key), _value));
	}

	IteratorType insert(const KeyType& _key, ValueType&& _value)
	{
		return IteratorType(this, innerMapInsert(_key, std::move(_value)));
	}

	IteratorType insert(KeyType&& _key, ValueType&& _value)
	{
		return IteratorType(this, innerMapInsert(std::move(_key), std::move(_value)));
	}

	IteratorType insert(const PairType& _pair)
	{
		return IteratorType(this, innerMapPairInsert(_pair));
	}

	IteratorType insert(PairType&& _pair) 
	{
		return IteratorType(this, innerMapPairInsert(std::move(_pair)));
	}

	void insert(std::initializer_list<PairType> _initList)
	{
		for (auto&& elem : _initList)
			innerMapPairInsert(std::move(elem));
	}

	template<typename InputIterator>
	void insert(InputIterator _first, InputIterator _last) 
	{
		while (_first != _last)
		{
			innerMapPairInsert(*_first);
			++_first;
		}

		innerMapPairInsert(*_last);
	}

	//Erasing----------------------------------------------------------------------------------------------------------------------------------

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

	IteratorType find(const KeyType& _key) noexcept
	{
		return IteratorType(this, innerFind(_key, hasher(_key)));
	}

	ConstIteratorType find(const KeyType& _key) const noexcept
	{
		return ConstIteratorType(this, innerFind(_key, hasher(_key)));
	}

	std::pair<IteratorType, IteratorType> equalRange(const KeyType& _key) noexcept
	{
		NodeType* first = nullptr;
		NodeType* last = nullptr;

		markEqualRange(_key, first, last);

		return { IteratorType(this, first), IteratorType(this, last) };
	}

	std::pair<ConstIteratorType, ConstIteratorType> equalRange(const KeyType& _key) const noexcept
	{
		NodeType* first = nullptr;
		NodeType* last = nullptr;

		markEqualRange(_key, first, last);

		return { ConstIteratorType(this, first), ConstIteratorType(this, last) };
	}

	//Iterator----------------------------------------------------------------------------------------------------------------------------------

	IteratorType begin() noexcept
	{
		return IteratorType(this, getBegin());
	}

	ConstIteratorType begin() const noexcept
	{
		return ConstIteratorType(this, getBegin());
	}

	IteratorType beforeBegin() noexcept
	{
		return IteratorType(this, beforeBeginPtr);
	}

	ConstIteratorType beforeBegin() const noexcept
	{
		return ConstIteratorType(this, beforeBeginPtr);
	}

	IteratorType back() noexcept
	{
		return IteratorType(this, getBack());
	}

	ConstIteratorType back() const noexcept
	{
		return ConstIteratorType(this, getBack());
	}

	IteratorType end() noexcept
	{
		return IteratorType(this, endPtr);
	}

	ConstIteratorType end() const noexcept
	{
		return ConstIteratorType(this, endPtr);
	}

	ConstIteratorType cbegin() const noexcept
	{
		return ConstIteratorType(this, getBegin());
	}

	ConstIteratorType cbeforeBegin() const noexcept
	{
		return ConstIteratorType(this, beforeBeginPtr);
	}

	ConstIteratorType cback() const noexcept
	{
		return ConstIteratorType(this, getBack());
	}

	ConstIteratorType cend() const noexcept
	{
		return ConstIteratorType(this, endPtr);
	}
};


template<typename KeyType, typename ValueType, typename Hasher = std::hash<KeyType>, typename EqualComp = std::equal_to<KeyType>>
class HashMap : public BHM_NAMESPACE::BasicHashMap<false, BHM_NAMESPACE::HashMapNode<KeyType, ValueType>, Hasher, EqualComp>
{
	using BasicMap = BHM_NAMESPACE::BasicHashMap<false, BHM_NAMESPACE::HashMapNode<KeyType, ValueType>, Hasher, EqualComp>;

//Public Methods--------------------------------------------------------------------------------------------------------------------------------
public:

	using typename BasicMap::PairType;

	//Constructors------------------------------------------------------------------------------------------------------------------------------

	HashMap() : BasicMap() {};

	HashMap(size_t _bucketCount) : BasicMap(_bucketCount) {};

	HashMap(size_t _bucketCount, std::initializer_list<PairType>& _initList) : BasicMap(_bucketCount, _initList) {};

	template<typename InputIterator>
	HashMap(size_t _bucketCount, InputIterator _first, InputIterator _last) : BasicMap(_bucketCount, _first, _last) {};

	HashMap(const HashMap& _other) : BasicMap(_other) {};

	HashMap(HashMap&& _other) noexcept : BasicMap(std::move(_other)) {};
};


template<typename KeyType, typename ValueType, typename Hasher = std::hash<KeyType>, typename EqualComp = std::equal_to<KeyType>>
class HashMultiMap : public BHM_NAMESPACE::BasicHashMap<true, BHM_NAMESPACE::HashMapNode<KeyType, ValueType>, Hasher, EqualComp>
{
	using BasicMap = BHM_NAMESPACE::BasicHashMap<true, BHM_NAMESPACE::HashMapNode<KeyType, ValueType>, Hasher, EqualComp>;

//Public Methods--------------------------------------------------------------------------------------------------------------------------------
public:
	
	using typename BasicMap::PairType;

	//Constructors------------------------------------------------------------------------------------------------------------------------------
	HashMultiMap() : BasicMap() {};

	HashMultiMap(size_t _bucketCount) : BasicMap(_bucketCount) {};

	HashMultiMap(size_t _bucketCount, std::initializer_list<PairType>& _initList) : BasicMap(_bucketCount, _initList) {};

	template<typename InputIterator>
	HashMultiMap(size_t _bucketCount, InputIterator _first, InputIterator _last) : BasicMap(_bucketCount, _first, _last) {};

	HashMultiMap(const HashMultiMap& _other) : BasicMap(_other) {};

	HashMultiMap(HashMultiMap&& _other) noexcept : BasicMap(std::move(_other)) {};
};


template<typename HashTable>
class HashMapIterator : public BD_ITER_NAMESPACE::MapIterator<HashTable>
{
	using NodeType = typename HashTable::NodeType;

	using BasicIter = BD_ITER_NAMESPACE::MapIterator<HashTable>;
	using BasicIter::cont;
	using BasicIter::node;

	friend class BHM_NAMESPACE::BasicHashMap<false, NodeType, typename HashTable::HasherType, typename HashTable::EqualCompType>; // Äëÿ HashMap
	friend class BHM_NAMESPACE::BasicHashMap<true, NodeType, typename HashTable::HasherType, typename HashTable::EqualCompType>; // Äëÿ HashMultiMap

public:

	HashMapIterator(HashTable* _htable, NodeType* _node) : 
		BasicIter(_htable, _node) {};


	HashMapIterator& operator++() noexcept
	{
		cont->toNext(node);

		return *this;
	}

	HashMapIterator operator++(int) noexcept
	{
		HashMapIterator temp = *this;

		cont->toNext(node);

		return temp;
	}

	HashMapIterator& operator--() noexcept
	{
		cont->toPrev(node);

		return *this;
	}

	HashMapIterator operator--(int) noexcept
	{
		HashMapIterator temp = *this;

		cont->toPrev(node);

		return temp;
	}
};

template<typename HashTable>
class ConstHashMapIterator : public BD_ITER_NAMESPACE::ConstMapIterator<HashTable>
{
	using NodeType = typename HashTable::NodeType;

	using BasicIter = BD_ITER_NAMESPACE::ConstMapIterator<HashTable>;
	using BasicIter::cont;
	using BasicIter::node;

	friend class BHM_NAMESPACE::BasicHashMap<false, NodeType, typename HashTable::HasherType, typename HashTable::EqualCompType>; // Äëÿ HashMap
	friend class BHM_NAMESPACE::BasicHashMap<true, NodeType, typename HashTable::HasherType, typename HashTable::EqualCompType>; // Äëÿ HashMultiMap

public:

	ConstHashMapIterator(const HashTable* _htable, NodeType* _node) :
		BasicIter(_htable, _node) {};


	ConstHashMapIterator& operator++() noexcept
	{
		cont->toNext(node);

		return *this;
	}

	ConstHashMapIterator operator++(int) noexcept
	{
		ConstHashMapIterator temp = *this;

		cont->toNext(node);

		return temp;
	}

	ConstHashMapIterator& operator--() noexcept
	{
		cont->toPrev(node);

		return *this;
	}

	ConstHashMapIterator operator--(int) noexcept
	{
		ConstHashMapIterator temp = *this;

		cont->toPrev(node);

		return *this;
	}
};

#endif // !_HASH_MAP_H_

