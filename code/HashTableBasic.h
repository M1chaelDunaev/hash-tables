#ifndef _HASH_TABLE_BASIC_H_
#define _HASH_TABLE_BASIC_H_

#include <utility>
#include <functional>

namespace BHT_NAMESPACE
{
	struct HashTableDefaultProperties
	{
		static size_t deafaultBucketCount()
		{
			return 50;
		}

		static float defaultMaxLoadFactor()
		{
			return 0.7f;
		}

		static float defaultGainFactor()
		{
			return 2.0f;
		}
	};


	template<typename KeyType, typename Hasher>
	constexpr bool isValidHasher =
		noexcept (static_cast<size_t>(std::declval<const Hasher&>()(std::declval<const KeyType&>())));

	template<typename KeyType, typename Comparator>
	constexpr bool isValidComparator =
		noexcept (static_cast<bool>(std::declval<const Comparator&>()(std::declval<const KeyType&>(), std::declval<const KeyType&>())));


	template<bool Multi, typename NodeType, typename Hasher, typename EqualComp>
	class HashTable;
}


template<bool Multi, typename NodeType, typename Hasher, typename EqualComp>
class BHT_NAMESPACE::HashTable
{
protected:

	using DefProps = HashTableDefaultProperties;
	using KeyType = typename NodeType::KeyType;

//Included Structs------------------------------------------------------------------------------------------------------------------------------

	struct Bucket
	{
		NodeType* head = nullptr;
	};

//Properties(fields)----------------------------------------------------------------------------------------------------------------------------

	static constexpr bool isMulti = Multi;

	Bucket* bucketArray = nullptr;

	size_t bucketCount = DefProps::deafaultBucketCount();
	size_t elementCount = 0;

	float loadFactor = 0.0f;
	float maxLoadFactor = DefProps::defaultMaxLoadFactor();
	float gainFactor = DefProps::defaultGainFactor();

	Hasher hasher;
	EqualComp comp;

	NodeType* beginPtr = nullptr;
	NodeType* backPtr = nullptr;
	NodeType* beforeBeginPtr;
	NodeType* endPtr;

//Protected Methods-----------------------------------------------------------------------------------------------------------------------------

	//Constructors & Destructor-----------------------------------------------------------------------------------------------------------------

	HashTable()
	{
		if (isValidHasher<KeyType, Hasher> && isValidComparator<KeyType, EqualComp>) {};
		
		bucketArray = new Bucket[bucketCount];
		beforeBeginPtr = reinterpret_cast<NodeType*>(new char[sizeof(NodeType)]);
		endPtr = reinterpret_cast<NodeType*>(new char[sizeof(NodeType)]);
	}

	HashTable(size_t _bucketCount) : bucketCount(_bucketCount), HashTable() {};

	~HashTable()
	{
		clear();

		delete[] bucketArray;
		delete[] reinterpret_cast<char*>(beforeBeginPtr);
		delete[] reinterpret_cast<char*>(endPtr);
	}

	//Specific Methods--------------------------------------------------------------------------------------------------------------------------

	void checkLoadFactor() noexcept
	{
		loadFactor = static_cast<float>(elementCount) / bucketCount;

		if (loadFactor > maxLoadFactor)
			reCreate(bucketCount * gainFactor);
	}

	void reCreate(size_t _newBucketCount) noexcept
	{
		Bucket* oldBucketArray = bucketArray;
		size_t oldBucketCount = bucketCount;

		bucketCount = _newBucketCount;

		bucketArray = new Bucket[bucketCount];

		NodeType* current = nullptr;
		NodeType* prev = nullptr;

		// Обход массива бакетов
		for (size_t i = 0; i < oldBucketCount; ++i)
		{
			current = oldBucketArray[i].head;
			while (current)
			{
				prev = current;
				current = current->next;

				placeExistNode(prev);
			}
		}

		setBeginBack();

		delete[] oldBucketArray;
	}

	void placeNewNode(NodeType* _newNode) noexcept
	{
		size_t index = _newNode->hash % bucketCount;

		// Размещаем новый узел в начале бакета
		if (bucketArray[index].head)
		{
			bucketArray[index].head->prev = _newNode;
			_newNode->next = bucketArray[index].head;
		}

		bucketArray[index].head = _newNode;
	}

	void placeNewNodeIfSameFound(NodeType* _foundNode, NodeType* _newNode) noexcept
	{
		// Размещаем новый узел в начале группы equal узлов
		if (_foundNode->prev)
		{
			_foundNode->prev->next = _newNode;
			_newNode->prev = _foundNode->prev;
		}
		else
		{
			bucketArray[_newNode->hash % bucketCount].head = _newNode;
		}

		_foundNode->prev = _newNode;
		_newNode->next = _foundNode;
	}

	void placeExistNode(NodeType* _node) noexcept
	{
		_node->prev = _node->next = nullptr; // Обнуляем предыдущие значения указателей

		placeNewNode(_node);
	}

	void updateBeginBack(NodeType* _node) noexcept
	{
		if (beginPtr)
		{
			if ((_node->hash % bucketCount) <= (beginPtr->hash % bucketCount))
			{
				beginPtr = _node; // Если индексы бакетов равны, т.к элемент встает в начало бакета, он становится новым begin
				return;
			}

			if ((_node->hash % bucketCount) > (backPtr->hash % bucketCount))
			{
				backPtr = _node; // Если индексы бакетов равны, т.к элемент встает в начало бакета, он не может стать новым back
			}
		}
		else
		{
			beginPtr = backPtr = _node;
		}
	}

	void setBeginBack() noexcept
	{
		// Устанавливаем beginPtr
		for (size_t i = 0; i < bucketCount; ++i)
		{
			if (bucketArray[i].head)
			{
				beginPtr = bucketArray[i].head;
				break;
			}
		}

		// Устанавливаем backPtr
		for (size_t i = bucketCount - 1; i > 0; --i) //Идем с конца массива бакетов
		{
			if (bucketArray[i].head)
			{
				backPtr = bucketArray[i].head;

				while (backPtr->next)
					backPtr = backPtr->next;

				return;
			}
		}

		// Расматриваем индекс 0 отдельно, т.к он не попал в цикл
		if (bucketArray[0].head)
		{
			backPtr = bucketArray[0].head;

			while (backPtr->next)
				backPtr = backPtr->next;
		}
	}

	//Construction------------------------------------------------------------------------------------------------------------------------------

	void copyConstruct(const HashTable& _other)
	{
		// Копируем поля _other 
		bucketCount = _other.bucketCount;
		elementCount = _other.elementCount;
		loadFactor = _other.loadFactor;
		maxLoadFactor = _other.maxLoadFactor;
		gainFactor = _other.gainFactor;

		delete[] bucketArray;
		bucketArray = new Bucket[bucketCount];

		NodeType* current = nullptr;
		NodeType* newNode = nullptr;

		// Обход массива бакетов _other
		for (size_t i = 0; i < bucketCount; ++i)
		{
			current = _other.bucketArray[i].head;
			while (current)
			{
				newNode = new NodeType(current);
				placeNewNode(newNode);
				updateBeginBack(newNode);

				current = current->next;
			}
		}
	}

	void moveConstruct(HashTable& _other) noexcept
	{
		// Копируем поля _other 
		bucketCount = _other.bucketCount;
		elementCount = _other.elementCount;
		loadFactor = _other.loadFactor;
		maxLoadFactor = _other.maxLoadFactor;
		gainFactor = _other.gainFactor;

		delete[] bucketArray;
		bucketArray = _other.bucketArray;

		// Приводим _other к default состоянию
		_other.bucketCount = DefProps::deafaultBucketCount();
		_other.maxLoadFactor = DefProps::defaultMaxLoadFactor();
		_other.gainFactor = DefProps::defaultGainFactor();
		_other.elementCount = 0;
		_other.loadFactor = 0.0;

		_other.bucketArray = new Bucket[_other.bucketCount];
	}

	//Search------------------------------------------------------------------------------------------------------------------------------------

	NodeType* innerFind(const KeyType& _key, size_t _hash) const noexcept
	{
		NodeType* current = bucketArray[_hash % bucketCount].head;
		while (current)
		{
			if (comp(_key, current->getKey()))
				return current;

			current = current->next;
		}

		return endPtr;
	}

	void markEqualRange(const KeyType& _key, NodeType*& _first, NodeType*& _last) const noexcept
	{
		_first = _last = innerFind(_key, hasher(_key));

		if (_first == endPtr)
			return;

		while (_last->next && (comp(_last->next->getKey(), _key)))
			_last = _last->next;
	}

	//Insertion---------------------------------------------------------------------------------------------------------------------------------

	template<typename KT>
	NodeType* innerSetInsert(KT&& _key)
	{
		size_t hash = hasher(_key);
		NodeType* newNode = nullptr;

		if (!isMulti)
		{
			// Если элемент с таким ключом уже есть возвращаем endPtr
			if (innerFind(_key, hash) != endPtr)
				return endPtr;

			newNode = new NodeType(hash, std::forward<KT>(_key));

			placeNewNode(newNode);
		}
		else
		{
			NodeType* res = innerFind(_key, hash);

			newNode = new NodeType(hash, std::forward<KT>(_key));

			if (res != endPtr)
				placeNewNodeIfSameFound(res, newNode);
			else
				placeNewNode(newNode);
		}

		++elementCount;
		updateBeginBack(newNode);
		checkLoadFactor();

		return newNode;
	}

	template<typename KT, typename VT>
	NodeType* innerMapInsert(KT&& _key, VT&& _value)
	{
		size_t hash = hasher(_key);
		NodeType* newNode = nullptr;

		if (!isMulti)
		{
			// Если элемент с таким ключом уже есть возвращаем endPtr
			if (innerFind(_key, hash) != endPtr)
				return endPtr;

			newNode = new NodeType(hash, std::forward<KT>(_key), std::forward<VT>(_value));

			placeNewNode(newNode);
		}
		else
		{
			NodeType* res = innerFind(_key, hash);

			newNode = new NodeType(hash, std::forward<KT>(_key), std::forward<VT>(_value));

			if (res != endPtr)
				placeNewNodeIfSameFound(res, newNode);
			else
				placeNewNode(newNode);
		}

		++elementCount;
		updateBeginBack(newNode);
		checkLoadFactor();

		return newNode;
	}

	template<typename Pair>
	NodeType* innerMapPairInsert(Pair&& _pair)
	{
		size_t hash = hasher(_pair.first);
		NodeType* newNode = nullptr;

		if (!isMulti)
		{
			// Если элемент с таким ключом уже есть возвращаем endPtr
			if (innerFind(_pair.first, hash) != endPtr)
				return endPtr;

			newNode = new NodeType(hash, std::forward<Pair>(_pair));

			placeNewNode(newNode);
		}
		else
		{
			NodeType* res = innerFind(_pair.first, hash);

			newNode = new NodeType(hash, std::forward<Pair>(_pair));

			if (res != endPtr)
				placeNewNodeIfSameFound(res, newNode);
			else
				placeNewNode(newNode);
		}

		++elementCount;
		updateBeginBack(newNode);
		checkLoadFactor();

		return newNode;
	}

	//Erasing-----------------------------------------------------------------------------------------------------------------------------------

	size_t eraseSingleNode(NodeType* _node)
	{
		if (_node->prev)
		{
			_node->prev->next = _node->next;
		}
		else
		{
			bucketArray[_node->hash % bucketCount].head = _node->next;

			if (_node == beginPtr)
				toNext(beginPtr);

			if (beginPtr == endPtr) // Если beginPtr стал равен endPtr, обнуляем его
				beginPtr = nullptr;
		}

		if (_node->next)
		{
			_node->next->prev = _node->prev;
		}
		else 
		{
			if (_node == backPtr)
				toPrev(backPtr);

			if (backPtr == _node) // Если backPtr не изменился, он первый элемент, следовательно обнуляем его
				backPtr = nullptr;
		}

		delete _node;
		--elementCount;
		checkLoadFactor();

		return 1;
	};

	//Iterator Functional-----------------------------------------------------------------------------------------------------------------------

	NodeType* getBegin() const noexcept
	{
		if (elementCount == 0)
			return endPtr;

		return beginPtr;
	}

	NodeType* getBack() const noexcept
	{
		if (elementCount == 0)
			return endPtr;

		return backPtr;
	}

	void toNext(NodeType*& _current) const noexcept
	{
		if (_current == endPtr)
			return;

		if (_current == backPtr)
		{
			_current = endPtr;
			return;
		}

		if (_current == beforeBeginPtr)
		{
			_current = getBegin(); // Контейнер может быть пустым
			return;
		}

		if (_current->next)
		{
			_current = _current->next;
			return;
		}

		//Обход массива вперед с элемента следующего после _current
		for (size_t i = (_current->hash % bucketCount + 1); i < bucketCount; ++i)
		{
			if (bucketArray[i].head)
			{
				_current = bucketArray[i].head;
				return;
			}
		}
	}

	void toPrev(NodeType*& _current) const noexcept
	{
		if (_current == beforeBeginPtr)
			return;

		if (_current == beginPtr)
		{
			_current = beforeBeginPtr;
			return;
		}

		if (_current == endPtr)
		{
			_current = getBack(); // Контейнер может быть пустым
			return;
		}

		if (_current->prev)
		{
			_current = _current->prev;
			return;
		}

		for (size_t i = (_current->hash % bucketCount - 1); i > 0; --i)
		{
			if (bucketArray[i].head)
			{
				_current = bucketArray[i].head;

				while (_current->next)
					_current = _current->next;

				return;
			}
		}

		// Расматриваем индекс 0 отдельно, т.к он не попал в цикл
		if (bucketArray[0].head)
		{
			_current = bucketArray[0].head;

			while (_current->next)
				_current = _current->next;
		}
	}

//Public Methods--------------------------------------------------------------------------------------------------------------------------------
public:

	//Getting Properties------------------------------------------------------------------------------------------------------------------------

	size_t size() const noexcept
	{
		return elementCount;
	}

	size_t getBucketCount() const noexcept
	{
		return bucketCount;
	}

	bool isEmpty() const noexcept
	{
		return (elementCount == 0) ? true : false;
	}

	float getLoadFactor() const noexcept
	{
		return loadFactor;
	}

	float getMaxLoadFactor() const noexcept
	{
		return maxLoadFactor;
	}

	float getGainFactor() const noexcept
	{
		return gainFactor;
	}

	//Setting Properties------------------------------------------------------------------------------------------------------------------------

	void reserve(size_t _requiredElementCount) noexcept
	{
		// maxElementCount = bucketCount * maxLoadFactor

		// Если текущего количесвта бакетов хватит для _requiredElementCount, ничего не делаем
		if ((_requiredElementCount / maxLoadFactor) <= bucketCount)
			return;

		// Выделяем бакеты, в количестве на 10% больше минимально небходимого для _requiredElementCount
		reCreate((_requiredElementCount / maxLoadFactor) * 1.1f);
		checkLoadFactor();
	}

	bool setBucketCount(size_t _newBucketCount) noexcept
	{
		// Если _newBucketCount меньше необходимого для текущего elementCount, ничего не делаем
		if (_newBucketCount <= (elementCount / maxLoadFactor))
			return false;

		// Выделяем бакеты, в количестве на 10% больше запрашиваемого
		reCreate(_newBucketCount * 1.1f);
		checkLoadFactor();
		return true;
	}

	bool setMaxLoadFactor(float _newMaxLoadFactor) noexcept
	{
		// MaxLoadFactor не может быть меньше 0.0
		if (_newMaxLoadFactor <= 0.0f)
			return false;

		maxLoadFactor = _newMaxLoadFactor;
		checkLoadFactor();
		return true;
	}

	bool setGainFactor(float _newGainFactor) noexcept
	{
		// gainFactor не может быть меньше 0.0
		if (_newGainFactor <= 0.0f)
			return false;

		gainFactor = _newGainFactor;
		return true;
	}

	//Modifying---------------------------------------------------------------------------------------------------------------------------------

	void clear()
	{
		NodeType* current = nullptr;
		NodeType* prev = nullptr;

		// Обход массива бакетов
		for (size_t i = 0; i < bucketCount; ++i)
		{
			current = bucketArray[i].head;
			while (current)
			{
				prev = current;
				current = current->next;

				delete prev;
			}
		}

		elementCount = 0;
		beginPtr = backPtr = nullptr;
		checkLoadFactor();
	}

	void shrinkToFit() noexcept
	{
		if (elementCount == 0)
		{
			reCreate(DefProps::deafaultBucketCount());
			return;
		}

		reCreate(elementCount / maxLoadFactor + 1);
	}

	void swap(HashTable& _other) noexcept
	{
		Bucket* tempBucketArray = bucketArray;
		NodeType* tempBeginPtr = beginPtr;
		NodeType* tempBackPtr = backPtr;
		size_t tempBucketCount = bucketCount;
		size_t tempElementCount = elementCount;
		float tempLoadFactor = loadFactor;
		float tempMaxLoadFactor = maxLoadFactor;
		float tempGainFactor = gainFactor;

		bucketArray = _other.bucketArray;
		beginPtr = _other.beginPtr;
		backPtr = _other.backPtr;
		bucketCount = _other.bucketCount;
		elementCount = _other.elementCount;
		loadFactor = _other.loadFactor;
		maxLoadFactor = _other.maxLoadFactor;
		gainFactor = _other.gainFactor;

		_other.bucketArray = tempBucketArray;
		_other.beginPtr = tempBeginPtr;
		_other.backPtr = tempBackPtr;
		_other.bucketCount = tempBucketCount;
		_other.elementCount = tempElementCount;
		_other.loadFactor = tempLoadFactor;
		_other.maxLoadFactor = tempMaxLoadFactor;
		_other.gainFactor = tempGainFactor;
	}

	void merge(HashTable& _source) noexcept
	{
		NodeType* current = nullptr;
		NodeType* prev = nullptr;

		if (!isMulti)
		{
			// Обход _source
			for (size_t i = 0; i < _source.bucketCount; ++i)
			{
				current = _source.bucketArray[i].head;
				_source.bucketArray[i].head = nullptr;

				while (current)
				{
					prev = current;
					current = current->next;

					if (!innerFind(prev->getKey(), prev->hash))
					{
						placeExistNode(prev);

						++elementCount;
						updateBeginBack(prev);
						checkLoadFactor();
					}
				}
			}
		}
		else // Если multi
		{
			for (size_t i = 0; i < _source.bucketCount; ++i)
			{
				current = _source.bucketArray[i].head;
				_source.bucketArray[i].head = nullptr;

				// Обход _source
				while (current)
				{
					prev = current;
					current = current->next;

					NodeType* res = innerFind(prev->getKey(), prev->hash); // Ищем equal элементы

					if (res != endPtr)
					{
						prev->prev = prev->next = nullptr;
						placeNewNodeIfSameFound(res, prev);
					}
					else
					{
						placeExistNode(prev);
					}

					++elementCount;
					updateBeginBack(prev);
					checkLoadFactor();
				}
			}
		}

		_source.elementCount = 0;
		_source.loadFactor = 0.0f;
		_source.beginPtr = _source.backPtr = nullptr;
	}

	//Search------------------------------------------------------------------------------------------------------------------------------------

	bool contains(const KeyType& _key) const noexcept
	{
		return innerFind(_key, hasher(_key)) != endPtr;
	}

	size_t countByKey(const KeyType& _key) const noexcept
	{
		NodeType* current = innerFind(_key, hasher(_key));
		size_t count = 0;

		if (current == endPtr)
			return 0;

		while (current)
		{
			if (comp(current->getKey(), _key))
			{
				++count;
				current = current->next;
			}
			else break;
		}

		return count;
	}

	//Erasing-----------------------------------------------------------------------------------------------------------------------------------

	size_t eraseEqual(const KeyType& _key)
	{
		NodeType* first = nullptr;
		NodeType* last = nullptr;

		markEqualRange(_key, first, last);

		if (first == endPtr)
			return 0;

		size_t count = 0;
		NodeType* current = nullptr;

		while (first != last)
		{
			current = first;
			first = first->next;

			count += eraseSingleNode(current);
		}

		return count + eraseSingleNode(last);
	};
};

#endif
